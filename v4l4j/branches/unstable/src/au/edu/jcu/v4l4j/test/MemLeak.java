package au.edu.jcu.v4l4j.test;

import java.io.IOException;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class MemLeak implements Runnable{
	private boolean stop;
	private VideoDevice vd;
	
	public MemLeak() throws V4L4JException{
		stop = false;
	}
	
	public void stop(){
		stop = true;
	}
	
	public void run(){
		FrameGrabber fg;
		int count = 0;
		
		while(!stop){
			count = 0;
			try {
				vd = new VideoDevice("/dev/video0");
				vd.getControlList();
				fg = vd.getJPEGFrameGrabber(640, 480, 1, 1, 80);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			try {
				vd = new VideoDevice("/dev/video0");
				vd.getControlList();
				fg = vd.getRGBFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			try {
				vd = new VideoDevice("/dev/video0");
				vd.getControlList();
				fg = vd.getBGRFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			try {
				vd = new VideoDevice("/dev/video0");
				vd.getControlList();
				fg = vd.getYUVFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			try {
				vd = new VideoDevice("/dev/video0");
				vd.getControlList();
				fg = vd.getYVUFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			vd.releaseControlList();
			vd.release();
		}
		
	}
	
	public void release(){
	}

	public static void main(String[] args) throws V4L4JException, IOException, InterruptedException{
		MemLeak m = new MemLeak();
		Thread t = new Thread(m,"CaptureThread");
		t.start();
		System.in.read();
		m.stop();
		t.join();
		m.release();
	}
	
}
