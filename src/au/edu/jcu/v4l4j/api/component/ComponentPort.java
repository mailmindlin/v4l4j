package au.edu.jcu.v4l4j.api.component;

import java.nio.ByteBuffer;
import java.util.Set;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.control.Control;

public interface ComponentPort {
	
	/**
	 * Get the index of this port.
	 */
	int getIndex();
	
	/**
	 * Get the stream type of data that passes through this port.
	 */
	StreamType getPortType();
	
	/**
	 * Get the component that this is a port for
	 */
	Component getComponent();
	
	/**
	 * Get if this port is an input port.
	 * @see #isOutput()
	 */
	boolean isInput();
	
	/**
	 * Get if this port is an output port.
	 * @see #isInput()
	 */
	boolean isOutput();
	
	/**
	 * Whether this port is enabled
	 * @return enabled
	 */
	boolean isEnabled();
	
	/**
	 * Enable/disable this port.
	 * Note that this method may block.
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
	
	/**
	 * Get the MIME type string for this port.
	 * May be null.
	 * @return MIME type string
	 */
	String getMIMEType();
	
	/**
	 * Register a pre-allocated ByteBuffer with this component.
	 * @param buffer
	 *     Allocated ByteBuffer to use as a FrameBuffer with this component
	 * @returns
	 *     FrameBuffer wrapping passed ByteBuffer
	 * @throws IllegalArgumentException
	 *     If the passed buffer cannot be used (e.g., this port requires
	 *     a direct ByteBuffer, and the one passed to this method wasn't).
	 * @throws IllegalStateException
	 *     If this component is in a state that does not allow a buffer to
	 *     be used.
	 * @throws BufferUnderflowException
	 *     If the buffer passed is too small for this port (smaller than the
	 *     value returned from {@link #bufferSize()}).
	 */
	FrameBuffer useBuffer(ByteBuffer buffer) throws IllegalArgumentException, IllegalStateException, BufferUnderflowException;
	
	/**
	 * Request that this port allocate a buffer of the given length.
	 * @param length
	 *     Length of buffer to allocate
	 */
	FrameBuffer allocateBuffer(int length) throws IllegalArgumentException, IllegalStateException;
	
	/**
	 * Queue a FrameBuffer to be emptied by this port.
	 * Buffers that are passed to this method are returned via the
	 * {@link #onBufferEmpty(Consumer<FrameBuffer>)} callback.
	 */
	void empty(FrameBuffer buffer);
	
	/**
	 * Queue a FrameBuffer to be filled by this port.
	 * Buffers that are passed to this method are returned via the 
	 * {@link #onBufferFilled(Consumer<FrameBuffer>)} callback.
	 */
	void fill(FrameBuffer buffer);
	
	/**
	 * Register a callback for the onBufferEmpty event.
	 * Only one handler may be registered at any time, so calling this method
	 * will deregister the previous handler, if any.
	 * @param handler
	 *     Handler for buffers that have been emptied. If null, the previous
	 *     handler will be deregistered, but no new handler will be registered.
	 */
	void onBufferEmpty(Consumer<FrameBuffer> handler);
	
	/**
	 * Registers a callback for the onBufferFilled event.
	 */
	void onBufferFill(Consumer<FrameBuffer> handler);
	
	Set<String> getControlNames();
	
	<U, V extends Control<U>> V getControlByName(String name);
}