package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.ImageCodingType;

public interface ImagePortDefinition extends IVPortDefinition {
	ImageCodingType getCodingType();
}
