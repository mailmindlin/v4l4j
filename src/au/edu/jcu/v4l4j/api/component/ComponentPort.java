package au.edu.jcu.v4l4j.api.component;

import java.nio.ByteBuffer;
import java.util.Set;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.control.Control;

public interface ComponentPort {
	int getIndex();
	
	StreamType getPortType();
	
	Component getComponent();
	
	boolean isInput();
	
	boolean isOutput();
	/**
	 * Whether this port is enabled
	 * @return
	 */
	boolean isEnabled();
	/**
	 * Enable/disable this port
	 * @param aflag
	 * @return
	 */
	boolean setEnabled(boolean aflag);
	/**
	 * Whether this port is populated.
	 * A port is populated when at least {@link #actualBuffers()} buffers of at least {@link #bufferSize()} are allocated.  
	 * @return
	 */
	boolean isPopulated();
	/**
	 * Minimum number of buffers required by this port
	 * @return
	 */
	int minimumBuffers();
	/**
	 * Number of buffers required by this port before it is populated
	 * @return
	 */
	int actualBuffers();
	/**
	 * Minumum size of buffers used for this port
	 * @return
	 */
	int bufferSize();
	
	String getMIMEType();
	
	FrameBuffer useBuffer(ByteBuffer buffer);
	
	FrameBuffer allocateBuffer(int length);
	
	void empty(FrameBuffer buffer);
	
	void fill(FrameBuffer buffer);
	
	void onBufferEmpty(Consumer<FrameBuffer> handler);
	
	void onBufferFill(Consumer<FrameBuffer> handler);
	
	Set<String> getControlNames();
	
	<U, V extends Control<U>> V getControlByName(String name);
}