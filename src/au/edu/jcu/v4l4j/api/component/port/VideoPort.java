package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.VideoCompressionType;

public interface VideoPort extends IVPort {
	
	VideoCompressionType getCompression();
}
