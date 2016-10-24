package au.edu.jcu.v4l4j;

import static au.edu.jcu.v4l4j.V4L4JConstants.*;

public enum ImagePalette {
	// RGB formats
	RGB332(8, IMF_RGB332),
	RGB444(16, IMF_RGB444),
	RGB555(16, IMF_RGB555),
	RGB565(16, IMF_RGB565),
	RGB555X(16, IMF_RGB555X),
	RGB565X(16, IMF_RGB565X),
	BGR24(24, IMF_BGR24),
	RGB24(24, IMF_RGB24),
	BGR32(32, IMF_BGR32),
	RGB32(32, IMF_RGB32),

	// Grey formats
	GREY(8, IMF_GREY),
	Y4(4, IMF_Y4),
	Y6(6, IMF_Y6),
	Y10(10, IMF_Y10),
	Y16(16, IMF_Y16),

	// palette formats
	PAL8(8, IMF_PAL8),

	// YUV formats
	YVU410(9, IMF_YVU410),
	YVU420(12, IMF_YVU420),
	YUYV(16, IMF_YUYV),
	YYUV(16, IMF_YYUV),
	YVYU(16, IMF_YVYU),
	UYVY(16, IMF_UYVY),
	VYUY(16, IMF_VYUY),
	YUV422P(16, IMF_YUV422P),
	YUV411P(16, IMF_YUV411P),
	Y41P(12, IMF_Y41P),
	YUV444(16, IMF_YUV444),
	YUV555(16, IMF_YUV555),
	YUV565(16, IMF_YUV565),
	YUV32(32, IMF_YUV32),
	YUV410(9, IMF_YUV410),
	YUV420(12, IMF_YUV420),
	HI240(8, IMF_HI240),
	HM12(8, IMF_HM12),

	// Two planes - Y and Cb/Cr interleaved
	NV12(12, IMF_NV12),
	NV21(12, IMF_NV21),
	NV16(16, IMF_NV16),
	NV61(16, IMF_NV61),

	// Bayer formats
	SBGGR8(8, IMF_SBGGR8),
	SGBRG8(8, IMF_SGBRG8),
	SGRBG8(8, IMF_SGRBG8),
	SRGGB8(8, IMF_SRGGB8),
	SBGGR10(10, IMF_SBGGR10),
	SGBRG10(10, IMF_SGBRG10),
	SGRBG10(10, IMF_SGRBG10),
	SRGGB10(10, IMF_SRGGB10),
	SGRBG10DPCM8(8, IMF_SGRBG10DPCM8),
	SBGGR16(16, IMF_SBGGR16),

	// Compressed formats
	MJPEG(-1, IMF_MJPEG),
	JPEG(-1, IMF_JPEG),
	DV(-1, IMF_DV),
	MPEG(-1, IMF_MPEG),

	// Vendor-specific formats
	CPIA1(-1, IMF_CPIA1),
	WNVA(-1, IMF_WNVA),
	SN9C10X(-1, IMF_SN9C10X),
	SN9C20X_I420(-1, IMF_SN9C20X_I420),
	PWC1(-1, IMF_PWC1),
	PWC2(-1, IMF_PWC2),
	ET61X251(-1, IMF_ET61X251),
	SPCA501(-1, IMF_SPCA501),
	SPCA505(-1, IMF_SPCA505),
	SPCA508(-1, IMF_SPCA508),
	SPCA561(-1, IMF_SPCA561),
	PAC207(-1, IMF_PAC207),
	MR97310A(-1, IMF_MR97310A),
	SN9C2028(-1, IMF_SN9C2028),
	SQ905C(-1, IMF_SQ905C),
	PJPG(-1, IMF_PJPG),
	OV511(-1, IMF_OV511),
	OV518(-1, IMF_OV518),
	STV0680(-1, IMF_STV0680),
	TM6000(-1, IMF_TM6000),
	CIT_YYVYUY(-1, IMF_CIT_YYVYUY),
	KONICA420(-1, IMF_KONICA420),
	NB_SUPPORTED_PALETTES(-1, 74),
	/**
	 * Use {@link #YUV420}
	 */
	@Deprecated
	YUV420P(12, 75),
	/**
	 * Use {@value #YUYV}
	 */
	@Deprecated
	YUV422(8, 76),
	/**
	 * Use {@value #YUV411P}
	 */
	@Deprecated
	YUV411(16, 77);
	
	public static ImagePalette lookup(int palette) {
		if (palette < 0 || palette > ImagePalette.values().length)
			return null;
		return ImagePalette.values()[palette];
	}
	
	protected final int depth, index;
	
	ImagePalette(int depth, int index) {
		this.depth = depth;
		this.index = index;
	}
	
	public int getColorDepth() {
		return this.depth;
	}
	
	public int getIndex() {
		return index;
	}
}
