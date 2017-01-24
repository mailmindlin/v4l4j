package au.edu.jcu.v4l4j.api.stream;

public interface ContentStreamProvider extends AutoCloseable {
	ContentStream create(String uri);
	ContentStream open(String uri, ContentStreamAccessType access);
}
