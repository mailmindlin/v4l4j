package au.edu.jcu.v4l4j.encoder.h264;

public class X264 {
	public static final int CSP_MASK = 0x00ff;
	public static final int CSP_NONE = 0x0000;
	/**
	 * yuv 4:2:0 planar
	 */
	public static final int CSP_I420 = 0x0001;
	/**
	 * yvu 4:2:0 planar
	 */
	public static final int CSP_YV12 = 0x0002;
	/**
	 * yuv 4:2:0, with one y plane and one packed u+v
	 */
	public static final int CSP_NV12 = 0x0003;
	/**
	 * yuv 4:2:0, with one y plane and one packed v+u
	 */
	public static final int CSP_NV21 = 0x0004;
	public static final int CSP_I422 = 0x0005; /* yuv 4:2:2 planar */
	public static final int CSP_YV16 = 0x0006; /* yvu 4:2:2 planar */
	public static final int CSP_NV16 = 0x0007; /*
												 * yuv 4:2:2, with one y plane
												 * and one packed u+v
												 */
	public static final int CSP_V210 = 0x0008; /*
												 * 10-bit yuv 4:2:2 packed in 32
												 */
	public static final int CSP_I444 = 0x0009; /* yuv 4:4:4 planar */
	public static final int CSP_YV24 = 0x000a; /* yvu 4:4:4 planar */
	public static final int CSP_BGR = 0x000b; /* packed bgr 24bits */
	public static final int CSP_BGRA = 0x000c; /* packed bgr 32bits */
	public static final int CSP_RGB = 0x000d; /* packed rgb 24bits */
	public static final int CSP_MAX = 0x000e; /* end of list */
	/**
	 * the csp is vertically flipped
	 */
	public static final int CSP_VFLIP = 0x1000;
	/**
	 * the csp has a depth of 16 bits per pixel component
	 */
	public static final int CSP_HIGH_DEPTH = 0x2000;

	/* Slice type */
	/**
	 * Let x264 choose the right type
	 */
	public static final int TYPE_AUTO = 0x0000;
	public static final int TYPE_IDR = 0x0001;
	public static final int TYPE_I = 0x0002;
	public static final int TYPE_P = 0x0003;
	/**
	 * Non-disposable B-frame
	 */
	public static final int TYPE_BREF = 0x0004;
	public static final int TYPE_B = 0x0005;
	/**
	 * 
	 * IDR or I depending on b_open_gop option
	 */
	public static final int TYPE_KEYFRAME = 0x0006;
	
	//for parameter setup
	public static enum Tune {
		FILM,
		ANIMATION,
		GRAIN,
		STILL_IMAGE,
		PSNR,
		SSIM,
		FAST_DECODE,
		ZERO_LATENCY;
	}
	public static final String TUNE_FILM = "film";
	public static final String TUNE_ANIMATION = "animation";
	public static final String TUNE_GRAIN = "grain";
	public static final String TUNE_STILL_IMAGE = "stillimage";
	public static final String TUNE_PSNR = "psnr";
	public static final String TUNE_SSIM = "ssim";
	public static final String TUNE_FAST_DECODE = "fastdecode";
	public static final String TUNE_ZERO_LATENCY = "zerolatency";
	
	public static enum Preset {
		ULTRA_FAST,
		SUPER_FAST,
		VERY_FAST,
		FASTER,
		FAST,
		MEDIUM,
		SLOW,
		SLOWER,
		VERY_SLOW,
		PLACEBO;
	}
	public static final String PRESET_ULTRA_FAST = "ultrafast";
	public static final String PRESET_SUPER_FAST = "superfast";
	public static final String PRESET_VERY_FAST = "veryfast";
	public static final String PRESET_FASTER = "faster";
	public static final String PRESET_FAST = "fast";
	public static final String PRESET_MEDIUM = "medium";
	public static final String PRESET_SLOW = "slow";
	public static final String PRESET_SLOWER = "slower";
	public static final String PRESET_VERY_SLOW = "veryslow";
	public static final String PRESET_PLACEBO = "placebo";
	
	public static enum Profile {
		BASELINE,
		MAIN,
		HIGH,
		HIGH10,
		HIGH422,
		HIGH444
	}

	public static final String PROFILE_BASELINE = "baseline";
	public static final String PROFILE_MAIN = "main";
	public static final String PROFILE_HIGH = "high";
	public static final String PROFILE_HIGH10 = "high10";
	public static final String PROFILE_HIGH422 = "high422";
	public static final String PROFILE_HIGH444 = "high444";
	
	public static final int RC_CQP = 0;
	public static final int RC_CRF = 1;
	public static final int RC_ABR = 2;
	public static enum RateControlMethod {
		CQP,
		CRF,
		ABR;
	}
}
