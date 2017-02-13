package au.edu.jcu.v4l4j.impl.omx;

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
	public Component getComponent() {
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
		this.component.setPortEnabled(this.getIndex(), aflag);
		return this.enabled = aflag;
	}

	@Override
	public boolean isPopulated() {
		return this.populated;
	}

	@Override
	public FrameBuffer allocateBuffer(int length) {
		FrameBuffer result = this.component.allocateBufferOnPort(this.getIndex(), length);
		this.bufferCountActual++;
		return result;
	}
}