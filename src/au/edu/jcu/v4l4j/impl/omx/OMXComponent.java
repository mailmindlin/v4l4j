package au.edu.jcu.v4l4j.impl.omx;

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
	
	protected OMXComponent(OMXComponentProvider provider, String name) {
		this.provider = provider;
		this.name = name;
		this.pointer = getComponentHandle(name);
	}
	
	private final native long getComponentHandle(String name);
	
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
				stateI = 0;//Actually STATE_INVALID
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
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<AudioPort> getAudioPorts() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<ImagePort> getImagePorts() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<VideoPort> getVideoPorts() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<ComponentPort> getOtherPorts() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ComponentPort getPort(int index) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<ComponentRole> getRoles() {
		// TODO Auto-generated method stub
		return null;
	}
}