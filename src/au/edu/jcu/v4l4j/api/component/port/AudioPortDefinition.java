package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.AudioEncodingType;

public interface AudioPortDefinition extends PortDefinition {
	AudioEncodingType getEncoding();
}
