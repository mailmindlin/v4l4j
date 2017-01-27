package au.edu.jcu.v4l4j.impl.v4l;

import java.io.FileNotFoundException;
import java.nio.file.Files;
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
import au.edu.jcu.v4l4j.exceptions.JNIException;
import au.edu.jcu.v4l4j.exceptions.ReleaseException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * A video source provided by the V4L component provider.
 * TODO: thread safety
 * @author mailmindlin
 */
public class VideoDevice implements Component {
	protected final V4LComponentProvider provider;
	protected final Path path;
	protected final VideoDeviceCapturePort capturePort = new VideoDeviceCapturePort(this, 1);
	protected ComponentState state;
	protected long pointer;
	

	/**
	 * This JNI method initializes the libv4's struct video_device
	 * 
	 * @param device
	 *            the name of the device file
	 * @return 
	 * @throws JNIException 
	 * @throws V4L4JException
	 *             if there is an error
	 */
	private static native long doInit(String device) throws JNIException;

	/**
	 * This JNI method releases resources used by libvideo's struct
	 * video_device, as allocated by <code>doInit()</code>
	 * 
	 * @param o
	 *            A C pointer to a struct vl4j_device
	 * @throws JNIException 
	 * @throws ReleaseException
	 *             if the device is still in use.
	 */
	private static native void doRelease(long o) throws JNIException;
	
	/**
	 * FrameGrabber for this device. Created on state transition to IDLE
	 */
	protected FrameGrabber frameGrabber;
	
	public VideoDevice(V4LComponentProvider provider, Path path) {
		this.provider = provider;
		this.path = path;
		this.state = ComponentState.LOADED;
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
		return this.state;
	}
	
	protected void doSetState(ComponentState state) {
		synchronized (this) {
			this.state = state;
		}
	}
	
	/**
	 * Attempt transition from UNLOADED to LOADED
	 */
	protected void attemptLoad() {
		
	}
	
	/**
	 * Attempt transition from LOADED to IDLE
	 * Preconditions: state is LOADED
	 * 
	 * The device file will be locked, and a pointer to stuff will be retrieved by JNI calls 
	 * @return 
	 * @throws FileNotFoundException 
	 */
	protected ComponentState attemptInit() throws FileNotFoundException {
		synchronized (this) {
			if (!Files.exists(this.path)) {
				//TODO Should transition to WAIT_FOR_RESOURCES here
				throw new FileNotFoundException("Could not find device file " + this.path);
			}
			String path = this.path.toString();
			this.pointer = doInit(path);
			//TODO support transition to WAIT_FOR_RESOURCES on failure
			this.doSetState(ComponentState.IDLE);
			return ComponentState.IDLE;
		}
	}
	
	/**
	 * Attempt transition from IDLE to PAUSED
	 */
	protected ComponentState attemtFGInit() {
		synchronized (this) {
			if (this.frameGrabber != null)//Shouldn't happen
				throw new IllegalStateException("FrameGrabber already initialized");
			this.frameGrabber = new FrameGrabber(this);
		}
		return ComponentState.PAUSED;
	}

	/**
	 * Attempt transition from PAUSED to EXECUTING
	 * For IDLE to EXECUTING, call #attemptFGInit() first.
	 */
	protected ComponentState attemptStart() {
		synchronized (this) {
			this.frameGrabber.start();
		}
		return ComponentState.EXECUTING;
	}
	
	/**
	 * Attempt transition from EXECUTING to IDLE
	 */
	protected ComponentState attemptStop() {
		synchronized (this) {
			this.frameGrabber.stop();
		}
		return ComponentState.IDLE;
	}

	@Override
	public ComponentState setState(ComponentState newState) throws FileNotFoundException {
		if (newState == null)
			throw new IllegalArgumentException("May not transition to state 'null'");
		synchronized (this) {
			final ComponentState oldState = getState();
			switch (oldState) {
				case UNLOADED:
					//TODO fill in
					break;
				case LOADED:
					if (newState == ComponentState.IDLE)
						return attemptInit();
					else if (newState == ComponentState.WAIT_FOR_RESOURCES)
						;
					break;
				case IDLE:
					switch (newState) {
						case PAUSED:
							return attemptFGInit();
						case EXECUTING:
							return attemptStart();
						case LOADED:
							return attemptDeinit();
					}
					break;
				case EXECUTING:
					switch (newState) {
						case IDLE:
							return attemptStop();
						case PAUSED:
							return attemptPause();
					}
					break;
				case PAUSED:
					switch (newState) {
						case IDLE:
							return attemptStop();
						case EXECUTING:
							return atemptResume();
					}
					break;
			}
		}
		throw new IllegalArgumentException("Invalid state transition " + oldState + " => " + newState);
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
