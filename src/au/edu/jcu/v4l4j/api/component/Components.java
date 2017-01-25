package au.edu.jcu.v4l4j.api.component;

import java.nio.file.Path;
import java.util.Iterator;
import java.util.ServiceLoader;

public class Components {
	private static Iterator<ComponentProvider> getProviders() {
		ClassLoader cl = ClassLoader.getSystemClassLoader();
		ServiceLoader<ComponentProvider> sl = ServiceLoader.load(ComponentProvider.class, cl);
		return sl.iterator();
	}
	
	public static Component getForName(String name) {
		Iterator<ComponentProvider> providers = getProviders();
		while (providers.hasNext()) {
			ComponentProvider provider = providers.next();
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
		Iterator<ComponentProvider> providers = getProviders();
		while (providers.hasNext()) {
			ComponentProvider provider = providers.next();
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
