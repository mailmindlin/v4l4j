/*
* Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
package au.edu.jcu.v4l4j.examples.server;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Vector;

import au.edu.jcu.v4l4j.ControlList;
import au.edu.jcu.v4l4j.JPEGFrameGrabber;
import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class creates a tcp server socket and waits for incoming connections frm a web browser
 * video streaming apps such as VLC and ffplay. It uses two threads: a  server threads to handle
 * incoming tcp connections and figure out what to sent, and a capture thread to retrieve a video
 * frame, and send it to all connected client in the form of an MJPEG stream.
 * When a connection is made, the server thread parses the first line it receives to decide what 
 * to send back:
 * If the line contains no recognisable keywords (such as when a browser requests "GET / HTTP/1.1")
 * a simple html page with two frames is sent back. One frame asks for the "control" page, the
 * other asks for the "webcam" page. 
 * If the line contains the keyword "control", a basic html page with a table containing a row
 * for each control. Each row is an HTML form displaying the current control value, and allowing
 * the user to change it.
 * If the line contains the keyword "webcam", a basic page with an img tag is sent. The img tag
 * has a url pointing to "stream.jpg".
 * If the line contains the keyword "stream", this client is added to a client list.
 * The capture thread continuously retrieves frame and sends it to all currently-connected clients.
 * <br>
 * To use this class, run it, and point your browser to "localhost:8080". You can also view the
 * video stream in VLC (Select "Media"->"Open network stream...", then select "HTTP" as the 
 * protocol and enter "localhost:8080/stream" as the address). You can also use ffplay to view
 * the stream (Run <code>ffplay -f mjpeg http://localhost:8080/stream</code>).
 * @author gilles
 *
 */
public class CamServer implements Runnable, CaptureCallback{
	private ServerSocket 				serverSocket;
	private VideoDevice					videoDevice;
	private JPEGFrameGrabber			frameGrabber;
	private ControlList					controlList;
	private Thread						serverThread;
	private Vector<ClientConnection> 	clients;
	private String 						httpLineFromClient;
	private long						frameCount;
	private long						lastFrameTimestamp;
	
	private static final int			MAIN_PAGE = 0;
	private static final int			WEBCAM_PAGE = 1;
	private static final int			CONTROL_PAGE = 2;
	private static final int			VIDEO_STREAM = 3;
	private static final int			UPDATE_CONTROL_VALUE = 4;


	public static void main(String[] args) throws V4L4JException, IOException{
		String dev = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0"; 
		int w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		int h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		int port = (System.getProperty("test.port")!=null) ? Integer.parseInt(System.getProperty("test.port")) : 8080;
		int fps =  (System.getProperty("test.fps")!=null) ? Integer.parseInt(System.getProperty("test.fps")) : 15;

 
		CamServer server = new CamServer(dev, w, h, port, fps);
		server.start();
		System.out.println("Press enter to exit.");
		System.in.read();
		server.stop();
	}

	/**
	 * Builds a camera server object capturing frames from the given device
	 * at the given resolution and sending them out to clients connected
	 * to the given TCP port number.
	 * @param dev the video device file
	 * @param width the capture width
	 * @param height the capture height
	 * @param port the TCP port to listen on for incoming connections
	 * @param fps the frame rate used for capture
	 * @throws V4L4JException if a JPEG frame grabber cant be created 
	 * @throws IOException if a server socket on the given port cant be created
	 */
	public CamServer(String dev, int width, int height, int port, int fps) throws V4L4JException, IOException {
		videoDevice = new VideoDevice(dev);
		frameGrabber = videoDevice.getJPEGFrameGrabber(width, height, 0, 0, 80);
		frameGrabber.setCaptureCallback(this);
		try {
			System.out.println("setting frame rate to "+fps);
			frameGrabber.setFrameInterval(1, fps);
		} catch (Exception e){
			System.out.println("Couldnt set the frame interval");
		}

		controlList = videoDevice.getControlList();
		clients = new Vector<ClientConnection>();

		// initialise tcp port to listen on
		serverSocket = new ServerSocket(port);
		
		System.out.println("Server listening at "+
				serverSocket.getInetAddress().getHostAddress() + ":"+ serverSocket.getLocalPort());

		// create server thread
		serverThread = new Thread(this, "Server thread");
	}

	public void start() {
		// start the tcp server thread
		serverThread.start();
	}

