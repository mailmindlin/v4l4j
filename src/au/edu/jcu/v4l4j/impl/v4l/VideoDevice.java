package au.edu.jcu.v4l4j.impl.v4l;

import java.nio.file.Path;
import java.util.Collections;
import java.util.Set;

import au.edu.jcu.v4l4j.api.component.Component;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.ComponentProvider;
import au.edu.jcu.v4l4j.api.component.ComponentRole;
import au.edu.jcu.v4l4j.api.component.ComponentState;
import au.edu.jcu.v4l4j.api.component.port.AudioPort;
import au.edu.jcu.v4l4j.api.component.port.ImagePort;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;

public class VideoDevice implements Component {
	protected final V4LComponentProvider provider;
	protected final Path path;
	protected final VideoDeviceCapturePort capturePort = new VideoDeviceCapturePort(this, 1);
	
	public VideoDevice(V4LComponentProvider provider, Path path) {
		this.provider = provider;
		this.path = path;
	}
	
	@Override
	public String getName() {
		return V4LComponentProvider.PREFIX + ".camera";
	}

	@Override
	public ComponentProvider getProvider() {
		return provider;
	}

	@Override
	public ComponentState getState() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ComponentState setState(ComponentState state) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<ComponentPort> getPorts() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<AudioPort> getAudioPorts() {
		return Collections.emptySet();
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
