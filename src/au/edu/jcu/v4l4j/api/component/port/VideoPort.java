package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.VideoCompressionType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;

public interface VideoPort extends ComponentPort {
	int getFrameWidth();
	int getFrameHeight();
	int getStride();
	int getSliceHeight();
	VideoCompressionType getCompression();
}