	public void stop() {
		// close the server socket first
		try {
			serverSocket.close();
		} catch (IOException e) {
			// error closing the server socket
		}
		
		// now interrupt the server thread				
		if (serverThread.isAlive()){
			serverThread.interrupt();
			try {
				serverThread.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}		
		
		// stop the capture
		try {
			frameGrabber.stopCapture();
		} catch (Exception e){}// the capture is already stopped

		// Stop all client connections
		// no need for the lock - the capture is already stopped which means
		// no more calls to nextFrame(), so we are the only one to access 
		// the client list
		for(ClientConnection client:clients)
			client.stop();

		// release v4l4j frame grabber, control list and video device
		videoDevice.releaseFrameGrabber();
		videoDevice.releaseControlList();
		videoDevice.release();
	}

	/**
	 * the main server thread loop: wait for a client to connect to the server socket,
	 * then read a single line and look for specific keywords to determine which action
	 * to take: send the main html page, the webcam page, the control list page, or 
	 * the webcam stream  
	 * 
	 * @throws IOException if there is an error accepting a connection on the server socket
	 * @throws V4L4JException  if there is an error starting the capture when the first
	 * client connects
	 */
	private void serverMainLoop() throws IOException, V4L4JException{
		BufferedReader 			inStream = null;
		DataOutputStream 		outStream = null; 
		int 					requestedAction = MAIN_PAGE;

		// Wait for new incoming connection
		Socket clientSocket = serverSocket.accept();
		System.out.println("Connection from "+
				clientSocket.getInetAddress().getHostAddress()+
				":"+clientSocket.getPort());

		// Create input/output streams then check what page
		// was requested
		try {
			inStream = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
			outStream = new DataOutputStream(clientSocket.getOutputStream());
			requestedAction = parseLine(inStream);
		} catch (IOException e){
			// error setting up in and out streams with this client, abort
			inStream.close();
			outStream.close();
			clientSocket.close();
			return;
		}

		// if the video stream was requested, create a ClientConnection object
		// and add it to the list of clients. The capture thread is in charged
		// of sending mpjeg frames to all clients in the list
		if (requestedAction == VIDEO_STREAM) {

			// add new client to clients list 
			try {
				synchronized (clients){
					clients.add(new ClientConnection(clientSocket, inStream, outStream));

					// if this is the first client, start the capture
					if (clients.size() == 1) {
						frameCount = 0;
						frameGrabber.startCapture();
					}
				}
			} catch (IOException e) {
				// error connecting with client
				System.out.println("Disconnected from "+
						clientSocket.getInetAddress().getHostAddress()+
						":"+clientSocket.getPort());
				
				inStream.close();
				outStream.close();
				clientSocket.close();
			}
			
			// exit at this stage 
			return;
		}
		
		// check what other page was requested
		try {
			switch(requestedAction){
			case WEBCAM_PAGE:
				// send webcam viewer page
				ClientConnection.sendWebcamPage(outStream);
				break;
			case UPDATE_CONTROL_VALUE:
				// parse http line and update the requested control's value
				ClientConnection.updateControlValue(controlList, frameGrabber, httpLineFromClient);
				//fallthrough so we re-send the control list
			case CONTROL_PAGE:
				// send the control list page
				ClientConnection.sendControlListPage(controlList, frameGrabber.getJPGQuality(), outStream);
				break;
			case MAIN_PAGE:
			default:
				// send the main page
				ClientConnection.sendMainPage(outStream);
				break;
			}
		} catch (Exception e){
			e.printStackTrace();
		} finally {
			// close the connection with the client
			try {
				System.out.println("Disconnected from "+
						clientSocket.getInetAddress().getHostAddress()+
						":"+clientSocket.getPort());
				
				inStream.close();
				outStream.close();
				clientSocket.close();
			} catch (Exception e) {}
		}
	}

	private int parseLine(BufferedReader in) throws IOException{
		// read the first line to determine which page to send
		httpLineFromClient = in.readLine();
		
		if (httpLineFromClient == null)
			throw new IOException("Read null line");

		// if the line contains the word webcam, we want the video viewing page
		if (httpLineFromClient.indexOf("webcam") != -1)
			return WEBCAM_PAGE;

		// if the line contains the word control, we want the control list page
		if (httpLineFromClient.indexOf("control") != -1)
			return CONTROL_PAGE;
		
		// if the line contains the word stream, we want the control list page
		if (httpLineFromClient.indexOf("stream") != -1)
			return VIDEO_STREAM;
		
		// if the line contains the word update, we want to update a control's value
		if (httpLineFromClient.indexOf("update") != -1)
			return UPDATE_CONTROL_VALUE;
		
		return MAIN_PAGE;
	}

	/**
	 * Implements the server thread.
	 */
	@Override
	public void run() {
		try {
			// run the main loop only until we are interrupted 
			while (! Thread.interrupted())
				serverMainLoop();

		} catch (V4L4JException e){
			// error starting the capture when the first client connected
		} catch (Exception e){
			// error accepting new client connection over server socket
			// or closing connection with a client
		}
		System.out.println("Server thread exiting");
	}

	@Override
	public void nextFrame(VideoFrame frame) {
		Vector<ClientConnection> copyClients = null;

		// copy client vector
		synchronized(clients){
			copyClients = new Vector<ClientConnection>(clients);
		}

		frameCount++;
		if (frameCount == 1) {
			lastFrameTimestamp = System.currentTimeMillis();
		} else {
			long delta = System.currentTimeMillis() - lastFrameTimestamp;
			if (delta > 10000) {
				System.out.println("FPS: " + ((float) (frameCount - 1) * 1000 / delta));
				frameCount = 0;
				lastFrameTimestamp = 0;
			}
		}
		// send the frame to each client
		for(ClientConnection client: copyClients){
			try {
				client.sendNextFrame(frame);
			} catch (IOException e) {
				// error sending frame to this client
				
				// close the connection
				client.stop();
				
				// remove client from list
				synchronized(clients){
					clients.remove(client);

					// stop capture if there are no more clients
					if (clients.size() == 0)
						frameGrabber.stopCapture();
				}
			}
		}

		// recycle frame
		frame.recycle();
	}

	@Override
	public void exceptionReceived(V4L4JException e) {
		// Error capturing frames, stop the frame grabber
		frameGrabber.stopCapture();
	}
}
