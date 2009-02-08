package au.edu.jcu.v4l4j.examples;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
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
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.ControlException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class capture frames from a video device and display it in a JFrame
 * @author gilles
 *
 */
public class WebcamViewer extends WindowAdapter implements Runnable{
	private JLabel video, fps;
	private JFrame f;
	private JPanel controlPanel, captureButtons;
	private JScrollPane controlScrollPane;
	private JPanel videoPanel;
	private JButton startCap, stopCap;
	private long start = 0;
	private int n, width, height, qty, std, channel;
	private FrameGrabber fg;
	private Hashtable<String,Control> controls; 
	private Thread captureThread;
	private boolean stop;
	private VideoDevice vd;
	private static int FPS_REFRESH = 1000; //in  msecs
	
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
    public WebcamViewer(String dev, int w, int h, int s, int c, int q) throws V4L4JException{
    	vd = new VideoDevice(dev);
		fg = null;
		width = w;
		height = h;
		std = s;
		channel = c;
		qty = q;
    	controls = vd.getControlList();
        initGUI();
        stop = false;
        captureThread = null;
        
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
        video.setPreferredSize(new Dimension(width, height));
        video.setAlignmentX(Component.CENTER_ALIGNMENT);
        videoPanel.add(video);
        
        captureButtons = new JPanel();
        captureButtons.setLayout(new  BoxLayout(captureButtons, BoxLayout.LINE_AXIS));
        startCap = new JButton("Start");
        startCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        stopCap = new JButton("Stop");
        stopCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        fps = new JLabel("FPS: 0.0");
        fps.setAlignmentX(Component.CENTER_ALIGNMENT);
        captureButtons.add(Box.createGlue());
        captureButtons.add(stopCap);
        captureButtons.add(Box.createGlue());
        captureButtons.add(fps);
        captureButtons.add(Box.createGlue());
        captureButtons.add(startCap);
        captureButtons.add(Box.createGlue());
        startCap.addMouseListener(new MouseAdapter() {
        	public void mouseClicked(MouseEvent e) {
        		if (e.getComponent() == startCap)
        			startCapture();
        	}
        });
        stopCap.addMouseListener(new MouseAdapter() {
        	public void mouseClicked(MouseEvent e) {
        		if (e.getComponent() == stopCap)
        			stopCapture();
        	}
        });
        videoPanel.add(Box.createRigidArea(new Dimension(0, 5)));
        videoPanel.add(captureButtons);
        
        controlPanel = new JPanel();
        controlScrollPane = new JScrollPane(controlPanel);
        controlScrollPane.getVerticalScrollBar().setBlockIncrement(40);
        controlScrollPane.getVerticalScrollBar().setUnitIncrement(25);
        controlScrollPane.setPreferredSize(new Dimension(300, height));
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
    	ControlGUI gui;
    	for(Control c: controls.values()) {
    		gui = getControlGUI(c);
    		if(gui!=null)
    			controlPanel.add(gui.getPanel());
    	}
    }
    
