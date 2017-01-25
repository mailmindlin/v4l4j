package au.edu.jcu.v4l4j.api;

/**
 * Enum of image format palettes. Matches (or *should* match) the enum in
 * <code>libvideo.h</code>.
 * <h6>Resources</h6>
 * <dl>
 * <dt>General</dt>
 * <dd>
 * 	<ul>
 * 		<li><a href=
 * "https://linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt.html">linuxtv.org/downloads/v4l-dvb-apis/uapi/v4l/pixfmt.html</a></li>
 * 	</ul>
 * </dd>
 * <dt>YUV formats</dt>
 * <dd>
 * 	<ul>
 * 		<li><a href="https://wiki.videolan.org/YUV/">wiki.videolan.org/YUV</a></li>
 * 		<li><a href="https://en.wikipedia.org/wiki/YUV">wikipedia.org/wiki/YUV</a></li>
 * 	</ul>
 * </dd>
 * </dl>
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
	RGB332(8),
	/**
	 * RGB that fits in two bytes
	 */
	RGB444(16),
	/**
	 * <ul>
	 * <li><strong>1</strong> bit unused</li>
	 * <li><strong>5</strong> bits red</li>
	 * <li><strong>5</strong> bits green</li>
	 * <li><strong>5</strong> bits blue</li>
	 * </ul>
	 */
	RGB555(16),
	/**
	 * <ul>
	 * <li><strong>5</strong> bits red</li>
	 * <li><strong>6</strong> bits green</li>
	 * <li><strong>5</strong> bits blue</li>
	 * </ul>
	 */
	RGB565(16),
	/**
	 * Not really sure. Let me know if you figure what
	 * the difference between this and {@link #RGB555} is.
	 */
	RGB555X(16),
	/**
	 * Not really sure.
	 */
	RGB565X(16),
	/**
	 * BGR that fits in 3 bytes:
	 * <ul>
	 * <li><strong>8</strong> bits blue</li>
	 * <li><strong>8</strong> bits green</li>
	 * <li><strong>8</strong> bits red</li>
	 * </ul>
	 * @see #RGB24
	 */
	BGR24(24),
	/**
	 * RGB that fits in 3 bytes:
	 * <ul>
	 * <li><strong>3</strong> bits red</li>
	 * <li><strong>3</strong> bits green</li>
	 * <li><strong>2</strong> bits blue</li>
	 * </ul>
	 * @see #BGR24
	 */
	RGB24(24),
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
	BGR32(32),
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
	RGB32(32),

	// Gray formats
	/**
	 * Grayscale format, <strong>8</strong> bits/pixel.
	 * Also known as Y8.
	 */
	GRAY(8),
	/**
	 * Grayscale format; <strong>4</strong> bits/pixel.
	 */
	Y4(4),
	/**
	 * Grayscale format; <strong>6</strong> bits/pixel.
	 */
	Y6(6),
	/**
	 * Grayscale format; <strong>10</strong> bits/pixel.
	 */
	Y10(10),
	/**
	 * Grayscale format; <strong>16</strong> bits/pixel.
	 */
	Y16(16),
	/**
	 * Grayscale format; <strong>24</strong> bits/pixel.
	 */
	Y24(24),
	/**
	 * Grayscale format; <strong>32</strong> bits/pixel.
	 */
	Y32(32),

	// palette formats
	/**
	 * Starts with 256-entry ARGB palette.
	 * Single plane, each pixel is a 1-byte (8-bit) index into the palette.
	 */
	PAL8(8),

	// YUV formats
	/**
	 * YVU 4:1:0 planar image format.
	 * <ul>
	 * <li>1 Y plane, 1 byte/pixel</li>
	 * <li>1 Cr plane, 1 byte/16 pixels (4x4 square)</li>
	 * <li>1 Cb plane, 1 byte/16 pixels (4x4 square)</li>
	 * </ul>
	 */
	YVU410(9),
	/**
	 * YVU 4:2:0 planar image format.
	 * <ul>
	 * <li>1 Y plane, 1 byte/pixel</li>
	 * <li>1 Cr plane, 1 byte/4 pixels (2x2 square)</li>
	 * <li>1 Cb plane, 1 byte/4 pixels (2x2 square)</li>
	 * </ul>
	 */
	YVU420(12),
	/**
	 * YUV 4:2:2 interleaved image format.
	 * <table>
	 * <thead><tr><th>0</th><th>1</th><th>2</th><th>3</th></tr></thead>
	 * <tbody><tr><td>Y</td><td>U</td><td>Y</td><td>V</td></tr></tbody>
	 * </table>
	 * For each two horizontally adjacent pixels, there are 2 Y values,
	 * and 1 U/V value.
	 */
	YUYV(16),
	/**
	 * YUV 4:2:2 interleaved image format.
	 * <table>
	 * <thead><tr><th>0</th><th>1</th><th>2</th><th>3</th></tr></thead>
	 * <tbody><tr><td>Y</td><td>Y</td><td>U</td><td>V</td></tr></tbody>
	 * </table>
	 * For each two horizontally adjacent pixels, there are 2 Y values,
	 * and 1 U/V value.
	 */
	YYUV(16),
	/**
	 * YUV 4:2:2 interleaved image format.
	 * <table>
	 * <thead><tr><th>0</th><th>1</th><th>2</th><th>3</th></tr></thead>
	 * <tbody><tr><td>Y</td><td>V</td><td>Y</td><td>U</td></tr></tbody>
	 * </table>
	 * For each two horizontally adjacent pixels, there are 2 Y values,
	 * and 1 U/V value.
	 */
	YVYU(16),
	/**
	 * YUV 4:2:2 interleaved image format.
	 * <table>
	 * <thead><tr><th>0</th><th>1</th><th>2</th><th>3</th></tr></thead>
	 * <tbody><tr><td>U</td><td>Y</td><td>V</td><td>Y</td></tr></tbody>
	 * </table>
	 * For each two horizontally adjacent pixels, there are 2 Y values,
	 * and 1 U/V value.
	 */
	UYVY(16),
	/**
	 * YUV 4:2:2 interleaved image format.
	 * <table>
	 * <thead><tr><th>0</th><th>1</th><th>2</th><th>3</th></tr></thead>
	 * <tbody><tr><td>V</td><td>Y</td><td>U</td><td>Y</td></tr></tbody>
	 * </table>
	 * For each two horizontally adjacent pixels, there are 2 Y values,
	 * and 1 U/V value.
	 */
	VYUY(16),
	YUV422P(16),
	YUV411P(16),
	Y41P(12),
	YUV444(16),
	YUV555(16),
	YUV565(16),
	YUV32(32),
	YUV410(9),
	YUV420(12),
	HI240(8),
	HM12(8),

	// Two planes - Y and Cb/Cr interleaved
	/**
	 * YUV 4:2:0 semi-planar image format.
	 * <ul>
	 * <li>1 Y plane, 1 byte/pixel</li>
	 * <li>1 U/V interleaved plane, 2 bytes/4 pixels (2x2 square)</li>
	 * </ul>
	 * In the U/V interleaved plane, the U component comes first.
	 */
	NV12(12),
	/**
	 * YUV 4:2:0 semi-planar image format.
	 * <ul>
	 * <li>1 Y plane, 1 byte/pixel</li>
	 * <li>1 V/U interleaved plane, 2 bytes/4 pixels (2x2 square)</li>
	 * </ul>
	 * In the V/U interleaved plane, the V component comes first.
	 */
	NV21(12),
	/**
	 * YUV 4:2:2 semi-planar image format.
	 * <ul>
	 * <li>1 Y plane, 1 byte/pixel</li>
	 * <li>1 U/V interleaved plane, 2 bytes/2 pixels (horizontally adjacent)</li>
	 * </ul>
	 * In the U/V interleaved plane, the U component comes first.
	 */
	NV16(16),
	/**
	 * YUV 4:2:2 semi-planar image format.
	 * <ul>
	 * <li>1 Y plane, 1 byte/pixel</li>
	 * <li>1 V/U interleaved plane, 2 bytes/2 pixels (horizontally adjacent)</li>
	 * </ul>
	 * In the V/U interleaved plane, the U component comes first.
	 */
	NV61(16),

	// Bayer formats
	SBGGR8(8),
	SGBRG8(8),
	SGRBG8(8),
	SRGGB8(8),
	SBGGR10(10),
	SGBRG10(10),
	SGRBG10(10),
	SRGGB10(10),
	SGRBG10DPCM8(8),
	SBGGR16(16),

	// Compressed formats
	MJPEG,
	JPEG,
	DV,
	MPEG,
	H264,
	AVC,
	H263,
	MPEG1,
	MPEG2,
	MPEG4,
	WMV,
	RV,

	// Vendor-specific formats
	CPIA1,
	WNVA,
	SN9C10X,
	SN9C20X_I420,
	PWC1,
	PWC2,
	ET61X251,
	SPCA501,
	SPCA505,
	SPCA508,
	SPCA561,
	PAC207,
	MR97310A,
	SN9C2028,
	SQ905C,
	PJPG,
	OV511,
	OV518,
	STV0680,
	TM6000,
	CIT_YYVYUY,
	KONICA420,
	NB_SUPPORTED_PALETTES,
	/**
	 * Use {@link #YUV420}
	 */
	@Deprecated
	YUV420P(12),
	/**
	 * Use {@value #YUYV}
	 */
	@Deprecated
	YUV422(8),
	/**
	 * Use {@value #YUV411P}
	 */
	@Deprecated
	YUV411(16),
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
	
	ImagePalette(int depth) {
		this.depth = depth;
		this.index = this.ordinal();
	}
	
	ImagePalette() { 
		this(-1);
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
