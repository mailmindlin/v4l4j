package au.edu.jcu.v4l4j.impl.v4l;

import java.nio.ByteBuffer;
import java.util.Set;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.VideoCompressionType;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;

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
		return this.enabled = enabled;
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
		//This port must be an output
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
	public FrameBuffer useBuffer(ByteBuffer buffer) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FrameBuffer allocateBuffer(int length) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void empty(FrameBuffer buffer) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void fill(FrameBuffer buffer) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public Set<String> getControlNames() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Control<?> getControlByName(String name) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ImagePalette getColorFormat() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void onBufferEmpty(Consumer<FrameBuffer> handler) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onBufferFill(Consumer<FrameBuffer> handler) {
		// TODO Auto-generated method stub
		
	}

}