    private ControlGUI getControlGUI(Control c){
    	ControlGUI ctrl = null;
    	if(c.getType() == V4L4JConstants.SLIDER)
    		ctrl = new SliderControl(c);
    	else if (c.getType() == V4L4JConstants.BUTTON)
    		ctrl = new ButtonControl(c);
    	else if (c.getType() == V4L4JConstants.SWITCH)
    		ctrl = new SwitchControl(c);
    	else if (c.getType() == V4L4JConstants.DISCRETE)
    		ctrl = new MenuControl(c);
    	return ctrl;
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
    	else if(System.currentTimeMillis()>start+FPS_REFRESH) {
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
    
    private void startCapture(){
    	if(captureThread == null){
    		try {
    			fg = vd.getJPEGFrameGrabber(width, height, channel, std, qty);
    			video.setPreferredSize(new Dimension(fg.getWidth(), fg.getHeight()));
    			controlScrollPane.setPreferredSize(new Dimension(300, fg.getHeight()));
    			f.pack();
				fg.startCapture();
			} catch (V4L4JException e) {
				System.out.println("Failed to start capture");
				e.printStackTrace();
				return;
			}
			stop = false;
	    	captureThread = new Thread(this, "Capture Thread");
	        captureThread.start();
    	}
    }
    
    private void stopCapture(){
    	if(captureThread != null && captureThread.isAlive()){
    		stop = true;
    		try {
				captureThread.join();
			} catch (InterruptedException e1) {}
			captureThread = null;
			fg.stopCapture();
			vd.releaseFrameGrabber();
    	}
    }
    
    /**
     * Catch window closing event so we can free up resources before exiting
     * @param e
     */
	public void windowClosing(WindowEvent e) {
		stopCapture();		
		try {
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
			w = V4L4JConstants.MAX_WIDTH;
		}
		try{			
			h = Integer.parseInt(args[2]);
		} catch  (Exception e) {
			h = V4L4JConstants.MAX_HEIGHT;
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
	
	public interface ControlGUI{
		public JPanel getPanel();
	}
	
	public class ControlModelGUI implements ControlGUI{
		protected JPanel contentPanel;
		private JLabel value;
		protected Control ctrl;
		
		public ControlModelGUI(Control c){
			ctrl = c;
			initControlGUI();
		}
		
		private void initControlGUI(){
			contentPanel = new JPanel();
			contentPanel.setAlignmentX(Component.CENTER_ALIGNMENT);

			TitledBorder b = BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(EtchedBorder.LOWERED), ctrl.getName());
			b.setTitleJustification(TitledBorder.LEFT);
			contentPanel.setBorder(b);
  
			
			if(ctrl.getType()!=V4L4JConstants.BUTTON && ctrl.getType()!=V4L4JConstants.SWITCH) {
				contentPanel.setLayout(new BoxLayout(contentPanel, BoxLayout.PAGE_AXIS));
				value = new JLabel("Value: ");
				contentPanel.add(value);			
				contentPanel.add(Box.createRigidArea(new Dimension(5, 0)));
			} else {
				contentPanel.setLayout(new GridLayout());
				value = null;
			}

		}
		
		public final void updateValue(int v){
			if(value!=null)
				value.setText("Value: "+ String.valueOf(v));
		}
		
		public final JPanel getPanel(){
			return contentPanel;
		}	 
	}
	
	public class SliderControl extends ControlModelGUI implements ChangeListener{
		private JSlider slider;
		
		public SliderControl(Control c){
			super(c);
			int v = c.getMiddleValue();
			try {v = c.getValue();} catch (ControlException e) {}
			slider = new JSlider(JSlider.HORIZONTAL, c.getMinValue(), c.getMaxValue(), v);

			setSlider();
			contentPanel.add(slider);
			updateValue(v);
		}
		
		private void setSlider(){
			Hashtable<Integer, JLabel> labels = new Hashtable<Integer, JLabel>();
			int length = (ctrl.getMaxValue() - ctrl.getMinValue()) / ctrl.getStepValue() + 1;
			int middle = ctrl.getMiddleValue();
			
			slider.setSnapToTicks(true);
			slider.setPaintTicks(false);
			slider.setMinorTickSpacing(ctrl.getStepValue());
			labels.put(ctrl.getMinValue(), new JLabel(String.valueOf(ctrl.getMinValue())));
			labels.put(ctrl.getMaxValue(), new JLabel(String.valueOf(ctrl.getMaxValue())));
			labels.put(middle, new JLabel(String.valueOf(middle)));
			
			if(length < 100 && length >10) {
				slider.setMajorTickSpacing(middle/2);
				slider.setPaintTicks(true);
			} else  if (length < 10){
				slider.setMajorTickSpacing(middle);
				slider.setPaintTicks(true);
			}
			slider.setLabelTable(labels);
			slider.setPaintLabels(true);
			
			slider.addChangeListener(this);
		}
		
		@Override
		public void stateChanged(ChangeEvent e) {
			 JSlider source = (JSlider)e.getSource();
			 if (!source.getValueIsAdjusting()) {
				 try {
					ctrl.setValue(source.getValue());
					updateValue(source.getValue());
				} catch (ControlException e1) {
					JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
				}
			 }			
		}

	}
	
	public class ButtonControl extends ControlModelGUI implements ActionListener{
		private JButton button;
		
		public ButtonControl(Control c){
			super(c);
			button = new JButton("Activate");
			button.setAlignmentX(Component.CENTER_ALIGNMENT);
			button.addActionListener(this);
			contentPanel.add(button);
		}

		@Override
		public void actionPerformed(ActionEvent e) {
			try {
				ctrl.setValue(0);
			} catch (ControlException e1) {
				JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
			}			
		}
	}
	
	public class SwitchControl extends ControlModelGUI implements ItemListener{
		private JCheckBox box;
		
		public SwitchControl(Control c){
			super(c);
			int v = c.getMiddleValue();
			box = new JCheckBox();
			box.setAlignmentX(Component.CENTER_ALIGNMENT);
			try {v = c.getValue();} catch (ControlException e){}
			box.setSelected(v == 1);				
			box.addItemListener(this);
			contentPanel.add(box);
		}
		
		@Override
		public void itemStateChanged(ItemEvent e) {
			try {
				if(e.getStateChange() == ItemEvent.DESELECTED)
					ctrl.setValue(0);
				else
					ctrl.setValue(1);
			} catch (ControlException e1) {
				JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
			}		
		}
	}
	
	public class MenuControl extends ControlModelGUI implements ActionListener{
		private JComboBox box;
		
		public MenuControl(Control c){
			super(c);

			int v = c.getMiddleValue();
			box = new JComboBox(ctrl.getDiscreteValueNames());
			try {v = c.getValue();} catch (ControlException e){}
			box.setSelectedIndex(ctrl.getDiscreteValueIndex(v));				
			initPanel();
		}
		
		
		private void initPanel(){
			box.addActionListener(this);
			contentPanel.add(box);
		}

		@Override
		public void actionPerformed(ActionEvent e) {
			try {
				ctrl.setValue(ctrl.getDiscreteValues().elementAt(box.getSelectedIndex()));
			} catch (ControlException e1) {
				JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
			}
			
		}
	}
}
