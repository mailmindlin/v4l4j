package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

public class UnionPrototype implements StructFieldType {
	protected Set<StructField> options;
	protected transient int alignment = -1;
	protected transient int size = -1;
	
	public UnionPrototype(Set<StructField> options) {
		this.options = new HashSet<>(options);
	}
	
	@Override
	public int getAlignment() {
		if (this.alignment < 0) {
			int alignment = 0;
			for (StructField option : options)
				alignment = Math.max(alignment, option.getAlignment());
			this.alignment = alignment;
		}
		return this.alignment;
	}

	@Override
	public int getSize() {
		if (this.size < 0) {
			int size = 0;
			for (StructField option : options)
				size = Math.max(size, option.getSize());
			this.size = size;
		}
		return this.size;
	}
	
	public Set<StructField> getOptions() {
		return Collections.unmodifiableSet(this.options);
	}

	@Override
	public boolean expands() {
		return false;
	}

	@Override
	public void write(ByteBuffer buffer, Object params) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		// TODO Auto-generated method stub
		return null;
	}
	
}
