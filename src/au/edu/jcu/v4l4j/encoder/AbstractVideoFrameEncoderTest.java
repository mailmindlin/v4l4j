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
		AbstractVideoFrameEncoder encoder = new AbstractVideoFrameEncoder(5, 7, null, null);
		
		int numConverters = -encoder.getConverterIds(encoder.object, null);
		Assert.assertEquals(numConverters, 1);
		
		int[] converterIds = new int[numConverters];
		numConverters = encoder.getConverterIds(encoder.object, converterIds);
		System.out.println(Arrays.toString(converterIds));
		Assert.assertEquals(numConverters, 1);
//		fail("Not yet implemented");
	}
	
	public static void main(String[] args) {
		AbstractVideoFrameEncoder encoder = new AbstractVideoFrameEncoder(5, 7, null, null);
		int numConverters = -encoder.getConverterIds(encoder.object, null);
		System.out.println("# Converters: " + numConverters);
		
		int[] converterIds = new int[numConverters];
		numConverters = encoder.getConverterIds(encoder.object, converterIds);
		System.out.println("# Converters: " + numConverters);
		System.out.println(Arrays.toString(converterIds));
	}
}
