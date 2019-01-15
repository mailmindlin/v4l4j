package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Objects;
import java.util.Set;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.port.PortDefinition;
import au.edu.jcu.v4l4j.api.control.Control;

public class OMXComponentPort implements ComponentPort {
	protected final int id;
	protected final StreamType type;
	protected final OMXComponent component;
	protected OMXPortDefinition portDefinition;
	protected final HashMap<String, AbstractOMXQueryControl<?>> controls = new HashMap<>();
	protected Consumer<FrameBuffer> onBufferFilled;
	protected Consumer<FrameBuffer> onBufferEmptied;
	
	protected OMXComponentPort(OMXComponent component, int id, String mime, StreamType type) {
		Objects.requireNonNull(component, "Component may not be null");
		this.component = component;
		this.id = id;
		this.type = type;
	}
	
	protected OMXComponentPort(OMXComponent component, int id, String mime, int[] info) {
		this.component = component;
		this.id = id;
		this.portDefinition = OMXPortDefinition.from(mime, info);
		this.type = this.portDefinition.type;
	}
	
	protected void initControls() {
		//BaseOMXQueryControl portFormatControl = new BaseOMXQueryControl(this.component, "format", OMXConstants.INDEX_ParamVideoPortFormat, this.getIndex(), OMXConstants.PARAM_VIDEO_PORTFORMATTYPE, null);
		//BaseOMXQueryControl supplierTypeControl = new BaseOMXQueryControl(this.component, "_supplierTypeBase", OMXConstants.INDEX_ParamCompBufferSupplier, this.getIndex(), null, null);
		//supplierTypeControl.children.add(new NumberOMXQueryControl(supplierTypeControl, this.id, "supplierType", "eBufferSupplier", null));
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
	public boolean isEnabled() {
		return this.getDefinition().isEnabled();
	}
	
	@Override
	public boolean setEnabled(boolean aflag) {
		this.getComponent().setPortEnabled(this.getIndex(), aflag);
		if (this.portDefinition != null)
			this.portDefinition.enabled = aflag;
		return aflag;
	}
	
	@Override
	public OMXFrameBuffer allocateBuffer(int length) {
		OMXFrameBuffer result = this.getComponent().allocateBufferOnPort(this.getIndex(), length);
		//TODO: track on portDefinition?
		return result;
	}
	
	@Override
	public FrameBuffer useBuffer(ByteBuffer buffer) {
		OMXFrameBuffer result = this.getComponent().useBufferOnPort(this.getIndex(), buffer);
		//TODO: track on portDefinition?
		return result;
	}
	
	@Override
	public void releaseBuffer(FrameBuffer buffer) {
		Objects.requireNonNull(buffer, "Cannot release null");
		this.getComponent().releaseThisBuffer(this.getIndex(), (OMXFrameBuffer) buffer);
	}
	
	@Override
	public void empty(FrameBuffer buffer) {
		Objects.requireNonNull(buffer, "Buffer may not be null");
		this.getComponent().emptyThisBuffer((OMXFrameBuffer) buffer);
	}
	
	@Override
	public void fill(FrameBuffer buffer) {
		Objects.requireNonNull(buffer, "Buffer may not be null");
		this.getComponent().fillThisBuffer((OMXFrameBuffer) buffer);
	}
	
	@Override
	public void flush() {
		this.getComponent().flushThisPort(this.getIndex());
	}
	
	@Override
	public StreamType getPortType() {
		return this.type;
	}
	
	@Override
	public Set<String> getControlNames() {
		return this.controls.keySet();
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public <U, V extends Control<U>> V getControlByName(String name) {
		return (V) this.controls.get(name);
	}
	
	@Override
	public void onBufferEmpty(Consumer<FrameBuffer> handler) {
		synchronized (this) {
			this.onBufferEmptied = handler;
		}
	}
	
	@Override
	public void onBufferFill(Consumer<FrameBuffer> handler) {
		synchronized (this) {
			this.onBufferFilled = handler;
		}
	}

	@Override
	public PortDefinition getDefinition(boolean flush) {
		if (flush || this.portDefinition == null) {
			//TODO: do we need this lock?
			synchronized (this.getComponent()) {
				int[] info = new int[16];
				String mime = this.getComponent().getPortData(this.getIndex(), info);
				return this.portDefinition = OMXPortDefinition.from(mime, info);
			}
		}
		
		return this.portDefinition;
	}
}