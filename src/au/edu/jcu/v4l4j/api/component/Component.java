package au.edu.jcu.v4l4j.api.component;

import java.util.Set;

public interface Component {
	String getName();
	ComponentProvider getProvider();
	
	ComponentPort getPort(int id);
	Set<ComponentPort> getPorts();
}
