/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a   
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

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
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.Tuner;
import au.edu.jcu.v4l4j.TunerInfo;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.ControlException;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class create a graphical interface to capture frames 
 * from a video device and display it. The interface also gives access to the 
 * video controls. Frames are captured from a video device in a format that
 * v4l4j can encode in JPEG. If the given video device does not support
 * such format, 
 * 
 * @author gilles
 *
 */
public class JPEGVideoViewer extends WindowAdapter implements Runnable{
	private JLabel video, fps, freq;
	private JFrame f;
	private JComboBox formats;
	private JPanel controlPanel, captureButtons;
	private JScrollPane controlScrollPane;
	private JPanel videoPanel;
	private JButton startCap, stopCap;
	private JSpinner freqSpinner;
	private Tuner tuner;
	private TunerInfo tinfo;
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
	 * @param s the capture standard
	 * @param c the capture channel
	 * @param q the JPEG compression quality
	 * @throws V4L4JException if any parameter if invalid,  or if the video device 
	 * does not support an image format that can be converted to JPEG
	 */
    public JPEGVideoViewer(String dev, int w, int h, int s, int c, int q) throws V4L4JException{
    	vd = new VideoDevice(dev);
		if(!vd.supportJPEGConversion()){
			String msg = "Image from this video device cannot be converted\n"
				+ "to JPEG. Please submit a bug report about this,\n"
				+"so support for your device can be added to v4l4j.\n"
				+"See README file in the v4l4j/ directory for directions.";
			JOptionPane.showMessageDialog(f, msg);
			throw new V4L4JException(msg);
		}
    	
		fg = null;
		width = w;
		height = h;
		std = s;
		channel = c;
		qty = q;
    	controls = vd.getControlList().getTable();
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
        f.setIconImage(new ImageIcon("v4l4j.gif").getImage());
        
        videoPanel = new JPanel();
        videoPanel.setLayout(new BoxLayout(videoPanel, BoxLayout.PAGE_AXIS));
        
        video = new JLabel();
        video.setPreferredSize(new Dimension(width, height));
        video.setAlignmentX(Component.CENTER_ALIGNMENT);
        video.setAlignmentY(Component.CENTER_ALIGNMENT);
        videoPanel.add(video);
        
        captureButtons = new JPanel();
        captureButtons.setLayout(new  BoxLayout(captureButtons, BoxLayout.LINE_AXIS));
        formats = new JComboBox(vd.getDeviceInfo().getFormatList().getRGBEncodableFormats().toArray());
        formats.setAlignmentX(Component.CENTER_ALIGNMENT);
        startCap = new JButton("Start");
        startCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        stopCap = new JButton("Stop");
        stopCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        fps = new JLabel("FPS: 0.0");
        fps.setAlignmentX(Component.CENTER_ALIGNMENT);
        freq = new JLabel("Frequency");
        freq.setAlignmentX(Component.CENTER_ALIGNMENT);
        freq.setVisible(false);
        freqSpinner = new JSpinner();
        freqSpinner.setAlignmentX(Component.CENTER_ALIGNMENT);
        freqSpinner.setVisible(false);
        freqSpinner.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent e) {
				tuner.setFrequency(
						((SpinnerNumberModel) freqSpinner.getModel()).getNumber().doubleValue()
						);
			}
        	
        });
        captureButtons.add(Box.createGlue());
        captureButtons.add(formats);
        captureButtons.add(Box.createGlue());
        captureButtons.add(startCap);
        captureButtons.add(Box.createGlue());
        captureButtons.add(freq);
        captureButtons.add(Box.createGlue());
        captureButtons.add(stopCap);
        captureButtons.add(Box.createGlue());
        captureButtons.add(freqSpinner);
        captureButtons.add(Box.createGlue());
        captureButtons.add(fps);
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
        f.setTitle("JPEG Capture from "+vd.getDeviceInfo().getName());
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
    		fps.setText(String.format("FPS: %5.2f", (float) 1000*n/(System.currentTimeMillis()-start)));
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
			JOptionPane.showMessageDialog(f, "Failed to capture image:\n"+e.getMessage());
		} catch(Throwable t){
			t.printStackTrace();
			System.out.println("Failed to capture image");
			JOptionPane.showMessageDialog(f, "Failed to capture image:\n"+t.getMessage());
		}
    }
    
    private void startCapture(){
    	if(captureThread == null){
    		try {
    			fg = vd.getJPEGFrameGrabber(width, height, channel, std, qty, 
    					(ImageFormat) formats.getSelectedItem());
				fg.startCapture();
			} catch (V4L4JException e) {
				System.out.println("Failed to start capture");
				JOptionPane.showMessageDialog(f, "Failed to start capture:\n"+e.getMessage());
				e.printStackTrace();
				return;
			}
			//update width and height
			width = fg.getWidth();
			height = fg.getHeight();
	
			//Update GUI:
			
			//set the size of the video label & control pane
			video.setMaximumSize(new Dimension(width, height));
			video.setSize(new Dimension(width, height));			
			controlScrollPane.setPreferredSize(new Dimension(300, fg.getHeight()));
			formats.setEnabled(false);			
			
			//show tuner frequency adjust if there s a tuner			
			try {
				tuner = fg.getTuner();
				tinfo = vd.getDeviceInfo().getInputs().get(channel).getTunerInfo();
				freqSpinner.setModel(new SpinnerNumberModel(
						new Double(tuner.getFrequency()), 
						new Double(tinfo.getRangeLow()),
						new Double(tinfo.getRangeHigh()),
						new Double(1)));
				freq.setVisible(true);
				freqSpinner.setVisible(true);
			} catch (NoTunerException nte){}//No tuner for input			
			f.pack();
			
			stop = false;
	    	captureThread = new Thread(this, "Capture Thread");
	        captureThread.start();
	        System.out.println("Image format: "+fg.getImageFormat().getName());
	        
    	}
    }
    
    private void stopCapture(){
    	if(captureThread != null) {
    		if(captureThread.isAlive()){
    			stop = true;
    			try {
    				captureThread.join();
    			} catch (InterruptedException e1) {}
    		}
			fg.stopCapture();
			vd.releaseFrameGrabber();
			captureThread = null;
			formats.setEnabled(true);
			freq.setVisible(false);
			freqSpinner.setVisible(false);
    	}
    }
    
    /**
     * Catch window closing event so we can free up resources before exiting
     * @param e
     */
	public void windowClosing(WindowEvent e) {
		stopCapture();		
		vd.releaseFrameGrabber();
		vd.releaseControlList();
		vd.release();			
    	f.dispose();		
	}
	
	

	public static void main(String[] args) throws V4L4JException, IOException {

		String dev;
		int w, h, std, channel;

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
		
		new JPEGVideoViewer(dev,w,h,std,channel,80);
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
			int v = c.getDefaultValue();
			try {v = c.getValue();} catch (ControlException e) {}
			slider = new JSlider(JSlider.HORIZONTAL, c.getMinValue(), c.getMaxValue(), v);

			setSlider();
			contentPanel.add(slider);
			updateValue(v);
		}
		
		private void setSlider(){
			Hashtable<Integer, JLabel> labels = new Hashtable<Integer, JLabel>();
			int length = (ctrl.getMaxValue() - ctrl.getMinValue()) / ctrl.getStepValue() + 1;
			int middle = ctrl.getDefaultValue();
			
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
				 int v = 0;
				 try {
					v = ctrl.setValue(source.getValue());
				} catch (ControlException e1) {
					JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
					try {v = ctrl.getValue();} catch (ControlException ce) {v = ctrl.getDefaultValue();}
				} finally{
					updateValue(v);
					source.removeChangeListener(this);
					source.setValue(v);
					source.addChangeListener(this);
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
			int v = c.getDefaultValue();
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

			int v = c.getDefaultValue();
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
