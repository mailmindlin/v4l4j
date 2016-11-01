package au.edu.jcu.v4l4j.encoder;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.Test;

/**
 * Test for {@link AbstractVideoFrameEncoder} JNI methods
 * @author mailmindlin
 */
public class AbstractVideoFrameEncoderTest {

	@Test
	public void testBuffer() throws Exception {
		try (V4lconvertBuffer buffer = new V4lconvertBuffer(256, 256, 256)) {
			assertEquals(256, buffer.getSourceBuffer().capacity());
			assertEquals(256, buffer.getOutputBuffer().capacity());
			assertEquals(256, buffer.getIntermediateBuffer().capacity());
		} catch (Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
	
	@Test
	public void test1() throws Exception {
		try {
//			AbstractVideoFrameEncoder encoder = new AbstractVideoFrameEncoder(5, 7, ImagePalette.RGB32, ImagePalette.BGR32);
			
//			int[] converters = encoder.getConverterIds();
//			System.out.println("RGB32 => BGR32 converters: " + Arrays.toString(converters));
			
//			encoder.close();
		} catch (Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
}
