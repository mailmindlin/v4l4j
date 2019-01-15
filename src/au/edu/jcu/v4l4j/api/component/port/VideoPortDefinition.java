package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.VideoCompressionType;

public interface VideoPortDefinition extends IVPortDefinition {
	ImagePalette getCompression();
}
