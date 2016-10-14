package au.edu.jcu.v4l4j.encoder;

import java.util.Arrays;

import org.junit.Test;

import au.edu.jcu.v4l4j.ImagePalette;
import junit.framework.Assert;

/**
 * Test for {@link AbstractVideoFrameEncoder} JNI methods
 * @author mailmindlin
 */
public class AbstractVideoFrameEncoderTest {

	@Test
	public void test() throws Exception {
		try {
			AbstractVideoFrameEncoder encoder = new AbstractVideoFrameEncoder(5, 7, ImagePalette.RGB32, ImagePalette.YUV420);
			
			int[] converters = encoder.getConverterIds();
			System.out.println("RGB32 => YUV420 converters: " + Arrays.toString(converters));
			
			encoder.close();
		} catch (Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
	
	@Test
	public void test1() throws Exception {
		try {
			AbstractVideoFrameEncoder encoder = new AbstractVideoFrameEncoder(5, 7, ImagePalette.RGB32, ImagePalette.BGR32);
			
			int[] converters = encoder.getConverterIds();
			System.out.println("RGB32 => BGR32 converters: " + Arrays.toString(converters));
			
			encoder.close();
		} catch (Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
}
