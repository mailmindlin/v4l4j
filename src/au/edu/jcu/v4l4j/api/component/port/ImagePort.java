package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.ImageCodingType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;

public interface ImagePort extends ComponentPort {
	int getFrameWidth();
	int getFrameHeight();
	int getStride();
	int getSliceHeight();
	ImageCodingType getCodingType();
}
