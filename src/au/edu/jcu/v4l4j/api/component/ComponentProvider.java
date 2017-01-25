package au.edu.jcu.v4l4j.api.component;

import java.nio.file.Path;
import java.util.Enumeration;

public interface ComponentProvider {
	Component get(String name);
	Component get(Path path);
	boolean supports(String name);
	boolean supports(Path path);
	Enumeration<String> enumerateNames();
	Enumeration<Path> enumeratePaths();
}
