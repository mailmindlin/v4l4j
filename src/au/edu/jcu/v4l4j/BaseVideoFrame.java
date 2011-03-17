package au.edu.jcu.v4l4j;

import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.WritableRaster;

import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;

/**
 * This is the base class for all {@link VideoFrame} objects.
 * It implements some of the functionalities shared by all video frames.
 * It must be subclassed - and subclasses are responsible for instantiating the
 * raster and bufferedImage members
 * @author gilles
 *
 */
class BaseVideoFrame implements VideoFrame{
	protected int					frameLength;
	protected AbstractGrabber 		frameGrabber;
	protected byte					frameBuffer[];
	
	protected long					sequenceNumber;
	protected long					captureTime;
	
	protected V4L4JDataBuffer		dataBuffer;
	protected boolean				recycled;
	protected V4L4JRaster			raster;
	protected BufferedImage			bufferedImage;
	

	/** 
	 * This method creates the base for a video frame. It will instantiate
	 * and initialise all members except raster and bufferedImage, which
	 * falls under the responsibility of the subclass.
	 * @param grabber the frame grabber to which this frame must be
	 * returned to when recycled.
	 * @param bufferSize the size of the byte array to create for this frame.
	 */
	protected BaseVideoFrame(AbstractGrabber grabber, int bufferSize) {
		frameGrabber = grabber;
		frameBuffer = new byte[bufferSize];
		dataBuffer = new V4L4JDataBuffer(frameBuffer);
		raster = null;
		bufferedImage = null;
		recycled = true;
	}
	
	/**
	 * This method marks this frame as ready to be delivered to the user, as its
	 * buffer has just been filled with a new frame of the given length. 
	 * @param length the length of the new frame.
	 * @param sequence this frame's sequence number
	 * @param timeUs this frame capture timestamp in elapsed microseconds since startup
	 */
	final synchronized void prepareForDelivery(int length, long sequence, long timeUs){
		frameLength = length;
		dataBuffer.setNewFrameSize(length);
		sequenceNumber = sequence;
		captureTime = timeUs;
		recycled = false;
	}
	
	/**
	 * This method is used by the owning frame grabber so it can wait until
	 * this frame is recycled.
	 * @throws InterruptedException if we were interrupted while waiting
	 * for the frame to be recycled.
	 */
	final synchronized void waitTillRecycled() throws InterruptedException {
		while(! recycled)
			wait();
	}
	
	/**
	 * This method is used by the owning frame grabber to get a reference
	 * to the byte array used to hold the frame data.
	 * @return the byte array used to hold the frame data
	 */
	final byte[] getByteArray() {
		return frameBuffer;
	}
	
	@Override
	public final synchronized int getFrameLength(){
		checkIfRecycled();
		return frameLength;
	}
	
	@Override
	public final synchronized byte[] getBytes(){
		checkIfRecycled();
		return frameBuffer;
	}

	@Override
	public final synchronized DataBuffer getDataBuffer() {
		checkIfRecycled();
		return dataBuffer;
	}
	
	@Override
	public final synchronized WritableRaster getRaster() {
		checkIfRecycled();		
		return refreshRaster();
	}
	
	@Override
	public final synchronized BufferedImage getBufferedImage(){
		checkIfRecycled();
		return refreshBufferedImage();
	}
	
	@Override
	public final synchronized long getSequenceNumber(){
		checkIfRecycled();
		return sequenceNumber;
	}
	
	@Override
	public final synchronized long getCaptureTime(){
		checkIfRecycled();
		return captureTime;
	}
	
	@Override
	public final synchronized void recycle() {
		if (! recycled){
			frameGrabber.recycleVideoBuffer(this);
			recycled = true;
			notifyAll();
		}
	}

	/**
	 * Subclasses can override this method to either
	 * return a {@link WritableRaster} for this video frame, or throw
	 * a {@link UnsupportedMethod} exception if this video frame cannot
	 * generate a {@link WritableRaster}.
	 * @return  a {@link WritableRaster}.
	 * @throws UnsupportedMethod exception if a raster cannot be generated
	 * for this video frame (because of its image format for instance) 
	 */
	protected WritableRaster refreshRaster() {
		if (raster == null)
			throw new UnsupportedMethod("A raster can not be generated for this "
					+ "image format ("+frameGrabber.getImageFormat().toString()+")");

		return raster;
	}

	/**
	 * Subclasses can override this method to either
	 * return a {@link BufferedImage} for this video frame, or throw
	 * a {@link UnsupportedMethod} exception if this video frame cannot
	 * generate a {@link BufferedImage}.
	 * @return  a {@link BufferedImage}.
	 * @throws UnsupportedMethod exception if a buffered image cannot be generated
	 * for this video frame (because of its image format for instance) 
	 */
	protected BufferedImage refreshBufferedImage() {
		if (bufferedImage == null)
			throw new UnsupportedMethod("A Bufferedimage can not be generated for this "
					+ "image format ("+frameGrabber.getImageFormat().toString()+")");

		return bufferedImage;
	}
	
	/**
	 * This method must be called with this video frame lock held, and
	 * throws a {@link StateException} if it is recycled.
	 * @throws StateException if this video frame is recycled.
	 */
	private final void checkIfRecycled() throws StateException {
		if (recycled)
			throw new StateException("This video frame has been recycled");
	}
}
