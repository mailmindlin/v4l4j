package au.edu.jcu.v4l4j.api.component;

import java.util.Set;

import au.edu.jcu.v4l4j.api.component.port.AudioPort;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public interface Component {
	/**
	 * Get component name
	 * @return
	 */
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
	 * @throws Exception on failure to complete transition.
	 */
	ComponentState setState(ComponentState state) throws Exception;
	
	/**
	 * Enumerate all the ports on this component
	 * @return
	 */
	Set<? extends ComponentPort> getPorts();
	Set<? extends AudioPort> getAudioPorts();
	Set<? extends ImagePort> getImagePorts();
	Set<? extends VideoPort> getVideoPorts();
	Set<? extends ComponentPort> getOtherPorts();
	
	/**
	 * Get the port at the given index
	 * @param index
	 * @return
	 */
	ComponentPort getPort(int index);
	
	Set<ComponentRole> getRoles();
}
