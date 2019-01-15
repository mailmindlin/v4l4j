package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.component.ComponentPort;

public interface AudioPort extends ComponentPort {
	@Override
	default AudioPortDefinition getDefinition() {
		return getDefinition(false);
	}
	
	@Override
	AudioPortDefinition getDefinition(boolean flush);
}
