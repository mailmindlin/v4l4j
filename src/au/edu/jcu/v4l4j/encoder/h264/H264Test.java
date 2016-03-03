package au.edu.jcu.v4l4j.encoder.h264;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class H264Test {

	@Test
	public void testPicture() {
		H264Picture picture = new H264Picture(600, 800, X264.CSP_RGB);
		assertEquals(picture.getCsp(), X264.CSP_RGB);
		assertEquals(picture.getWidth(), 600);
		assertEquals(picture.getHeight(), 800);

		picture.close();
	}

	@Test
	public void testParameters() {
		try (H264Parameters params = new H264Parameters()) {
			params.initWithPreset(X264.Preset.ULTRA_FAST, X264.Tune.ZERO_LATENCY);

			params.setCsp(X264.CSP_RGB);
			params.setInputDimension(600, 800);
	
			params.applyFastFirstPass();
	
			assertEquals(params.getWidth(), 600);
			assertEquals(params.getHeight(), 800);
	
			assertEquals(params.getCsp(), X264.CSP_RGB);
		}
	}
	
	@Test
	public void testEncoder() throws Exception {
		int width = 600;
		int height = 800;
		int csp = X264.CSP_I422;
		H264Encoder encoder = new H264Encoder(width, height, csp);
		H264Picture picIn = new H264Picture(width, height, csp);
		
		VideoDevice device = new VideoDevice("/dev/video0");
		ImageFormat yuyvFormat = device.getDeviceInfo().getFormatList().getNativeFormatOfType(ImagePalette.YUYV);
		System.out.println(yuyvFormat);
		
		encoder.close();
		picIn.close();
		device.releaseFrameGrabber();
		device.release(false);
	}
}
