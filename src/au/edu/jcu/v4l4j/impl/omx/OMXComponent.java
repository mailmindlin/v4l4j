package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Consumer;
import java.util.stream.Collectors;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.component.Component;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.ComponentProvider;
import au.edu.jcu.v4l4j.api.component.ComponentRole;
import au.edu.jcu.v4l4j.api.component.ComponentState;
import au.edu.jcu.v4l4j.api.component.port.AudioPort;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public class OMXComponent implements Component {
	
	/**
	 * Get the offsets/lengths of the AUDIO, VIDEO, IMAGE, and OTHER blocks of ports on this component.
	 * @param pointer
	 *     Pointer to native memory for this component
	 * @param result
	 *     Array of size 8 that the result will be placed in.
	 */
	private static native void getPortOffsets(long pointer, int[] result);
	
	/**
	 * Get the state of the component.
	 * @param pointer
	 *     Pointer to native memory for this component
	 */
	private static native int getComponentState(long pointer);
	
	/**
	 * Attempt to set the state of this component
	 * @param pointer
	 *     Pointer to native memory for this component
	 */
	private static native void setComponentState(long pointer, int state);
	
	/**
	 * Get the definition for a port. The result array should have a length of
	 * at least 20. Results are stored as follows:
	 * <table border="1" style="border-collapse:collapse">
	 * <tr><th>#</th><th>Video</th><th>Image</th><th>Audio</th><th>Other</th></tr>
	 * <tr><th>0</th><td colspan="4">Length of used cells</td></tr>
	 * <tr><th>1</th><td colspan="4">Port direction (1=input,0=output)</td></tr>
	 * <tr><th>2</th><td colspan="4">Buffer count</td></tr>
	 * <tr><th>3</th><td colspan="4">Minimum buffer count</td></tr>
	 * <tr><th>4</th><td colspan="4">Buffer size</td></tr>
	 * <tr><th>5</th><td colspan="4">Enabled (1=true,0=false)</td></tr>
	 * <tr><th>6</th><td colspan="4">Populated (1=true,0=false)</td></tr>
	 * <tr><th>7</th><td colspan="4">Domain (port type)</td></tr>
	 * <tr><th>8</th><td colspan="4">Contiguous buffers (1=true,0=false)</td></tr>
	 * <tr><th>9</th><td colspan="4">Buffer alignment</td></tr>
	 * <tr><th>10</th><td colspan="3">Error flag concealment (1=true,0=false)</td><td>Format</td></tr>
	 * <tr><th>11</th><td colspan="2">Frame width</td><td>Encoding</td></tr>
	 * <tr><th>12</th><td colspan="2">Frame height</td></tr>
	 * <tr><th>13</th><td colspan="2">Stride</td></tr>
	 * <tr><th>14</th><td colspan="2">Slice height</td></tr>
	 * <tr><th>15</th><td colspan="2">Color format</td></tr>
	 * <tr><th>16</th><td>Compression</td><td>Compression</td></tr>
	 * <tr><th>17</th><td>Bitrate</td></tr>
	 * <tr><th>18</th><td>Framerate</td></tr>
	 * </table>
	 * @param pointer
	 * @param portIndex
	 * @param result
	 * @return MIME type of port, or null if not available
	 */
	private static native String getPortInfo(long pointer, int portIndex, int[] result);
	
	private static native void setPortInfo(long pointer, int portIndex, int[] values);
	
	/**
	 * Enable or disable a port
	 * @param pointer
	 *     The pointer to this OMXComponent's native memory
	 * @param portIndex
	 *     The index of the port to enable/disable
	 * @param enabled
	 *     State in which the caller wants the port in
	 */
	private static native void enablePort(long pointer, int portIndex, boolean enabled);
	
	private static native OMXFrameBuffer doUseBuffer(long pointer, int portIndex, boolean allocate, int bufferSize, ByteBuffer buffer);
	
	private static native void doEmptyThisBuffer(long pointer, long bufferPtr, int position, int size, int sequence, long timestamp);
	
	private static native void doFillThisBuffer(long pointer, long bufferPtr);
	
	private static native int getPortFormats(long pointer, int portIndex, List<OMXVideoFormatOption> options);
	
	/**
	 * Access OMX config/parameter.
	 * Wraps macros OMX_
	 * 
	 * @param pointer
	 *            Pointer to native data
	 * @param isConfig
	 *            True if the index to lookup is a config. Else parameter
	 * @param read
	 *            Whether to get the value of the config/parameter. Else set the
	 *            value.
	 * @param configIndex
	 *            Index of config/parameter to get/set
	 * @param data
	 *            Data to get/set
	 */
	private static native int doAccessConfig(long pointer, boolean isConfig, boolean read, boolean throwOnError, int configIndex, ByteBuffer data);
	
	/**
	 * The name of this component
	 */
	private final String name;
	/**
	 * A pointer to the native memory of this component.
	 */
	private final long pointer;
	/**
	 * The provider that created this component
	 */
	private final OMXComponentProvider provider;
	
	protected int audioPortMinIdx;
	protected int numAudioPorts;
	protected int videoPortMinIdx;
	protected int numVideoPorts;
	protected int imagePortMinIdx;
	protected int numImagePorts;
	protected int otherPortMinIdx;
	protected int numOtherPorts;
	
	protected Set<OMXComponentPort> ports;
	
	protected Set<OMXFrameBuffer> buffers = new HashSet<>();
	protected Map<Long, OMXFrameBuffer> queuedBuffers = new ConcurrentHashMap<>();
	
	protected OMXComponent(OMXComponentProvider provider, String name) {
		this.provider = provider;
		this.name = name;
		this.pointer = getComponentHandle(name);
	}
	
	/**
	 * Get a handle to this' component and allocate all native memory
	 */
	private final native long getComponentHandle(String name);
	
	protected void setPortEnabled(int portIndex, boolean enabled) {
		OMXComponent.enablePort(this.pointer, portIndex, enabled);
	}
	
	protected OMXFrameBuffer allocateBufferOnPort(int portIndex, int bufferSize) {
		OMXFrameBuffer result = OMXComponent.doUseBuffer(this.pointer, portIndex, true, bufferSize, null);
		this.buffers.add(result);
		return result;
	}
	
	protected OMXFrameBuffer useBufferOnPort(int portIndex, ByteBuffer buffer) {
		if (!buffer.isDirect())
			// We might be able to change this by leveraging the UseBuffer's
			// functionality
			// when NULL is passed, but not ATM
			throw new IllegalArgumentException("Only direct buffers can be used");
		OMXFrameBuffer result = OMXComponent.doUseBuffer(this.pointer, portIndex, false, buffer.capacity(), buffer);
		this.buffers.add(result);
		return result;
	}
	
	protected void emptyThisBuffer(OMXFrameBuffer buffer) {
		this.queuedBuffers.put(buffer.pointer, buffer);
		OMXComponent.doEmptyThisBuffer(this.pointer, buffer.pointer, buffer.buffer.position(), buffer.buffer.limit(), (int) buffer.getSequenceNumber(), buffer.getTimestamp());
	}
	
	protected void fillThisBuffer(OMXFrameBuffer buffer) {
		this.queuedBuffers.put(buffer.pointer, buffer);
		OMXComponent.doFillThisBuffer(this.pointer, buffer.pointer);
	}
	
	protected String getPortData(int portIndex, int[] info) {
		return OMXComponent.getPortInfo(this.pointer, portIndex, info);
	}
	
	public void getConfig(boolean isConfig, int configIdx, ByteBuffer data) {
		OMXComponent.doAccessConfig(this.pointer, isConfig, true, true, configIdx, data);
	}
	
	public void setConfig(boolean isConfig, int configIdx, ByteBuffer data) {
		OMXComponent.doAccessConfig(this.pointer, isConfig, false, true, configIdx, data);
	}
	
	public int accessConfig(boolean isConfig, boolean read, boolean throwOnError, int queryIdx, ByteBuffer data) {
		return OMXComponent.doAccessConfig(this.pointer, isConfig, read, throwOnError, queryIdx, data);
	}
	
	protected void setPortData(int portIndex, int[] info) {
		
	}
	
	public long getPointer() {
		return this.pointer;
	}
	
	protected void doInitPorts() {
		synchronized (this) {
			if (this.ports != null)
				return;
			
			Set<OMXComponentPort> ports = new HashSet<>();
			this.ports = ports;
			int[] idx = new int[8];
			OMXComponent.getPortOffsets(this.pointer, idx);
			
			this.audioPortMinIdx = idx[0];
			this.numAudioPorts = idx[1];
			if (this.numAudioPorts > 0) {
				int[] portInfo = new int[20];
				for (int i = 0; i < this.numAudioPorts; i++) {
					int portIndex = this.audioPortMinIdx + i;
					String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
					ports.add(new OMXAudioPort(this, portIndex, mime, portInfo));
				}
			}
			
			this.videoPortMinIdx = idx[2];
			this.numVideoPorts = idx[3];
			if (this.numVideoPorts > 0) {
				int[] portInfo = new int[20];
				for (int i = 0; i < this.numVideoPorts; i++) {
					int portIndex = this.videoPortMinIdx + i;
					String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
					ports.add(new OMXVideoPort(this, portIndex, mime, portInfo));
				}
			}
			
			this.imagePortMinIdx = idx[4];
			this.numImagePorts = idx[5];
			if (this.numImagePorts > 0) {
				int[] portInfo = new int[20];
				for (int i = 0; i < this.numImagePorts; i++) {
					int portIndex = this.imagePortMinIdx + i;
					String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
					ports.add(new OMXImagePort(this, portIndex, mime, portInfo));
				}
			}
			
			this.otherPortMinIdx = idx[6];
			this.numOtherPorts = idx[7];
			if (this.numOtherPorts > 0) {
				int[] portInfo = new int[20];
				for (int i = 0; i < this.numOtherPorts; i++) {
					int portIndex = this.otherPortMinIdx + i;
					String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
					ports.add(new OMXComponentPort(this, portIndex, mime, portInfo));
				}
			}
		}
	}
	
	@Override
	public String getName() {
		return this.name;
	}
	
	@Override
	public ComponentProvider getProvider() {
		return this.provider;
	}
	
	@Override
	public ComponentState getState() {
		int state = getComponentState(this.pointer);
		System.out.println("State: " + Integer.toHexString(state));
		switch (state) {
			case 0:
				return ComponentState.INVALID;
			case 1:
				return ComponentState.LOADED;
			case 2:
				return ComponentState.IDLE;
			case 3:
				return ComponentState.EXECUTING;
			case 4:
				return ComponentState.PAUSED;
			case 5:
				return ComponentState.WAIT_FOR_RESOURCES;
		}
		throw new IllegalArgumentException("Unknown state: " + state);
	}
	
	@Override
	public ComponentState setState(ComponentState state) throws Exception {
		//Map ComponentState enum to native enum index
		int stateI = 0;
		switch (state) {
			case INVALID:
			case UNLOADED:
				stateI = 0;// Actually STATE_INVALID
				break;
			case LOADED:
				stateI = 1;
				break;
			case IDLE:
				stateI = 2;
				break;
			case EXECUTING:
				stateI = 3;
				break;
			case PAUSED:
				stateI = 4;
				break;
			case WAIT_FOR_RESOURCES:
				stateI = 5;
				break;
			
		}
		setComponentState(this.pointer, stateI);
		//TODO block for callback
		return getState();
	}
	
	@Override
	public Set<OMXComponentPort> getPorts() {
		if (this.ports == null)
			this.doInitPorts();
		return new HashSet<>(this.ports);
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public Set<? extends AudioPort> getAudioPorts() {
		if (this.ports == null)
			this.doInitPorts();
		return (Set<? extends AudioPort>) (Set<?>) this.ports.stream()
				.filter(port->(port.getPortType() == StreamType.AUDIO))
				.collect(Collectors.toSet());
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public Set<? extends ImagePort> getImagePorts() {
		if (this.ports == null)
			this.doInitPorts();
		return (Set<? extends ImagePort>) (Set<?>) this.ports.stream()
				.filter(port->(port.getPortType() == StreamType.IMAGE))
				.collect(Collectors.toSet());
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public Set<? extends VideoPort> getVideoPorts() {
		if (this.ports == null)
			this.doInitPorts();
		return (Set<? extends VideoPort>) (Set<?>) this.ports.stream()
				.filter(port->(port.getPortType() == StreamType.VIDEO))
				.collect(Collectors.toSet());
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public Set<? extends ComponentPort> getOtherPorts() {
		if (this.ports == null)
			this.doInitPorts();
		return (Set<? extends ComponentPort>) (Set<?>) this.ports.stream()
				.filter(port->{
					StreamType type = port.getPortType();
					return type != StreamType.AUDIO && type != StreamType.VIDEO && type != StreamType.IMAGE;
				}).collect(Collectors.toSet());
	}
	
	@Override
	public OMXComponentPort getPort(int index) {
		return getPorts().stream()
				.filter(port->(port.getIndex() == index))
				.findAny().orElse(null);
	}
	
	@Override
	public Set<ComponentRole> getRoles() {
		// TODO Auto-generated method stub
		return null;
	}
	
	private void printExceptionFromHandler(String handlerName, Exception e, int portIdx) {
		String threadName = Thread.currentThread().getName();
		//Rewrite stack trace
		StackTraceElement[] oldTrace = e.getStackTrace();
		StackTraceElement[] newTrace = new StackTraceElement[oldTrace.length + 1];
		System.arraycopy(oldTrace, 0, newTrace, 0, oldTrace.length);
		newTrace[newTrace.length - 1] = new StackTraceElement(getClass().getName(), handlerName, this.toString(), portIdx);
		e.setStackTrace(newTrace);
		//Print to stderr
		System.err.println("Exception in thread \"" + threadName + "\":");
		e.printStackTrace();
	}
	
	/**
	 * Called by JNI code from OMX event handlers.
	 */
	private void onBufferDone(OMXFrameBuffer buffer, boolean emptied, int ticks, long timestamp, int offset, int filled, int flags) {
		String handlerName = emptied ? "OMX_BUFFER_EMPTY_HANDLER" : "OMX_BUFFER_FILL_HANDLER";
		if (buffer == null) {
			System.err.println("OMX null buffer in " + handlerName);
			return;
		}
		//Update fields in buffer
		buffer.prepare(ticks, timestamp, offset, filled, flags);
		
		Consumer<FrameBuffer> handler;
		int portId = emptied ? buffer.getInputPort() : buffer.getOutputPort();
		OMXComponentPort port = this.getPort(portId);
		if (port == null) {
			System.err.println("OMX Could not find port " + portId + " in " + handlerName);
			return;
		}
		synchronized (this) {
			if (emptied)
				handler = port.onBufferEmptied;
			else
				handler = port.onBufferFilled;
		}
		if (handler == null) {
			//We can't throw an exception from here
			System.err.println("No handler for event " + handlerName + " on port " + portId + " of " + this);
			return;
		}
		
		try {
			handler.accept(buffer);
		} catch (Exception e) {
			printExceptionFromHandler(handlerName, e, port.getIndex());
		}
	}
}
