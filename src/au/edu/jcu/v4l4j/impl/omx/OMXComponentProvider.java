package au.edu.jcu.v4l4j.impl.omx;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import au.edu.jcu.v4l4j.V4L4JUtils;
import au.edu.jcu.v4l4j.api.component.Component;
import au.edu.jcu.v4l4j.api.component.ComponentProvider;
import au.edu.jcu.v4l4j.api.component.ComponentRole;

public class OMXComponentProvider implements ComponentProvider {
	
	static {
		V4L4JUtils.loadLibrary();
	}
	
	/**
	 * Enumerate component names (wrapper for <code>OMX_ComponentNameEnum</code>).
	 * @param names
	 *     List of names that discovered ones will be added to.
	 * @param startIndex
	 *     Component index to start at
	 * @return index of last valid component. If no valid components are found, startIndex - 1 will be returned
	 */
	private static native int enumComponents(List<String> names, int startIndex);
	
	/**
	 * Return a set of component names that satisfy the given role (wrapper for <code>OMX_OMX_GetComponentsOfRole</code>).
	 * @param role
	 *     Role to query
	 * @param maxCount
	 *     Maximum number of roles to request (for memory safety; set to -1 for infinity)
	 * @return Set of component names for the given role
	 */
	private static native Set<String> getComponentsByRole(Set<String> result, String role, int maxCount);
	
	private static native boolean init();
	private static native void deinit();
	
	private static volatile int refCount = 0;
	
	public static OMXComponentProvider getInstance() {
		synchronized (OMXComponentProvider.class) {
			if (refCount++ == 0)
				if (!init())
					return null;
			return new OMXComponentProvider();
		}
	}
	
	private transient ArrayList<String> discoveredComponents = null;
	
	
	protected OMXComponentProvider() {
	}
	
	protected boolean isValidPath(Path path) {
		if (path == null)
			return true;
		try {
			return Files.isSameFile(path, Paths.get("/"));
		} catch (IOException e) {
			return false;
		}
	}

	@Override
	public Component get(Path path, String name) {
		//We only support the root path
		if (!isValidPath(path))
			throw new IllegalArgumentException("Invalid path: " + path);
		return new OMXComponent(this, name);
	}

	@Override
	public Iterator<Path> availablePaths() throws IOException {
		return Arrays.asList(Paths.get("/")).iterator();
	}

	@Override
	public Set<String> availableNames(Path path, ComponentRole role) {
		if (!isValidPath(path))
			return Collections.emptySet();
		if (discoveredComponents == null)
			discoveredComponents = new ArrayList<>();
		enumComponents(discoveredComponents, discoveredComponents.size());
		return new HashSet<>(discoveredComponents);
	}

	@Override
	public void close() throws IOException {
		synchronized (OMXComponentProvider.class) {
			//If this is the last open OMXCompoenentProvider, let's unload
			//libomxil
			if (--refCount == 0)
				deinit();
		}
	}

}
