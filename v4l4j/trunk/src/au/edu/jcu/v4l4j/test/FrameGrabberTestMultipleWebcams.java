package au.edu.jcu.v4l4j.test;

import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberTestMultipleWebcams {
	private FrameGrabber fg1;
	private FrameGrabber fg2;
	private int initialQuality = 80;

	@Before
	public void setUp() throws Exception {
		try {
			fg1 = new FrameGrabber("/dev/video0", 320, 240, 0, 0, initialQuality);
			fg2 = new FrameGrabber("/dev/video1", 320, 240, 0, 0, initialQuality);
		} catch (V4L4JException e) {
			fail("Cant create a framegrabbers, make sure two webcams are connected and the drivers are loaded.");
			throw e;
		}
		
		try{
			fg1.init();
			fg2.init();
		} catch (V4L4JException e) {
			fail("Cant init the framegrabbers, make sure two webcams are connected and the drivers are loaded.");
			throw e;
		}
	}

	@After
	public void tearDown() throws Exception {
		fg1.remove();
		fg2.remove();
	}
	
	@Test
	public void testDoNothing() {
	}

	@Test
	public void testStartCapture() {
		try {
			fg1.startCapture();
		} catch (V4L4JException e) {
			fail("Failed to start capture on fg1");
		}
		
		try {
			fg2.startCapture();
		} catch (V4L4JException e) {
			fail("Failed to start capture");
		}
		
		
		try {
			fg1.stopCapture();
		} catch (V4L4JException e) {
			fail("failed to stop capture");
		}
		
		try {
			fg2.stopCapture();
		} catch (V4L4JException e) {
			fail("Failed to start capture");
		}
	}

	@Test
	public void testGetFrame(){
		try {
			fg1.startCapture();
			fg2.startCapture();
			fg1.getFrame();
			fg2.getFrame();
			fg2.getFrame();
			fg2.getFrame();
			fg1.getFrame();
			fg2.stopCapture();
			fg1.stopCapture();
		} catch (V4L4JException e) {
			fail();
		}
	}
	
	@Test
	public void testListControls(){
		Control[] v1 = fg1.getControls(), v2 = fg2.getControls();
		System.out.println("Listing controls:");
		for(int i=0; i<v1.length;i++)
			System.out.println("Control "+v1[i].getName());
		for(int i=0; i<v2.length;i++)
			System.out.println("Control "+v2[i].getName());
	}
	
	@Test
	public void testMultipleOpen() throws V4L4JException {
		fg1.remove();
		fg1.init();
		fg1.startCapture();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
		fg1.remove();
		fg1.init();
		fg1.startCapture();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
	}

}
