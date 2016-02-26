package au.edu.jcu.v4l4j.encoder.h264;

import static org.junit.Assert.*;

import org.junit.Test;

import junit.framework.Assert;

public class H264Test {

	@Test
	public void testPicture() {
		H264Picture picture = new H264Picture(X264.X264_CSP_RGB, 600, 800);
		assertEquals(picture.getCsp(), X264.X264_CSP_RGB);
		assertEquals(picture.getWidth(), 600);
		assertEquals(picture.getHeight(), 800);
		
		picture.close();
	}

	@Test
	public void testParameters() {
		try (H264Parameters params = new H264Parameters()) {
			params.initDefault();
//			params.initWithPreset(H264Parameters.PRESET_ULTRA_FAST, H264Parameters.TUNE_ZERO_LATENCY);
			
			params.setCsp(X264.X264_CSP_RGB);
			params.setInputDimension(600, 800);
			
			params.applyFastFirstPass();
			
			assertEquals(params.getWidth(), 600);
			assertEquals(params.getHeight(), 800);
			
			assertEquals(params.getCsp(), X264.X264_CSP_RGB);
		}
	}
}
