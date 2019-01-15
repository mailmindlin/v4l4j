package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.component.port.AudioPort;
import au.edu.jcu.v4l4j.api.component.port.AudioPortDefinition;

public class OMXAudioPort extends OMXComponentPort implements AudioPort {
	
	protected OMXAudioPort(OMXComponent component, int id, String mime, int[] info) {
		super(component, id, mime, info);
		
	}
	
	@Override
	public AudioPortDefinition getDefinition(boolean flush) {
		return (AudioPortDefinition) super.getDefinition(flush);
	}

}
