package au.edu.jcu.v4l4j.encoder.h264;

import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.io.Closeable;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;

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
	
	protected static native long alloc();
	
	protected static native int doGetStride(long ptr, int plane);
	
	protected static native int doGetNumPlanes(long ptr);
	
	protected static native ByteBuffer doGetBuffer(long ptr, int plane);
	
	protected static native void putInPlane(long ptr, int plane, ByteBuffer buf);
	
	protected static native void getFromPlane(long ptr, int plane, int offset, int len, ByteBuffer buf);
	
	protected static native long init(int width, int height, int csp);
	
	public static H264Picture allocate(int width, int height, int csp) {
		return new H264Picture(init(width, height, csp), width, height, csp);
	}
	
	@SuppressWarnings("unchecked")
	protected WeakReference<ByteBuffer>[] buffer = (WeakReference<ByteBuffer>[])new WeakReference[4];
	protected final long object;
	protected final int csp;
	protected final int width;
	protected final int height;
	
	/**
	 * Release the native memory behind this picture
	 */
	@Override
	public native void close();
	
	/**
	 * Initialize picture with given pointer
	 * @param pointer address of struct
	 */
	protected H264Picture(long pointer) {
		this.object = pointer;
		this.width = 0;
		this.height = 0;
		this.csp = 0;
	}
	
	protected H264Picture(long pointer, int width, int height, int csp) {
		this.object = pointer;
		this.width = width;
		this.height = height;
		this.csp = csp;
	}
	
	/**
	 * Initialize with given width, height, and CSP
	 * @param csp color space
	 * @param width width
	 * @param height height
	 */
	public H264Picture(int width, int height, int csp) {
		this.csp = csp;
		this.width = width;
		this.height = height;
		this.object = init(csp, width, height);
	}
	
	public H264Picture() {
		this(alloc());
	}
	
	/**
	 * Get colorspace
	 * @return csp
	 */
	public int getCsp() {
		return csp;
	}
	/**
	 * Get width of picture
	 * @return width
	 */
	public int getWidth() {
		return width;
	}
	/**
	 * Get height of picture
	 * @return height
	 */
	public int getHeight() {
		return height;
	}

	@Override
	public FrameGrabber getFrameGrabber() {
		return null;
	}

	@Override
	public int getFrameLength() {
		return getBuffer().limit();
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
		ByteBuffer buf = this.getBuffer();
		if (buf.hasArray())
			return buf.array();
		byte[] result = new byte[buf.remaining()];
		buf.get(result);
		return result;
	}

	@Override
	public DataBuffer getDataBuffer() {
		return new DataBuffer(DataBuffer.TYPE_UNDEFINED, 0, 4) {
			@Override
			public int getElem(int bank, int i) {
				ByteBuffer buf = getBuffer();
				return 0;
			}

			@Override
			public void setElem(int bank, int i, int val) {
				
			}
		};
	}

	@Override
	public Raster getRaster() throws UnsupportedMethod {
		throw new UnsupportedMethod("Cannot get raster for H264 image");
	}

	@Override
	public BufferedImage getBufferedImage() throws UnsupportedMethod {
		throw new UnsupportedMethod("Cannot get BufferedImage for H264 image");
	}

	@Override
	public void recycle() {
		// TODO Auto-generated method stub
		
	}

	public ByteBuffer getBuffer(int plane) {
		ByteBuffer result;
		if (this.buffer == null || (result = this.buffer.get()) == null)
			this.buffer = new WeakReference<>(result = doGetBuffer(this.object, plane));
		return result.duplicate();
	}
}
