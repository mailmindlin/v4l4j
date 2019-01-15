package au.edu.jcu.v4l4j.examples;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.YUVFrameGrabber;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class AsciiCam {
	
	static final String palette = " .'`^\",:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao#MW&8%B@$";
	
	public static void main(String...fred) throws V4L4JException {
		VideoDevice dev = new VideoDevice(System.getProperty("test.device", "/dev/video0"));
		int width = Integer.getInteger("test.width", 160);
		int height = Integer.getInteger("test.height", 120);
		int channel = Integer.getInteger("test.channel", 0);
		int std = Integer.getInteger("test.standard", V4L4JConstants.STANDARD_WEBCAM);
		
		//Try to get terminal size
		//thanks to stackoverflow.com/a/1286677/2759984
		final int termWidth, termHeight;
		{
			int tmp = 153;
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
			termHeight = tmp - 4;
		}
		
//		if (System.getProperty("v4l4j.num_driver_buffers") == null)
//			System.setProperty("v4l4j.num_driver_buffers", "1");

		YUVFrameGrabber fg = dev.getYUVFrameGrabber(width, height, channel, std);
		
		Runtime.getRuntime().addShutdownHook(new Thread(()->{
			fg.stopCapture();
			dev.releaseFrameGrabber();
			dev.release();
		}));
		
		final int xSkip = width / termWidth;
		final int ySkip = height / termHeight;
		final char[] chars = new char[256];
		for (int i = 0; i < 256; i++)
			chars[i] = map(i);
		
		System.out.println("W: " + termWidth + ", H: " + termHeight);
		
		fg.setCaptureCallback(frame-> {
			try {
				ByteBuffer bytes = frame.getBuffer();
				/*StringBuffer sb = new StringBuffer((termWidth + 3) * termHeight);
				for (int row = 0; row < termHeight; row++) {
					int rowOffset = row * ySkip * width;
					sb.append('|');
					for (int col = 0; col < termWidth; col++) {
						int index = rowOffset + col * xSkip;
						int value = bytes.get(index) & 0xFF;
						sb.append(chars[value]);
					}
					sb.append('|');
					sb.append('\n');
				}
				synchronized (System.out) {
					System.out.println("\033[2J");
					System.out.println(sb.toString());
					System.out.flush();
				}*/
				StringBuffer sb = new StringBuffer((termWidth + 8));
				sb.append("\033[");
				for (int row = 0; row < termHeight; row++) {
					int rowOffset = row * ySkip * width;
					sb.setLength(2);
					sb.append(row).append(";0H");
					for (int col = 0; col < termWidth; col++) {
						int index = rowOffset + col * xSkip;
						int value = bytes.get(index) & 0xFF;
						sb.append(chars[value]);
					}
					System.out.print(sb.toString());
				}
				frame.recycle();
			} catch (Exception e) {
				e.printStackTrace();
				throw e;
			}
		}, error -> {
			error.printStackTrace();
			fg.stopCapture();
			System.exit(1);
		});
		fg.startCapture();
	}
	/*static char map(int value) {
		return palette.charAt(value * palette.length() / 256);
	}
	/**/
	static char map(int value) {
		if (value >= 230)
			return '@';
		if (value >= 200)
			return '#';
		if (value >= 180)
			return '8';
		if (value >= 160)
			return '&';
		if (value >= 130)
			return 'o';
		if (value >= 100)
			return ':';
		if (value >= 70)
			return '*';
		if (value >= 50)
			return '.';
		return ' ';
	}/**/
	
	static class AsciiRenderer {
		char[] palette;
		int width;
		int height;
		int termWidth;
		int termHeight;
		int xSkip;
		int ySkip;
		void init() {
			short[] table = new short[256];
			for (int i = 0; i < 256; i++) {
				int y = palette[i] + 1;
				if (y > 255)
					y = 255;
				else if (y < 0)
					y = 0;
				table[i] = (short) y;
			}
			
		}
		void render(ByteBuffer yuvData) {
			for (int row = 0; row < height; row++) {
				int rowOffset = row * ySkip * width;
				for (int col = 0; col < 10; col++) {
					int pos = rowOffset + col * xSkip;
					
					int i1 = yuvData.get(pos) & 0xFF;
					int i2 = yuvData.get(pos + 1) & 0xFF;
					int i3 = yuvData.get(pos + xSkip) & 0xFF;
					int i4 = yuvData.get(pos + xSkip + 1) & 0xFF;
					
					int val = (i1 + i2 + i3 + i4) >> 2;
				}
			}
		}
	}
}
