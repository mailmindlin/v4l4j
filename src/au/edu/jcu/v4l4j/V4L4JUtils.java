package au.edu.jcu.v4l4j;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.HashMap;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class V4L4JUtils {
	private static final int BUFFER_SIZE = 4096;
	/**
	 * Whether the library has been previously initialized.
	 * Doesn't really matter if this is thread safe, as {@link #loadLibrary()}
	 * shouldn't be called too often.
	 */
	private static boolean initialized = false;
	
	private static boolean reallyInitialized = false;
	/**
	 * <strong>DO NOT USE.</strong> This method is not for you. It's only public
	 * because there are multiple packages that need it.
	 * <p>
	 * The purpose of this method is to consolidate all library-loading code to one
	 * place, supporting features like loading from the jar. Please trust that this
	 * will be called by v4l4j, and don't touch it ever.
	 * </p>
	 * @throws UnsatisfiedLinkError If there's a problem loading libv4l4j
	 * @throws SecurityException If called from outside v4l4j
	 */
	public static final void loadLibrary() throws UnsatisfiedLinkError {
		if (initialized)
			return;
		final Runtime runtime = Runtime.getRuntime();
		try {
			runtime.loadLibrary("v4l4j");
			initialized = true;
			reallyInitialized = true;
		} catch (UnsatisfiedLinkError e0) {
			// Basically, this just needs to be unstable *enough* that nobody will
			// consider calling this for any reason. However, we don't have to incur
			// the cost for most calls.

			StackTraceElement ste = getCaller();
			if (ste != null && !ste.getClassName().startsWith("au.edu.jcu.v4l4j"))
				throw new SecurityException("Library should only be loaded internally by v4l4j.");
			
			try {
				synchronized (V4L4JUtils.class) {
					//Try to load the library from the jar
					
					if (reallyInitialized)
						return;
					
					// Find a pretty good path to load from inside the jar
					String path = calculatePath();
					
					// Load the library to a temporary file
					File libFile = null;
					try (InputStream is = V4L4JUtils.class.getResourceAsStream(path)) {
						if (is == null)
							throw new RuntimeException("Unable to find JNI library @ " + path);
						// Create a temporary file to write the library to
						libFile = File.createTempFile("libv4l4j", ".so");
						// I've heard that this can fail to delete the file because the JVM locks the file
						// after it's loaded as a library, but it didn't seem to in my testing (Raspbian)
						libFile.deleteOnExit();
						
						try (OutputStream os = new BufferedOutputStream(new FileOutputStream(libFile))) {
							// Copy the file from inside the jar
							byte[] buffer = new byte[BUFFER_SIZE];
							int len;
							while ((len = is.read(buffer)) >= 0)
								os.write(buffer, 0, len);
						}
					} catch (IOException e) {
						// Delete the temporary file, as we won't be needing it anymore
						if (libFile != null)
							libFile.delete();
						throw e;
					}
					
					//Now load the library from the file that we just wrote
					runtime.load(libFile.getAbsolutePath());
					
					V4L4JUtils.initialized = true;
					V4L4JUtils.reallyInitialized = true;
					
					//TODO only enable in debug mode
					System.out.println("Successfully loaded libv4l4j.so from " + path + " => " + libFile);
				}
			} catch (Exception e1) {
				System.err.println("Unable to load v4l4j JNI library");
				e1.printStackTrace();
				throw e0;
			}
		}
	}
	
	private static final String calculatePath() {
		String path = lookupPath();
		if (path != null)
			return path;
		path = "/libv4l4j-" + computeProperty("os.name", "").toLowerCase() + "-" + computeProperty("os.arch", "").toLowerCase() + "-" + computeProperty("sun.arch.abi", "").toLowerCase() + ".so";
		System.err.println("Falling back to " + path);
		return path;
	}
	
	private static final String lookupPath() {
		try (InputStream is = V4L4JUtils.class.getResourceAsStream("com/edu/jcu/v4l4j/resources/libs.txt");
				BufferedReader br = is == null ? null : new BufferedReader(new InputStreamReader(is))) {
			if (br == null) {
				//libs.txt not found
				System.out.println("No libs.txt to handle library-loading with");
				return null;
			}
			
			HashMap<String, String> properties = new HashMap<>();
			down: while (br.ready()) {
				String line = br.readLine();
				int[] tmp = readNext(line, 0);
				String lib = line.substring(tmp[0], tmp[1]);
				do {
					tmp = readNext(line, tmp[2] + 1);
					String propVal = properties.computeIfAbsent(line.substring(tmp[0], tmp[1]), V4L4JUtils::computeProperty);
					tmp = readNext(line, tmp[2] + 1);
					if (!Pattern.matches(line.substring(tmp[0], tmp[1]), propVal))
						continue down;
				} while (tmp[2] + 1 < line.length());
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	private static final int[] readNext(String str, int idx) {
		char start = str.charAt(idx);
		if (start == '"' || start == '\'') {
			int i = idx;
			while (str.charAt(++i) != start);
			return new int[]{idx + 1, i - 1, i};
		}
		int result = str.indexOf(' ', idx);
		return new int[] {idx, result, result};
	}
	
	private static final String computeProperty(String name, String def) {
		try {
			return computeProperty(name);
		} catch (Exception e) {
			e.printStackTrace();
			return def;
		}
	}
	private static final String computeProperty(String name) {
		if (name.startsWith("$")) {
			String v = System.getenv(name.substring(1));
			return v == null ? "" : v;
		} else if (name.startsWith("[")) {
			Runtime rt = Runtime.getRuntime();
			try {
				Process p = rt.exec(name.substring(1, name.length() - 2));
				try {
					if (p.waitFor(10, TimeUnit.SECONDS)) {
						//Solution to the halting problem!
						p.destroyForcibly();
						throw new RuntimeException("Unable to complete running command (timeout): " + name);
					}
				} catch (InterruptedException e) {
					throw new RuntimeException("Interrupted while running command " + name);
				}
				StringBuffer sb = new StringBuffer();
				char[] tmp = new char[BUFFER_SIZE];
				int len;
				try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
					while ((len = br.read(tmp)) > 0)
						sb.append(tmp, 0, len);
				}
				return sb.toString();
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		} else {
			return System.getProperty(name, "");
		}
	}
	
	private static StackTraceElement getCaller() {
		StackTraceElement[] stElements = Thread.currentThread().getStackTrace();
		for (int i = 1; i < stElements.length; i++) {
			StackTraceElement ste = stElements[i];
			if (!ste.getClassName().equals(V4L4JUtils.class.getName()) && ste.getClassName().indexOf("java.lang.Thread") != 0)
				return ste;
		}
		return null;
	}
	
	public static int getPropertyAsInt(String propname, int def) {
		return Integer.parseInt(System.getProperty(propname, "" + def));
	}
}
