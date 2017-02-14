package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.AudioEncodingType;
import au.edu.jcu.v4l4j.api.component.port.AudioPort;

public class OMXAudioPort extends OMXComponentPort implements AudioPort {

	protected OMXAudioPort(OMXComponent component, int id) {
		super(component, id);
	}

	@Override
	public AudioEncodingType getEncoding() {
		// TODO Auto-generated method stub
		return null;
	}

}
