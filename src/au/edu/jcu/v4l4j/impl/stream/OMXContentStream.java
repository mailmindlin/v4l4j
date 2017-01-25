package au.edu.jcu.v4l4j.impl.stream;

import java.nio.ByteBuffer;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.stream.ContentStream;
import au.edu.jcu.v4l4j.api.stream.ContentStreamProvider;

public class OMXContentStream implements ContentStream {
	private final OMXContentStreamProvider provider;
	private final long pointer;
	
	protected OMXContentStream(OMXContentStreamProvider provider, long pointer) {
		this.provider = provider;
		this.pointer = pointer;
	}
	
	@Override
	public Object getProperty(String key) {
		return this.provider.getStreamConfig(this.pointer, key);
	}

	@Override
	public OMXContentStream setProperty(String key, Object value) {
		this.provider.setStreamConfig(this.pointer, key, value);
		return this;
	}

	@Override
	public OMXContentStream seekTo(Origin origin, long position) {
		// TODO Auto-generated method stub
		return null;
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
