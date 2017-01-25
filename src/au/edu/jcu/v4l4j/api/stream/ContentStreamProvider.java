package au.edu.jcu.v4l4j.api.stream;

public interface ContentStreamProvider extends AutoCloseable {
	/**
	 * Create a stream at the given URI
	 * @param uri 
	 * @return created content stream, with READ_WRITE access
	 */
	ContentStream create(String uri);
	
	ContentStream open(String uri, ContentStream.AccessType access);
}
