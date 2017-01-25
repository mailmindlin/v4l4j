package au.edu.jcu.v4l4j.impl.v4l;

import java.util.Set;

import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.VideoCompressionType;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public class VideoDeviceCapturePort implements VideoPort {
	protected final int id;
	protected final VideoDevice device;
	protected boolean enabled = false;
	
	protected VideoDeviceCapturePort(VideoDevice device, int id) {
		this.device = device;
		this.id = id;
	}
	
	@Override
	public StreamType getPortType() {
		return StreamType.VIDEO;
	}

	@Override
	public boolean isEnabled() {
		return this.enabled;
	}

	@Override
	public boolean setEnabled(boolean enabled) {
		// TODO Auto-generated method stub
		return false;
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

	@Override
	public int getIndex() {
		return this.id;
	}

	@Override
	public VideoDevice getComponent() {
		return this.device;
	}

	@Override
	public boolean isInput() {
		return false;
	}

	@Override
	public boolean isOutput() {
		return true;
	}

	@Override
	public boolean isPopulated() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public int minimumBuffers() {
		return 1;
	}

	@Override
	public int actualBuffers() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int bufferSize() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public String getMIMEType() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<String> getProperties() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Object getProperty(String key) {
		return null;
	}

	@Override
	public Object setProperty(String key, Object value) {
		// TODO Auto-generated method stub
		return null;
	}

}
