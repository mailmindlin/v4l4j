package au.edu.jcu.v4l4j.encoder;

import static org.junit.Assert.*;

import java.util.Arrays;

import org.junit.Test;

import junit.framework.Assert;

/**
 * Test for {@link AbstractVideoFrameEncoder} JNI methods
 * @author mailmindlin
 */
public class AbstractVideoFrameEncoderTest {

	@Test
	public void test() {
		try {
			AbstractVideoFrameEncoder encoder = new AbstractVideoFrameEncoder(5, 7, null, null);
			
			int numConverters = -encoder.getConverterIds(encoder.object, null);
			Assert.assertEquals(numConverters, 1);
			
			int[] converterIds = new int[numConverters];
			numConverters = encoder.getConverterIds(encoder.object, converterIds);
			System.out.println(Arrays.toString(converterIds));
			Assert.assertEquals(numConverters, 1);
			
			encoder.release();
	//		fail("Not yet implemented");
		} catch (Exception e) {
			e.printStackTrace();
			throw e;
		}
	}
}
