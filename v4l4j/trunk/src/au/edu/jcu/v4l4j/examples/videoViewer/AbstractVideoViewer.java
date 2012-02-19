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
package au.edu.jcu.v4l4j.examples.videoViewer;

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
import java.awt.image.BufferedImage;
import java.awt.image.Raster;
import java.util.Hashtable;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSlider;
import javax.swing.JSpinner;
import javax.swing.JTextField;
import javax.swing.SpinnerNumberModel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.FrameInterval;
import au.edu.jcu.v4l4j.FrameInterval.DiscreteInterval;
import au.edu.jcu.v4l4j.FrameInterval.Type;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.Tuner;
import au.edu.jcu.v4l4j.TunerInfo;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.examples.videoViewer.AbstractVideoViewer.IntervalGUI.Interval;
import au.edu.jcu.v4l4j.exceptions.ControlException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class builds a GUI to display a video stream and video controls from
 * a {@link VideoDevice}.
 * @author gilles
 *
 */
public abstract class AbstractVideoViewer extends WindowAdapter implements CaptureCallback{
	private JLabel video, fps, freq;
	private JFrame f;
	private JComboBox formats;
	private IntervalGUI intervals;
	private JPanel controlPanel, captureButtons;
	private JScrollPane controlScrollPane;
	private JPanel videoPanel;
	private JButton startCap, stopCap;
	private JSpinner freqSpinner;
	private Tuner tuner;
	private TunerInfo tinfo;
	private long start = 0;
	private int n, width, height;
	private FrameGrabber fg;
	private Hashtable<String,Control> controls; 
	protected VideoDevice vd;
	private static ImageIcon v4l4jIcon = createImageIcon("resources/v4l4j.png");
	private static int FPS_REFRESH = 1000; //in  msecs
	
	/**
	 * The method builds a new VideoViewer object
	 * @param d the video device
	 */
    public AbstractVideoViewer(VideoDevice d) {
    	vd = d;
		fg = null;
    	controls = vd.getControlList().getTable();     
    }
    
    /**
     * this method creates an {@link ImageIcon} from an existing resource
     * @param path the path to the image
     * @return the {@link ImageIcon}
     */
    public static ImageIcon createImageIcon(String path) {
	    java.net.URL imgURL = AbstractVideoViewer.class.getClassLoader().getResource(path);
	    if (imgURL != null)
	        return new ImageIcon(imgURL);
	    
	    return null;
    }
    
    /**
     * This method returns the {@link VideoDevice} associated with this viewer.
     * @return the {@link VideoDevice} associated with this viewer.
     */
    public VideoDevice getVideoDevice(){
    	return vd;
    }
 
