package au.edu.jcu.v4l4j.api.component;

import java.util.Set;

import au.edu.jcu.v4l4j.api.component.port.AudioPort;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public interface Component {
	String getName();
	ComponentProvider getProvider();
	/**
	 * Get the component's current state
	 * @return the component's current state
	 */
	ComponentState getState();
	/**
	 * Attempt to set the component's state
	 * @param state
	 * @return the state of the component after this call
	 */
	ComponentState setState(ComponentState state);
	Set<ComponentPort> getPorts();
	Set<AudioPort> getAudioPorts();
	Set<ImagePort> getImagePorts();
	Set<VideoPort> getVideoPorts();
	Set<ComponentPort> getOtherPorts();
	ComponentPort getPort(int index);
}
