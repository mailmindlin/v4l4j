package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import au.edu.jcu.v4l4j.api.component.Component;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.ComponentProvider;
import au.edu.jcu.v4l4j.api.component.ComponentRole;
import au.edu.jcu.v4l4j.api.component.ComponentState;
import au.edu.jcu.v4l4j.api.component.port.AudioPort;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public class OMXComponent implements Component {
	
	private static native void getPortOffsets(long pointer, int[] result);
	
	private static native int getComponentState(long pointer);
	
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
	
	private static native void enablePort(long pointer, int portIndex, boolean enabled);
	
	private static native OMXFrameBuffer doAllocateBuffer(long pointer, int portIndex, int bufferSize);
	
	private static native OMXFrameBuffer doUseBuffer(long pointer, int portIndex, ByteBuffer buffer);
	
	private static native void doEmptyThisBuffer(long pointer, long bufferPtr);
	
	private static native void doFillThisBuffer(long pointer, long bufferPtr);
	
	private static native int getPortFormats(long pointer, int portIndex, List<OMXVideoFormatOption> options);
	
	private static native void doGetConfig(long pointer, int configIndex, ByteBuffer data);
	
	private static native void doSetConfig(long pointer, int configIndex, ByteBuffer data);
	
	private final String name;
	private final long pointer;
	private final OMXComponentProvider provider;
	
	protected int audioPortMinIdx;
	protected int numAudioPorts;
	protected int videoPortMinIdx;
	protected int numVideoPorts;
	protected int imagePortMinIdx;
	protected int numImagePorts;
	protected int otherPortMinIdx;
	protected int numOtherPorts;
	
	protected List<AudioPort> audioPorts;
	protected List<VideoPort> videoPorts;
	protected List<ImagePort> imagePorts;
	protected List<ComponentPort> otherPorts;
	
	protected OMXComponent(OMXComponentProvider provider, String name) {
		this.provider = provider;
		this.name = name;
		this.pointer = getComponentHandle(name);
	}
	
	private final native long getComponentHandle(String name);
	
	protected void setPortEnabled(int portIndex, boolean enabled) {
		OMXComponent.enablePort(this.pointer, portIndex, enabled);
	}
	
	protected OMXFrameBuffer allocateBufferOnPort(int portIndex, int bufferSize) {
		return OMXComponent.doAllocateBuffer(this.pointer, portIndex, bufferSize);
	}
	
	protected OMXFrameBuffer useBufferOnPort(int portIndex, ByteBuffer buffer) {
		if (!buffer.isDirect())
			// We might be able to change this by leveraging the UseBuffer's
			// functionality
			// when NULL is passed, but not ATM
			throw new IllegalArgumentException("Only direct buffers can be used");
		return OMXComponent.doUseBuffer(this.pointer, portIndex, buffer);
	}
	
	protected void emptyThisBuffer(OMXFrameBuffer buffer) {
		OMXComponent.doEmptyThisBuffer(this.pointer, buffer.pointer);
	}
	
	protected void fillThisBuffer(OMXFrameBuffer buffer) {
		OMXComponent.doFillThisBuffer(this.pointer, buffer.pointer);
	}
	
	protected String getPortData(int portIndex, int[] info) {
		return OMXComponent.getPortInfo(this.pointer, portIndex, info);
	}
	
	protected void setPortData(int portIndex, int[] info) {
		
	}
	
	public long getPointer() {
		return this.pointer;
	}
	
	protected void doInitPorts() {
		int[] idx = new int[8];
		OMXComponent.getPortOffsets(this.pointer, idx);
		this.audioPortMinIdx = idx[0];
		this.numAudioPorts = idx[1];
		if (this.numAudioPorts == 0)
			this.audioPorts = Collections.emptyList();
		else {
			this.audioPorts = new ArrayList<>(this.numAudioPorts);
			int[] portInfo = new int[20];
			for (int i = 0; i < this.numAudioPorts; i++) {
				int portIndex = this.audioPortMinIdx + i;
				String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
				this.audioPorts.add(new OMXAudioPort(this, portIndex, mime, portInfo));
			}
		}
		
		this.videoPortMinIdx = idx[2];
		this.numVideoPorts = idx[3];
		if (this.numVideoPorts == 0)
			this.videoPorts = Collections.emptyList();
		else {
			this.videoPorts = new ArrayList<>(this.numVideoPorts);
			int[] portInfo = new int[20];
			for (int i = 0; i < this.numVideoPorts; i++) {
				int portIndex = this.videoPortMinIdx + i;
				String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
				this.videoPorts.add(new OMXVideoPort(this, portIndex, mime, portInfo));
			}
		}
		
		this.imagePortMinIdx = idx[4];
		this.numImagePorts = idx[5];
		if (this.numImagePorts == 0)
			this.imagePorts = Collections.emptyList();
		else {
			this.imagePorts = new ArrayList<>(this.numImagePorts);
			int[] portInfo = new int[20];
			for (int i = 0; i < this.numImagePorts; i++) {
				int portIndex = this.imagePortMinIdx + i;
				String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
				this.imagePorts.add(new OMXImagePort(this, portIndex, mime, portInfo));
			}
		}
		
		this.otherPortMinIdx = idx[6];
		this.numOtherPorts = idx[7];
		if (this.numOtherPorts == 0)
			this.otherPorts = Collections.emptyList();
		else {
			this.otherPorts = new ArrayList<>(this.numOtherPorts);
			int[] portInfo = new int[20];
			for (int i = 0; i < this.numOtherPorts; i++) {
				int portIndex = this.otherPortMinIdx + i;
				String mime = OMXComponent.getPortInfo(this.pointer, portIndex, portInfo);
				this.otherPorts.add(new OMXComponentPort(this, portIndex, mime, portInfo));
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
		return getState();
	}
	
	@Override
	public Set<ComponentPort> getPorts() {
		HashSet<ComponentPort> result = new HashSet<>();
		result.addAll(this.getAudioPorts());
		result.addAll(this.getVideoPorts());
		result.addAll(this.getImagePorts());
		result.addAll(this.getOtherPorts());
		return result;
	}
	
	@Override
	public Set<AudioPort> getAudioPorts() {
		if (this.audioPorts == null)
			this.doInitPorts();
		return new HashSet<>(this.audioPorts);
	}
	
	@Override
	public Set<ImagePort> getImagePorts() {
		if (this.imagePorts == null)
			this.doInitPorts();
		return new HashSet<>(this.imagePorts);
	}
	
	@Override
	public Set<VideoPort> getVideoPorts() {
		if (this.videoPorts == null)
			this.doInitPorts();
		return new HashSet<>(this.videoPorts);
	}
	
	@Override
	public Set<ComponentPort> getOtherPorts() {
		if (this.otherPorts == null)
			this.doInitPorts();
		return new HashSet<>(this.otherPorts);
	}
	
	@Override
	public ComponentPort getPort(int index) {
		return getPorts().stream()
				.filter(port->(port.getIndex() == index))
				.findAny().orElse(null);
	}
	
	@Override
	public Set<ComponentRole> getRoles() {
		// TODO Auto-generated method stub
		return null;
	}
}