package au.edu.jcu.v4l4j.impl.v4l;

import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import au.edu.jcu.v4l4j.V4L4JUtils;
import au.edu.jcu.v4l4j.api.component.Component;
import au.edu.jcu.v4l4j.api.component.ComponentProvider;
import au.edu.jcu.v4l4j.api.component.ComponentRole;

public class V4LComponentProvider implements ComponentProvider {
	
	static {
		V4L4JUtils.loadLibrary();
	}
	
	public static final String PREFIX = "v4l.v4l4j.";
	private static final Set<String> names = Collections.unmodifiableSet(new HashSet<>(Arrays.asList(PREFIX + "camera")));

	@Override
	public Component get(Path path, String name) {
		if (path == null || !Files.exists(path))
			throw new IllegalArgumentException("Invalid path: " + path);
		if (!name.equals(PREFIX + "camera"))
			throw new IllegalArgumentException("Invalid name: " + name);
		return new VideoDevice(this, path);
	}

	@Override
	public Iterator<Path> availablePaths() throws IOException {
		Set<Path> paths = new HashSet<>();
		try (DirectoryStream<Path> stream = Files.newDirectoryStream(Paths.get("/dev").toAbsolutePath(), "/dev/video*")) {
			for (Path path : stream)
				paths.add(path);
		}
		return paths.iterator();
	}

	@Override
	public Set<String> availableNames(Path path, ComponentRole role) {
		if (path == null || !Files.exists(path))
			return Collections.emptySet();
		return names;
	}

}
