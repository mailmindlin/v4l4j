package au.edu.jcu.v4l4j.examples;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.nio.ByteBuffer;

import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JLabel;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class capture frames from a video device and display it in a JFrame
 * @author gilles
 *
 */
public class WebcamViewer extends WindowAdapter implements Runnable {
	private JLabel l;
	private JFrame f;
	private long start = 0;
	private int n;
	private FrameGrabber fg;
	private Thread captureThread;
	private boolean stop;
	
	/**
	 * Builds a WebcamViewer object
	 * @param dev the video device file to capture from
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param std the capture standard
	 * @param channel the capture channel
	 * @param qty the JPEG compression quality
	 * @throws V4L4JException if any parameter if invalid
	 */
    public WebcamViewer(String dev, int w, int h, int std, int channel, int qty) throws V4L4JException{
        initFrameGrabber(dev, w, h, std, channel, qty);
        initGUI();
        stop = false;
        captureThread = new Thread(this, "Capture Thread");
        captureThread.start();
    }
    
    /** 
     * Creates the graphical interface components and initialises them
     */
    private void initGUI(){
        f = new JFrame();
        l = new JLabel();
        f.getContentPane().add(l);
        f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        f.addWindowListener(this);
        f.setVisible(true);
        f.setSize(fg.getWidth(), fg.getHeight());    	
    }
    
    /**
     * Initialises the FrameGrabber object with the given parameters
	 * @param dev the video device file to capture from
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param std the capture standard
	 * @param channel the capture channel
	 * @param qty the JPEG compression quality
	 * @throws V4L4JException if any parameter if invalid
     */
    private void initFrameGrabber(String dev, int w, int h, int std, int channel, int qty) throws V4L4JException{
		fg= new FrameGrabber(dev, w, h, channel, std, qty);
		fg.init();
		fg.startCapture();
		System.out.println("Starting capture at "+fg.getWidth()+"x"+fg.getHeight());	    	
    }
    
    /**
     * Updates the image shown in the JLabel
     * @param b
     */
    public void setImage(byte[] b) {
    	l.setIcon(new ImageIcon(b));
    	
    	// Computes the frame rate
    	if(start==0)
    		start = System.currentTimeMillis();
    	else if(System.currentTimeMillis()>start+10000) {
			System.out.println("FPS: "+ (((float) 1000*n/(System.currentTimeMillis()-start))  ));
			start = System.currentTimeMillis();
			n = 0;
		} else
			n++;
    }
    
    /**
     * Implements the capture thread: get a frame from the FrameGrabber, and display it
     */
    public void run(){
		ByteBuffer bb;
		byte[] b;
		try {			
			while(!stop){
				bb = fg.getFrame();
				b = new byte[bb.limit()];
				bb.get(b);
				setImage(b);
			}
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to capture image");
		}
    }
    
    /**
     * Catch window closing event so we can free up resources before exiting
     * @param e
     */
	public void windowClosing(WindowEvent e) {
		if(captureThread.isAlive()){
    		stop = true;
    		try {
				captureThread.join();
			} catch (InterruptedException e1) {}
    	}
		
		try {
			fg.stopCapture();
			fg.remove();
		} catch (V4L4JException e1) {
			e1.printStackTrace();
			System.out.println("Failed to stop capture");
		}
		
    	f.dispose();		
	}
	
	

	public static void main(String[] args) throws V4L4JException, IOException {

		String dev;
		int w, h, std, channel, qty;

		//Check if we have the required args
		//otherwise put sensible values in
		try {
			dev = args[0];
		} catch (Exception e){
			dev = "/dev/video0";
		}
		try {
			w = Integer.parseInt(args[1]);
		} catch (Exception e){
			w = FrameGrabber.MAX_WIDTH;
		}
		try{			
			h = Integer.parseInt(args[2]);
		} catch  (Exception e) {
			h = FrameGrabber.MAX_HEIGHT;
		}
		try {
			std = Integer.parseInt(args[3]);
		} catch (Exception e) {
			std = 0;
		}
		try {
			channel = Integer.parseInt(args[4]);
		} catch (Exception e){
			channel = 0;
		}
		try {
			qty = Integer.parseInt(args[5]);
		} catch (Exception e){
			qty = 80;
		}
		
		new WebcamViewer(dev,w,h,std,channel,qty);
	}
}
