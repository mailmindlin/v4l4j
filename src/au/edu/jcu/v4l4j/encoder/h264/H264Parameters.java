package au.edu.jcu.v4l4j.encoder.h264;

import java.io.Closeable;
import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;

import au.edu.jcu.v4l4j.V4L4JUtils;

/**
 * Wrapper for <code>x264_param_t</code>
 * 
 * @author mailmindlin
 */
public class H264Parameters implements Externalizable, Closeable {
	static {
		V4L4JUtils.loadLibrary();
	}
	
	public final long object;

	/**
	 * Allocate a x264_param_t
	 * 
	 * @return pointer
	 */
	protected static native long allocate();
	
	@Override
	public native void close();

	/**
	 * Initialize with default parameters
	 */
	public native void initDefault();
	/**
	 * Calls <code>x264_param_default_preset</code>.
	 * 
	 * <p>
	 * The same as x264_param_default, but also use the passed preset and tune
	 * to modify the default settings. (either can be NULL, which implies no
	 * preset or no tune, respectively)
	 * </p>
	 * 
	 * <p>
	 * The presets can also be indexed numerically, as in:
	 * <code>x264_param_default_preset(ptr, "3", ... )</code> with ultrafast
	 * mapping to "0" and placebo mapping to "9". This mapping may of course
	 * change if new presets are added in between, but will always be ordered
	 * from fastest to slowest.
	 * </p>
	 * <p>
	 * Warning: the speed of these presets scales dramatically. Ultrafast is a
	 * full 100 times faster than placebo! Multiple tunings can be used if
	 * separated by a delimiter in ",./-+", however multiple psy tunings cannot
	 * be used. film, animation, grain, stillimage, psnr, and ssim are psy
	 * tunings.
	 * </p>
	 * 
	 * @param preset
	 *            preset to initialize with
	 * @param tune
	 *            tune to initialize with
	 */
	protected native void initWithPreset(int preset, int tune);
	
	/**
	 * Initialize the parameters with the given preset.
	 * @param preset The preset to use
	 * @param tune The tune to use
	 * @throws NullPointerException if preset or tune are null
	 */
	public void initWithPreset(X264.Preset preset, X264.Tune tune) {
		initWithPreset(preset.ordinal(), tune.ordinal());
	}

	/**
	 * If first-pass mode is set (rc.b_stat_read == 0, rc.b_stat_write == 1),
	 * modify the encoder settings to disable options generally not useful on
	 * the first pass.
	 * 
	 */
	public native void applyFastFirstPass();
	
	public native void applyProfile(int profile);
	
	public void applyProfile(X264.Profile profile) {
		applyProfile(profile.ordinal());
	}
	
	/**
	 * Set one parameter by name. Returns 0 on success, or returns one of the
	 * following errors. note: BAD_VALUE occurs only if it can't even parse the
	 * value, numerical range is not checked until x264_encoder_open() or
	 * x264_encoder_reconfig(). value=NULL means "true" for boolean options, but
	 * is a BAD_VALUE for non-booleans.
	 * 
	 * @param object
	 * @param name
	 * @param value
	 * @return
	 */
	protected native int setParamByName(String name, boolean value);
//See stackoverflow.com/a/2940700 for more details
	//TODO javadoc
	protected native int setParamByName(String name, int value);
	protected native int setParamByName(String name, String value);
	public native void setInputDimension(int width, int height);
	public native void setCsp(int csp);
	public native void setVfrInput(boolean aflag);
	public native void setRepeatHeaders(boolean aflag);
	public native void setAnnexb(boolean aflag);
	public native void setThreads(int numThreads);
	public native void setFPS(int numerator, int denominator);
	public native void setKeyintMax(int value);
	public native void setIntraRefresh(boolean value);
	public native void setRfConstant(float value, float max);
	public native void setRcMethod(int method);
	public native int getCsp();
	public native int getWidth();
	public native int getHeight();
	
	public void setRcMethod(X264.RateControlMethod method) {
		setRcMethod(method.ordinal());
	}
	public H264Parameters() {
		this(allocate());
	}
	
	protected H264Parameters(long ptr) {
		this.object = ptr;
	}

	public native void setCrop(int left, int top, int right, int bottom);

	@Override
	public void readExternal(ObjectInput arg0) throws IOException, ClassNotFoundException {
		// TODO finish
	}

	@Override
	public void writeExternal(ObjectOutput arg0) throws IOException {
		// TODO Auto-generated method stub
		
	}

	public class CropRectangle {

	}
}
