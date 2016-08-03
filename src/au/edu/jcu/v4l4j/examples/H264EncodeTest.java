package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.encoder.h264.H264Parameters;
import au.edu.jcu.v4l4j.encoder.h264.X264.Preset;
import au.edu.jcu.v4l4j.encoder.h264.X264.Tune;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class H264EncodeTest {
	public static void main(String...fred) throws V4L4JException {
		//Get webcam
		VideoDevice device = new VideoDevice(VideoDevice.available().iterator().next());
		
		//Setup encoder
		H264Parameters params = new H264Parameters();
		params.initWithPreset(Preset.ULTRA_FAST, Tune.ZERO_LATENCY);
		params.setInputDimension(640, 480);
	}
}