    /** 
     * This method creates the graphical interface components and initialises 
     * them. It then makes them visible.
     * @param i an array of {@link ImageFormat}s to be added in the format list
	 * @param fmtName the name of format of images displayed in the title bar. 
	 * If this array is empty, the format list is disabled and capture cannot
	 * be started 
	 * @param width the capture width
	 * @param height the capture height
     */
    public void initGUI(Object[] i, final int width, final int height, String fmtName){
        f = new JFrame();
        f.setLayout(new BoxLayout(f.getContentPane(),BoxLayout.LINE_AXIS));
        f.setIconImage(v4l4jIcon.getImage());
        
        videoPanel = new JPanel();
        videoPanel.setLayout(new BoxLayout(videoPanel, BoxLayout.PAGE_AXIS));
        
        video = new JLabel(v4l4jIcon);
        video.setPreferredSize(new Dimension(width, height));
        video.setAlignmentX(Component.CENTER_ALIGNMENT);
        video.setAlignmentY(Component.CENTER_ALIGNMENT);
        videoPanel.add(video);
        
        captureButtons = new JPanel();
        captureButtons.setLayout(new  BoxLayout(captureButtons, 
        		BoxLayout.LINE_AXIS));
        
        formats = new JComboBox(i);
        formats.setAlignmentX(Component.CENTER_ALIGNMENT);
        Dimension d = new Dimension(100, 25);
        formats.setSize(d);
        formats.setPreferredSize(d);
        formats.setMaximumSize(d);
        
        FrameInterval intv = null;
        try {
        	intv = vd.getDeviceInfo().listIntervals((ImageFormat) i[0], width, height);
        }catch (V4L4JException e){}
        
        intervals = new IntervalGUI();
        intervals.setInterval(intv);
        
        formats.addActionListener(new ActionListener() {			
			@Override
			public void actionPerformed(ActionEvent arg0) {
				FrameInterval intv = null;
		        try {
		        	intv = vd.getDeviceInfo().listIntervals(
		        			(ImageFormat) formats.getSelectedItem(), width, height
		        			);
		        }catch (V4L4JException e){}
		        
		        intervals.setInterval(intv);
				
			}
		}
		);
        startCap = new JButton("Start");
        startCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        if(i.length==0){
        	startCap.setEnabled(false);
        	formats.setEnabled(false);
        	intervals.setEnabled(false);
        }else
        	startCap.setEnabled(true);
        stopCap = new JButton("Stop");
        stopCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        stopCap.setEnabled(false);
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
						((SpinnerNumberModel) freqSpinner.getModel()).
							getNumber().doubleValue()
						);
			}
        	
        });
        captureButtons.add(Box.createGlue());
        captureButtons.add(formats);
        captureButtons.add(Box.createGlue());
        captureButtons.add(intervals.getPanel());
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
        			startCapture();
        	}
        });
        stopCap.addMouseListener(new MouseAdapter() {
        	public void mouseClicked(MouseEvent e) {
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
        try {
			f.setTitle(fmtName+" capture from "+vd.getDeviceInfo().getName());
		} catch (V4L4JException e1) {
			f.setTitle(fmtName+" capture from "+vd.getDevicefile());
		}
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
    	if(c.getType() == V4L4JConstants.CTRL_TYPE_SLIDER)
    		ctrl = new SliderControl(c);
    	else if (c.getType() == V4L4JConstants.CTRL_TYPE_BUTTON)
    		ctrl = new ButtonControl(c);
    	else if (c.getType() == V4L4JConstants.CTRL_TYPE_SWITCH)
    		ctrl = new SwitchControl(c);
    	else if (c.getType() == V4L4JConstants.CTRL_TYPE_DISCRETE)
    		ctrl = new MenuControl(c);
    	return ctrl;
    }
        
    /**
     * This method draws a new image as in a {@link JComponent} using a 
     * {@link Raster}. The image must be in RGB24 format.
     * @param b the image as a byte array
     */
    private void drawBufferedImage(BufferedImage image) {
    	// Computes the frame rate
    	if(start==0)
    		start = System.currentTimeMillis();
    	else if(System.currentTimeMillis()>start+FPS_REFRESH) {
    		fps.setText(String.format("FPS: %5.2f", 
    				(float) 1000*n/(System.currentTimeMillis()-start)));
			start = System.currentTimeMillis();
			n = 0;
		}
    	
        video.getGraphics().drawImage(image, 0, 0, width, height, null);
        n++;    	
    }
    
    /**
     * This abstract method must be implemented by subclasses and return
     * and frame grabber which captures frames in the given native image format 
     * @param imf the native image format the returned frame grabber must use
     * @return a frame grabber
     * @throws V4L4JException if a frame grabber cant be created
     */
    abstract protected FrameGrabber getFrameGrabber(ImageFormat imf) throws V4L4JException;
    
    /**
     * This method is called when the Start capture button is hit.
     * It creates a frame grabber, resizes the UI and start the capture
     */
    private void startCapture(){    	
    	//get the frame interval value
    	Interval i = intervals.getInterval();

    	//create the frame grabber
    	try {
    		fg = getFrameGrabber( (ImageFormat) formats.getSelectedItem());
    	} catch (V4L4JException e){
    		JOptionPane.showMessageDialog(f, 
    				"Error obtaining the frame grabber");
    		e.printStackTrace();
    		return;
    	}

    	// enable push mode
    	fg.setCaptureCallback(this);
    	
    	//set the frame interval if not null
    	try {
    		if(i!=null)
    			fg.setFrameInterval(i.num, i.denom);
    	} catch (Exception e){
    		//cant set the frame interval, keep going
    		JOptionPane.showMessageDialog(f, 
			"Error setting the frame rate");
    	}


    	//Update UI
    	width = fg.getWidth();
    	height = fg.getHeight();

    	//set the size of the video label & control pane
    	video.setMaximumSize(new Dimension(width, height));
    	video.setSize(new Dimension(width, height));			
    	controlScrollPane.setPreferredSize(
    			new Dimension(300, height));

    	// start capture
    	try {
    		fg.startCapture();
    	} catch (V4L4JException e) {
    		JOptionPane.showMessageDialog(f, 
    				"Failed to start capture:\n"+e.getMessage());
    		e.printStackTrace();
    		vd.releaseFrameGrabber();
    		return;
    	}

    	// more UI update
    	formats.setEnabled(false);
    	intervals.setEnabled(false);
    	startCap.setEnabled(false);
    	stopCap.setEnabled(true);

    	//show tuner frequency adjust if there s a tuner			
    	try {
    		tuner = fg.getTuner();
    		tinfo = vd.getDeviceInfo().getInputs()
    		.get(fg.getChannel()).getTunerInfo();
    		freqSpinner.setModel(new SpinnerNumberModel(
    				new Double(tuner.getFrequency()), 
    				new Double(tinfo.getRangeLow()),
    				new Double(tinfo.getRangeHigh()),
    				new Double(1)));
    		freq.setVisible(true);
    		freqSpinner.setVisible(true);
    	} catch (V4L4JException nte){}//No tuner for input
    	f.pack();

    	System.out.println("Input format: "+fg.getImageFormat().getName());
    }
    
    /**
     * This method is called when the Stop capture button is hit.
     * It terminates the capture, releases resources and reset the UI
     */
    private void stopCapture(){
    	try {
    		if (fg != null)
    			fg.stopCapture();
    	} catch (StateException e){} 
    	// the frame grabber may be already stopped
    	
    	vd.releaseFrameGrabber();
    	formats.setEnabled(true);
    	intervals.setEnabled(true);
    	freq.setVisible(false);
    	freqSpinner.setVisible(false);
    	video.setIcon(v4l4jIcon);
    	startCap.setEnabled(true);
    	stopCap.setEnabled(false);
    	video.validate();
    	video.repaint();
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
	
	/** 
	 * this method is called by v4l4j during capture when a new frame is available
	 */
	@Override
	public void nextFrame(VideoFrame frame) {
		// every time a frame arrives, get the BufferedImage and draw it
		drawBufferedImage(frame.getBufferedImage());
		// then recycle the frame
		frame.recycle();
	}

	/**
	 * This method is called by v4l4j during capture if there is an error
	 */
	@Override
	public void exceptionReceived(V4L4JException e) {
		JOptionPane.showMessageDialog(null, "Error getting next frame.\n"+e.getMessage());
	}

	
	/*
	 * The classes beyond this point are for UI support
	 */
	public interface ControlGUI{
		public JPanel getPanel();
	}
	
	public class IntervalGUI implements ControlGUI {
		protected JPanel contentPanel;
		private FrameInterval intv;
		private JComboBox discreteList;
		private JTextField stepwiseValue;
		
		public IntervalGUI(){
			contentPanel = new JPanel();
			contentPanel.setAlignmentX(Component.CENTER_ALIGNMENT);
			contentPanel.setLayout(new BoxLayout(contentPanel, BoxLayout.LINE_AXIS));
		}
		
		public void setInterval(FrameInterval i){
			intv = i;
			contentPanel.removeAll();
			contentPanel.revalidate();
			
			if(intv==null)
				return;
			
			if(intv.getType()==Type.DISCRETE){
				discreteList = new JComboBox(
						intv.getDiscreteIntervals().toArray()
				);
				contentPanel.add(discreteList);
			} else if(intv.getType()==Type.STEPWISE){
				stepwiseValue = new JTextField(8);
				stepwiseValue.setText(
						intv.getStepwiseInterval().maxIntv.numerator+
						"/"+
						intv.getStepwiseInterval().maxIntv.denominator);
				stepwiseValue.setToolTipText(
				"Min:"+
				intv.getStepwiseInterval().minIntv.numerator+
				"/"+
				intv.getStepwiseInterval().minIntv.denominator+
				" - Max: "+
				intv.getStepwiseInterval().maxIntv.numerator+
				"/"+
				intv.getStepwiseInterval().maxIntv.denominator+
				" - Step: "+
				intv.getStepwiseInterval().stepIntv.numerator+
				"/"+
				intv.getStepwiseInterval().stepIntv.denominator
				);
				contentPanel.add(stepwiseValue);
			} //else unsupported: do nothing
		}
		
		public Interval getInterval(){
			if(intv==null)
				return null;
			else if(intv.getType()==Type.DISCRETE) {
				DiscreteInterval i = (DiscreteInterval) discreteList.getSelectedItem();
				return new Interval(i.numerator, i.denominator);
			} else if (intv.getType()==Type.STEPWISE) {
				return validateStepwsiseValue();
			} else
				return null;
		}
		
		public void setEnabled(boolean v){
			if(intv==null)
				return;
			else if(intv.getType()==Type.DISCRETE) {
				discreteList.setEnabled(v);
			} else if (intv.getType()==Type.STEPWISE) {
				stepwiseValue.setEnabled(v);
			} 
		}
		
		private Interval validateStepwsiseValue(){
			String tokens[] = stepwiseValue.getText().split("/");
			if(tokens.length==2){
				int num = 0, denom = 1;
				
				// get the int values from the text field
				try {
					num = Integer.parseInt(tokens[0]);
					denom = Integer.parseInt(tokens[1]);
				} catch (NumberFormatException e){
					//invalid frame interval
					String msg = "The frame interval is invalid. It must\n"+
								"be in the form 'X/Y'";
					JOptionPane.showMessageDialog(null, msg);
					throw e;
				}
				
				//make sure the int values are within the boundaries
				if(num<0 || denom <0) {
					String msg = "The frame interval is invalid.";
					JOptionPane.showMessageDialog(null, msg);
					throw new NumberFormatException();
				}
				
				if(
					(compareFrac(intv.getStepwiseInterval().minIntv.getNum(),
						intv.getStepwiseInterval().minIntv.getDenom(),
						num, denom)<=0)
					&&
					(compareFrac(intv.getStepwiseInterval().minIntv.getNum(),
							intv.getStepwiseInterval().minIntv.getDenom(),
							num, denom)>=0)
					){
					//values are ok
					return new Interval(num,denom);
					
				} else {
					//values are outside frame intv range
					String msg = "The frame interval ("+num+"/"+denom+") is " +
							"outside the\nallowed range ("+
							intv.getStepwiseInterval().minIntv.getNum()+"/"+
							intv.getStepwiseInterval().minIntv.getDenom()+" to"+
							intv.getStepwiseInterval().maxIntv.getNum()+"/"+
							intv.getStepwiseInterval().minIntv.getDenom()+")";
							
					JOptionPane.showMessageDialog(null, msg);
					return null;
				}
			} else {
				String msg = "The frame interval is invalid. It must\n"+
				"be in the form 'X/Y'";
				JOptionPane.showMessageDialog(null, msg);
				throw new NumberFormatException();
			}
		}
		
		private int compareFrac(int n1, int d1, int n2, int d2){
			long a = n1*d2, b = n2*d1;
			return (a<b) ? -1 : (a==b) ? 0 : 1;
		}
		
		public JPanel getPanel(){
			return contentPanel;
		}
		
		public class Interval {
			public int num, denom;
			public Interval(int n, int d){
				num = n;
				denom = d;
			}
		}
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
  
			
			if(ctrl.getType()!=V4L4JConstants.CTRL_TYPE_BUTTON && ctrl.getType()!=V4L4JConstants.CTRL_TYPE_SWITCH) {
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
		String[] names = new String[0];
		Integer[] values = new Integer[0];
		
		public MenuControl(Control c){
			super(c);
			names = (String[]) ctrl.getDiscreteValueNames().toArray(names);
			values = (Integer[]) ctrl.getDiscreteValues().toArray(values);

			int v = c.getDefaultValue();
			box = new JComboBox(names);
			try {v = c.getValue();} catch (ControlException e){}
			box.setSelectedIndex(ctrl.getDiscreteValues().indexOf(v));				
			initPanel();
		}
		
		
		private void initPanel(){
			box.addActionListener(this);
			contentPanel.add(box);
		}

		@Override
		public void actionPerformed(ActionEvent e) {
			try {
				ctrl.setValue(values[box.getSelectedIndex()].intValue());
			} catch (ControlException e1) {
				JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
			}
			
		}
	}
}
