package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.encoder.h264.H264Encoder;
import au.edu.jcu.v4l4j.encoder.h264.H264Parameters;
import au.edu.jcu.v4l4j.encoder.h264.H264Picture;
import au.edu.jcu.v4l4j.encoder.h264.X264;
import au.edu.jcu.v4l4j.encoder.h264.X264.Preset;
import au.edu.jcu.v4l4j.encoder.h264.X264.Profile;
import au.edu.jcu.v4l4j.encoder.h264.X264.Tune;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class H264EncodeTest {
	public static void main(String...fred) throws V4L4JException {
		//Get webcam
		VideoDevice device = new VideoDevice(VideoDevice.available().iterator().next());
		
		//Setup encoder
		int fps = 25;
		int width = 640;
		int height = 480;
		H264Parameters params = new H264Parameters();
		params.initWithPreset(Preset.ULTRA_FAST, Tune.ZERO_LATENCY);
		params.setInputDimension(width, height);
		params.setThreads(1);
		params.setFPS(fps, 1);
		//Intra refresh
		params.setKeyintMax(fps);
		params.setIntraRefresh(true);
		//Rate control
		//TODO support rc method
		params.setRfConstant(25, 35);
		//Streaming
		params.setRepeatHeaders(true);
		params.setAnnexb(true);
		params.applyProfile(Profile.BASELINE);
		
		H264Encoder encoder = new H264Encoder(params);
		H264Picture pictureIn = new H264Picture(width, height, X264.CSP_I420);
		H264Picture pictureOut = new H264Picture();
		
	}
}
