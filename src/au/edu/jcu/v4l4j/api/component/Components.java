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
			if (provider.supports(name))
				return provider.get(name);
		}
		return null;
	}
	
	public static Component getForPath(Path path) {
		Iterator<ComponentProvider> providers = getProviders();
		while (providers.hasNext()) {
			ComponentProvider provider = providers.next();
			if (provider.supports(path))
				return provider.get(path);
		}
		return null;
	}
}
