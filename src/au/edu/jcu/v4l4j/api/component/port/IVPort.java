package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.component.ComponentPort;

/**
 * Superinterface for both VideoPort and ImagePort, because they share a lot
 * of methods.
 * @author mailmindlin
 */
public interface IVPort extends ComponentPort {
	@Override
	default IVPortDefinition getDefinition() {
		return getDefinition(false);
	}
	
	@Override
	IVPortDefinition getDefinition(boolean flush);
}
