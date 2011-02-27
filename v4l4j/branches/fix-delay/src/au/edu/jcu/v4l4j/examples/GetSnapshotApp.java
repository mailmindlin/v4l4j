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
	private JFrame			frame;
	private JLabel			label;
	private JButton			button;

	private int				width, height;
	private String			device;
	private VideoDevice		videoDevice;
	private FrameGrabber	frameGrabber;
	private VideoFrame		lastVideoFrame;


	/**
	 * Start a new GetSnapshot UI
	 * @param d the path to the device file to use
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @throws V4L4JException if there is a problem capturing from the given device
	 */
	public GetSnapshotApp(String d, int w, int h) throws V4L4JException {
		device = d;
		width = w;
		height = h;
		lastVideoFrame = null;

		initFrameGrabber();
		initGUI();
	}

	private void initFrameGrabber() throws V4L4JException{
		videoDevice = new VideoDevice(device);
		frameGrabber = videoDevice.getJPEGFrameGrabber(width, height, V4L4JConstants.INPUT_TYPE_CAMERA, V4L4JConstants.STANDARD_WEBCAM, 80);
		width = frameGrabber.getWidth();
		height = frameGrabber.getHeight();
		frameGrabber.setPushSourceMode(this);
		frameGrabber.startCapture();
	}

	private void initGUI() {
		frame = new JFrame();
		frame.setLayout(new BoxLayout(frame.getContentPane(),BoxLayout.PAGE_AXIS));

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
		frame.getContentPane().add(Box.createRigidArea(new Dimension(0, 5)));
		frame.getContentPane().add(button);


		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.addWindowListener(this);

		frame.pack();
		frame.setVisible(true);
	}

	/**
	 * This method displays the current video frame.
	 */
	private void getSnapshot() {
		VideoFrame lastFrame = null;
		
		synchronized (this) {
			// If there is a current frame ...
			if (lastVideoFrame != null) {
				// Make a local copy the pointer to it,
				lastFrame = lastVideoFrame;
				
				// and set the class member to null so it does not get recycled
				// in nextFrame(), since we are going to recycle it
				// ourselves once drawn in the label.
				lastVideoFrame = null;
			}
		}
		
		// Draw the frame and recycle it
		if (lastFrame != null) {
			label.getGraphics().drawImage(lastFrame.getBufferedImage(), 0, 0, width, height, null);
			lastFrame.recycle();
		}
	}

	@Override
	public void windowClosing(WindowEvent e) {
		try {
			frameGrabber.stopCapture();
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

	public static void main(String args[]) throws V4L4JException{
		String dev = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0"; 
		int w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		int h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;

		new GetSnapshotApp(dev, w, h);
	}
}
