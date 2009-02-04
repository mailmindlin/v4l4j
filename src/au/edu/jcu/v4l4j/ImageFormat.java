package au.edu.jcu.v4l4j;

import java.util.List;

/**
 * Objects of this class represent an image format. <code>ImageFormat</code> are not directly
 * instanciated. Instead, they can be enumerated by instantiating a <code>VideoDevice</code>, and 
 * checking the <code>DeviceInfo</code> object associated with it:
 * <br><code>
 * <br>VideoDevice vd = new VideoDevice("/dev/video0");
 * <br>vd.init();
 * <br>List<ImageFormat> fmts = vd.getDeviceInfo().getFormats();
 * <br>for(ImageFormat im: fmts)
 * <br>&nbsp;&nbsp; System.out.println("Format name: "+im.getName()+" - Index: "+im.getIndex());
 * <br>vd.release();
 * </code>
 * 
 * @author gilles
 *
 */
public class ImageFormat {
	
	/**
	 * Native method used to populate the formats attribute
	 */
	private static native void listFormats();
	
	/**
	 * The name of this format
	 */
	private String name;
	
	/**
	 * the libv4l Id for this format
	 */
	private int libv4lID;
	
	/**
	 * List of image formats supported by libv4l
	 */
	public static List<ImageFormat> formats;
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
		listFormats();		
	}
	
	
	ImageFormat(String n, int i) {
		name = n;
		libv4lID = i;
	}

	/**
	 * This method returns the name of this image format
	 * @return the name
	 */
	public String getName() {
		return name;
	}

	/**
	 * this method returns the index for this format  
	 * @return the index of this format
	 */
	public int getIndex() {
		return libv4lID;
	}
}
