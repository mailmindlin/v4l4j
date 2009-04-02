package au.edu.jcu.v4l4j;

import java.util.List;
import java.util.Vector;

/**
 * ImageFormatList objects group together a list of native {@link ImageFormat}s
 * supported by a {@link VideoDevice}. This list is obtained by calling
 * {@link #getList()}. Additionally, v4l4j can convert some native formats to 
 * RGB24, BGR24, JPEG, YUV420 and YVU420. The 
 * {@link #getRGBEncodableFormats()}, {@link #getBGREncodableFormats()}, 
 * {@link #getJPEGEncodableFormats()}, {@link #getYUVEncodableFormats()} 
 * & {@link #getYVUEncodableFormats()} methods returns the subset of native 
 * formats that can be converted to these formats. 
 * <code>ImageFormatList</code>s are not directly instantiated. Instead, you 
 * can get the list of {@link ImageFormat}s supported by a {@link VideoDevice} 
 * by calling {@link DeviceInfo#getFormatList()} on its associated 
 * {@link DeviceInfo}.
 * @author gilles
 *
 */	
public class ImageFormatList {
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	/**
	 * Native method used to populate the {@link #formats}, {@link #JPEGformats}
	 * {@link #RGBformats}, {@link #BGRformats}, {@link #YUV420formats} &
	 * {@link #YVU420formats} members, given a v4l4j_device. This method must
	 * be called while the device info interface of libvideo is checked out.
	 * @param object a JNI pointer to a struct v4l4j_device with 
	 * the device info interface of libvideo checked out
	 */
	private native void listFormats(long object);

	
	/**
	 * The list of native image formats produced by a video device.
	 */
	private List<ImageFormat> formats;
	
	/**
	 * The list of native image formats that can be JPEG encoded
	 */
	private List<ImageFormat> JPEGformats;
	
	/**
	 * The list of native image formats that can be converted to RGB24
	 */
	private List<ImageFormat> RGBformats;
	
	/**
	 * The list of native image formats that can be converted to BGR24
	 */
	private List<ImageFormat> BGRformats;
	
	/**
	 * The list of native image formats that can be converted to YUV420
	 */
	private List<ImageFormat> YUV420formats;
	
	/**
	 * The list of native image formats that can be converted to YVU420
	 */
	private List<ImageFormat> YVU420formats;
	
	/**
	 * This method builds a list of {@link ImageFormat}s. This method must
	 * be called while the device info interface of libvideo is checked out.
	 * @param o the JNI C pointer to struct v4l4j_device
	 */
	ImageFormatList(long o){
		formats = new Vector<ImageFormat>();
		JPEGformats = new Vector<ImageFormat>();
		RGBformats = new Vector<ImageFormat>();
		BGRformats = new Vector<ImageFormat>();
		YUV420formats = new Vector<ImageFormat>();
		YVU420formats = new Vector<ImageFormat>();
		listFormats(o);
		sortLists();
	}
	
	/**
	 * This method sorts the {@link #JPEGformats}, {@link #RGBformats},
	 * {@link #BGRformats}, {@link #YUV420formats} & {@link #YVU420formats} 
	 * lists, so that formats more suited to conversion are first.
	 */
	private void sortLists(){
		//sort RGBformats
		//put native formats first and libv4l_converted ones next
		moveNativeFirst(RGBformats);
		//if native RGB24 is supported, put it first
		moveToFirst(RGBformats, V4L4JConstants.IMF_RGB24);
		
		//sort BGRformats
		//if native BGR24 is supported, put it first
		moveToFirst(BGRformats, V4L4JConstants.IMF_BGR24);
		
		//sort YUV420formats
		//if native YUV420 is supported, put it first
		moveToFirst(YUV420formats, V4L4JConstants.IMF_YUV420);
		
		//sort YVU420formats
		//if native YVU420 is supported, put it first
		moveToFirst(YVU420formats, V4L4JConstants.IMF_YVU420);
		
		//sort JPEGformats
		//put native formats first and libv4l_converted ones next
		moveNativeFirst(JPEGformats);		
		//if native JPEG is supported, put it first
		moveToFirst(JPEGformats, V4L4JConstants.IMF_JPEG);
	}
	
