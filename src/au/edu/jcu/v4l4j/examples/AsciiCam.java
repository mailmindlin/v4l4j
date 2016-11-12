package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.YUVFrameGrabber;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class AsciiCam {
	
	public static void main(String...fred) throws V4L4JException {
		VideoDevice dev = new VideoDevice((System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0");
		int width = (System.getProperty("test.width") != null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		int height = (System.getProperty("test.height") != null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		int channel = (System.getProperty("test.channel") != null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
		int std = (System.getProperty("test.standard") != null) ? Integer.parseInt(System.getProperty("test.standard")) : V4L4JConstants.STANDARD_WEBCAM;
		
		//Try to get terminal size
		//thanks to stackoverflow.com/a/1286677/2759984
		final int termWidth, termHeight;
		{
			int tmp = 80;
			try {
				tmp = Integer.parseInt(System.getenv("COLUMNS"));
			} catch (NumberFormatException | NullPointerException e) {
				//Swallow
			}
			termWidth = tmp;
			tmp = 43;
			try {
				tmp = Integer.parseInt(System.getenv("ROWS"));
			} catch (NumberFormatException | NullPointerException e) {
				//Swallow
			}
			termHeight = tmp;
		}
		
		YUVFrameGrabber fg = dev.getYUVFrameGrabber(width, height, channel, std);
		
		Runtime.getRuntime().addShutdownHook(new Thread(()->{
			fg.stopCapture();
			dev.releaseFrameGrabber();
			dev.release();
		}));
		
		int xSkip = width / termWidth;
		int ySkip = height / termHeight;
		final char[] chars = " .,:;i1tfLCG08@".toCharArray();
		
		fg.setCaptureCallback(frame-> {
			byte[] bytes = frame.getBytes();
			StringBuffer sb = new StringBuffer((termWidth - 1) * termHeight);
			for (int row = 0; row < termHeight; row++) {
				for (int col = 0; col < termWidth; col++) {
					int value = bytes[row * width + col];
					sb.append(chars[value / 16]);
				}
				sb.append('\n');
			}
			System.out.println(sb);
			System.out.flush();
			frame.recycle();
		}, error -> {
			fg.stopCapture();
			error.printStackTrace();
			System.exit(1);
		});
	}
}
