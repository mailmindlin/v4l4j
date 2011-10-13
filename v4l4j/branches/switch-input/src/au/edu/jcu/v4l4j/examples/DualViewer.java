/*
* Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class creates a JFrame with two JLabels, and displays streams from
 * two VideoDevices
 * @author gilles
 *
 */
public class DualViewer extends WindowAdapter implements CaptureCallback {

	private JFrame frame;
	private JPanel panel;
	
	private FrameGrabber frameGrabberLeft, frameGrabberRight;
	private VideoDevice videoDevLeft, videoDevRight;
	private JLabel labelLeft, labelRight;
	private int widthLeft, heightLeft;
	private int widthRight, heightRight;

	public static void main(String[] args) throws V4L4JException, IOException {

		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				new DualViewer("/dev/video0","/dev/video1", 640, 480, V4L4JConstants.STANDARD_WEBCAM, 0, 80);
			}
		});
	}
	
	/**
	 * Builds a DualViewer object
	 * @param devLeft the video device file to capture from, which will be displayed on the left
	 * @param devRight the video device file to capture from, which will be displayed on the right  
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param std the capture standard
	 * @param channel the capture channel
	 * @param qty the JPEG compression quality
	 */
	public DualViewer(String devLeft, String devRight, int w, int h, int std, int channel, int qty){
		initGUI(w, h);
		
		try {
			initFrameGrabber(devLeft, devRight, w, h, std, channel, qty);
		} catch (Exception e) {
			System.err.println("Error starting the capture on one of the video devices");
			e.printStackTrace();
			windowClosing(null);
		}
	}

	/** 
	 * Creates the graphical interface components and initialises them
	 */
	private void initGUI(int w, int h){
		frame = new JFrame();
		panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.LINE_AXIS));
		
		labelLeft = new JLabel();
		labelRight = new JLabel();
		
		labelLeft.setPreferredSize(new Dimension(w, h));
		labelRight.setPreferredSize(new Dimension(w, h));
		
		panel.add(labelLeft);
		panel.add(labelRight);
		
		frame.getContentPane().add(panel);
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.addWindowListener(this);
		
		frame.pack();
		frame.setVisible(true);   
	}

	/**
	 * Initialise two FrameGrabber objects with the given parameters
	 * @param devLeft the video device file to capture from
	 * @param devRight the video device file to capture from
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param std the capture standard
	 * @param channel the capture channel
	 * @param qty the JPEG compression quality
	 * @throws V4L4JException if any parameter if invalid
	 */
	private void initFrameGrabber(String devLeft, String devRight, int w, int h, int std, int channel, int qty) throws V4L4JException{
		videoDevLeft = new VideoDevice(devLeft);
		videoDevRight = new VideoDevice(devRight);
		
		frameGrabberLeft = videoDevLeft.getJPEGFrameGrabber(w, h, channel, std, qty);
		frameGrabberRight = videoDevRight.getJPEGFrameGrabber(w, h, channel, std, qty);
		
		frameGrabberLeft.setCaptureCallback(this);
		frameGrabberRight.setCaptureCallback(this);
		
		frameGrabberLeft.startCapture();
		frameGrabberRight.startCapture();
		
		widthLeft = frameGrabberLeft.getWidth();
		heightLeft = frameGrabberLeft.getHeight();
		widthRight = frameGrabberRight.getWidth();
		heightRight = frameGrabberRight.getHeight();
		
		System.out.println("Starting capture at:\n" +
				"Left: "+widthLeft+"x"+heightLeft+
				"\nRight: "+widthRight+"x"+heightRight);
		
		frame.setSize(widthLeft+widthRight , heightLeft);
		
		labelLeft.setMaximumSize(new Dimension(widthLeft, heightLeft));
		labelRight.setMaximumSize(new Dimension(widthRight, heightRight));
		labelLeft.setBorder(BorderFactory.createLineBorder(Color.RED));
		labelRight.setBorder(BorderFactory.createLineBorder(Color.RED));
	}

	/**
	 * Stop the capture and release the frame grabbers
	 */
	private void releaseVideo() {
		try {
			frameGrabberLeft.stopCapture();
		} catch (Exception e){}
		
		try {
			frameGrabberRight.stopCapture();
		} catch (Exception e) {}
		
		try {
			videoDevLeft.releaseFrameGrabber();
		} catch (Exception e){}
		
		try {
			videoDevRight.releaseFrameGrabber();
		} catch (Exception e){}
	}
	/**
	 * Catch window closing event so we can free up resources before exiting
	 * @param e
	 */
	public void windowClosing(WindowEvent e) {	
		releaseVideo();
		frame.dispose();            
	}

	@Override
	public void nextFrame(VideoFrame frame) {
		if (frame.getFrameGrabber().equals(frameGrabberLeft))
			labelLeft.getGraphics().drawImage(frame.getBufferedImage(), 0, 0, widthLeft, heightLeft, null);
		else
			labelRight.getGraphics().drawImage(frame.getBufferedImage(), 0, 0, widthRight, heightRight, null);
		
		frame.recycle();
	}

	@Override
	public void exceptionReceived(V4L4JException e) {
		JOptionPane.showMessageDialog(frame, "Error while capturing:\n"+e.toString());
	}
}

