package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.VideoCompressionType;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public class OMXVideoPort extends OMXComponentPort implements VideoPort {
	
	protected int frameWidth;
	protected int frameHeight;
	protected int stride;
	protected int sliceHeight;
	protected int compression;

	protected OMXVideoPort(OMXComponent component, int id, String mime, int[] info) {
		super(component, id, mime, info);
	}
	
	@Override
	public void initControls() {
		BaseOMXQueryControl portFormatControl = new BaseOMXQueryControl(this.component, "format", OMXConstants.INDEX_ParamVideoPortFormat, this.getIndex(), OMXConstants.PARAM_PORTFORMATTYPE);
		BaseOMXQueryControl bitrateControl = new BaseOMXQueryControl(this.component, "bitrate", OMXConstants.INDEX_ParamVideoBitrate, this.getIndex(), OMXConstants.PARAM_BITRATETYPE);
		
	}
	
	@Override
	protected void pullInfo(int[] info) {
		super.pullInfo(info);
		if (info[0] < 19)
			throw new IllegalArgumentException("Not enough info (expected 19; actual " + info[0] + ")");
		
		this.frameWidth = info[11];
		this.frameHeight = info[12];
		this.stride = info[13];
		this.sliceHeight = info[14];
		
		//TODO map palette
		
		this.compression = info[16];
	}

	@Override
	public StreamType getPortType() {
		return StreamType.VIDEO;
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
	public VideoCompressionType getCompression() {
		int compression = this.compression;
		VideoCompressionType[] compressionTypes = VideoCompressionType.values();
		if (compression < 0 || compression > compressionTypes.length)
			return VideoCompressionType.UNUSED;
		return compressionTypes[compression];
	}

	@Override
	public ImagePalette getColorFormat() {
		// TODO Auto-generated method stub
		return null;
	}
}