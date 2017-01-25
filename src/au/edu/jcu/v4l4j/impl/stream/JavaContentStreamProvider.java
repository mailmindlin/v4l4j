package au.edu.jcu.v4l4j.impl.stream;

import java.util.HashMap;

import au.edu.jcu.v4l4j.api.stream.ContentStream;
import au.edu.jcu.v4l4j.api.stream.ContentStream.AccessType;
import au.edu.jcu.v4l4j.api.stream.ContentStreamProvider;

public class JavaContentStreamProvider implements ContentStreamProvider {

	private HashMap<String, JavaContentStream> streams = new HashMap<>();
	
	@Override
	public ContentStream open(String uri, AccessType accessType) {
		JavaContentStream stream = streams.get(uri);
		if (stream == null)
			throw new IllegalArgumentException("No content stream exists at the uri '" + uri + "'");
		return null;
	}

	@Override
	public ContentStream create(String uri) {
		if (streams.containsKey(uri))
			throw new IllegalArgumentException("A content stream already exists at the uri '" + uri + "'");
		JavaContentStream stream = new JavaContentStream(this, uri);
		streams.put(uri, stream);
		return stream;
	}

	@Override
	public void close() {
		streams.values().forEach(JavaContentStream::close);
		streams.clear();
	}
}
