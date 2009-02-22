package au.edu.jcu.v4l4j;

import java.util.List;
import java.util.Vector;

/**
 * ImageFormatList objects group together a list of {@link ImageFormat}s
 * supported by a {@link VideoDevice}. <code>ImageFormatList</code>s are
 * not directly instantiated. Instead, you can get the list of {@link ImageFormat}s
 * supported by a {@link VideoDevice} by calling {@link DeviceInfo#getFormatList()}
 * on its associated {@link DeviceInfo}.
 * Additionally, <code>ImageFormatList</code> also provides the list of all
 * {@link ImageFormat}s supported by v4l4j ({@link #getKnownFormats()}, and the list
 * of all {@link ImageFormat}s that v4l4j can JPEG-encode 
 * ({@link #getKnownJPEGEncodableFormats()}).
 * @author gilles
 *
 */	
public class ImageFormatList {
	
	/**
	 * Native method used to populate the knowFormats 
	 * and knownJPEGEncFormats attributes
	 */
	private static native void listFormats();
	
	/**
	 * this is a static list of image formats supported by v4l4j
	 */
	private static List<ImageFormat> knownFormats;
	
	/**
	 * this is a static list of image formats supported by v4l4j
	 * that CAN be JPEG encoded
	 */
	private static List<ImageFormat> knownJPEGEncFormats;
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
		listFormats();
	}
	
	/**
	 * The image formats in this list
	 */
	private List<ImageFormat> formats;
	
	/**
	 * The image formats in this list that can be jpeg encoded
	 */
	private List<ImageFormat> JPEGformats;
	
	/**
	 * This method builds a list of {@link ImageFormat}s.
	 * @param l a list of <code>ImageFormat</code>s.
	 */
	ImageFormatList(List<ImageFormat> l){
		formats = new Vector<ImageFormat>(l);
		JPEGformats = new Vector<ImageFormat>();
		for(ImageFormat f:formats)
			if(knownJPEGEncFormats.contains(f))
				JPEGformats.add(f);
	}
	
	/**
	 * This method returns the {@link ImageFormat}s
	 * contained in this list
	 * @return the {@link ImageFormat}s
	 * contained in this list
	 */
	public List<ImageFormat>getList(){
		return new Vector<ImageFormat>(formats);
	}
	
	/**
	 * This method looks for an {@link ImageFormat} in
	 * this list given its name, and returns it, or <code>null</code>
	 * if not found.
	 * @param n the name of the <code>ImageFormat</code>.
	 * @return the <code>ImageFormat</code>, or <code>null</code> if not found in the list.
	 */
	public ImageFormat getFormat(String n){
		for(ImageFormat f:formats)
			if(f.getName().equals(n))
				return f;
		return null;
	}
	
	/**
	 * This method looks for an {@link ImageFormat} given its index,
	 * and returns it, or <code>null</code> if not found.
	 * @param i the index of the <code>ImageFormat</code> to look up
	 * @return the <code>ImageFormat</code> or <code>null</code> if not found.
	 */
	public ImageFormat getFormat(int i){
		for(ImageFormat f:formats)
			if(f.getIndex()==i)
				return f;
		return null;
	}
	
	/**
	 * This method returns a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to JPEG by v4l4j.
	 * @return a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to JPEG by v4l4j.
	 */
	public List<ImageFormat> getJPEGEncodableFormats(){
		return new Vector<ImageFormat>(JPEGformats);
	}	
	
	/**
	 * This methods return a list of all the <code>ImageFormat</code>
	 * known to v4l4j that can be JPEG-encoded.
	 * @return a list of all the <code>ImageFormat</code> known to v4l4j
	 * that can be JPEg-encoded
	 */
	public static List<ImageFormat> getKnownJPEGEncodableFormats(){
		return new Vector<ImageFormat>(knownJPEGEncFormats);
	}

	/**
	 * This methods return a list of all the <code>ImageFormat</code>
	 * known to v4l4j.
	 * @return a list of all the <code>ImageFormat</code> known to v4l4j
	 */
	public static List<ImageFormat> getKnownFormats(){
		return new Vector<ImageFormat>(knownFormats);
	}
}

