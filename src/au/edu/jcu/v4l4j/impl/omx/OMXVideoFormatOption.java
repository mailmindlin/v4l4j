package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.Rational;

public class OMXVideoFormatOption {
	public final ImagePalette compression;
	public final ImagePalette format;
	public final Rational framerate;
	public OMXVideoFormatOption(int omxCompression, int omxFormat, int q16Framerate) {
		this.compression = OMXConstants.mapVideoEncodingType(omxCompression);
		this.format = null;//TODO finish
		this.framerate = OMXConstants.decodeQ16(q16Framerate);
	}
}
