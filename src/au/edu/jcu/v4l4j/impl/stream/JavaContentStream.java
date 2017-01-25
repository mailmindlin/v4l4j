package au.edu.jcu.v4l4j.impl.stream;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.stream.ContentStream;
import au.edu.jcu.v4l4j.api.stream.ContentStreamProvider;

public class JavaContentStream implements ContentStream {
	protected final JavaContentStreamProvider provider;
	protected HashMap<String, Object> properties;
	protected final String uri;
	protected ByteBuffer buffer;
	protected int bufferLength;
	protected int bufferOffset;
	
	protected JavaContentStream(JavaContentStreamProvider provider, String uri) {
		if (provider == null || uri == null)
			throw new NullPointerException("Neither provider nor uri may be null");
		this.provider = provider;
		this.uri = uri;
	}
	
	protected JavaContentStream(JavaContentStreamProvider provider, String uri, ByteBuffer buffer) {
		this.provider = provider;
		this.uri = uri;
		this.buffer = buffer;
	}
	
	@Override
	public ContentStream seekTo(ContentStream.Origin origin, long position) {
		// TODO Auto-generated method stub
		return this;
	}

	@Override
	public long getCurrentPosition(Origin origin) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public ByteBuffer getReadBuffer(int length, boolean forbidCopy) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void releaseReadBuffer(ByteBuffer buffer) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public ContentStream write(ByteBuffer buffer) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ContentStreamProvider getProvider() {
		return this.provider;
	}

	@Override
	public void close() {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public Object getProperty(String key) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ContentStream setProperty(String key, Object value) {
		// TODO Auto-generated method stub
		return null;
	}

	public static class JavaContentStreamRef extends JavaContentStream {
		protected final JavaContentStream parent;
		protected JavaContentStreamRef(JavaContentStream parent, AccessType access) {
			super(parent.provider, parent.uri, parent.buffer);
			this.parent = parent;
		}
	}

	@Override
	public ContentStream onBytesAvailableToRead(int bytesRequested, Consumer<ContentStream> callback) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ContentStream onBytesAvailalbeToWrite(int bytesRequested, Consumer<ContentStream> callback) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ContentStream onEOS(Consumer<ContentStream> callback) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ContentStream onDisconnect(Consumer<ContentStream> callback) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public AccessType getAccessType() {
		// TODO Auto-generated method stub
		return null;
	}
}
