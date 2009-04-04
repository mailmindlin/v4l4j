package au.edu.jcu.v4l4j.examples;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.nio.ByteBuffer;
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
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class builds a GUI to display a video stream and video controls from
 * a {@link VideoDevice}. The video device is created from its device name,
 * and frame capture and processing is done using an {@link ImageProcessor}.
 * This class offers two methods for drawing a frame on the screen: 
 * {@link #setImageIcon(byte[])} & {@link #setImageRaster(byte[])}.
 * The former can be used with TIFF, PNG & JPEG images but is slower than the 
 * latter, which can be used with RGB images and is substantially faster, and
 * less CPU-intensive. With little modification, the latter can be adjusted
 * to accept more image formats.
 * @author gilles
 *
 */
public class VideoViewer extends WindowAdapter implements Runnable{
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
	private int n, width, height;
	private FrameGrabber fg;
	private Hashtable<String,Control> controls; 
	private Thread captureThread;
	private boolean stop;
	private VideoDevice vd;
	private ImageProcessor processor;
	private BufferedImage img;
	private WritableRaster raster;
	private static ImageIcon v4l4jIcon = createImageIcon("resources/v4l4j.png");
	private static int FPS_REFRESH = 1000; //in  msecs
	
	/**
	 * The method builds a new VideoViewer object
	 * @param d the video device
	 * @param p the image processor to which we will send frames as they are
	 * captured
	 */
    public VideoViewer(VideoDevice d, ImageProcessor p) {
    	vd = d;
		fg = null;
		processor = p;
    	controls = vd.getControlList().getTable();
        stop = false;
        captureThread = null;      
    }
    
    /**
     * this method creates an {@link ImageIcon} from an existing resource
     * @param path the path to the image
     * @return the {@link ImageIcon}
     */
    public static ImageIcon createImageIcon(String path) {
	    java.net.URL imgURL = VideoViewer.class.getClassLoader().getResource(path);
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
     */
    public void initGUI(Object[] i, int width, int height, String fmtName){
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
        startCap = new JButton("Start");
        startCap.setAlignmentX(Component.CENTER_ALIGNMENT);
        if(i.length==0){
        	startCap.setEnabled(false);
        	formats.setEnabled(false);
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
        f.setTitle(fmtName+" capture from ");//+vd.getDeviceInfo().getName());
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
     * This method draws a new image as an {@link ImageIcon} in a JLabel.
     * The image must be either in TIFF, PNG or JPEG format.
     * @param b the image as a byte array
     */
    public void setImageIcon(byte[] b) {
    	video.setIcon(new ImageIcon(b));
    	
    	// Computes the frame rate
    	if(start==0)
    		start = System.currentTimeMillis();
    	else if(System.currentTimeMillis()>start+FPS_REFRESH) {
    		fps.setText(String.format("FPS: %5.2f", 
    				(float) 1000*n/(System.currentTimeMillis()-start)));
			start = System.currentTimeMillis();
			n = 0;
		} else
			n++;
    }
    
    /**
     * This method draws a new image as in a {@link JComponent} using a 
     * {@link Raster}. The image must be in RGB24 format.
     * @param b the image as a byte array
     */
    public void setImageRaster(byte[] b) {
        raster.setDataElements(0, 0, width, height, b);
        video.getGraphics().drawImage(img, 0, 0, width, height, null);

    	
    	// Computes the frame rate
    	if(start==0)
    		start = System.currentTimeMillis();
    	else if(System.currentTimeMillis()>start+FPS_REFRESH) {
    		fps.setText(String.format("FPS: %5.2f", 
    				(float) 1000*n/(System.currentTimeMillis()-start)));
			start = System.currentTimeMillis();
			n = 0;
		} else
			n++;
    }
    
    /**
     * Implements the capture thread: get a frame from the FrameGrabber, and 
     * send it for processing
     */
    public void run(){
		ByteBuffer bb;
		byte[] b;
		try {			
			while(!stop){
				bb = fg.getFrame();
				b = new byte[bb.limit()];
				bb.get(b);
				processor.processImage(b);
			}
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to capture image");
			JOptionPane.showMessageDialog(f, 
					"Failed to capture image:\n"+e.getMessage());
		} catch(Throwable t){
			t.printStackTrace();
			System.out.println("Failed to capture image");
			JOptionPane.showMessageDialog(f, 
					"Failed to capture image:\n"+t.getMessage());
		}
    }
    
    private void startCapture(){
    	if(captureThread == null){
    		try {
    			fg = processor.getGrabber(
    					(ImageFormat) formats.getSelectedItem());
				fg.startCapture();
			} catch (V4L4JException e) {
				System.out.println("Failed to start capture");
				JOptionPane.showMessageDialog(f, 
						"Failed to start capture:\n"+e.getMessage());
				e.printStackTrace();
				return;
			}
	
			//Update GUI:
			width = fg.getWidth();
			height = fg.getHeight();
			
			//set the size of the video label & control pane
			video.setMaximumSize(new Dimension(width, height));
			video.setSize(new Dimension(width, height));			
			controlScrollPane.setPreferredSize(
					new Dimension(300, height));
			formats.setEnabled(false);
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
			
			//Create the BufferedImage
			//Thanks to Sergio Blanco for sharing the BufferedImage related code 
			//below and in setImageRaster()
	        raster = Raster.createInterleavedRaster(
	        		new DataBufferByte(new byte[width*height*3] ,width*height*3) ,
	        		width,
	        		height,
	        		3 * width,
	        		3, new int[]{0, 1, 2},null);
	        
	        ColorSpace cs = ColorSpace.getInstance(ColorSpace.CS_sRGB);
	        ColorModel cm = new ComponentColorModel(cs,
	        		false,
	        		false,
	        		Transparency.OPAQUE,
	        		DataBuffer.TYPE_BYTE);
	        img = new BufferedImage(cm,raster,false,null);
			
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
			video.setIcon(v4l4jIcon);
			video.validate();
			video.repaint();
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
		
		public MenuControl(Control c){
			super(c);

			int v = c.getDefaultValue();
			box = new JComboBox(ctrl.getDiscreteValues().toArray());
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
				ctrl.setValue(ctrl.getDiscreteValues().get(box.getSelectedIndex()));
			} catch (ControlException e1) {
				JOptionPane.showMessageDialog(null, "Error setting value.\n"+e1.getMessage());
			}
			
		}
	}
}
