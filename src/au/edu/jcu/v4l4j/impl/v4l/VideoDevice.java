package au.edu.jcu.v4l4j.impl.v4l;

import java.io.FileNotFoundException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
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
	protected final String name;
	protected final Path path;
	protected final VideoDeviceCapturePort previewPort = new VideoDeviceCapturePort(this, 1);
	protected final VideoDeviceCapturePort capturePort = new VideoDeviceCapturePort(this, 2);
	protected final Set<VideoPort> videoPorts = new HashSet<>(Arrays.asList(previewPort, capturePort));
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
	
	public VideoDevice(V4LComponentProvider provider, Path path, String name) {
		this.provider = provider;
		this.path = path;
		this.state = ComponentState.LOADED;
		this.name = name;
	}
	
	@Override
	public String getName() {
		return this.name;
	}

	@Override
	public ComponentProvider getProvider() {
		return provider;
	}

	@Override
	public ComponentState getState() {
		return this.state;
	}
	
	protected ComponentState doSetState(ComponentState state) {
		synchronized (this) {
			return this.state = state;
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
			return this.doSetState(ComponentState.IDLE);
		}
	}
	
	/**
	 * Attempt transition from IDLE to PAUSED
	 */
	protected ComponentState attemptFGInit() {
		synchronized (this) {
			if (this.frameGrabber != null)//Shouldn't happen
				throw new IllegalStateException("FrameGrabber already initialized");
			this.frameGrabber = new FrameGrabber(this);
			return this.doSetState(ComponentState.PAUSED);
		}
	}

	/**
	 * Attempt transition from PAUSED to EXECUTING
	 * For IDLE to EXECUTING, call #attemptFGInit() first.
	 */
	protected ComponentState attemptStart() {
		synchronized (this) {
			this.frameGrabber.start();
			return this.doSetState(ComponentState.EXECUTING);
		}
	}
	
	/**
	 * Attempt transition from EXECUTING to PAUSED
	 */
	protected ComponentState attemptPause() {
		synchronized (this) {
			this.frameGrabber.stop();
			return this.doSetState(ComponentState.PAUSED);
		}
	}
	
	protected ComponentState attemptFGDeinit() {
		synchronized (this) {
			this.frameGrabber.release();
			this.frameGrabber = null;
			return this.doSetState(ComponentState.IDLE);
		}
	}
	
	protected ComponentState attemptDeinit() {
		synchronized (this) {
			doRelease(this.pointer);
			return this.doSetState(ComponentState.LOADED);
		}
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
							if (attemptPause() != ComponentState.PAUSED)
								return getState();
							return attemptFGDeinit();
						case PAUSED:
							return attemptPause();
					}
					break;
				case PAUSED:
					switch (newState) {
						case IDLE:
							return attemptFGDeinit();
						case EXECUTING:
							return attemptStart();
					}
					break;
			}
			throw new IllegalArgumentException("Invalid state transition " + oldState + " => " + newState);
		}
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
		switch (index) {
			case 1:
				return this.previewPort;
			case 2:
				return this.capturePort;
		}
		return null;
	}

	@Override
	public Set<ComponentRole> getRoles() {
		// TODO Auto-generated method stub
		return null;
	}

}
