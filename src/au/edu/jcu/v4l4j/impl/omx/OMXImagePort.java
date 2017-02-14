package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.ImageCodingType;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;

public class OMXImagePort extends OMXComponentPort implements ImagePort {

	protected OMXImagePort(OMXComponent component, int id) {
		super(component, id);
	}

	@Override
	public int getFrameWidth() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getFrameHeight() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getStride() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getSliceHeight() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public ImageCodingType getCodingType() {
		// TODO Auto-generated method stub
		return null;
	}

}
