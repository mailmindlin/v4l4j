package au.edu.jcu.v4l4j.encoder;

import au.edu.jcu.v4l4j.BaseVideoFrame;

public abstract class AbstractConvertedVideoFrame extends BaseVideoFrame {

	protected AbstractConvertedVideoFrame(int bufferSize) {
		super(null, bufferSize);
	}
	
	@Override
	protected synchronized void prepareForDelivery(int length, int index, long sequence, long timeUs) {
		super.prepareForDelivery(length, index, sequence, timeUs);
	}
}
