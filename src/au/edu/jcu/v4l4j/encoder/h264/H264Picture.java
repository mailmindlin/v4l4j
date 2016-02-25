package au.edu.jcu.v4l4j.encoder.h264;

import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.io.Closeable;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;

/**
 * Wrapper for <code>x264_picture_t</code>.
 * @author mailmindlin
 */
public class H264Picture implements Closeable, VideoFrame {
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	protected final long object;
	
	/**
	 * Allocates native struct and initializes it.
	 * @param csp
	 * @param width
	 * @param height
	 * @return
	 */
	protected native long init(int csp, int width, int height);
	
	/**
	 * Release the native memory behind this picture
	 */
	@Override
	public native void close();
	
	/**
	 * Initialize picture with given pointer
	 * @param pointer address of struct
	 */
	public H264Picture(long pointer) {
		this.object = pointer;
	}
	
	/**
	 * Initialize with given width, height, and CSP
	 * @param csp color space
	 * @param width width
	 * @param height height
	 */
	public H264Picture(int csp, int width, int height) {
		this.object = init(csp, width, height);
	}
	
	/**
	 * 
	 * @return csp
	 */
	public native int getCsp();
	/**
	 * Get width of picture
	 * @return width
	 */
	public native int getWidth();
	/**
	 * Get height of picture
	 * @return height
	 */
	public native int getHeight();

	@Override
	public FrameGrabber getFrameGrabber() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getFrameLength() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public long getSequenceNumber() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public long getCaptureTime() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public byte[] getBytes() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public DataBuffer getDataBuffer() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Raster getRaster() throws UnsupportedMethod {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public BufferedImage getBufferedImage() throws UnsupportedMethod {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void recycle() {
		// TODO Auto-generated method stub
		
	}
}
