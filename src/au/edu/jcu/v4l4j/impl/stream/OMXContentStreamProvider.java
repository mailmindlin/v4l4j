package au.edu.jcu.v4l4j.impl.stream;

import java.io.IOException;

import au.edu.jcu.v4l4j.api.stream.ContentStream;
import au.edu.jcu.v4l4j.api.stream.ContentStream.AccessType;
import au.edu.jcu.v4l4j.api.stream.ContentStreamProvider;

/**
 * ContentStreamProvider that wraps the OMX ContentPipe API.
 * @author mailmindlin
 */
public final class OMXContentStreamProvider implements ContentStreamProvider {
	private static final Object lock = new Object();
	private static volatile OMXContentStreamProvider instance;
	
	public static OMXContentStreamProvider getInstance() {
		synchronized (lock) {
			OMXContentStreamProvider inst = OMXContentStreamProvider.instance;
			if (inst == null) {
				//Instantiate
			}
			return inst;
		}
	}
	
	private static native long doGetInstance();
	private static native Object doGetConfig(long cpInst, long chInst, String key);
	private static native void doSetConfig(long cpInst, long chInst, String key, Object value);
	private static native long doCreateStream(long cpInst, String uri);
	private static native long doOpenStream(long cpInst, String uri, int accessType);
	private static native void doReleaseStream(long cpInst, long chInst);
	
	private int refCount = 0;
	private final long pointer;
	
	private OMXContentStreamProvider(long pointer) {
		this.pointer = pointer;
	}
	
	@Override
	public ContentStream open(String uri, AccessType accessType) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ContentStream create(String uri) {
		// TODO Auto-generated method stub
		return null;
	}
	
	void setStreamConfig(long chInst, String key, Object value) {
		OMXContentStreamProvider.doSetConfig(this.pointer, chInst, key, value);
	}
	
	Object getStreamConfig(long chInst, String key) {
		return OMXContentStreamProvider.doGetConfig(this.pointer, chInst, key);
	}

	@Override
	public void close() throws IOException {
		synchronized (lock) {
			if (--refCount <= 0) {
				//Release
			}
		}
	}
	
}
