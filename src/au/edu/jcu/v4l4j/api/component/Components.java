package au.edu.jcu.v4l4j.api.component;

import java.nio.file.Path;
import java.util.Iterator;
import java.util.ServiceLoader;

public class Components {
	/**
	 * @return Registered {@link ComponentProvider}'s
	 */
	private static Iterable<ComponentProvider> providers() {
		ClassLoader cl = ClassLoader.getSystemClassLoader();
		ServiceLoader<ComponentProvider> sl = ServiceLoader.load(ComponentProvider.class, cl);
		return sl;
	}
	
	public static Component getForName(String name) {
		for (ComponentProvider provider : providers()) {
			try {
				Component c = provider.get(name);
				if (c != null)
					return c;
			} catch (Exception e) {
				//Swallow
			}
		}
		return null;
	}
	
	public static Component getForPath(Path path, String name) {
		for (ComponentProvider provider : providers()) {
			try {
				Component c = provider.get(path, name);
				if (c != null)
					return c;
			} catch (Exception e) {
				//Swallow
			}
		}
		return null;
	}
}
