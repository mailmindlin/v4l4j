package au.edu.jcu.v4l4j.impl.omx;

import java.util.Arrays;
import java.util.List;

import au.edu.jcu.v4l4j.api.AudioEncodingType;
import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.Rational;

public class OMXConstants {
	public static final List<String> BITRATE_CONTROLS = Arrays.asList("disable", "variable", "constant", "variableSkipFrames", "constantSkipFrames");
	
	//Audio encoding types
	public static final int AUDIO_ENCODING_UNUSED = 0;
	public static final int AUDIO_ENCODING_AUTO_DETECT = 1;
	public static final int AUDIO_ENCODING_PCM = 2;
	public static final int AUDIO_ENCODING_ADPCM = 3;
	public static final int AUDIO_ENCODING_AMR = 4;
	public static final int AUDIO_ENCODING_GSMFR = 5;
	public static final int AUDIO_ENCODING_GSMEFR = 6;
	public static final int AUDIO_ENCODING_GSMHR = 7;
	public static final int AUDIO_ENCODING_PDCFR = 8;
	public static final int AUDIO_ENCODING_PDCEFR = 9;
	public static final int AUDIO_ENCODING_PDCHR = 10;
	public static final int AUDIO_ENCODING_TDMAFR = 11;
	public static final int AUDIO_ENCODING_TDMAEFR = 12;
	public static final int AUDIO_ENCODING_QCELP8 = 13;
	public static final int AUDIO_ENCODING_QCELP13 = 14;
	public static final int AUDIO_ENCODING_EVRC = 15;
	public static final int AUDIO_ENCODING_SMV = 16;
	public static final int AUDIO_ENCODING_G711 = 17;
	public static final int AUDIO_ENCODING_G723 = 18;
	public static final int AUDIO_ENCODING_G726 = 19;
	public static final int AUDIO_ENCODING_G729 = 20;
	public static final int AUDIO_ENCODING_AAC = 21;
	public static final int AUDIO_ENCODING_MP3 = 22;
	public static final int AUDIO_ENCODING_SBC = 23;
	public static final int AUDIO_ENCODING_VORBIS = 24;
	public static final int AUDIO_ENCODING_WMA = 25;
	public static final int AUDIO_ENCODING_RA = 26;
	public static final int AUDIO_ENCODING_MIDI = 27;
	//Extensions
	public static final int AUDIO_ENCODING_FLAC     = 0x7F000001;
	public static final int AUDIO_ENCODING_DPP      = 0x7F000002;
	public static final int AUDIO_ENCODING_DTS      = 0x7F000003;
	public static final int AUDIO_ENCODING_WMAPRO   = 0x7F000004;
	public static final int AUDIO_ENCODING_ATRAC3   = 0x7F000005;
	public static final int AUDIO_ENCODING_ATRACX   = 0x7F000006;
	public static final int AUDIO_ENCODING_ATRACAAL = 0x7F000007;
	
	//Video encoding types
	public static final int VIDEO_ENCODING_UNUSED = 0;
	public static final int VIDEO_ENCODING_AUTO_DETECT = 1;
	public static final int VIDEO_ENCODING_MPEG2 = 2;
	public static final int VIDEO_ENCODING_H263 = 3;
	public static final int VIDEO_ENCODING_MPEG4 = 4;
	public static final int VIDEO_ENCODING_WMV = 5;
	public static final int VIDEO_ENCODING_RV = 6;
	public static final int VIDEO_ENCODING_AVC = 7;
	public static final int VIDEO_ENCODING_MJPEG = 8;
			
	static AudioEncodingType mapAudioEncodingType(int idx) {
		switch (idx) {
			case AUDIO_ENCODING_UNUSED:
				return AudioEncodingType.UNUSED;
			case AUDIO_ENCODING_AUTO_DETECT:
				return AudioEncodingType.AUTO_DETECT;
			case AUDIO_ENCODING_PCM:
				return AudioEncodingType.PCM;
			case AUDIO_ENCODING_ADPCM:
				return AudioEncodingType.ADPCM;
			case AUDIO_ENCODING_AMR:
				return AudioEncodingType.AMR;
			case AUDIO_ENCODING_GSMFR:
				return AudioEncodingType.GSMFR;
			case AUDIO_ENCODING_GSMEFR:
				return AudioEncodingType.GSMEFR;
			case AUDIO_ENCODING_GSMHR:
				return AudioEncodingType.GSMHR;
			case AUDIO_ENCODING_PDCFR:
				return AudioEncodingType.PDCFR;
			case AUDIO_ENCODING_PDCEFR:
				return AudioEncodingType.PDCEFR;
			case AUDIO_ENCODING_PDCHR:
				return AudioEncodingType.PDCHR;
			case AUDIO_ENCODING_TDMAFR:
				return AudioEncodingType.TDMAFR;
			case AUDIO_ENCODING_TDMAEFR:
				return AudioEncodingType.TDMAEFR;
			case AUDIO_ENCODING_QCELP8:
				return AudioEncodingType.QCELP8;
			case AUDIO_ENCODING_QCELP13:
				return AudioEncodingType.QCELP13;
			case AUDIO_ENCODING_EVRC:
				return AudioEncodingType.EVRC;
			case AUDIO_ENCODING_SMV:
				return AudioEncodingType.SMV;
			case AUDIO_ENCODING_G711:
				return AudioEncodingType.G711;
			case AUDIO_ENCODING_G723:
				return AudioEncodingType.G723;
			case AUDIO_ENCODING_G726:
				return AudioEncodingType.G726;
			case AUDIO_ENCODING_G729:
				return AudioEncodingType.G729;
			case AUDIO_ENCODING_AAC:
				return AudioEncodingType.AAC;
			case AUDIO_ENCODING_MP3:
				return AudioEncodingType.MP3;
			case AUDIO_ENCODING_SBC:
				return AudioEncodingType.SBC;
			case AUDIO_ENCODING_VORBIS:
				return AudioEncodingType.VORBIS;
			case AUDIO_ENCODING_WMA:
				return AudioEncodingType.WMA;
			case AUDIO_ENCODING_RA:
				return AudioEncodingType.RA;
			case AUDIO_ENCODING_MIDI:
				return AudioEncodingType.MIDI;
			default:
				throw new IllegalArgumentException("Unknown audio encoding type: " + idx);
		}
	}
	
