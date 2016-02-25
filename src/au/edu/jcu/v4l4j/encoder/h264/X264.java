package au.edu.jcu.v4l4j.encoder.h264;

public class X264 {
	public static final int X264_CSP_MASK = 0x00ff;
	public static final int X264_CSP_NONE = 0x0000;
	public static final int X264_CSP_I420 = 0x0001;/* yuv 4:2:0 planar */
	public static final int X264_CSP_YV12 = 0x0002;/* yvu 4:2:0 planar */
	public static final int X264_CSP_NV12 = 0x0003; /*
													 * yuv 4:2:0, with one y
													 * plane and one packed u+v
													 */
	public static final int X264_CSP_NV21 = 0x0004; /*
													 * yuv 4:2:0, with one y
													 * plane and one packed v+u
													 */
	public static final int X264_CSP_I422 = 0x0005; /* yuv 4:2:2 planar */
	public static final int X264_CSP_YV16 = 0x0006; /* yvu 4:2:2 planar */
	public static final int X264_CSP_NV16 = 0x0007; /*
													 * yuv 4:2:2, with one y
													 * plane and one packed u+v
													 */
	public static final int X264_CSP_V210 = 0x0008; /*
													 * 10-bit yuv 4:2:2 packed
													 * in 32
													 */
	public static final int X264_CSP_I444 = 0x0009; /* yuv 4:4:4 planar */
	public static final int X264_CSP_YV24 = 0x000a; /* yvu 4:4:4 planar */
	public static final int X264_CSP_BGR = 0x000b; /* packed bgr 24bits */
	public static final int X264_CSP_BGRA = 0x000c; /* packed bgr 32bits */
	public static final int X264_CSP_RGB = 0x000d; /* packed rgb 24bits */
	public static final int X264_CSP_MAX = 0x000e; /* end of list */
	/**
	 * the csp is vertically flipped
	 */
	public static final int X264_CSP_VFLIP = 0x1000;
	/**
	 * the csp has a depth of
	 * 16 bits per pixel
	 * component
	 */
	public static final int X264_CSP_HIGH_DEPTH = 0x2000;

	/* Slice type */
	public static final int X264_TYPE_AUTO = 0x0000; /*
														 * Let x264 choose the
														 * right type
														 */
	public static final int X264_TYPE_IDR = 0x0001;
	public static final int X264_TYPE_I = 0x0002;
	public static final int X264_TYPE_P = 0x0003;
	public static final int X264_TYPE_BREF = 0x0004; /*
														 * Non-disposable B-frame
														 */
	public static final int X264_TYPE_B = 0x0005;
	/**
	 * 
	 * IDR or I depending on b_open_gop option
	 */
	public static final int X264_TYPE_KEYFRAME = 0x0006;
}
