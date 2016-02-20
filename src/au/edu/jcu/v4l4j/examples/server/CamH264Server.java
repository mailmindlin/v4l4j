package au.edu.jcu.v4l4j.examples.server;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Vector;

import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.JPEGFrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class CamH264Server implements Runnable, CaptureCallback {
	private ServerSocket serverSocket;
	private VideoDevice videoDevice;
	private JPEGFrameGrabber frameGrabber;
	private Thread serverThread;
	private Vector<ClientStreamingConnection> clients;
	private long frameCount;
	private long lastFrameTimestamp;
	
	public static void main(String[] args) throws V4L4JException, IOException {
		String dev = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0";
		int w = (System.getProperty("test.width") != null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		int h = (System.getProperty("test.height") != null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		int port = (System.getProperty("test.port") != null) ? Integer.parseInt(System.getProperty("test.port")) : 8080;
		int fps = (System.getProperty("test.fps") != null) ? Integer.parseInt(System.getProperty("test.fps")) : 15;

		CamH264Server server = new CamH264Server(dev, w, h, port, fps);
		server.start();
		System.out.println("Press enter to exit.");
		System.in.read();
		server.stop();
	}
	

	/**
	 * Builds a camera server object capturing frames from the given device at
	 * the given resolution and sending them out to clients connected to the
	 * given TCP port number.
	 * 
	 * @param dev
	 *            the video device file
	 * @param width
	 *            the capture width
	 * @param height
	 *            the capture height
	 * @param port
	 *            the TCP port to listen on for incoming connections
	 * @param fps
	 *            the frame rate used for capture
	 * @throws V4L4JException
	 *             if a JPEG frame grabber cant be created
	 * @throws IOException
	 *             if a server socket on the given port cant be created
	 */
	public CamH264Server(String dev, int width, int height, int port, int fps) throws V4L4JException, IOException {
		this.videoDevice = new VideoDevice(dev);
		this.frameGrabber = videoDevice.getJPEGFrameGrabber(width, height, 0, 0, 80);
		this.frameGrabber.setCaptureCallback(this);
		try {
			System.out.println("setting frame rate to " + fps);
			frameGrabber.setFrameInterval(1, fps);
		} catch (Exception e) {
			System.out.println("Couldnt set the frame interval");
		}

		clients = new Vector<>();

		// initialise tcp port to listen on
		serverSocket = new ServerSocket(port);

		System.out.println("Server listening at " + serverSocket.getInetAddress().getHostAddress() + ":"
				+ serverSocket.getLocalPort());

		// create server thread
		serverThread = new Thread(this, "Server thread");
	}

	public void start() {
		// start the tcp server thread
		serverThread.start();
	}

	public void stop() {
		System.out.println("Stopping server...");
		// close the server socket first
		try {
			serverSocket.close();
		} catch (IOException e) {
			// error closing the server socket
		}

		// now interrupt the server thread
		if (serverThread.isAlive()) {
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
		} catch (Exception e) {
		} // the capture is already stopped

		// Stop all client connections
		// no need for the lock - the capture is already stopped which means
		// no more calls to nextFrame(), so we are the only one to access
		// the client list
		for (ClientStreamingConnection client : clients)
			client.close();

		// release v4l4j frame grabber, control list and video device
		videoDevice.releaseFrameGrabber();
		videoDevice.releaseControlList();
		videoDevice.release();
	}

	/**
	 * the main server thread loop: wait for a client to connect to the server
	 * socket, then read a single line and look for specific keywords to
	 * determine which action to take: send the main html page, the webcam page,
	 * the control list page, or the webcam stream
	 *
	 * @throws IOException
	 *             if there is an error accepting a connection on the server
	 *             socket
	 * @throws V4L4JException
	 *             if there is an error starting the capture when the first
	 *             client connects
	 */
	private void serverMainLoop() throws IOException, V4L4JException {
		// Wait for new incoming connection
		try {
			Socket clientSocket = serverSocket.accept();
			System.out.println("Connection from " + clientSocket.getInetAddress().getHostAddress() + ":" + clientSocket.getPort());
			
			System.out.println("Serving video stream");
			
			// add new client to clients list
			synchronized (clients) {
				clients.add(new ClientStreamingConnection(clientSocket));

				// if this is the first client, start the capture
				if (clients.size() == 1) {
					frameCount = 0;
					frameGrabber.startCapture();
				}
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Implements the server thread.
	 */
	@Override
	public void run() {
		try {
			// run the main loop only until we are interrupted
			while (!Thread.interrupted())
				serverMainLoop();

		} catch (V4L4JException e) {
			// error starting the capture when the first client connected
		} catch (Exception e) {
			// error accepting new client connection over server socket
			// or closing connection with a client
			// TODO figure out which exceptions should be caught and which
			// shouldn't
			e.printStackTrace();
		} finally {
			System.out.println("Server thread exiting");
		}
	}

	@Override
	public void nextFrame(VideoFrame frame) {
		Vector<ClientStreamingConnection> copyClients = null;

		// copy client vector
		synchronized (clients) {
			copyClients = new Vector<>(clients);
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
		for (ClientStreamingConnection client : copyClients) {
			try {
				client.sendNextFrame(frame);
			} catch (IOException e) {
				// error sending frame to this client

				// close the connection
				client.close();

				// remove client from list
				synchronized (clients) {
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