	static int unmapAudioEncodingType(AudioEncodingType encoding) {
		switch (encoding) {
			case UNUSED:
				return AUDIO_ENCODING_UNUSED;
			case AUTO_DETECT:
				return AUDIO_ENCODING_AUTO_DETECT;
			case PCM:
				return AUDIO_ENCODING_PCM;
			case ADPCM:
				return AUDIO_ENCODING_ADPCM;
			case AMR:
				return AUDIO_ENCODING_AMR;
			case GSMFR:
				return AUDIO_ENCODING_GSMFR;
			case GSMEFR:
				return AUDIO_ENCODING_GSMEFR;
			case GSMHR:
				return AUDIO_ENCODING_GSMHR;
			case PDCFR:
				return AUDIO_ENCODING_PDCFR;
			case PDCEFR:
				return AUDIO_ENCODING_PDCEFR;
			case PDCHR:
				return AUDIO_ENCODING_PDCHR;
			case TDMAFR:
				return AUDIO_ENCODING_TDMAFR;
			case TDMAEFR:
				return AUDIO_ENCODING_TDMAEFR;
			case QCELP8:
				return AUDIO_ENCODING_QCELP8;
			case QCELP13:
				return AUDIO_ENCODING_QCELP13;
			case EVRC:
				return AUDIO_ENCODING_EVRC;
			case SMV:
				return AUDIO_ENCODING_SMV;
			case G711:
				return AUDIO_ENCODING_G711;
			case G723:
				return AUDIO_ENCODING_G723;
			case G726:
				return AUDIO_ENCODING_G726;
			case G729:
				return AUDIO_ENCODING_G729;
			case AAC:
				return AUDIO_ENCODING_AAC;
			case MP3:
				return AUDIO_ENCODING_MP3;
			case SBC:
				return AUDIO_ENCODING_SBC;
			case VORBIS:
				return AUDIO_ENCODING_VORBIS;
			case WMA:
				return AUDIO_ENCODING_WMA;
			case RA:
				return AUDIO_ENCODING_RA;
			case MIDI:
				return AUDIO_ENCODING_MIDI;
			case FLAC:
				return AUDIO_ENCODING_FLAC;
			case ATRAC3:
				return AUDIO_ENCODING_ATRAC3;
			case ATRACAAL:
				return AUDIO_ENCODING_ATRACAAL;
			case ATRACX:
				return AUDIO_ENCODING_ATRACX;
			case DPP:
				return AUDIO_ENCODING_DPP;
			case DTS:
				return AUDIO_ENCODING_DTS;
			case WMAPRO:
				return AUDIO_ENCODING_WMAPRO;
			default:
				throw new IllegalArgumentException("OMX does not support the audio encoding '" + encoding + "'");
		}
	}
	
	static ImagePalette mapVideoEncodingType(int idx) {
		switch (idx) {
			case VIDEO_ENCODING_UNUSED:
				return null;
			case VIDEO_ENCODING_MPEG2:
				return ImagePalette.MPEG2;
			case VIDEO_ENCODING_H263:
				return ImagePalette.H263;
			case VIDEO_ENCODING_MPEG4:
				return ImagePalette.MPEG4;
			case VIDEO_ENCODING_WMV:
				return ImagePalette.WMV;
			case VIDEO_ENCODING_RV:
				return ImagePalette.RV;
			case VIDEO_ENCODING_AVC:
				return ImagePalette.AVC;
			case VIDEO_ENCODING_MJPEG:
				return ImagePalette.MJPEG;
			case VIDEO_ENCODING_AUTO_DETECT:
			default:
				throw new IllegalArgumentException("Cannot map video encoding " + idx);
		}
	}
		
	static int unmapVideoEncodingType(ImagePalette palette) {
		if (palette == null)//Can't put it in the switch
			return VIDEO_ENCODING_UNUSED;
		switch (palette) {
			case MPEG2:
				return VIDEO_ENCODING_MPEG2;
			case H263:
				return VIDEO_ENCODING_H263;
			case MPEG4:
				return VIDEO_ENCODING_MPEG4;
			case WMV:
				return VIDEO_ENCODING_WMV;
			case RV:
				return VIDEO_ENCODING_RV;
			case AVC:
				return VIDEO_ENCODING_AVC;
			case MJPEG:
				return VIDEO_ENCODING_MJPEG;
			default:
				throw new IllegalArgumentException("Cannot unmap video encoding " + palette);
		}
	}
	
	static Rational decodeQ16(int q16) {
		//TODO finish
		return null;
	}
	
	static int encodeQ16(Rational r) {
		//TODO finish
		return -1;
	}
}
