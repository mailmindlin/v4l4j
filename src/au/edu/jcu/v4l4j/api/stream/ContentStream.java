package au.edu.jcu.v4l4j.api.stream;

import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.Set;
import java.util.function.Consumer;

public interface ContentStream extends AutoCloseable {
	
	Object getProperty(String key);
	
	Object setProperty(String key, Object value);
	
	default Set<String> getPropertyNames() {
		return Collections.emptySet();
	}
	
	ContentStream onBytesAvailableToRead(int bytesRequested, Consumer<ContentStream> callback);
	
	ContentStream onBytesAvailalbeToWrite(int bytesRequested, Consumer<ContentStream> callback);
	
	ContentStream onEOS(Consumer<ContentStream> callback);
	
	ContentStream onDisconnect(Consumer<ContentStream> callback);
	
	ContentStream seekTo(Origin origin, long offset);
	
	long getCurrentPosition(Origin origin);
	
	ByteBuffer getReadBuffer(int length, boolean forbidCopy);
	
	void releaseReadBuffer(ByteBuffer buffer);
	
	ContentStream write(ByteBuffer buffer);
	
	ContentStreamProvider getProvider();
	
	AccessType getAccessType();
	
	@Override
	void close();
	
	public enum AccessType {
		/**
		 * Read only access
		 */
		READ,
		/**
		 * Write only access
		 */
		WRITE,
		/**
		 * Read and write access
		 */
		READ_WRITE;
	}
	
	public static enum Origin {
		/**
		 * Origin is the first byte of the stream
		 */
		START,
		/**
		 * Origin is the beginning of the available content
		 */
		FIRST,
		/**
		 * Origin is the current position in the content
		 */
		CURRENT,
		/**
		 * Origin is the end of the available content
		 */
		LAST,
		/**
		 * Origin is the end of the stream
		 */
		END;
	}
}
