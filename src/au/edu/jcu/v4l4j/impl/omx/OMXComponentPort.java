package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.component.Component;
import au.edu.jcu.v4l4j.api.component.ComponentPort;

public abstract class OMXComponentPort implements ComponentPort {
	protected final int id;
	protected final OMXComponent component;
	protected boolean input;
	protected int bufferCountActual;
	protected int bufferCountMin;
	protected int bufferSize;
	protected boolean enabled;
	protected boolean populated;
	
	protected OMXComponentPort(OMXComponent component, int id) {
		this.component = component;
		this.id = id;
	}

	@Override
	public int getIndex() {
		return this.id;
	}

	@Override
	public OMXComponent getComponent() {
		return this.component;
	}
	
	@Override
	public boolean isInput() {
		return this.input;
	}

	@Override
	public boolean isOutput() {
		return !isInput();
	}

	@Override
	public boolean isEnabled() {
		return this.enabled;
	}

	@Override
	public boolean setEnabled(boolean aflag) {
		this.getComponent().setPortEnabled(this.getIndex(), aflag);
		return this.enabled = aflag;
	}

	@Override
	public boolean isPopulated() {
		return this.populated;
	}

	@Override
	public OMXFrameBuffer allocateBuffer(int length) {
		OMXFrameBuffer result = this.getComponent().allocateBufferOnPort(this.getIndex(), length);
		this.bufferCountActual++;
		return result;
	}

	@Override
	public int minimumBuffers() {
		return this.bufferCountMin;
	}

	@Override
	public int actualBuffers() {
		return this.bufferCountActual;
	}

	@Override
	public int bufferSize() {
		return this.bufferSize;
	}

	@Override
	public FrameBuffer useBuffer(ByteBuffer buffer) {
		this.getComponent().useBufferOnPort(this.getIndex(), buffer);
		return null;
	}

	@Override
	public void empty(FrameBuffer buffer) {
		this.getComponent().emptyThisBuffer((OMXFrameBuffer) buffer);
	}

	@Override
	public void fill(FrameBuffer buffer) {
		this.getComponent().fillThisBuffer((OMXFrameBuffer) buffer);
	}
}