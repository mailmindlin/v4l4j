package au.edu.jcu.v4l4j.examples;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.PushSourceCallback;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class creates a user interface to capture and display the current image
 * from a video device.
 * @author gilles
 *
 */
public class GetSnapshotApp  extends WindowAdapter implements PushSourceCallback{
	private static int		width, height, std, channel;
	private static String	device;

	private JFrame			frame;
	private JLabel			label;
	private JButton			button;

	private VideoDevice		videoDevice;
	private FrameGrabber	frameGrabber;
	private VideoFrame		lastVideoFrame;


	public static void main(String args[]) throws V4L4JException{
		device = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0"; 
		width = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		height = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : V4L4JConstants.STANDARD_WEBCAM;
		channel = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;

		new GetSnapshotApp();
	}

	/**
	 * Start a new GetSnapshot UI
	 * @throws V4L4JException if there is a problem capturing from the given device
	 */
	public GetSnapshotApp() throws V4L4JException {
		lastVideoFrame = null;

		initFrameGrabber();
		initGUI();
	}

	/**
	 * This method creates the VideoDevice and frame grabber.
	 * It enables push mode and starts the capture
	 * @throws V4L4JException
	 */
	private void initFrameGrabber() throws V4L4JException{
		videoDevice = new VideoDevice(device);
		frameGrabber = videoDevice.getJPEGFrameGrabber(width, height, channel, std, 80);
		width = frameGrabber.getWidth();
		height = frameGrabber.getHeight();
		frameGrabber.setPushSourceMode(this);
		frameGrabber.startCapture();
	}

	/**
	 * This method builds the UI
	 */
	private void initGUI() {
		frame = new JFrame();
		frame.setLayout(new BoxLayout(frame.getContentPane(),BoxLayout.LINE_AXIS));

		label = new JLabel(width +" x " + height);
		label.setPreferredSize(new Dimension(width, height));
		label.setMaximumSize(new Dimension(width, height));
		label.setSize(new Dimension(width, height));
		label.setAlignmentX(Component.CENTER_ALIGNMENT);
		label.setAlignmentY(Component.CENTER_ALIGNMENT);

		button = new JButton("Get snapshot");       
		button.addMouseListener(new MouseAdapter() {
			public void mouseClicked(MouseEvent e) {
				getSnapshot();
			}
		});

		frame.getContentPane().add(label);
		frame.getContentPane().add(Box.createGlue());
		frame.getContentPane().add(button);


		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.addWindowListener(this);

		frame.pack();
		frame.setVisible(true);
	}

	/**
	 * This method is called when the Get snapshot button is hit and 
	 * it displays the current video frame.
	 */
	private void getSnapshot() {
		VideoFrame lastFrameCopy = null;
		
		synchronized (this) {
			// If there is a current frame ...
			if (lastVideoFrame != null) {
				// Make a local copy
				lastFrameCopy = lastVideoFrame;
				
				// and set the class member to null so it does not get recycled
				// in nextFrame(), since we are going to recycle it
				// ourselves once drawn in the label.
				lastVideoFrame = null;
			}
		}
		
		// Draw the frame and recycle it
		if (lastFrameCopy != null) {
			label.getGraphics().drawImage(lastFrameCopy.getBufferedImage(), 0, 0, width, height, null);
			lastFrameCopy.recycle();
		}
	}

	@Override
	public void windowClosing(WindowEvent e) {
		try {
			frameGrabber.stopCapture();
		} catch (Exception ex) {
			// frame grabber may be already stopped, so ignore this
		}
		
		try {
			videoDevice.releaseFrameGrabber();
			videoDevice.release();
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		frame.dispose();
	}


	@Override
	public void exceptionReceived(V4L4JException e) {
		label.setText(e.toString());
	}

	@Override
	public synchronized void nextFrame(VideoFrame frame) {
		// Recycle the previous frame if there is one
		if (lastVideoFrame != null)
			lastVideoFrame.recycle();

		// Store a pointer to this new frame
		lastVideoFrame = frame;
	}
}
