package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.util.Set;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.VideoCompressionType;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public class OMXVideoPort extends OMXComponentPort implements VideoPort {

	protected OMXVideoPort(OMXComponent component, int id) {
		super(component, id);
	}

	@Override
	public StreamType getPortType() {
		return StreamType.VIDEO;
	}

	@Override
	public String getMIMEType() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void fill(FrameBuffer buffer) {
		// TODO Auto-generated method stub
		
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
	public VideoCompressionType getCompression() {
		// TODO Auto-generated method stub
		return null;
	}
	
}