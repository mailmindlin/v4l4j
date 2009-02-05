package au.edu.jcu.v4l4j.examples;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Hashtable;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.V4l4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.ControlException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class capture frames from a video device and display it in a JFrame
 * @author gilles
 *
 */
public class WebcamViewer extends WindowAdapter implements Runnable {
	private JLabel video, fps;
	private JFrame f;
	private JPanel controlPanel;
	private JScrollPane controlScrollPane;
	private JPanel videoPanel;
	private long start = 0;
	private int n;
	private FrameGrabber fg;
	private Hashtable<String,Control> controls; 
	private Thread captureThread;
	private boolean stop;
	private VideoDevice vd;
	
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
        //captureThread.start();
    }
    
    /** 
     * Creates the graphical interface components and initialises them
     */
    private void initGUI(){
        f = new JFrame();
        f.setLayout(new BoxLayout(f.getContentPane(),BoxLayout.LINE_AXIS));
        
        videoPanel = new JPanel();
        videoPanel.setLayout(new BoxLayout(videoPanel, BoxLayout.PAGE_AXIS));
        
        video = new JLabel();
        video.setPreferredSize(new Dimension(fg.getWidth(), fg.getHeight()));
        video.setAlignmentX(Component.CENTER_ALIGNMENT);
        videoPanel.add(video);
        
        fps = new JLabel("FPS: 0.0");
        fps.setAlignmentX(Component.CENTER_ALIGNMENT);
        videoPanel.add(fps);
        
        controlPanel = new JPanel();
        controlScrollPane = new JScrollPane(controlPanel);
        controlScrollPane.getVerticalScrollBar().setBlockIncrement(40);
        controlScrollPane.getVerticalScrollBar().setUnitIncrement(25);
        controlScrollPane.setPreferredSize(new Dimension(300, fg.getHeight()));
        controlPanel.setLayout(new BoxLayout(controlPanel, BoxLayout.PAGE_AXIS));
        
        f.getContentPane().add(videoPanel);
        f.getContentPane().add(controlScrollPane);
        
        initControlPane();

        f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        f.addWindowListener(this);
        f.pack();
        f.setVisible(true);
    }
    
    private void initControlPane(){
    	controls = vd.getControlList();
    	for(Control c: controls.values())
    		controlPanel.add(new ControlPanel(c).getPanel());
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
    	vd = new VideoDevice(dev);
		fg= vd.getJPEGFrameGrabber(w, h, channel, std, qty);
		fg.startCapture();
		System.out.println("Starting capture at "+fg.getWidth()+"x"+fg.getHeight());	    	
    }
    
    /**
     * Updates the image shown in the JLabel
     * @param b
     */
    public void setImage(byte[] b) {
    	video.setIcon(new ImageIcon(b));
    	
    	// Computes the frame rate
    	if(start==0)
    		start = System.currentTimeMillis();
    	else if(System.currentTimeMillis()>start+10000) {
			//System.out.println("FPS: "+ (((float) 1000*n/(System.currentTimeMillis()-start))  ));
    		fps.setText("FPS: "+(float) 1000*n/(System.currentTimeMillis()-start));
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
			vd.releaseFrameGrabber();
			vd.releaseControlList();
			vd.release();
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
			w = V4l4JConstants.MAX_WIDTH;
		}
		try{			
			h = Integer.parseInt(args[2]);
		} catch  (Exception e) {
			h = V4l4JConstants.MAX_HEIGHT;
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
	
	public static class ControlPanel extends MouseAdapter{
		private JPanel contentPanel, buttonPanel, labelPanel;
		private JButton up, down;
		private JLabel value, name;
		private Control ctrl;
		
		public ControlPanel(Control c){
			ctrl = c;
			initControlGUI();
			updateValue();
		}
		
		private void initControlGUI(){
			contentPanel = new JPanel();
			contentPanel.setLayout(new BoxLayout(contentPanel, BoxLayout.LINE_AXIS));
			buttonPanel = new JPanel();
			buttonPanel.setLayout(new BoxLayout(buttonPanel, BoxLayout.PAGE_AXIS));
			labelPanel = new JPanel();
			labelPanel.setLayout(new BoxLayout(labelPanel, BoxLayout.PAGE_AXIS));
			
    		contentPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
    		contentPanel.setBorder(BorderFactory.createEmptyBorder(1, 0, 1, 0));
			
			up = new JButton("+");
			up.setFont(new Font("Serif", Font.PLAIN, 8));

			up.setAlignmentX(Component.CENTER_ALIGNMENT);
			down = new JButton("-");
			down.setAlignmentX(Component.CENTER_ALIGNMENT);
			
			value = new JLabel("Value: ",JLabel.CENTER);
			name = new JLabel(ctrl.getName(), JLabel.CENTER);
			
			buttonPanel.add(up);
			buttonPanel.add(down);
			
			labelPanel.add(name);
			labelPanel.add(Box.createRigidArea(new Dimension(0, 3)));
			labelPanel.add(value);
			
			contentPanel.add(buttonPanel);
			contentPanel.add(labelPanel);
			
			up.addMouseListener(this);
			down.addMouseListener(this);
		}
		
		public void mousePressed(MouseEvent e) {
			JButton b = (JButton) e.getComponent() ;
			//JOptionPane.showMessageDialog(null, b.getText()+ " for "+ ctrl.getName());
			if(b.getText().equals("-"))
				try {
						ctrl.decreaseValue();
				} catch (ControlException e1) {
					JOptionPane.showMessageDialog(contentPanel, "The value can not be decreased\n"+e1.getMessage());
				}
			else
				try {
						ctrl.increaseValue();
				} catch (ControlException e1) {
					JOptionPane.showMessageDialog(contentPanel, "The value can not be increased\n"+e1.getMessage());
				} 

			updateValue();	
		}
		
		public void updateValue(){
			try {
				setValue(ctrl.getValue());
			} catch (ControlException e1) {}
		}
		
		public void setValue(String v){
			value.setText("Value: "+v);
		}
		
		public void setValue(int v){
			value.setText("Value: "+v);
		}
		
		public JPanel getPanel(){
			return contentPanel;
		}
		 
	}
}
