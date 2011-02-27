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
public class CamServer implements Runnable{
	private ServerSocket 				serverSocket;
	private VideoDevice					videoDevice;
	private JPEGFrameGrabber			frameGrabber;
	private ControlList					controlList;
	private Thread						captureThread;
	private Thread						serverThread;
	private Vector<ClientConnection> 	clients;
	private String 						httpLineFromClient;
	
	private static final int			MAIN_PAGE = 0;
	private static final int			WEBCAM_PAGE = 1;
	private static final int			CONTROL_PAGE = 2;
	private static final int			VIDEO_STREAM = 3;
	private static final int			UPDATE_CONTROL_VALUE = 4;

	/**
	 * Builds a camera server object capturing frames from the given device
	 * at the given resolution and sending them out to clients connected
	 * to the given TCP port number.
	 * @param dev the video device file
	 * @param width the capture width
	 * @param height the capture height
	 * @param port the TCP port to listen on for incoming connections
	 * @throws V4L4JException if a JPEG frame grabber cant be created 
	 * @throws IOException if a server socket on the given port cant be created
	 */
	public CamServer(String dev, int width, int height, int port) throws V4L4JException, IOException {
		videoDevice = new VideoDevice(dev);
		frameGrabber = videoDevice.getJPEGFrameGrabber(width, height, 0, 0, 80);
		controlList = videoDevice.getControlList();
		clients = new Vector<ClientConnection>();

		// initialise tcp port to listen on
		serverSocket = new ServerSocket(port);
		
		System.out.println("Server listening at "+
				serverSocket.getInetAddress().getHostAddress() + ":"+ serverSocket.getLocalPort());

		// create capture and server threads
		captureThread = new Thread(this, "Capture thread");
		serverThread = new Thread(new Runnable() {

			/**
			 * implements the server thread: while we are not interrupted
			 * and the capture thread is running, we run the main loop.
			 * Before exiting, we close all client connections
			 */
			@Override
			public void run() {
				Vector<ClientConnection> copyClients = null;
				try {
					// run the main loop only until either the capture thread exits
					// or we are interrupted 
					while ((! Thread.interrupted()) && captureThread.isAlive()) {
						serverMainLoop();
					}
				} catch (IOException e){
					// error accepting new client connection over server socket
					// or closing connection with a client
				} catch (V4L4JException e){
					// error starting the capture when the first client connected
					e.printStackTrace();
				}

				// Stop all client connections
				synchronized(clients){
					copyClients = new Vector<ClientConnection>(clients);
				}

				for(ClientConnection client:copyClients)
					client.stop();

				System.out.println("Server thread exiting");
			}
		}, "Server thread");

	}

	public void start() {
		// start the video capture thread
		captureThread.start();
		// wait until the capture thread has started
		while (! captureThread.isAlive())
			try {
				Thread.sleep(50);
			} catch (InterruptedException e) {
				// we shouldnt be here really...
				e.printStackTrace();
			}
			
		// start the tcp server thread
		serverThread.start();
	}

	public void stop() {
		// if the capture thread is alive interrupt it and close it 
		if (captureThread.isAlive()){
			captureThread.interrupt();
			try {
				captureThread.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		// same with the server thread, but this time, to interrupt it
		// close the server socket first
		try {
			serverSocket.close();
		} catch (IOException e) {
			// error closing the server socket
		}		
		if (serverThread.isAlive()){
			serverThread.interrupt();
			try {
				serverThread.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}	

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

					// if first client
					if (clients.size() == 1) {
						// start the capture
						frameGrabber.startCapture();
						
						// and wake up the capture thread to start 
						// sending mpjeg frames
						clients.notifyAll();
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
	 * Implements the capture thread: make sure there is at least
	 * one client connected, get a video frame, and send it to all clients. Then
	 * repeat until we are interrupted.
	 */
	@Override
	public void run() {
		Vector<ClientConnection> copyClients = null;
		VideoFrame frame = null; 

		try {
			while(! Thread.interrupted()){
				// make sure there is at least one client
				waitForAtLeast1Client();

				// get latest frame
				frame = frameGrabber.getVideoFrame();

				// copy client vector
				synchronized(clients){
					copyClients = new Vector<ClientConnection>(clients);
				}

				// send it to each client
				for(ClientConnection client: copyClients){
					try {
						client.sendNextFrame(frame);
					} catch (IOException e) {
						// error sending frame to this client, remove it from list
						synchronized(clients){
							client.stop();
							clients.remove(client);

							// stop capture if last client
							if (clients.size() == 0)
								frameGrabber.stopCapture();
						}
					}
				}

				// recycle frame
				frame.recycle();
			}
		} catch (InterruptedException e) {
			// We were told to exit
		} catch (V4L4JException e) {
			// v4l4j capture error
			e.printStackTrace();
		}
		
		System.out.println("Capture thread exiting");

	}

	/**
	 * Wait until a client is connected
	 * @throws InterruptedException 
	 */
	public void waitForAtLeast1Client() throws InterruptedException {
		synchronized (clients) {
			while(clients.size() == 0)
				clients.wait();
		}
	}

	public static void main(String[] args) throws V4L4JException, IOException{
		String dev = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0"; 
		int w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		int h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		int port = (System.getProperty("test.port")!=null) ? Integer.parseInt(System.getProperty("test.port")) : 8080;
 
		CamServer server = new CamServer(dev, w, h, port);
		server.start();
		System.out.println("Press enter to exit.");
		System.in.read();
		server.stop();
	}
}