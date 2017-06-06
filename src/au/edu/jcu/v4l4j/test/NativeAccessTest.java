package au.edu.jcu.v4l4j.test;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.nio.ByteBuffer;

import org.junit.Test;

import au.edu.jcu.v4l4j.impl.jni.PrimitiveStructFieldType;

public class NativeAccessTest {
	private static ByteBuffer buffer(int size) {
		//TODO pool
		return ByteBuffer.allocate(size);
	}
	
	@Test
	public void testWriteInt8() {
		ByteBuffer buffer = buffer(PrimitiveStructFieldType.INT8.getSize());
		
		buffer.clear();
		PrimitiveStructFieldType.INT8.write(buffer, 0x00);
		buffer.flip();
		assertTrue(0x00 == ((Number) PrimitiveStructFieldType.INT8.read(buffer, null)).byteValue());
		
		buffer.clear();
		PrimitiveStructFieldType.INT8.write(buffer, 0xFF);
		buffer.flip();
		assertTrue(((byte) 0xFF) == ((Number) PrimitiveStructFieldType.INT8.read(buffer, null)).byteValue());
		
		try {
			buffer.clear();
			PrimitiveStructFieldType.INT8.write(buffer, 0xFF + 1);
			fail("Overflow not caught");
		} catch (Exception e) {
			//Expected
		}
	}
	
	@Test
	public void testWriteInt16() {
		ByteBuffer buffer = buffer(PrimitiveStructFieldType.INT16.getSize());
		
		buffer.clear();
		PrimitiveStructFieldType.INT16.write(buffer, 0x00);
		buffer.flip();
		assertTrue(0x00 == ((Number) PrimitiveStructFieldType.INT16.read(buffer, null)).shortValue());
		
		buffer.clear();
		PrimitiveStructFieldType.INT16.write(buffer, 0xFFFF);
		buffer.flip();
		assertTrue(((short) 0xFF) == ((Number) PrimitiveStructFieldType.INT16.read(buffer, null)).shortValue());
		
		try {
			buffer.clear();
			PrimitiveStructFieldType.INT16.write(buffer, 0xFFFF + 1);
			fail("Overflow not caught");
		} catch (Exception e) {
			//Expected
		}
	}
	
	@Test
	public void testWriteInt32() {
		ByteBuffer buffer = buffer(PrimitiveStructFieldType.INT32.getSize());
		
		buffer.clear();
		PrimitiveStructFieldType.INT32.write(buffer, 0);
		buffer.flip();
		assertTrue(0x00 == ((Number) PrimitiveStructFieldType.INT32.read(buffer, null)).intValue());
		
		buffer.clear();
		PrimitiveStructFieldType.INT32.write(buffer, 0xFFFFFFFF);
		buffer.flip();
		assertTrue(((int) 0xFFFFFFFF) == ((Number) PrimitiveStructFieldType.INT32.read(buffer, null)).intValue());
		
		try {
			buffer.clear();
			PrimitiveStructFieldType.INT32.write(buffer, 0xFFFFFFFFL + 1);
			fail("Overflow not caught");
		} catch (Exception e) {
			//Expected
		}
	}
}
