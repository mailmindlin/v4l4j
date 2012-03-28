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
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.Spring;
import javax.swing.SpringLayout;
import javax.swing.SwingUtilities;

import au.edu.jcu.v4l4j.DeviceInfo;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.InputInfo;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class DeviceChooser  extends WindowAdapter implements ActionListener{
	
	private static String v4lSysfsPath="/sys/class/video4linux/";
	
	private JFrame frame;
	private JPanel mainPanel;
	private JComboBox deviceFiles;
	private DeviceInfoPane info;
	private int width, height;
	
	/**
	 * This method build a new device chooser object
	 * @param dev the full path to a v4l device file, or null
	 * to try and autodetect some.
	 */
	public DeviceChooser(String dev, int w, int h){
		frame = new JFrame("V4L device file selection");
		mainPanel = new JPanel();
		info = null;
		
		width = w;
		height =h;
		System.out.println("w x h:"+w+" "+h);
		
		if(dev == null) {
			Object[] deviceList = listV4LDeviceFiles();
			if (deviceList == null) {
				System.out.println("No video devices detected");
				return;
			}
			deviceFiles = new JComboBox(deviceList);
		} else 
			deviceFiles = new JComboBox(new Object[] {dev});
		
		initGUI();
		actionPerformed(null);
		deviceFiles.addActionListener(this);
		
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.pack();
		frame.setVisible(true);
	}
	
	public void initGUI(){
		frame.getContentPane().add(mainPanel);
		frame.addWindowListener(this);
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.PAGE_AXIS));
		deviceFiles.setAlignmentX(Component.CENTER_ALIGNMENT);
		Dimension d = new Dimension(120,25);
		deviceFiles.setMaximumSize(d);
		mainPanel.add(deviceFiles);
		d = new Dimension(600,500);
		mainPanel.setMinimumSize(d);
		mainPanel.setSize(d);
		mainPanel.setPreferredSize(d);
		mainPanel.setMaximumSize(d);
		mainPanel.setBorder(BorderFactory.createEmptyBorder(1, 0, 0, 0));

	}
	
    /**
     * Catch window closing event so we can free up resources before exiting
     * @param e
     */
	public void windowClosing(WindowEvent e) {
		if(info!=null)
			info.close();
	}
	
	private Object[] listV4LDeviceFiles(){
		Vector<String> dev = new Vector<String>();
		File dir = new File(v4lSysfsPath);
		String[] files = dir.list();
	
		if (files == null)
			return null;
		
		for(String file: files)
			//the following test the presence of "video" in 
			//each file name - not very portable - relying on HAL
			//would be much better ...
			if(file.indexOf("video")!=-1)
				dev.add("/dev/"+file);

		if(dev.size()==0){
			System.err.println("Unable to detect any V4L device file\n"+
					"Set the 'test.device' property to a valid\nvideo "+
					"device file and run this program again ");
			System.exit(0);
		}
		
		return dev.toArray();
	}
	


	@Override
	public void actionPerformed(ActionEvent e) {
		String dev = (String)deviceFiles.getSelectedItem();

		if(info!=null){
			info.close();
			mainPanel.remove(info.getPanel());
		}
		
		try {
			info = new DeviceInfoPane(dev, this);
			info.getPanel().setAlignmentX(Component.CENTER_ALIGNMENT);
			mainPanel.add(info.getPanel());
		} catch (V4L4JException e1) {
			//e1.printStackTrace();
			System.out.println(e1.getMessage());
			info=null;
		}

		mainPanel.validate();
		mainPanel.repaint();
	}
	
	public void close(){
		frame.dispose();
	}
	
	public static class DeviceInfoPane implements ActionListener{
		private static String RGB_BUTTON_STR="Run RGB viewer";
		private static String JPEG_BUTTON_STR="Run JPEG viewer";
		private static String CTRL_ONLY_STR="Run control-only viewer";
		private JPanel mainPane, dataPane, buttonPane;
		private JLabel name, nativeFmt, JPEGEncFmts, RGBEncFmts, BGREncFmts,
				YUVEncFmts, YVUEncFmts, input, inputType, tunerType, standard;
		private JLabel nameValue, inputTypeValue, tunerTypeValue;
		private JLabel nativeFmtValues, JPEGEncFmtsValues, RGBEncFmtsValues,
				BGREncFmtsValues, YUVEncFmtsValues, YVUEncFmtsValues;
		private JComboBox inputs, standards;
		private DeviceInfo di;
		private VideoDevice vd;
		private JButton rgbView, jpegView;
		private DeviceChooser chooser;
		
		
		public DeviceInfoPane(String d, DeviceChooser c) throws V4L4JException{
			chooser = c;
			String tmp ="";
			vd = new VideoDevice(d);
			try {
				di = vd.getDeviceInfo();
			} catch (V4L4JException e){
				di = null;
				initControlsOnly();
				return;
			}
			
			mainPane = new JPanel();
			dataPane = new JPanel();
			buttonPane = new JPanel();

			name = new JLabel("Name :");
			nativeFmt = new JLabel("native image formats :");
			JPEGEncFmts = new JLabel("JPEG encodable formats :");
			RGBEncFmts = new JLabel("RGB24 encodable formats :");
			BGREncFmts = new JLabel("BGR24 encodable formats :");
			YUVEncFmts = new JLabel("YUV420 encodable formats :");
			YVUEncFmts = new JLabel("YVU420 encodable formats :");			
			input = new JLabel("Inputs :");
			inputType = new JLabel("Input type :");
			tunerType = new JLabel("Tuner type :");
			standard = new JLabel("Standards :");
			
			nameValue = new JLabel(di.getName());
			for(ImageFormat f: di.getFormatList().getNativeFormats())
				tmp += tmp.equals("")?f.getName():" - "+f.getName();
			nativeFmtValues = new JLabel(tmp);
			
			tmp="";
			for(ImageFormat f: di.getFormatList().getJPEGEncodableFormats())
				tmp += tmp.equals("")?f.getName():" - "+f.getName();
			JPEGEncFmtsValues = new JLabel(tmp);
			
			tmp="";
			for(ImageFormat f: di.getFormatList().getRGBEncodableFormats())
				tmp += tmp.equals("")?f.getName():" - "+f.getName();
			RGBEncFmtsValues = new JLabel(tmp);
			
			tmp="";
			for(ImageFormat f: di.getFormatList().getBGREncodableFormats())
				tmp += tmp.equals("")?f.getName():" - "+f.getName();
			BGREncFmtsValues = new JLabel(tmp);
			
			tmp="";
			for(ImageFormat f: di.getFormatList().getYUVEncodableFormats())
				tmp += tmp.equals("")?f.getName():" - "+f.getName();
			YUVEncFmtsValues = new JLabel(tmp);
			
			tmp="";
			for(ImageFormat f: di.getFormatList().getYVUEncodableFormats())
				tmp += tmp.equals("")?f.getName():" - "+f.getName();
			YVUEncFmtsValues = new JLabel(tmp);
			
			Vector<String> names = new Vector<String>();
			for(InputInfo i: di.getInputs())
				names.add(i.getName());
			inputs = new JComboBox(names.toArray());
			
			inputTypeValue = new JLabel();
			tunerTypeValue = new JLabel();
			
			standards = new JComboBox();
			
			rgbView = new JButton(RGB_BUTTON_STR);
			rgbView.addActionListener(this);
			jpegView = new JButton(JPEG_BUTTON_STR);
			jpegView.addActionListener(this);
			
			initGUI();
		}
		
		private void initControlsOnly(){
			mainPane = new JPanel();
			mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.PAGE_AXIS));
			name = new JLabel("Unable to get device information");
			name.setAlignmentX(Component.CENTER_ALIGNMENT);
			rgbView = new JButton(CTRL_ONLY_STR);
			rgbView.setAlignmentX(Component.CENTER_ALIGNMENT);
			rgbView.addActionListener(this);
			mainPane.add(Box.createVerticalStrut(5));
			mainPane.add(name);
			mainPane.add(Box.createVerticalStrut(5));
			mainPane.add(rgbView);
			
		}
		
		private void initGUI(){
			mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.PAGE_AXIS));
			
			Dimension labelsDim = new Dimension(200,25);
			Dimension listDim = new Dimension(Integer.MAX_VALUE,25);
			dataPane.setLayout(new SpringLayout());
			
			name.setPreferredSize(labelsDim);
			name.setMinimumSize(labelsDim);
			dataPane.add(name);
			dataPane.add(nameValue);
			
			nativeFmt.setPreferredSize(labelsDim);
			nativeFmt.setMinimumSize(labelsDim);
			dataPane.add(nativeFmt);
			dataPane.add(nativeFmtValues);		

			RGBEncFmts.setPreferredSize(labelsDim);
			RGBEncFmts.setMinimumSize(labelsDim);
			dataPane.add(RGBEncFmts);
			dataPane.add(RGBEncFmtsValues);

			BGREncFmts.setPreferredSize(labelsDim);
			BGREncFmts.setMinimumSize(labelsDim);
			dataPane.add(BGREncFmts);
			dataPane.add(BGREncFmtsValues);
		
			YUVEncFmts.setPreferredSize(labelsDim);
			BGREncFmts.setMinimumSize(labelsDim);
			dataPane.add(YUVEncFmts);
			dataPane.add(YUVEncFmtsValues);
			
			YVUEncFmts.setPreferredSize(labelsDim);
			YVUEncFmts.setMinimumSize(labelsDim);
			dataPane.add(YVUEncFmts);
			dataPane.add(YVUEncFmtsValues);
			
			JPEGEncFmts.setPreferredSize(labelsDim);
			JPEGEncFmts.setMinimumSize(labelsDim);
			dataPane.add(JPEGEncFmts);
			dataPane.add(JPEGEncFmtsValues);	
			
			input.setPreferredSize(labelsDim);
			input.setMinimumSize(labelsDim);
			dataPane.add(input);
			inputs.setMaximumSize(listDim);
			inputs.setPreferredSize(listDim);
			inputs.setSize(listDim);
			dataPane.add(inputs);

			inputType.setPreferredSize(labelsDim);
			inputType.setMinimumSize(labelsDim);
			dataPane.add(inputType);
			dataPane.add(inputTypeValue);

			tunerType.setPreferredSize(labelsDim);
			tunerType.setMinimumSize(labelsDim);
			dataPane.add(tunerType);
			dataPane.add(tunerTypeValue);

			standard.setPreferredSize(labelsDim);
			standard.setMinimumSize(labelsDim);
			dataPane.add(standard);
			standards.setMaximumSize(listDim);
			standards.setPreferredSize(listDim);
			standards.setSize(listDim);
			dataPane.add(standards);
						
			makeCompactGrid(dataPane, 11, 2, 1, 1, 0, 2);


			buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
			if(di.getFormatList().getRGBEncodableFormats().size()!=0){
				buttonPane.add(Box.createHorizontalGlue());
				buttonPane.add(rgbView);
			}
			if(di.getFormatList().getJPEGEncodableFormats().size()!=0){
				buttonPane.add(Box.createHorizontalGlue());
				buttonPane.add(jpegView);
			}
			if(di.getFormatList().getRGBEncodableFormats().size()==0 &&
					di.getFormatList().getJPEGEncodableFormats().size()==0
					){
				buttonPane.add(Box.createHorizontalGlue());
				rgbView.setText(CTRL_ONLY_STR);
				buttonPane.add(rgbView);
			}
			buttonPane.add(Box.createHorizontalGlue());
			
			mainPane.add(dataPane);
			mainPane.add(Box.createHorizontalStrut(5));
			mainPane.add(buttonPane);
			
			updateInput();
			
			inputs.addActionListener(this);
		}
		
		private void updateInput(){
			for(InputInfo i:di.getInputs()){
				if(i.getName().equals(inputs.getSelectedItem())){
					if(i.getType()==V4L4JConstants.INPUT_TYPE_CAMERA){
						inputTypeValue.setText("Camera");
						tunerTypeValue.setText("-");
					} else {
						inputTypeValue.setText("Tuner");
						try {
						tunerTypeValue.setText(
								i.getTunerInfo().getName()+" - "+
								(i.getTunerInfo().getType()==
									V4L4JConstants.TUNER_TYPE_RADIO?"Radio":"TV")
								);
						} catch (NoTunerException e){}//shouldnt happen
					}
					
					standards.removeAllItems();
					for(Integer std: i.getSupportedStandards()){
						if(std.intValue()==V4L4JConstants.STANDARD_NTSC)
							standards.addItem("NTSC");
						else if(std.intValue()==V4L4JConstants.STANDARD_PAL)
							standards.addItem("PAL");
						else if(std.intValue()==V4L4JConstants.STANDARD_SECAM)
							standards.addItem("SECAM");
						else if(std.intValue()==V4L4JConstants.STANDARD_WEBCAM)
							standards.addItem("WEBCAM");
					}
				}
			}
			dataPane.validate();
			dataPane.repaint();
		}

		public static void makeCompactGrid(Container parent,
				int rows, int cols,
				int initialX, int initialY,
				int xPad, int yPad) {
			SpringLayout layout;
			try {
				layout = (SpringLayout)parent.getLayout();
			} catch (ClassCastException exc) {
				System.err.println("The first argument to makeCompactGrid must use SpringLayout.");
				return;
			}

			//Align all cells in each column and make them the same width.
			Spring x = Spring.constant(initialX);
			for (int c = 0; c < cols; c++) {
				Spring width = Spring.constant(0);
				for (int r = 0; r < rows; r++) {
					width = Spring.max(width,
							getConstraintsForCell(r, c, parent, cols).
							getWidth());
				}
				for (int r = 0; r < rows; r++) {
					SpringLayout.Constraints constraints =
						getConstraintsForCell(r, c, parent, cols);
					constraints.setX(x);
					constraints.setWidth(width);
				}
				x = Spring.sum(x, Spring.sum(width, Spring.constant(xPad)));
			}

			//Align all cells in each row and make them the same height.
			Spring y = Spring.constant(initialY);
			for (int r = 0; r < rows; r++) {
				Spring height = Spring.constant(0);
				for (int c = 0; c < cols; c++) {
					height = Spring.max(height,
							getConstraintsForCell(r, c, parent, cols).
							getHeight());
				}
				for (int c = 0; c < cols; c++) {
					SpringLayout.Constraints constraints =
						getConstraintsForCell(r, c, parent, cols);
					constraints.setY(y);
					constraints.setHeight(height);
				}
				y = Spring.sum(y, Spring.sum(height, Spring.constant(yPad)));
			}

			//Set the parent's size.
			SpringLayout.Constraints pCons = layout.getConstraints(parent);
			pCons.setConstraint(SpringLayout.SOUTH, y);
			pCons.setConstraint(SpringLayout.EAST, x);
		}

		/* Used by makeCompactGrid. */
		private static SpringLayout.Constraints getConstraintsForCell(
				int row, int col,
				Container parent,
				int cols) {
			SpringLayout layout = (SpringLayout) parent.getLayout();
			Component c = parent.getComponent(row * cols + col);
			return layout.getConstraints(c);
		}

		@Override
		public void actionPerformed(ActionEvent e) {
			if(e.getSource() instanceof JComboBox){
				updateInput();
			} else if(e.getSource() instanceof JButton){
				JButton b = (JButton) e.getSource();
				chooser.close();
				
				//catch ctrl-only stuff
				if(b.getText().equals(CTRL_ONLY_STR))
						new ControlViewer(vd);
				else{
					
					int std=0, channel=0;
					if(((String)standards.getSelectedItem()).equals("PAL"))
						std = V4L4JConstants.STANDARD_PAL;
					else if(((String)standards.getSelectedItem()).equals("NTSC"))
						std = V4L4JConstants.STANDARD_NTSC;
					else if(((String)standards.getSelectedItem()).equals("SECAM"))
						std = V4L4JConstants.STANDARD_SECAM;
					else if(((String)standards.getSelectedItem()).equals("WEBCAM"))
						std = V4L4JConstants.STANDARD_WEBCAM;
					
					for(InputInfo i: di.getInputs())
						if(i.getName().equals((String) inputs.getSelectedItem()))
								channel = i.getIndex();
	
					if(b.getText().indexOf(RGB_BUTTON_STR)!=-1){
						try {
							new RGBViewer(vd, chooser.width, chooser.height,std, 
									channel);
						} catch (V4L4JException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					} else {
						try {
							new JPEGViewer(vd, chooser.width, chooser.height,std, 
									channel, 80);
						} catch (V4L4JException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
					}
				}
			}
		}
		
		public JPanel getPanel(){
			return mainPane;
		}
		
		public void close(){
			vd.release();
		}
	}
	

	public static void main(String[] args){
		final String dev = System.getProperty("test.device"); 
		final int w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		final int h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
 
		SwingUtilities.invokeLater(new Runnable(){

			@Override
			public void run() {
				new DeviceChooser(dev, w, h	);
			}			
		});
	}

}
