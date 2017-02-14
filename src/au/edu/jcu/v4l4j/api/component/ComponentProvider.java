package au.edu.jcu.v4l4j.api.component;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Iterator;
import java.util.Set;

public interface ComponentProvider extends AutoCloseable {
	default Component get(String name) {
		return get(null, name);
	}
	
	Component get(Path path, String name);
	
	Iterator<Path> availablePaths() throws IOException;
	
	Set<String> availableNames(Path path, ComponentRole role);
}