	/**
	 * This method moves the given image format <code>format</code>
	 * in the first position of the vector.
	 * @param v the vector if image format
	 * @param format the index of the format to be moved in first position
	 */
	private void moveToFirst(List<ImageFormat> v, int format){
		for(ImageFormat i : v)
			if(i.getIndex()==format){
				v.remove(i);
				v.add(0, i);
				break;
			}
	}
	
	/**
	 * This method moves the native formats in the given vector to the beginning
	 * of the vector.
	 * @param v the vector to be sorted
	 */
	private void moveNativeFirst(List<ImageFormat> v){
		int index=0;
		for(int i=0; i<v.size();i++)
			//if not native format
			if(!formats.contains(v.get(index))){
				//put it at the end
				v.add(v.remove(index));
			} else
				index++;
	}
	
	/**
	 * This method returns the native {@link ImageFormat}s contained in this 
	 * list.
	 * @return the {@link ImageFormat}s contained in this list
	 */
	public List<ImageFormat>getList(){
		return new Vector<ImageFormat>(formats);
	}
	
	/**
	 * This method looks for a native {@link ImageFormat} in this list given 
	 * its name, and returns it, or <code>null</code> if not found.
	 * @param n the name of the <code>ImageFormat</code>.
	 * @return the <code>ImageFormat</code>, or <code>null</code> 
	 * if not found in the list.
	 */
	public ImageFormat getFormat(String n){
		for(ImageFormat f:formats)
			if(f.getName().equals(n))
				return f;
		return null;
	}
	
	/**
	 * This method looks for a native {@link ImageFormat} given its index,
	 * and returns it, or <code>null</code> if not found. A list of all known 
	 * format indexes can be found in {@link V4L4JConstants} 
	 * (V4L4JConstants.IMF_*). 
	 * @param i the index of the <code>ImageFormat</code> to look up
	 * @return the <code>ImageFormat</code> or <code>null</code> if not found.
	 * @see {@link V4L4JConstants} : the image format index constant name start
	 * with V4L4JConstants.IMF_*
	 */
	public ImageFormat getFormat(int i){
		for(ImageFormat f:formats)
			if(f.getIndex()==i)
				return f;
		return null;
	}
	
	/**
	 * This method returns a list of native {@link ImageFormat}s contained
	 * in this object, that can be converted to JPEG by v4l4j. The list is 
	 * sorted: image formats better suited for the conversion are listed first.
	 * @return a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to JPEG by v4l4j.
	 */
	public List<ImageFormat> getJPEGEncodableFormats(){
		return new Vector<ImageFormat>(JPEGformats);
	}
	
	/**
	 * This method returns a list of native {@link ImageFormat}s contained
	 * in this object, that can be converted to RGB24 by v4l4j. The list is 
	 * sorted: image formats better suited for the conversion are listed first.
	 * @return a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to RGB24 by v4l4j.
	 */
	public List<ImageFormat> getRGBEncodableFormats(){
		return new Vector<ImageFormat>(RGBformats);
	}
	
	/**
	 * This method returns a list of native {@link ImageFormat}s contained
	 * in this object, that can be converted to BGR24 by v4l4j. The list is 
	 * sorted: image formats better suited for the conversion are listed first.
	 * @return a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to BGR24 by v4l4j.
	 */
	public List<ImageFormat> getBGREncodableFormats(){
		return new Vector<ImageFormat>(BGRformats);
	}
	
	/**
	 * This method returns a list of native {@link ImageFormat}s contained
	 * in this object, that can be converted to YUV420 by v4l4j. The list is 
	 * sorted: image formats better suited for the conversion are listed first.
	 * @return a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to YUV420 by v4l4j.
	 */
	public List<ImageFormat> getYUVEncodableFormats(){
		return new Vector<ImageFormat>(YUV420formats);
	}
	
	/**
	 * This method returns a list of native {@link ImageFormat}s contained
	 * in this object, that can be converted to YVU420 by v4l4j. The list is 
	 * sorted: image formats better suited for the conversion are listed first.
	 * @return a list of {@link ImageFormat}s contained
	 * in this object, that can be converted to YVU420 by v4l4j.
	 */
	public List<ImageFormat> getYVUEncodableFormats(){
		return new Vector<ImageFormat>(YVU420formats);
	}
}

