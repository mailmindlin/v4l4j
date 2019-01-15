package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;

/**
 * Contains information about a {@link ComponentPort}. 
 * @author mailmindlin
 */
public interface PortDefinition {
	StreamType getType();
	/**
	 * Get if this port is an input port.
	 * @return if input
	 * @see #isOutput()
	 */
	boolean isInput();
	
	/**
	 * Get if this port is an output port.
	 * @return if output
	 * @see #isInput()
	 */
	boolean isOutput();
	
	/**
	 * Number of buffers required by this port before it is
	 * {@link #isPopulated() populated}. Should be at least as large as
	 * {@link #minimumBuffers()}.
	 * 
	 * @return Number of buffers required to populate.
	 */
	int actualBuffers();
	
	/**
	 * Minimum number of buffers required by this port
	 * @return minimum number of buffers
	 */
	int minimumBuffers();
	
	/**
	 * Minimum size of buffers used for this port
	 * @return Minimum size of buffer (in bytes).
	 */
	int bufferSize();
	
	/**
	 * Whether this port is enabled
	 * @return enabled
	 */
	boolean isEnabled();
	
	/**
	 * Whether this port is populated. A port is populated when at least
	 * {@link #actualBuffers()} buffers of at least {@link #bufferSize()} are
	 * allocated.
	 * 
	 * @return Whether this port is populated or not
	 */
	boolean isPopulated();
	
	/**
	 * Get the MIME type string for this port.
	 * May be null.
	 * @return MIME type string
	 */
	String getMIMEType();
	
	/**
	 * @return Iff buffers are required to be in contiguous memory.
	 */
	boolean buffersContiguous();
	
	/**
	 * The alignment required for each of the buffers on this port. For example,
	 * a value of 4 means that each buffer must be 4-byte aligned.
	 * 
	 * @return Alignment requirement. Returns {@code 0} if no alignment
	 *         requirement is present.
	 */
	long bufferAlignment();
}
