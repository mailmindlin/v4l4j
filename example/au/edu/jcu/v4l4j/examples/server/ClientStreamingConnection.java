package au.edu.jcu.v4l4j.examples.server;

import java.io.Closeable;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import au.edu.jcu.v4l4j.VideoFrame;

public class ClientStreamingConnection implements Closeable {
	protected final Socket socket;
	protected final InputStream inStream;
	protected final OutputStream outStream;
	
	public ClientStreamingConnection(Socket socket) throws IOException {
		this.socket = socket;
		
		this.inStream = socket.getInputStream();
		this.outStream = socket.getOutputStream();
	}
	
	/**
	 * Send the given frame in an mjpeg frame header
	 * 
	 * @param frame
	 *            the frame to be send
	 * @throws IOException
	 *             if there is an error writing over the socket
	 */
	public void sendNextFrame(VideoFrame frame) throws IOException {
		outStream.write(frame.getBytes(), 0, frame.getFrameLength());
	}
	
	@Override
	public void close() {
		try {
			inStream.close();
			outStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			socket.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
