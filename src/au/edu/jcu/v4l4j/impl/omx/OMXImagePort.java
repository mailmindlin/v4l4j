package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.ImageCodingType;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;

public class OMXImagePort extends OMXComponentPort implements ImagePort {

	protected int frameWidth;
	protected int frameHeight;
	protected int stride;
	protected int sliceHeight;
	protected int compression;
	
	protected OMXImagePort(OMXComponent component, int id, String mime, int[] info) {
		super(component, id, mime, info);
	}
	
	@Override
	protected void pullInfo(int[] info) {
		super.pullInfo(info);
		if (info[0] < 17)
			throw new IllegalArgumentException("Not enough info (expected 17; actual " + info[0] + ")");
		
		this.frameWidth = info[11];
		this.frameHeight = info[12];
		this.stride = info[13];
		this.sliceHeight = info[14];
		
		//TODO map palette
		
		this.compression = info[16];
	}

	@Override
	public int getFrameWidth() {
		return this.frameWidth;
	}

	@Override
	public int getFrameHeight() {
		return this.frameHeight;
	}

	@Override
	public int getStride() {
		return this.stride;
	}

	@Override
	public int getSliceHeight() {
		return this.sliceHeight;
	}

	@Override
	public ImageCodingType getCodingType() {
		return ImageCodingType.values()[this.compression];
	}

}
