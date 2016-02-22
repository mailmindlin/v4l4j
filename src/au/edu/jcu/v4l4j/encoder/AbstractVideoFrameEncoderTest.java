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
			
			int numConverters = -encoder.getConverterIds(encoder.object, null);
			Assert.assertEquals(numConverters, 1);
			
			int[] converterIds = new int[numConverters];
			numConverters = encoder.getConverterIds(encoder.object, converterIds);
			System.out.println(Arrays.toString(converterIds));
			Assert.assertEquals(numConverters, 1);
			
			encoder.close();
	//		fail("Not yet implemented");
		} catch (Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
}
