package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.AudioEncodingType;
import au.edu.jcu.v4l4j.api.component.port.AudioPort;

public class OMXAudioPort extends OMXComponentPort implements AudioPort {
	protected AudioEncodingType encoding;
	
	protected OMXAudioPort(OMXComponent component, int id, String mime, int[] info) {
		super(component, id, mime, info);
		if (info[0] < 12)
			throw new IllegalArgumentException("Not enough info (expected 12; actual " + info[0] + ")");
		
		this.encoding = OMXConstants.mapAudioEncodingType(info[11]);
	}

	@Override
	public AudioEncodingType getEncoding() {
		return this.encoding;
	}

}
