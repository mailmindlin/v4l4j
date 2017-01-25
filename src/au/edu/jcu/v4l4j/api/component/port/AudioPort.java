package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.AudioEncodingType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;

public interface AudioPort extends ComponentPort {
	AudioEncodingType getEncoding();
}
