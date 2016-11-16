package au.edu.jcu.v4l4j;

import static au.edu.jcu.v4l4j.V4L4JConstants.*;

/**
 * Enum of image format palettes. Matches (or *should* match) the enum in
 * <code>libvideo.h</code>.
 * <p>
 * See <a href=
 * "https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt.html">linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt.html</a>
 * for help.
 * </p>
 * 
 * @author mailmindlin
 */
public enum ImagePalette {
	// RGB formats
	/**
	 * RGB that fits in a byte:
	 * <ul>
	 * <li><strong>3</strong> bits red</li>
	 * <li><strong>3</strong> bits green</li>
	 * <li><strong>2</strong> bits blue</li>
	 * </ul>
	 * @see <a href="http://unwind.se/bgr233/">unwind.se/bgr233</a>
	 */
	RGB332(8, IMF_RGB332),
	/**
	 * RGB that fits in two bytes
	 */
	RGB444(16, IMF_RGB444),
	/**
	 * <ul>
	 * <li><strong>1</strong> bit unused</li>
	 * <li><strong>5</strong> bits red</li>
	 * <li><strong>5</strong> bits green</li>
	 * <li><strong>5</strong> bits blue</li>
	 * </ul>
	 */
	RGB555(16, IMF_RGB555),
	/**
	 * <ul>
	 * <li><strong>5</strong> bits red</li>
	 * <li><strong>6</strong> bits green</li>
	 * <li><strong>5</strong> bits blue</li>
	 * </ul>
	 */
	RGB565(16, IMF_RGB565),
	/**
	 * Not really sure. Let me know if you figure what
	 * the difference between this and {@link #RGB555} is.
	 */
	RGB555X(16, IMF_RGB555X),
	/**
	 * Not really sure.
	 */
	RGB565X(16, IMF_RGB565X),
	/**
	 * BGR that fits in 3 bytes:
	 * <ul>
	 * <li><strong>8</strong> bits blue</li>
	 * <li><strong>8</strong> bits green</li>
	 * <li><strong>8</strong> bits red</li>
	 * </ul>
	 * @see #RGB24
	 */
	BGR24(24, IMF_BGR24),
	/**
	 * RGB that fits in 3 bytes:
	 * <ul>
	 * <li><strong>3</strong> bits red</li>
	 * <li><strong>3</strong> bits green</li>
	 * <li><strong>2</strong> bits blue</li>
	 * </ul>
	 * @see #BGR24
	 */
	RGB24(24, IMF_RGB24),
	/**
	 * BGR that fits in 4 bytes (contains alpha channel):
	 * <ul>
	 * <li><strong>8</strong> bits blue</li>
	 * <li><strong>8</strong> bits green</li>
	 * <li><strong>8</strong> bits red</li>
	 * <li><strong>8</strong> bits alpha</li>
	 * </ul>
	 * TODO make sure that the alpha channel is in the right place
	 * @see #RGB32
	 */
	BGR32(32, IMF_BGR32),
	/**
	 * RGB that fits in 4 bytes (contains alpha channel):
	 * <ul>
	 * <li><strong>8</strong> bits red</li>
	 * <li><strong>8</strong> bits green</li>
	 * <li><strong>8</strong> bits blue</li>
	 * <li><strong>8</strong> bits alpha</li>
	 * </ul>
	 * @see #BGR32
	 */
	RGB32(32, IMF_RGB32),

	// Gray formats
	/**
	 * Grayscale format, <strong>8</strong> bits/pixel.
	 * Also known as Y8.
	 */
	GRAY(8, IMF_GRAY),
	/**
	 * Grayscale format; <strong>4</strong> bits/pixel.
	 */
	Y4(4, IMF_Y4),
	/**
	 * Grayscale format; <strong>6</strong> bits/pixel.
	 */
	Y6(6, IMF_Y6),
	/**
	 * Grayscale format; <strong>10</strong> bits/pixel.
	 */
	Y10(10, IMF_Y10),
	/**
	 * Grayscale format; <strong>16</strong> bits/pixel.
	 */
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
	YUV411(16, 77),
	/**
	 * A known format that isn't on this list.
	 * Contrasts with null, which should be returned for unknown formats
	 * that are may be on this list.
	 */
	OTHER_KNOWN(-1, -1);
	
	public static ImagePalette lookup(int palette) {
		if (palette < 0 || palette > ImagePalette.values().length) {
			System.err.println("Unknown palette ID " + palette + ". Please report this problem to .");
			return null;
		}
		return ImagePalette.values()[palette];
	}
	
	/**
	 * The color depth for this palette
	 */
	private final int depth;
	
	/**
	 * The libVideo index of this palette. Pretty much the same as this's {@link #ordinal()}.
	 */
	private final int index;
	
	ImagePalette(int depth, int index) {
		this.depth = depth;
		this.index = index;
	}
	
	/**
	 * Get the color depth for this palette. Basically the width of 1 pixel.
	 * Will be invalid for color formats that are compressed.
	 * @return this palette's color depth, or -1 if not applicable
	 */
	public int getColorDepth() {
		return this.depth;
	}
	
	/**
	 * @return the libVideo index of this palette.
	 */
	public int getIndex() {
		return index;
	}
}
