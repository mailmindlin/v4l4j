package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.component.port.ImagePort;
import au.edu.jcu.v4l4j.api.component.port.ImagePortDefinition;

public class OMXImagePort extends OMXComponentPort implements ImagePort {
	protected OMXImagePort(OMXComponent component, int id, String mime, int[] info) {
		super(component, id, mime, info);
	}
	
	@Override
	protected void initControls() {
		super.initControls();
		BaseOMXQueryControl portFormatControl = new BaseOMXQueryControl(this.component, "format", OMXConstants.INDEX_ParamImagePortFormat, this.getIndex(), OMXConstants.PARAM_VIDEO_PORTFORMATTYPE, null);
		BaseOMXQueryControl flashControl = new BaseOMXQueryControl(this.component, "flash", OMXConstants.INDEX_ConfigFlashControl, this.getIndex(), null, null);
		
	}
	
	public ImagePortDefinition getDefinition(boolean flush) {
		return (ImagePortDefinition) super.getDefinition(flush);
	}
}
