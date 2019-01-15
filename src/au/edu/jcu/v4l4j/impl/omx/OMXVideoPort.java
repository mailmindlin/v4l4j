package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;
import au.edu.jcu.v4l4j.api.component.port.VideoPortDefinition;

public class OMXVideoPort extends OMXComponentPort implements VideoPort {

	protected OMXVideoPort(OMXComponent component, int id, String mime, int[] info) {
		super(component, id, mime, info);
	}
	
	@Override
	public void initControls() {
		BaseOMXQueryControl portFormatControl = new BaseOMXQueryControl(this.component, "format", OMXConstants.INDEX_ParamVideoPortFormat, this.getIndex(), OMXConstants.PARAM_VIDEO_PORTFORMATTYPE, null);
		BaseOMXQueryControl bitrateControl = OMXConstants.CTRL_VIDEO_CONFIG_BITRATE.build(this.component, this.getIndex());
	}

	@Override
	public StreamType getPortType() {
		return StreamType.VIDEO;
	}
	
	@Override
	public VideoPortDefinition getDefinition(boolean flush) {
		return (VideoPortDefinition) super.getDefinition(flush);
	}
}