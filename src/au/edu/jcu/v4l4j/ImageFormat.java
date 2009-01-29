package au.edu.jcu.v4l4j;

import java.util.List;

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
	
	
	public ImageFormat(String n, int i) {
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
	 * this method returns the libv4l ID for this format  
	 * @return the libv4l ID
	 */
	public int getLibv4lID() {
		return libv4lID;
	}
	
	public static void main(String[] args){
		for(ImageFormat i: formats)
			System.out.println(i.getName() + " - "+i.getLibv4lID());
	}
}
