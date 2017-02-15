package au.edu.jcu.v4l4j.impl.jni;

public class StructReadingContext {
	protected final StructReadingContext parent;
	protected final StructFieldType self;
	protected final Object value;

	protected StructReadingContext(StructReadingContext parent, StructFieldType self, Object value) {
		this.parent = parent;
		this.self = self;
		this.value = value;
	}
	
	public StructReadingContext child(StructFieldType self, Object value) {
		return new StructReadingContext(this, self, value);
	}
}