package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

public class NativeUnion implements AutoCloseable, NativeWrapper {
	public NativeUnion(UnionPrototype union, ByteBuffer buffer) {
		
	}
	
	@Override
	public long pointer() {
		return 0;
	}
	
	@Override
	public UnionPrototype type() {
		return null;
	}

	@Override
	public void close() throws Exception {
		// TODO Auto-generated method stub
		
	}

	@Override
	public ByteBuffer buffer() {
		// TODO Auto-generated method stub
		return null;
	}
}
