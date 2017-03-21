package au.edu.jcu.v4l4j.impl.omx;

import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import au.edu.jcu.v4l4j.api.AudioEncodingType;
import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.Rational;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;
import au.edu.jcu.v4l4j.impl.jni.UnionPrototype;

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
	public static final int AUDIO_ENCODING_FLAC	 = 0x7F000001;
	public static final int AUDIO_ENCODING_DPP	  = 0x7F000002;
	public static final int AUDIO_ENCODING_DTS	  = 0x7F000003;
	public static final int AUDIO_ENCODING_WMAPRO   = 0x7F000004;
	public static final int AUDIO_ENCODING_ATRAC3   = 0x7F000005;
	public static final int AUDIO_ENCODING_ATRACX   = 0x7F000006;
	public static final int AUDIO_ENCODING_ATRACAAL = 0x7F000007;
	
	//Image encoding types
	public static final int IMAGE_ENCODING_UNUSED = 0;
	public static final int IMAGE_ENCODING_AUTO_DETECT = 1;
	public static final int IMAGE_ENCODING_JPEG = 2;
	public static final int IMAGE_ENCODING_JPEG2K = 3;
	public static final int IMAGE_ENCODING_EXIF = 4;
	public static final int IMAGE_ENCODING_TIFF = 5;
	public static final int IMAGE_ENCODING_GIF = 6;
	public static final int IMAGE_ENCODING_PNG = 7;
	public static final int IMAGE_ENCODING_LZW = 8;
	public static final int IMAGE_ENCODING_BMP = 9;
	
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
	public static final int VIDEO_ENCODING_VP6 = 0x7F000001;
	public static final int VIDEO_ENCODING_VP7 = 0x7F000002;
	public static final int VIDEO_ENCODING_VP8 = 0x7F000003;
	public static final int VIDEO_ENCODING_YUV = 0x7F000004;
	public static final int VIDEO_ENCODING_SORENSON = 0x7F000005;
	public static final int VIDEO_ENCODING_THEORA = 0x7F000006;
	public static final int VIDEO_ENCODING_MVC = 0x7F000007;
	
	public static final int COLOR_FORMAT_UNUSED			= 0;
	public static final int COLOR_FORMAT_MONOCHROME		= 1;
	public static final int COLOR_FORMAT_8BITRGB332		= 2;
	public static final int COLOR_FORMAT_12BITRGB444	= 3;
	public static final int COLOR_FORMAT_16BITARGB4444	= 4;
	public static final int COLOR_FORMAT_16BITARGB1555	= 5;
	public static final int COLOR_FORMAT_16BITRGB565	= 6;
	public static final int COLOR_FORMAT_16BITBGR565	= 7;
	public static final int COLOR_FORMAT_18BITRGB666	= 8;
	public static final int COLOR_FORMAT_18BITARGB1665	= 9;
	public static final int COLOR_FORMAT_19BITARGB1666	= 10;
	public static final int COLOR_FORMAT_24BITRGB888	= 11;
	public static final int COLOR_FORMAT_24BITBGR888	= 12;
	public static final int COLOR_FORMAT_24BITARGB1887	= 13;
	public static final int COLOR_FORMAT_25BITARGB1888	= 14;
	public static final int COLOR_FORMAT_32BITBGRA8888	= 15;
	public static final int COLOR_FORMAT_32BITARGB8888	= 16;
	public static final int COLOR_FORMAT_YUV411PLANAR	= 17;
	public static final int COLOR_FORMAT_YUV411PACKEDPLANAR = 18;
	public static final int COLOR_FORMAT_YUV420PLANAR	= 19;
	public static final int COLOR_FORMAT_YUV420PACKEDPLANAR = 20;
	public static final int COLOR_FORMAT_YUV420SEMIPLANAR = 21;
	public static final int COLOR_FORMAT_YUV422PLANAR	= 22;
	public static final int COLOR_FORMAT_YUV422PACKEDPLANAR = 23;
	public static final int COLOR_FORMAT_YUV422SEMIPLANAR = 24;
	public static final int COLOR_FORMAT_YCBYCR			= 25;
	public static final int COLOR_FORMAT_YCRYCB			= 26;
	public static final int COLOR_FORMAT_CBYCRY			= 27;
	public static final int COLOR_FORMAT_CRYCBY			= 28;
	public static final int COLOR_FORMAT_YUV444INTERLEAVED = 29;
	public static final int COLOR_FORMAT_RAWBAYER8BIT	= 30;
	public static final int COLOR_FORMAT_RAWBAYER10BIT	= 31;
	public static final int COLOR_FORMAT_RAWBAYER8BITCOMPRESSED = 32;
	public static final int COLOR_FORMAT_L2				= 33;
	public static final int COLOR_FORMAT_L4				= 34;
	public static final int COLOR_FORMAT_L8				= 35;
	public static final int COLOR_FORMAT_L16			= 36;
	public static final int COLOR_FORMAT_L24			= 37;
	public static final int COLOR_FORMAT_L32			= 38;
	public static final int COLOR_FORMAT_YUV420PACKEDSEMIPLANAR = 39;
	public static final int COLOR_FORMAT_YUV422PACKEDSEMIPLANAR = 40;
	public static final int COLOR_FORMAT_18BITBGR666	= 41;
	public static final int COLOR_FORMAT_24BITARGB6666	= 42;
	public static final int COLOR_FORMAT_24BITABGR6666	= 43;
	//public static final int COLOR_FORMAT_KHRONOSEXTENSIONS = 0x6F000000; /**< RESERVED REGION FOR INTRODUCING KHRONOS STANDARD EXTENSIONS */ 
	//public static final int COLOR_FORMAT_VENDORSTARTUNUSED = 0x7F000000; /**< RESERVED REGION FOR INTRODUCING VENDOR EXTENSIONS */
	public static final int COLOR_FORMAT_32BITABGR8888	= 0x7F000001;
	public static final int COLOR_FORMAT_8BITPALETTE	= 0x7F000002;
	public static final int COLOR_FORMAT_YUVUV128		= 0x7F000003;
	public static final int COLOR_FORMAT_RAWBAYER12BIT	= 0x7F000004;
	public static final int COLOR_FORMAT_BRCMEGL		= 0x7F000005;
	public static final int COLOR_FORMAT_BRCMOPAQUE		= 0x7F000006;
	public static final int COLOR_FORMAT_YVU420PACKEDPLANAR	= 0x7F000007;
	public static final int COLOR_FORMAT_YVU420PACKEDSEMIPLANAR	= 0x7F000008;
	public static final int COLOR_FORMAT_RAWBAYER16BIT	= 0x7F000009;
	//public static final int COLOR_FORMAT_MAX = 0X7FFFFFFF;
	
	//Error codes
	public static final int ERROR_NONE = 0;
	public static final int ERROR_INSUFFICIENT_RESOURCES = 1;
	public static final int ERROR_INVALID_COMPONENT_NAME = 2;
	public static final int ERROR_COMPONENT_NOT_FOUND = 3;
	public static final int ERROR_NO_MORE = 4;
	
	//Config/param indicies
	public static final int INDEX_ComponentStartUnused				= 0x01000000;
	public static final int INDEX_ParamPriorityMgmt					= 0x01000001;	/**< reference: OMX_PRIORITYMGMTTYPE */
	public static final int INDEX_ParamAudioInit					= 0x01000002;	/**< reference: OMX_PORT_PARAM_TYPE */
	public static final int INDEX_ParamImageInit					= 0x01000003;	/**< reference: OMX_PORT_PARAM_TYPE */
	public static final int INDEX_ParamVideoInit					= 0x01000004;	/**< reference: OMX_PORT_PARAM_TYPE */
	public static final int INDEX_ParamOtherInit					= 0x01000005;	/**< reference: OMX_PORT_PARAM_TYPE */
	public static final int INDEX_ParamNumAvailableStreams			= 0x01000006;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamActiveStream					= 0x01000007;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamSuspensionPolicy				= 0x01000008;	/**< reference: OMX_PARAM_SUSPENSIONPOLICYTYPE */
	public static final int INDEX_ParamComponentSuspended			= 0x01000009;	/**< reference: OMX_PARAM_SUSPENSIONTYPE */
	public static final int INDEX_ConfigCapturing					= 0x0100000A;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigCaptureMode					= 0x0100000B;	/**< reference: OMX_CONFIG_CAPTUREMODETYPE */
	public static final int INDEX_AutoPauseAfterCapture				= 0x0100000C;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamContentURI					= 0x0100000D;	/**< reference: OMX_PARAM_CONTENTURITYPE */
	public static final int INDEX_ParamCustomContentPipe			= 0x0100000E;	/**< reference: OMX_PARAM_CONTENTPIPETYPE */
	public static final int INDEX_ParamDisableResourceConcealment	= 0x0100000F;	/**< reference: OMX_RESOURCECONCEALMENTTYPE */
	public static final int INDEX_ConfigMetadataItemCount			= 0x01000010;	/**< reference: OMX_CONFIG_METADATAITEMCOUNTTYPE */
	public static final int INDEX_ConfigContainerNodeCount			= 0x01000011;	/**< reference: OMX_CONFIG_CONTAINERNODECOUNTTYPE */
	public static final int INDEX_ConfigMetadataItem				= 0x01000012;	/**< reference: OMX_CONFIG_METADATAITEMTYPE */
	public static final int INDEX_ConfigCounterNodeID				= 0x01000013;	/**< reference: OMX_CONFIG_CONTAINERNODEIDTYPE */
	public static final int INDEX_ParamMetadataFilterType			= 0x01000014;	/**< reference: OMX_PARAM_METADATAFILTERTYPE */
	public static final int INDEX_ParamMetadataKeyFilter			= 0x01000015;	/**< reference: OMX_PARAM_METADATAFILTERTYPE */
	public static final int INDEX_ConfigPriorityMgmt				= 0x01000016;	/**< reference: OMX_PRIORITYMGMTTYPE */
	public static final int INDEX_ParamStandardComponentRole		= 0x01000017;	/**< reference: OMX_PARAM_COMPONENTROLETYPE */

	public static final int INDEX_PortStartUnused 					= 0x02000000;
	public static final int INDEX_ParamPortDefinition				= 0x02000001;	/**< reference: OMX_PARAM_PORTDEFINITIONTYPE */
	public static final int INDEX_ParamCompBufferSupplier			= 0x02000002;	/**< reference: OMX_PARAM_BUFFERSUPPLIERTYPE */
	public static final int INDEX_ReservedStartUnused 				= 0x03000000;

	/* Audio parameters and configurations */
	public static final int INDEX_AudioStartUnused					= 0x04000000;
	public static final int INDEX_ParamAudioPortFormat				= 0x04000001;	/**< reference: OMX_AUDIO_PARAM_PORTFORMATTYPE */
	public static final int INDEX_ParamAudioPcm						= 0x04000002;	/**< reference: OMX_AUDIO_PARAM_PCMMODETYPE */
	public static final int INDEX_ParamAudioAac						= 0x04000003;	/**< reference: OMX_AUDIO_PARAM_AACPROFILETYPE */
	public static final int INDEX_ParamAudioRa						= 0x04000004;	/**< reference: OMX_AUDIO_PARAM_RATYPE */
	public static final int INDEX_ParamAudioMp3						= 0x04000005;	/**< reference: OMX_AUDIO_PARAM_MP3TYPE */
	public static final int INDEX_ParamAudioAdpcm					= 0x04000006;	/**< reference: OMX_AUDIO_PARAM_ADPCMTYPE */
	public static final int INDEX_ParamAudioG723					= 0x04000007;	/**< reference: OMX_AUDIO_PARAM_G723TYPE */
	public static final int INDEX_ParamAudioG729					= 0x04000008;	/**< reference: OMX_AUDIO_PARAM_G729TYPE */
	public static final int INDEX_ParamAudioAmr						= 0x04000009;	/**< reference: OMX_AUDIO_PARAM_AMRTYPE */
	public static final int INDEX_ParamAudioWma						= 0x0400000A;	/**< reference: OMX_AUDIO_PARAM_WMATYPE */
	public static final int INDEX_ParamAudioSbc						= 0x0400000B;	/**< reference: OMX_AUDIO_PARAM_SBCTYPE */
	public static final int INDEX_ParamAudioMidi					= 0x0400000C;	/**< reference: OMX_AUDIO_PARAM_MIDITYPE */
	public static final int INDEX_ParamAudioGsm_FR					= 0x0400000D;	/**< reference: OMX_AUDIO_PARAM_GSMFRTYPE */
	public static final int INDEX_ParamAudioMidiLoadUserSound		= 0x0400000E;	/**< reference: OMX_AUDIO_PARAM_MIDILOADUSERSOUNDTYPE */
	public static final int INDEX_ParamAudioG726					= 0x0400000F;	/**< reference: OMX_AUDIO_PARAM_G726TYPE */
	public static final int INDEX_ParamAudioGsm_EFR					= 0x04000010;	/**< reference: OMX_AUDIO_PARAM_GSMEFRTYPE */
	public static final int INDEX_ParamAudioGsm_HR					= 0x04000011;	/**< reference: OMX_AUDIO_PARAM_GSMHRTYPE */
	public static final int INDEX_ParamAudioPdc_FR					= 0x04000012;	/**< reference: OMX_AUDIO_PARAM_PDCFRTYPE */
	public static final int INDEX_ParamAudioPdc_EFR					= 0x04000013;	/**< reference: OMX_AUDIO_PARAM_PDCEFRTYPE */
	public static final int INDEX_ParamAudioPdc_HR					= 0x04000014;	/**< reference: OMX_AUDIO_PARAM_PDCHRTYPE */
	public static final int INDEX_ParamAudioTdma_FR					= 0x04000015;	/**< reference: OMX_AUDIO_PARAM_TDMAFRTYPE */
	public static final int INDEX_ParamAudioTdma_EFR				= 0x04000016;	/**< reference: OMX_AUDIO_PARAM_TDMAEFRTYPE */
	public static final int INDEX_ParamAudioQcelp8					= 0x04000017;	/**< reference: OMX_AUDIO_PARAM_QCELP8TYPE */
	public static final int INDEX_ParamAudioQcelp13					= 0x04000018;	/**< reference: OMX_AUDIO_PARAM_QCELP13TYPE */
	public static final int INDEX_ParamAudioEvrc					= 0x04000019;	/**< reference: OMX_AUDIO_PARAM_EVRCTYPE */
	public static final int INDEX_ParamAudioSmv						= 0x0400001A;	/**< reference: OMX_AUDIO_PARAM_SMVTYPE */
	public static final int INDEX_ParamAudioVorbis					= 0x0400001B;	/**< reference: OMX_AUDIO_PARAM_VORBISTYPE */

	public static final int INDEX_ConfigAudioMidiImmediateEvent		= 0x0400001C;	/**< reference: OMX_AUDIO_CONFIG_MIDIIMMEDIATEEVENTTYPE */
	public static final int INDEX_ConfigAudioMidiControl			= 0x0400001D;	/**< reference: OMX_AUDIO_CONFIG_MIDICONTROLTYPE */
	public static final int INDEX_ConfigAudioMidiSoundBankProgram	= 0x0400001E;	/**< reference: OMX_AUDIO_CONFIG_MIDISOUNDBANKPROGRAMTYPE */
	public static final int INDEX_ConfigAudioMidiStatus				= 0x0400001F;	/**< reference: OMX_AUDIO_CONFIG_MIDISTATUSTYPE */
	public static final int INDEX_ConfigAudioMidiMetaEvent			= 0x04000020;	/**< reference: OMX_AUDIO_CONFIG_MIDIMETAEVENTTYPE */
	public static final int INDEX_ConfigAudioMidiMetaEventData		= 0x04000021;	/**< reference: OMX_AUDIO_CONFIG_MIDIMETAEVENTDATATYPE */
	public static final int INDEX_ConfigAudioVolume					= 0x04000022;	/**< reference: OMX_AUDIO_CONFIG_VOLUMETYPE */
	public static final int INDEX_ConfigAudioBalance				= 0x04000023;	/**< reference: OMX_AUDIO_CONFIG_BALANCETYPE */
	public static final int INDEX_ConfigAudioChannelMute			= 0x04000024;	/**< reference: OMX_AUDIO_CONFIG_CHANNELMUTETYPE */
	public static final int INDEX_ConfigAudioMute					= 0x04000025;	/**< reference: OMX_AUDIO_CONFIG_MUTETYPE */
	public static final int INDEX_ConfigAudioLoudness				= 0x04000026;	/**< reference: OMX_AUDIO_CONFIG_LOUDNESSTYPE */
	public static final int INDEX_ConfigAudioEchoCancelation		= 0x04000027;	/**< reference: OMX_AUDIO_CONFIG_ECHOCANCELATIONTYPE */
	public static final int INDEX_ConfigAudioNoiseReduction			= 0x04000028;	/**< reference: OMX_AUDIO_CONFIG_NOISEREDUCTIONTYPE */
	public static final int INDEX_ConfigAudioBass					= 0x04000029;	/**< reference: OMX_AUDIO_CONFIG_BASSTYPE */
	public static final int INDEX_ConfigAudioTreble					= 0x0400002A;	/**< reference: OMX_AUDIO_CONFIG_TREBLETYPE */
	public static final int INDEX_ConfigAudioStereoWidening			= 0x0400002B;	/**< reference: OMX_AUDIO_CONFIG_STEREOWIDENINGTYPE */
	public static final int INDEX_ConfigAudioChorus					= 0x0400002C;	/**< reference: OMX_AUDIO_CONFIG_CHORUSTYPE */
	public static final int INDEX_ConfigAudioEqualizer				= 0x0400002D;	/**< reference: OMX_AUDIO_CONFIG_EQUALIZERTYPE */
	public static final int INDEX_ConfigAudioReverberation			= 0x0400002E;	/**< reference: OMX_AUDIO_CONFIG_REVERBERATIONTYPE */
	public static final int INDEX_ConfigAudioChannelVolume			= 0x0400002F;	/**< reference: OMX_AUDIO_CONFIG_CHANNELVOLUMETYPE */

	/* Image specific parameters and configurations */
	public static final int INDEX_ImageStartUnused					= 0x05000000;
	public static final int INDEX_ParamImagePortFormat				= 0x05000001;	/**< reference: OMX_IMAGE_PARAM_PORTFORMATTYPE */
	public static final int INDEX_ParamFlashControl					= 0x05000002;	/**< reference: OMX_IMAGE_PARAM_FLASHCONTROLTYPE */
	public static final int INDEX_ConfigFocusControl				= 0x05000003;	/**< reference: OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE */
	public static final int INDEX_ParamQFactor						= 0x05000004;	/**< reference: OMX_IMAGE_PARAM_QFACTORTYPE */
	public static final int INDEX_ParamQuantizationTable			= 0x05000005;	/**< reference: OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE */
	public static final int INDEX_ParamHuffmanTable					= 0x05000006;	/**< reference: OMX_IMAGE_PARAM_HUFFMANTTABLETYPE */
	public static final int INDEX_ConfigFlashControl				= 0x05000007;	/**< reference: OMX_IMAGE_PARAM_FLASHCONTROLTYPE */

	/* Video specific parameters and configurations */
	public static final int INDEX_VideoStartUnused					= 0x06000000;
	public static final int INDEX_ParamVideoPortFormat				= 0x06000001;	/**< reference: OMX_VIDEO_PARAM_PORTFORMATTYPE */
	public static final int INDEX_ParamVideoQuantization			= 0x06000002;	/**< reference: OMX_VIDEO_PARAM_QUANTIZATIONTYPE */
	public static final int INDEX_ParamVideoFastUpdate				= 0x06000003;	/**< reference: OMX_VIDEO_PARAM_VIDEOFASTUPDATETYPE */
	public static final int INDEX_ParamVideoBitrate					= 0x06000004;	/**< reference: OMX_VIDEO_PARAM_BITRATETYPE */
	public static final int INDEX_ParamVideoMotionVector			= 0x06000005;	/**< reference: OMX_VIDEO_PARAM_MOTIONVECTORTYPE */
	public static final int INDEX_ParamVideoIntraRefresh			= 0x06000006;	/**< reference: OMX_VIDEO_PARAM_INTRAREFRESHTYPE */
	public static final int INDEX_ParamVideoErrorCorrection			= 0x06000007;	/**< reference: OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE */
	public static final int INDEX_ParamVideoVBSMC					= 0x06000008;	/**< reference: OMX_VIDEO_PARAM_VBSMCTYPE */
	public static final int INDEX_ParamVideoMpeg2					= 0x06000009;	/**< reference: OMX_VIDEO_PARAM_MPEG2TYPE */
	public static final int INDEX_ParamVideoMpeg4					= 0x0600000A;	/**< reference: OMX_VIDEO_PARAM_MPEG4TYPE */
	public static final int INDEX_ParamVideoWmv						= 0x0600000B;	/**< reference: OMX_VIDEO_PARAM_WMVTYPE */
	public static final int INDEX_ParamVideoRv						= 0x0600000C;	/**< reference: OMX_VIDEO_PARAM_RVTYPE */
	public static final int INDEX_ParamVideoAvc						= 0x0600000D;	/**< reference: OMX_VIDEO_PARAM_AVCTYPE */
	public static final int INDEX_ParamVideoH263					= 0x0600000E;	/**< reference: OMX_VIDEO_PARAM_H263TYPE */
	public static final int INDEX_ParamVideoProfileLevelQuerySupported = 0x0600000F;	/**< reference: OMX_VIDEO_PARAM_PROFILELEVELTYPE */
	public static final int INDEX_ParamVideoProfileLevelCurrent		= 0x06000010;	/**< reference: OMX_VIDEO_PARAM_PROFILELEVELTYPE */
	public static final int INDEX_ConfigVideoBitrate				= 0x06000011;	/**< reference: OMX_VIDEO_CONFIG_BITRATETYPE */
	public static final int INDEX_ConfigVideoFramerate				= 0x06000012;	/**< reference: OMX_CONFIG_FRAMERATETYPE */
	public static final int INDEX_ConfigVideoIntraVOPRefresh		= 0x06000013;	/**< reference: OMX_CONFIG_INTRAREFRESHVOPTYPE */
	public static final int INDEX_ConfigVideoIntraMBRefresh			= 0x06000014;	/**< reference: OMX_CONFIG_MACROBLOCKERRORMAPTYPE */
	public static final int INDEX_ConfigVideoMBErrorReporting		= 0x06000015;	/**< reference: OMX_CONFIG_MBERRORREPORTINGTYPE */
	public static final int INDEX_ParamVideoMacroblocksPerFrame		= 0x06000016;	/**< reference: OMX_PARAM_MACROBLOCKSTYPE */
	public static final int INDEX_ConfigVideoMacroBlockErrorMap		= 0x06000017;	/**< reference: OMX_CONFIG_MACROBLOCKERRORMAPTYPE */
	public static final int INDEX_ParamVideoSliceFMO				= 0x06000018;	/**< reference: OMX_VIDEO_PARAM_AVCSLICEFMO */
	public static final int INDEX_ConfigVideoAVCIntraPeriod			= 0x06000019;	/**< reference: OMX_VIDEO_CONFIG_AVCINTRAPERIOD */
	public static final int INDEX_ConfigVideoNalSize				= 0x0600001A;	/**< reference: OMX_VIDEO_CONFIG_NALSIZE */

	/* Image & Video common Configurations */
	public static final int INDEX_CommonStartUnused					= 0x07000000;
	public static final int INDEX_ParamCommonDeblocking				= 0x07000001;	/**< reference: OMX_PARAM_DEBLOCKINGTYPE */
	public static final int INDEX_ParamCommonSensorMode				= 0x07000002;	/**< reference: OMX_PARAM_SENSORMODETYPE */
	public static final int INDEX_ParamCommonInterleave				= 0x07000003;	/**< reference: OMX_PARAM_INTERLEAVETYPE */
	public static final int INDEX_ConfigCommonColorFormatConversion	= 0x07000004;	/**< reference: OMX_CONFIG_COLORCONVERSIONTYPE */
	public static final int INDEX_ConfigCommonScale					= 0x07000005;	/**< reference: OMX_CONFIG_SCALEFACTORTYPE */
	public static final int INDEX_ConfigCommonImageFilter			= 0x07000006;	/**< reference: OMX_CONFIG_IMAGEFILTERTYPE */
	public static final int INDEX_ConfigCommonColorEnhancement		= 0x07000007;	/**< reference: OMX_CONFIG_COLORENHANCEMENTTYPE */
	public static final int INDEX_ConfigCommonColorKey				= 0x07000008;	/**< reference: OMX_CONFIG_COLORKEYTYPE */
	public static final int INDEX_ConfigCommonColorBlend			= 0x07000009;	/**< reference: OMX_CONFIG_COLORBLENDTYPE */
	public static final int INDEX_ConfigCommonFrameStabilisation	= 0x0700000A;	/**< reference: OMX_CONFIG_FRAMESTABTYPE */
	public static final int INDEX_ConfigCommonRotate				= 0x0700000B;	/**< reference: OMX_CONFIG_ROTATIONTYPE */
	public static final int INDEX_ConfigCommonMirror				= 0x0700000C;	/**< reference: OMX_CONFIG_MIRRORTYPE */
	public static final int INDEX_ConfigCommonOutputPositio			= 0x0700000D;	/**< reference: OMX_CONFIG_POINTTYPE */
	public static final int INDEX_ConfigCommonInputCrop				= 0x0700000E;	/**< reference: OMX_CONFIG_RECTTYPE */
	public static final int INDEX_ConfigCommonOutputCrop			= 0x0700000F;	/**< reference: OMX_CONFIG_RECTTYPE */
	public static final int INDEX_ConfigCommonDigitalZoom			= 0x07000010;	/**< reference: OMX_CONFIG_SCALEFACTORTYPE */
	public static final int INDEX_ConfigCommonOpticalZoom			= 0x07000011;	/**< reference: OMX_CONFIG_SCALEFACTORTYPE*/
	public static final int INDEX_ConfigCommonWhiteBalance			= 0x07000012;	/**< reference: OMX_CONFIG_WHITEBALCONTROLTYPE */
	public static final int INDEX_ConfigCommonExposure				= 0x07000013;	/**< reference: OMX_CONFIG_EXPOSURECONTROLTYPE */
	public static final int INDEX_ConfigCommonContrast				= 0x07000014;	/**< reference: OMX_CONFIG_CONTRASTTYPE */
	public static final int INDEX_ConfigCommonBrightness			= 0x07000015;	/**< reference: OMX_CONFIG_BRIGHTNESSTYPE */
	public static final int INDEX_ConfigCommonBacklight				= 0x07000016;	/**< reference: OMX_CONFIG_BACKLIGHTTYPE */
	public static final int INDEX_ConfigCommonGamma					= 0x07000017;	/**< reference: OMX_CONFIG_GAMMATYPE */
	public static final int INDEX_ConfigCommonSaturation			= 0x07000018;	/**< reference: OMX_CONFIG_SATURATIONTYPE */
	public static final int INDEX_ConfigCommonLightness				= 0x07000019;	/**< reference: OMX_CONFIG_LIGHTNESSTYPE */
	public static final int INDEX_ConfigCommonExclusionRect			= 0x0700001A;	/**< reference: OMX_CONFIG_RECTTYPE */
	public static final int INDEX_ConfigCommonDithering				= 0x0700001B;	/**< reference: OMX_CONFIG_DITHERTYPE */
	public static final int INDEX_ConfigCommonPlaneBlend			= 0x0700001C;	/**< reference: OMX_CONFIG_PLANEBLENDTYPE */
	public static final int INDEX_ConfigCommonExposureValue			= 0x0700001D;	/**< reference: OMX_CONFIG_EXPOSUREVALUETYPE */
	public static final int INDEX_ConfigCommonOutputSize			= 0x0700001E;	/**< reference: OMX_FRAMESIZETYPE */
	public static final int INDEX_ParamCommonExtraQuantData			= 0x0700001F;	/**< reference: OMX_OTHER_EXTRADATATYPE */
	public static final int INDEX_ConfigCommonFocusRegion			= 0x07000020;	/**< reference: OMX_CONFIG_FOCUSREGIONTYPE */
	public static final int INDEX_ConfigCommonFocusStatus			= 0x07000021;	/**< reference: OMX_PARAM_FOCUSSTATUSTYPE */
	public static final int INDEX_ConfigCommonTransitionEffect		= 0x07000022;	/**< reference: OMX_CONFIG_TRANSITIONEFFECTTYPE */

	/* Reserved Configuration range */
	public static final int INDEX_OtherStartUnused					= 0x08000000;
	public static final int INDEX_ParamOtherPortFormat				= 0x08000001;	/**< reference: OMX_OTHER_PARAM_PORTFORMATTYPE */
	public static final int INDEX_ConfigOtherPower					= 0x08000002;	/**< reference: OMX_OTHER_CONFIG_POWERTYPE */
	public static final int INDEX_ConfigOtherStats					= 0x08000003;	/**< reference: OMX_OTHER_CONFIG_STATSTYPE */


	/* Reserved Time range */
	public static final int INDEX_TimeStartUnused					= 0x09000000;
	public static final int INDEX_ConfigTimeScale					= 0x09000001;	/**< reference: OMX_TIME_CONFIG_SCALETYPE */
	public static final int INDEX_ConfigTimeClockState				= 0x09000002;	/**< reference: OMX_TIME_CONFIG_CLOCKSTATETYPE */
	public static final int INDEX_ConfigTimeActiveRefClock			= 0x09000003;	/**< reference: OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE */
	public static final int INDEX_ConfigTimeCurrentMediaTime		= 0x09000004;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE (read only) */
	public static final int INDEX_ConfigTimeCurrentWallTime			= 0x09000005;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE (read only) */
	public static final int INDEX_ConfigTimeCurrentAudioReference	= 0x09000006;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE (write only) */
	public static final int INDEX_ConfigTimeCurrentVideoReference	= 0x09000007;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE (write only) */
	public static final int INDEX_ConfigTimeMediaTimeRequest		= 0x09000008;	/**< reference: OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE (write only) */
	public static final int INDEX_ConfigTimeClientStartTime			= 0x09000009;	/**<reference:  OMX_TIME_CONFIG_TIMESTAMPTYPE (write only) */
	public static final int INDEX_ConfigTimePosition				= 0x0900000A;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE */
	public static final int INDEX_ConfigTimeSeekMode				= 0x0900000B;	/**< reference: OMX_TIME_CONFIG_SEEKMODETYPE */


	public static final int INDEX_KhronosExtensions					= 0x6F000000;	/**< Reserved region for introducing Khronos Standard Extensions */
	/* Vendor specific area */
	public static final int INDEX_VendorStartUnused					= 0x7F000000;
	/* Vendor specific structures should be in the range of 0x7F000000
	   to 0x7FFFFFFE.  This range is not broken out by vendor, so
	   private indexes are not guaranteed unique and therefore should
	   only be sent to the appropriate component. */

	/* used for ilcs-top communication */
	public static final int INDEX_ParamMarkComparison				= 0x7F000001;	/**< reference: OMX_PARAM_MARKCOMPARISONTYPE */
	public static final int INDEX_ParamPortSummary					= 0x7F000002;	/**< reference: OMX_PARAM_PORTSUMMARYTYPE */
	public static final int INDEX_ParamTunnelStatus					= 0x7F000003;	/**< reference : OMX_PARAM_TUNNELSTATUSTYPE */
	public static final int INDEX_ParamBrcmRecursionUnsafe			= 0x7F000004;	/**< reference: OMX_PARAM_BRCMRECURSIONUNSAFETYPE */

	/* used for top-ril communication */
	public static final int INDEX_ParamBufferAddress				= 0x7F000005;	/**< reference : OMX_PARAM_BUFFERADDRESSTYPE */
	public static final int INDEX_ParamTunnelSetup					= 0x7F000006;	/**< reference : OMX_PARAM_TUNNELSETUPTYPE */
	public static final int INDEX_ParamBrcmPortEGL					= 0x7F000007;	/**< reference : OMX_PARAM_BRCMPORTEGLTYPE */
	public static final int INDEX_ParamIdleResourceCount			= 0x7F000008;	/**< reference : OMX_PARAM_U32TYPE */

	/* used for ril-ril communication */
	public static final int INDEX_ParamImagePoolDisplayFunction		= 0x7F000009;	/**<reference : OMX_PARAM_IMAGEDISPLAYFUNCTIONTYPE */
	public static final int INDEX_ParamBrcmDataUnit					= 0x7F00000A;	/**<reference: OMX_PARAM_DATAUNITTYPE */
	public static final int INDEX_ParamCodecConfig					= 0x7F00000B;	/**<reference: OMX_PARAM_CODECCONFIGTYPE */
	public static final int INDEX_ParamCameraPoolToEncoderFunction	= 0x7F00000C;	/**<reference : OMX_PARAM_CAMERAPOOLTOENCODERFUNCTIONTYPE */
	public static final int INDEX_ParamCameraStripeFunction			= 0x7F00000D;	/**<reference : OMX_PARAM_CAMERASTRIPEFUNCTIONTYPE */
	public static final int INDEX_ParamCameraCaptureEventFunction	= 0x7F00000E;	/**<reference : OMX_PARAM_CAMERACAPTUREEVENTFUNCTIONTYPE */

	/* used for client-ril communication */
	public static final int INDEX_ParamTestInterface				= 0x7F00000F;	/**< reference : OMX_PARAM_TESTINTERFACETYPE */

	// 0x7f000010
	public static final int INDEX_ConfigDisplayRegion				= 0x7F000010;	/**< reference : OMX_CONFIG_DISPLAYREGIONTYPE */
	public static final int INDEX_ParamSource						= 0x7F000011;	/**< reference : OMX_PARAM_SOURCETYPE */
	public static final int INDEX_ParamSourceSeed					= 0x7F000012;	/**< reference : OMX_PARAM_SOURCESEEDTYPE */
	public static final int INDEX_ParamResize						= 0x7F000013;	/**< reference : OMX_PARAM_RESIZETYPE */
	public static final int INDEX_ConfigVisualisation				= 0x7F000014;	/**< reference : OMX_CONFIG_VISUALISATIONTYPE */
	public static final int INDEX_ConfigSingleStep					= 0x7F000015;	/**<reference : OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigPlayMode					= 0x7F000016;	/**<reference: OMX_CONFIG_PLAYMODETYPE */
	public static final int INDEX_ParamCameraCamplusId				= 0x7F000017;	/**<reference : OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCommonImageFilterParameters	= 0x7F000018;	/**<reference : OMX_CONFIG_IMAGEFILTERPARAMSTYPE */
	public static final int INDEX_ConfigTransitionControl			= 0x7F000019;	/**<reference : OMX_CONFIG_TRANSITIONCONTROLTYPE */
	public static final int INDEX_ConfigPresentationOffset			= 0x7F00001A;	/**<reference: OMX_TIME_CONFIG_TIMESTAMPTYPE */
	public static final int INDEX_ParamSourceFunctions				= 0x7F00001B;	/**<reference: OMX_PARAM_STILLSFUNCTIONTYPE */
	public static final int INDEX_ConfigAudioMonoTrackControl		= 0x7F00001C;	/**<reference : OMX_CONFIG_AUDIOMONOTRACKCONTROLTYPE */
	public static final int INDEX_ParamCameraImagePool				= 0x7F00001D;	/**<reference : OMX_PARAM_CAMERAIMAGEPOOLTYPE */
	public static final int INDEX_ConfigCameraISPOutputPoolHeight	= 0x7F00001E;	/**<reference : OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamImagePoolSize				= 0x7F00001F;	/**<reference: OMX_PARAM_IMAGEPOOLSIZETYPE */

	// 0x7f000020
	public static final int INDEX_ParamImagePoolExternal				= 0x7F000020;	/**<reference: OMX_PARAM_IMAGEPOOLEXTERNALTYPE */
	public static final int INDEX_ParamRUTILFifoInfo					= 0x7F000021;	/**<reference: OMX_PARAM_RUTILFIFOINFOTYPE*/
	public static final int INDEX_ParamILFifoConfig						= 0x7F000022;	/**<reference: OMX_PARAM_ILFIFOCONFIG */
	public static final int INDEX_ConfigCameraSensorModes				= 0x7F000023;	/**<reference : OMX_CONFIG_CAMERASENSORMODETYPE */
	public static final int INDEX_ConfigBrcmPortStats					= 0x7F000024;	/**<reference : OMX_CONFIG_BRCMPORTSTATSTYPE */
	public static final int INDEX_ConfigBrcmPortBufferStats				= 0x7F000025;	/**<reference : OMX_CONFIG_BRCMPORTBUFFERSTATSTYPE */
	public static final int INDEX_ConfigBrcmCameraStats					= 0x7F000026;	/**<reference : OMX_CONFIG_BRCMCAMERASTATSTYPE */
	public static final int INDEX_ConfigBrcmIOPerfStats					= 0x7F000027;	/**<reference : OMX_CONFIG_BRCMIOPERFSTATSTYPE */
	public static final int INDEX_ConfigCommonSharpness					= 0x7F000028;	/**<reference : OMX_CONFIG_SHARPNESSTYPE */
	public static final int INDEX_ConfigCommonFlickerCancellation		= 0x7F000029;	/**reference : OMX_CONFIG_FLICKERCANCELTYPE */
	public static final int INDEX_ParamCameraSwapImagePools				= 0x7F00002A;	/**<reference : OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamCameraSingleBufferCaptureInput	= 0x7F00002B;	/**<reference : OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigCommonRedEyeRemoval				= 0x7F00002C;	/**<reference : OMX_CONFIG_REDEYEREMOVALTYPE  */
	public static final int INDEX_ConfigCommonFaceDetectionControl		= 0x7F00002D;	/**<reference : OMX_CONFIG_FACEDETECTIONCONTROLTYPE */
	public static final int INDEX_ConfigCommonFaceDetectionRegion		= 0x7F00002E;	/**<reference : OMX_CONFIG_FACEDETECTIONREGIONTYPE */
	public static final int INDEX_ConfigCommonInterlace					= 0x7F00002F;	/**<reference: OMX_CONFIG_INTERLACETYPE */

	// 0x7f000030
	public static final int INDEX_ParamISPTunerName						= 0x7F000030;	/**<reference: OMX_PARAM_CAMERAISPTUNERTYPE */
	public static final int INDEX_ParamCameraDeviceNumber				= 0x7F000031;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamCameraDevicesPresent				= 0x7F000032;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCameraInputFrame				= 0x7F000033;	/**<reference: OMX_CONFIG_IMAGEPTRTYPE */
	public static final int INDEX_ConfigStillColourDenoiseEnable		= 0x7F000034;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigVideoColourDenoiseEnable		= 0x7F000035;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigAFAssistLight					= 0x7F000036;	/**<reference: OMX_CONFIG_AFASSISTTYPE */
	public static final int INDEX_ConfigSmartShakeReductionEnable		= 0x7F000037;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigInputCropPercentages			= 0x7F000038;	/**<reference: OMX_CONFIG_INPUTCROPTYPE */
	public static final int INDEX_ConfigStillsAntiShakeEnable			= 0x7F000039;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigWaitForFocusBeforeCapture		= 0x7F00003A;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigAudioRenderingLatency			= 0x7F00003B;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigDrawBoxAroundFaces				= 0x7F00003C;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamCodecRequirements				= 0x7F00003D;	/**<reference: OMX_PARAM_CODECREQUIREMENTSTYPE */
	public static final int INDEX_ConfigBrcmEGLImageMemHandle			= 0x7F00003E;	/**<reference: OMX_CONFIG_BRCMEGLIMAGEMEMHANDLETYPE */
	public static final int INDEX_ConfigPrivacyIndicator				= 0x7F00003F;	/**<reference: OMX_CONFIG_PRIVACYINDICATORTYPE */

	// 0x7f000040
	public static final int INDEX_ParamCameraFlashType					= 0x7F000040;	/**<reference: OMX_PARAM_CAMERAFLASHTYPE */
	public static final int INDEX_ConfigCameraEnableStatsPass			= 0x7F000041;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigCameraFlashConfig				= 0x7F000042;	/**<reference: OMX_CONFIG_CAMERAFLASHCONFIGTYPE */
	public static final int INDEX_ConfigCaptureRawImageURI				= 0x7F000043;	/**<reference: OMX_PARAM_CONTENTURITYPE */
	public static final int INDEX_ConfigCameraStripeFuncMinLines		= 0x7F000044;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCameraAlgorithmVersionDeprecated= 0x7F000045;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCameraIsoReferenceValue			= 0x7F000046;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCameraCaptureAbortsAutoFocus	= 0x7F000047;	/**<reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmClockMissCount				= 0x7F000048;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigFlashChargeLevel				= 0x7F000049;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmVideoEncodedSliceSize		= 0x7F00004A;	/**<reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmAudioTrackGaplessPlayback	= 0x7F00004B;	/**< reference: OMX_CONFIG_BRCMAUDIOTRACKGAPLESSPLAYBACKTYPE */
	public static final int INDEX_ConfigBrcmAudioTrackChangeControl		= 0x7F00004C;	/**< reference: OMX_CONFIG_BRCMAUDIOTRACKCHANGECONTROLTYPE */
	public static final int INDEX_ParamBrcmPixelAspectRatio				= 0x7F00004D;	/**< reference: OMX_CONFIG_POINTTYPE */
	public static final int INDEX_ParamBrcmPixelValueRange				= 0x7F00004E;	/**< reference: OMX_PARAM_BRCMPIXELVALUERANGETYPE */
	public static final int INDEX_ParamCameraDisableAlgorithm			= 0x7F00004F;	/**< reference: OMX_PARAM_CAMERADISABLEALGORITHMTYPE */

	// 0x7f000050
	public static final int INDEX_ConfigBrcmVideoIntraPeriodTime		= 0x7F000050;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmVideoIntraPeriod			= 0x7F000051;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmAudioEffectControl			= 0x7F000052;	/**< reference: OMX_CONFIG_BRCMAUDIOEFFECTCONTROLTYPE */
	public static final int INDEX_ConfigBrcmMinimumProcessingLatency	= 0x7F000053;	/**< reference: OMX_CONFIG_BRCMMINIMUMPROCESSINGLATENCY */
	public static final int INDEX_ParamBrcmVideoAVCSEIEnable			= 0x7F000054;	/**< reference: OMX_PARAM_BRCMVIDEOAVCSEIENABLETYPE */
	public static final int INDEX_ParamBrcmAllowMemChange				= 0x7F000055;	/**< reference: OMX_PARAM_BRCMALLOWMEMCHANGETYPE */
	public static final int INDEX_ConfigBrcmVideoEncoderMBRowsPerSlice	= 0x7F000056;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamCameraAFAssistDeviceNumber_Deprecated= 0x7F000057;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamCameraPrivacyIndicatorDeviceNumber_Deprecated= 0x7F000058;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCameraUseCase					= 0x7F000059;	/**< reference: OMX_CONFIG_CAMERAUSECASETYPE */
	public static final int INDEX_ParamBrcmDisableProprietaryTunnels	= 0x7F00005A;	/**< reference: OMX_PARAM_BRCMDISABLEPROPRIETARYTUNNELSTYPE */
	public static final int INDEX_ParamBrcmOutputBufferSize				= 0x7F00005B;	/**<  reference: OMX_PARAM_BRCMOUTPUTBUFFERSIZETYPE */
	public static final int INDEX_ParamBrcmRetainMemory					= 0x7F00005C;	/**< reference: OMX_PARAM_BRCMRETAINMEMORYTYPE */
	public static final int INDEX_ConfigCanFocus_Deprecated				= 0x7F00005D;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmImmutableInput				= 0x7F00005E;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamDynamicParameterFile				= 0x7F00005F;	/**< reference: OMX_PARAM_CONTENTURITYPE */

	// 0x7f000060
	public static final int INDEX_ParamUseDynamicParameterFile			= 0x7F000060;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigCameraInfo						= 0x7F000061;	/**< reference: OMX_CONFIG_CAMERAINFOTYPE */
	public static final int INDEX_ConfigCameraFeatures					= 0x7F000062;	/**< reference: OMX_CONFIG_CAMERAFEATURESTYPE */
	public static final int INDEX_ConfigRequestCallback					= 0x7F000063;	/**< reference: OMX_CONFIG_REQUESTCALLBACKTYPE */ //Should be added to the spec as part of IL416c
	public static final int INDEX_ConfigBrcmOutputBufferFullCount		= 0x7F000064;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCommonFocusRegionXY				= 0x7F000065;	/**< reference: OMX_CONFIG_FOCUSREGIONXYTYPE */
	public static final int INDEX_ParamBrcmDisableEXIF					= 0x7F000066;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigUserSettingsId					= 0x7F000067;	/**< reference: OMX_CONFIG_U8TYPE */
	public static final int INDEX_ConfigCameraSettings					= 0x7F000068;	/**< reference: OMX_CONFIG_CAMERASETTINGSTYPE */
	public static final int INDEX_ConfigDrawBoxLineParams				= 0x7F000069;	/**< reference: OMX_CONFIG_DRAWBOXLINEPARAMS */
	public static final int INDEX_ParamCameraRmiControl_Deprecated		= 0x7F00006A;	/**< reference: OMX_PARAM_CAMERARMITYPE */
	public static final int INDEX_ConfigBurstCapture					= 0x7F00006B;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmEnableIJGTableScaling		= 0x7F00006C;	/**< reference: OMX_PARAM_IJGSCALINGTYPE */
	public static final int INDEX_ConfigPowerDown						= 0x7F00006D;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmSyncOutput					= 0x7F00006E;	/**< reference: OMX_CONFIG_BRCMSYNCOUTPUTTYPE */
	public static final int INDEX_ParamBrcmFlushCallback				= 0x7F00006F;	/**< reference: OMX_PARAM_BRCMFLUSHCALLBACK */

	// 0x7f000070
	public static final int INDEX_ConfigBrcmVideoRequestIFrame			= 0x7F000070;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmNALSSeparate					= 0x7F000071;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigConfirmView						= 0x7F000072;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigDrmView							= 0x7F000073;	/**< reference: OMX_CONFIG_DRMVIEWTYPE */
	public static final int INDEX_ConfigBrcmVideoIntraRefresh			= 0x7F000074;	/**< reference: OMX_VIDEO_PARAM_INTRAREFRESHTYPE */
	public static final int INDEX_ParamBrcmMaxFileSize					= 0x7F000075;	/**< reference: OMX_PARAM_BRCMU64TYPE */
	public static final int INDEX_ParamBrcmCRCEnable					= 0x7F000076;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmCRC							= 0x7F000077;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigCameraRmiInUse_Deprecated		= 0x7F000078;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmAudioSource					= 0x7F000079;	/**<reference: OMX_CONFIG_BRCMAUDIOSOURCETYPE */
	public static final int INDEX_ConfigBrcmAudioDestination			= 0x7F00007A;	/**< reference: OMX_CONFIG_BRCMAUDIODESTINATIONTYPE */
	public static final int INDEX_ParamAudioDdp							= 0x7F00007B;	/**< reference: OMX_AUDIO_PARAM_DDPTYPE */
	public static final int INDEX_ParamBrcmThumbnail					= 0x7F00007C;	/**< reference: OMX_PARAM_BRCMTHUMBNAILTYPE */
	public static final int INDEX_ParamBrcmDisableLegacyBlocks_Deprecated= 0x7F00007D;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmCameraInputAspectRatio		= 0x7F00007E;	/**< reference: OMX_PARAM_BRCMASPECTRATIOTYPE */
	public static final int INDEX_ParamDynamicParameterFileFailFatal	= 0x7F00007F;	/**< reference: OMX_CONFIG_BOOLEANTYPE */

	// 0x7f000080
	public static final int INDEX_ParamBrcmVideoDecodeErrorConcealment	= 0x7F000080;	/**< reference: OMX_PARAM_BRCMVIDEODECODEERRORCONCEALMENTTYPE */
	public static final int INDEX_ParamBrcmInterpolateMissingTimestamps	= 0x7F000081;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmSetCodecPerformanceMonitoring= 0x7F000082;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigFlashInfo						= 0x7F000083;	/**< reference: OMX_CONFIG_FLASHINFOTYPE */
	public static final int INDEX_ParamBrcmMaxFrameSkips				= 0x7F000084;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigDynamicRangeExpansion			= 0x7F000085;	/**< reference: OMX_CONFIG_DYNAMICRANGEEXPANSIONTYPE */
	public static final int INDEX_ParamBrcmFlushCallbackId				= 0x7F000086;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmTransposeBufferCount			= 0x7F000087;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigFaceRecognitionControl			= 0x7F000088;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigFaceRecognitionSaveFace			= 0x7F000089;	/**< reference: OMX_PARAM_BRCMU64TYPE */
	public static final int INDEX_ConfigFaceRecognitionDatabaseUri		= 0x7F00008A;	/**< reference: OMX_PARAM_CONTENTURITYPE */
	public static final int INDEX_ConfigClockAdjustment					= 0x7F00008B;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE */
	public static final int INDEX_ParamBrcmThreadAffinity				= 0x7F00008C;	/**< reference: OMX_PARAM_BRCMTHREADAFFINITYTYPE */
	public static final int INDEX_ParamAsynchronousOutput				= 0x7F00008D;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigAsynchronousFailureURI			= 0x7F00008E;	/**< reference: OMX_PARAM_CONTENTURITYPE */
	public static final int INDEX_ConfigCommonFaceBeautification		= 0x7F00008F;	/**< reference: OMX_CONFIG_BOOLEANTYPE */

	// 0x7f000090
	public static final int INDEX_ConfigCommonSceneDetectionControl		= 0x7F000090;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigCommonSceneDetected				= 0x7F000091;	/**< reference: OMX_CONFIG_SCENEDETECTTYPE */
	public static final int INDEX_ParamDisableVllPool					= 0x7F000092;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamVideoMvc							= 0x7F000093;	/**< reference: OMX_VIDEO_PARAM_MVCTYPE */
	public static final int INDEX_ConfigBrcmDrawStaticBox				= 0x7F000094;	/**< reference: OMX_CONFIG_STATICBOXTYPE */
	public static final int INDEX_ConfigBrcmClockReferenceSource		= 0x7F000095;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamPassBufferMarks					= 0x7F000096;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigPortCapturing					= 0x7F000097;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE */
	public static final int INDEX_ConfigBrcmDecoderPassThrough			= 0x7F000098;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmDecoderPassThrough			= INDEX_ConfigBrcmDecoderPassThrough;  /* deprecated */
	public static final int INDEX_ParamBrcmMaxCorruptMBs				= 0x7F000099;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmGlobalAudioMute				= 0x7F00009A;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamCameraCaptureMode				= 0x7F00009B;	/**< reference: OMX_PARAM_CAMERACAPTUREMODETYPE */
	public static final int INDEX_ParamBrcmDrmEncryption				= 0x7F00009C;	/**< reference: OMX_PARAM_BRCMDRMENCRYPTIONTYPE */
	public static final int INDEX_ConfigBrcmCameraRnDPreprocess			= 0x7F00009D;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmCameraRnDPostprocess		= 0x7F00009E;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmAudioTrackChangeCount		= 0x7F00009F;	/**< reference: OMX_PARAM_U32TYPE */

	// 0x7f0000a0
	public static final int INDEX_ParamCommonUseStcTimestamps		= 0x7F0000A0;	/**< reference: OMX_PARAM_TIMESTAMPMODETYPE */
	public static final int INDEX_ConfigBufferStall					= 0x7F0000A1;	/**< reference: OMX_CONFIG_BUFFERSTALLTYPE */
	public static final int INDEX_ConfigRefreshCodec				= 0x7F0000A2;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamCaptureStatus				= 0x7F0000A3;	/**< reference: OMX_PARAM_CAPTURESTATETYPE */
	public static final int INDEX_ConfigTimeInvalidStartTime		= 0x7F0000A4;	/**< reference: OMX_TIME_CONFIG_TIMESTAMPTYPE */
	public static final int INDEX_ConfigLatencyTarget				= 0x7F0000A5;	/**< reference: OMX_CONFIG_LATENCYTARGETTYPE */
	public static final int INDEX_ConfigMinimiseFragmentation		= 0x7F0000A6;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmUseProprietaryCallback	= 0x7F0000A7;	/**< reference: OMX_CONFIG_BRCMUSEPROPRIETARYTUNNELTYPE */
	public static final int INDEX_ParamPortMaxFrameSize				= 0x7F0000A8;	/**< reference: OMX_FRAMESIZETYPE */
	public static final int INDEX_ParamComponentName				= 0x7F0000A9;	/**< reference: OMX_PARAM_COMPONENTROLETYPE */
	public static final int INDEX_ConfigEncLevelExtension			= 0x7F0000AA;	/**< reference: OMX_VIDEO_CONFIG_LEVEL_EXTEND */
	public static final int INDEX_ConfigTemporalDenoiseEnable		= 0x7F0000AB;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmLazyImagePoolDestroy		= 0x7F0000AC;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmEEDEEnable				= 0x7F0000AD;	/**< reference: OMX_VIDEO_EEDE_ENABLE */
	public static final int INDEX_ParamBrcmEEDELossRate				= 0x7F0000AE;	/**< reference: OMX_VIDEO_EEDE_LOSSRATE */
	public static final int INDEX_ParamAudioDts						= 0x7F0000AF;	/**< reference: OMX_AUDIO_PARAM_DTSTYPE */

	// 0x7f0000b0
	public static final int INDEX_ParamNumOutputChannels			= 0x7F0000B0;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmHighDynamicRange		= 0x7F0000B1;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmPoolMemAllocSize		= 0x7F0000B2;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmBufferFlagFilter		= 0x7F0000B3;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmVideoEncodeMinQuant		= 0x7F0000B4;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmVideoEncodeMaxQuant		= 0x7F0000B5;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamRateControlModel				= 0x7F0000B6;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmExtraBuffers				= 0x7F0000B7;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigFieldOfView					= 0x7F0000B8;	/**< reference: OMX_CONFIG_BRCMFOVTYPE */
	public static final int INDEX_ParamBrcmAlignHoriz				= 0x7F0000B9;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmAlignVert				= 0x7F0000BA;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamColorSpace					= 0x7F0000BB;	/**< reference: OMX_PARAM_COLORSPACETYPE */
	public static final int INDEX_ParamBrcmDroppablePFrames			= 0x7F0000BC;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmVideoInitialQuant		= 0x7F0000BD;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmVideoEncodeQpP			= 0x7F0000BE;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmVideoRCSliceDQuant		= 0x7F0000BF;	/**< reference: OMX_PARAM_U32TYPE */

	// 0x7f0000c0
	public static final int INDEX_ParamBrcmVideoFrameLimitBits		= 0x7F0000C0;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmVideoPeakRate			= 0x7F0000C1;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmVideoH264DisableCABAC	= 0x7F0000C2;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmVideoH264LowLatency		= 0x7F0000C3;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmVideoH264AUDelimiters	= 0x7F0000C4;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmVideoH264DeblockIDC		= 0x7F0000C5;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigBrcmVideoH264IntraMBMode	= 0x7F0000C6;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ConfigContrastEnhance				= 0x7F0000C7;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamCameraCustomSensorConfig		= 0x7F0000C8;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmHeaderOnOpen				= 0x7F0000C9;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmUseRegisterFile			= 0x7F0000CA;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmRegisterFileFailFatal	= 0x7F0000CB;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmConfigFileRegisters		= 0x7F0000CC;	/**< reference: OMX_PARAM_BRCMCONFIGFILETYPE */
	public static final int INDEX_ParamBrcmConfigFileChunkRegisters = 0x7F0000CD;	/**< reference: OMX_PARAM_BRCMCONFIGFILECHUNKTYPE */
	public static final int INDEX_ParamBrcmAttachLog				= 0x7F0000CE;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamCameraZeroShutterLag			= 0x7F0000CF;	/**< reference: OMX_CONFIG_ZEROSHUTTERLAGTYPE */

	// 0x7f0000d0
	public static final int INDEX_ParamBrcmFpsRange						= 0x7F0000D0;	/**< reference: OMX_PARAM_BRCMFRAMERATERANGETYPE */
	public static final int INDEX_ParamCaptureExposureCompensation		= 0x7F0000D1;	/**< reference: OMX_PARAM_S32TYPE */
	public static final int INDEX_ParamBrcmVideoPrecodeForQP			= 0x7F0000D2;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmVideoTimestampFifo			= 0x7F0000D3;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamSWSharpenDisable					= 0x7F0000D4;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ConfigBrcmFlashRequired				= 0x7F0000D5;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmVideoDrmProtectBuffer		= 0x7F0000D6;	/**< reference: OMX_PARAM_BRCMVIDEODRMPROTECTBUFFERTYPE */
	public static final int INDEX_ParamSWSaturationDisable				= 0x7F0000D7;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmVideoDecodeConfigVD3			= 0x7F0000D8;	/**< reference: OMX_PARAM_BRCMVIDEODECODECONFIGVD3TYPE */
	public static final int INDEX_ConfigBrcmPowerMonitor				= 0x7F0000D9;	/**< reference: OMX_CONFIG_BOOLEANTYPE */
	public static final int INDEX_ParamBrcmZeroCopy						= 0x7F0000DA;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE */
	public static final int INDEX_ParamBrcmVideoEGLRenderDiscardMode	= 0x7F0000DB;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE */
	public static final int INDEX_ParamBrcmVideoAVC_VCLHRDEnable		= 0x7F0000DC;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE*/
	public static final int INDEX_ParamBrcmVideoAVC_LowDelayHRDEnable	= 0x7F0000DD;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE*/
	public static final int INDEX_ParamBrcmVideoCroppingDisable			= 0x7F0000DE;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE*/
	public static final int INDEX_ParamBrcmVideoAVCInlineHeaderEnable	= 0x7F0000DF;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE*/

	// 0x7f0000f0
	public static final int INDEX_ConfigBrcmAudioDownmixCoefficients	= 0x7f0000F0;	/**< reference: OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS */
	public static final int INDEX_ConfigBrcmAudioDownmixCoefficients8x8	= 0x7F0000F1;	/**< reference: OMX_CONFIG_BRCMAUDIODOWNMIXCOEFFICIENTS8x8 */
	public static final int INDEX_ConfigBrcmAudioMaxSample				= 0x7F0000F2;	/**< reference: OMX_CONFIG_BRCMAUDIOMAXSAMPLE */
	public static final int INDEX_ConfigCustomAwbGains					= 0x7F0000F3;	/**< reference: OMX_CONFIG_CUSTOMAWBGAINSTYPE */
	public static final int INDEX_ParamRemoveImagePadding				= 0x7F0000F4;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE*/
	public static final int INDEX_ParamBrcmVideoAVCInlineVectorsEnable	= 0x7F0000F5;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE */
	public static final int INDEX_ConfigBrcmRenderStats					= 0x7F0000F6;	/**< reference: OMX_CONFIG_BRCMRENDERSTATSTYPE */
	public static final int INDEX_ConfigBrcmCameraAnnotate				= 0x7F0000F7;	/**< reference: OMX_CONFIG_BRCMANNOTATETYPE */
	public static final int INDEX_ParamBrcmStereoscopicMode				= 0x7F0000F8;	/**< reference :OMX_CONFIG_BRCMSTEREOSCOPICMODETYPE */
	public static final int INDEX_ParamBrcmLockStepEnable				= 0x7F0000F9;	/**< reference: OMX_CONFIG_PORTBOOLEANTYPE */
	public static final int INDEX_ParamBrcmTimeScale					= 0x7F0000FA;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamCameraInterface					= 0x7F0000FB;	/**< reference: OMX_PARAM_CAMERAINTERFACETYPE */
	public static final int INDEX_ParamCameraClockingMode				= 0x7F0000FC;	/**< reference: OMX_PARAM_CAMERACLOCKINGMODETYPE */
	public static final int INDEX_ParamCameraRxConfig					= 0x7F0000FD;	/**< reference: OMX_PARAM_CAMERARXCONFIG_TYPE */
	public static final int INDEX_ParamCameraRxTiming					= 0x7F0000FE;	/**< reference: OMX_PARAM_CAMERARXTIMING_TYPE */
	public static final int INDEX_ParamDynamicParameterConfig			= 0x7F0000FF;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmVideoAVCSPSTimingEnable		= 0x7F000100;	/** reference: OMX_CONFIG_PORTBOOLEANTYPE */
	public static final int INDEX_ParamBrcmBayerOrder					= 0x7F000101;	/** reference: OMX_PARAM_BAYERORDERTYPE */
	public static final int INDEX_ParamBrcmMaxNumCallbacks				= 0x7F000102;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmJpegRestartInterval			= 0x7F000103;	/**< reference: OMX_PARAM_U32TYPE */
	public static final int INDEX_ParamBrcmSupportsSlices				= 0x7F000104;	/**< reference: OMX_CONFIG_PORT_BOOLEANTYPE */
	
	public static final StructPrototype VERSION_TYPE = StructPrototype.builder()
			.addInt8("nVersionMajor")
			.addInt8("nVersionMinor")
			.addInt8("nRevision")
			.addInt8("nStep")
			.build();
	
	public static final StructPrototype VIDEO_PORTDEFTYPE = StructPrototype.builder()
//			.addPointer(StringStructFieldType.ML_128, "cMIMEType")
//			.addPointer("cMIMEType")
			.addPointer("pNativeRender")
			.addInt32("nFrameWidth")
			.addInt32("nFrameHeight")
			.addInt32("nStride")
			.addInt32("nSliceHeight")
			.addInt32("nBitrate")
			.addInt32("xFramerate")
			.addBoolean("bFlagErrorConcealment")
			.addEnum(OMXConstants::mapVideoEncodingType, OMXConstants::unmapVideoEncodingType, "eCompressionFormat")
			.addEnum(OMXConstants::mapColorFormatType, OMXConstants::unmapColorFormatType, "eColorFormat")
			.addPointer("pNativeWindow")
			.build();
	
	public static final StructPrototype AUDIO_PORTDEFTYPE = StructPrototype.builder()
//			.addPointer(StringStructFieldType.ML_128, "cMIMEType")
			.addPointer("cMIMEType")
			.addPointer("pNativeRender")
			.addBoolean("bFlagErrorConcealment")
			.addEnum(OMXConstants::mapAudioEncodingType, OMXConstants::unmapAudioEncodingType, "eEncoding")
			.build();
	
	public static final StructPrototype IMAGE_PORTDEFTYPE = StructPrototype.builder()
//			.addPointer(StringStructFieldType.ML_128, "cMIMEType")
			.addPointer("cMIMEType")
			.addPointer("pNativeRender")
			.addInt32("nFrameWidth")
			.addInt32("nFrameHeight")
			.addInt32("nStride")
			.addInt32("nSliceHeight")
			.addBoolean("bFlagErrorConcealment")
			.addEnum(OMXConstants::mapVideoEncodingType, OMXConstants::unmapVideoEncodingType, "eCompressionFormat")
			.addInt32("eColorFormat")
			.addPointer("pNativeWindow")
			.build();
	
	public static final StructPrototype OTHER_PORTDEFTYPE = StructPrototype.builder()
			.addInt32("eFormat")
			.build();
	
	public static final StructPrototype PARAM_PORTDEFINITIONTYPE = StructPrototype.builder()
			.addInt32("nSize")
			.addStruct(VERSION_TYPE, "nVersion")
			.addInt32("nPortIndex")
			.addInt32("eDir")
			.addInt32("nBufferCountActual")
			.addInt32("nBufferCountMin")
			.addInt32("nBufferSize")
			.addBoolean("bEnabled")
			.addBoolean("bPopulated")
			.addInt32("eDomain")
			.add(UnionPrototype.builder()
					.addStruct(AUDIO_PORTDEFTYPE, "audio")
					.addStruct(VIDEO_PORTDEFTYPE, "video")
					.addStruct(IMAGE_PORTDEFTYPE, "image")
					.addStruct(OTHER_PORTDEFTYPE, "other")
					.build(), "format")
			.addBoolean("bBuffersContiguous")
			.addInt32("nBufferAlignment")
			.build();
	
	public static final StructPrototype PARAM_BITRATETYPE = StructPrototype.builder()
			.addInt32("nSize")
			.addStruct(VERSION_TYPE, "nVersion")
			.addInt32("nPortIndex")
			.addInt32("eControlRate")
			.addInt32("nTargetBitrate")
			.build();
	
	public static final StructPrototype PARAM_VIDEO_PORTFORMATTYPE = StructPrototype.builder()
			.addInt32("nSize")
			.addStruct(VERSION_TYPE, "nVersion")
			.addInt32("nPortIndex")
			.addInt32("nIndex")
			.addEnum(OMXConstants::mapVideoEncodingType, OMXConstants::unmapVideoEncodingType, "eCompressionFormat")
			.addEnum(OMXConstants::mapColorFormatType, OMXConstants::unmapColorFormatType, "eColorFormat")
			.addInt32("xFramerate")
			.build();
	
	public static final OMXControlPrototype CTRL_BITRATE = OMXControlPrototype.builder()
			.setName("bitrate")
			.setQuery(OMXConstants.INDEX_ConfigVideoBitrate)
			.setStruct(PARAM_BITRATETYPE)
			.withNumberField("eControlRate", "controlRate")
			.withNumberField("nTargetBitrate", "target")
			.build();
	
	public static final OMXControlPrototype CTRL_VIDEO_FORMAT = OMXControlPrototype.builder()
			.setName("format")
			.setQuery(OMXConstants.INDEX_ParamVideoPortFormat)
			.setStruct(OMXConstants.PARAM_VIDEO_PORTFORMATTYPE)
			.withNumberField("nIndex", "index")
			.withEnumField("eCompressionFormat", "compression", ImagePalette.class)
			.withEnumField("eColorFormat", "color", ImagePalette.class)
			.withNumberField("xFramerate", "framerate")
			.withEnumerator()
				.and()
			.build();
	
	private static ConcurrentHashMap<String, IndexInfo> indexMap;
	private static class IndexInfo {
		final int index;
		final boolean isConfig;
		public IndexInfo(int index, boolean isConfig) {
			this.index = index;
			this.isConfig = isConfig;
		}
	}
	
	private static void buildIndexMap() {
		if (indexMap != null)
			return;
		synchronized (OMXConstants.class) {
			//Check again, b/c it might have been built since the last check
			if (indexMap != null)
				return;
			indexMap = new ConcurrentHashMap<>();
			for (Field field : OMXConstants.class.getDeclaredFields()) {
				if (field.isSynthetic())
					continue;
				String fieldName = field.getName();
				if (fieldName == null || !fieldName.startsWith("INDEX_"))
					continue;
				String idxName = fieldName.substring("INDEX_".length());
				try {
					IndexInfo info = new IndexInfo(field.getInt(null), idxName.startsWith("Config"));
					indexMap.put(idxName, info);
				} catch (IllegalArgumentException | IllegalAccessException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	public static AudioEncodingType mapAudioEncodingType(int idx) {
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
	
	public static int unmapAudioEncodingType(AudioEncodingType encoding) {
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
	
	public static ImagePalette mapVideoEncodingType(int idx) {
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
			case VIDEO_ENCODING_VP6:
				return ImagePalette.VP6;
			case VIDEO_ENCODING_VP7:
				return ImagePalette.VP7;
			case VIDEO_ENCODING_VP8:
				return ImagePalette.VP8;
			case VIDEO_ENCODING_YUV:
				return ImagePalette.YUV420;
			case VIDEO_ENCODING_SORENSON:
				return ImagePalette.SORENSON;
			case VIDEO_ENCODING_THEORA:
				return ImagePalette.THEORA;
			case VIDEO_ENCODING_MVC:
				return ImagePalette.MVC;
			case VIDEO_ENCODING_AUTO_DETECT:
			default:
				throw new IllegalArgumentException("Cannot map video encoding " + idx);
		}
	}
		
	public static int unmapVideoEncodingType(ImagePalette palette) {
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
			case VP6:
				return VIDEO_ENCODING_VP6;
			case VP7:
				return VIDEO_ENCODING_VP7;
			case VP8:
				return VIDEO_ENCODING_VP8;
			case YUV420:
				return VIDEO_ENCODING_YUV;
			case SORENSON:
				return VIDEO_ENCODING_SORENSON;
			case THEORA:
				return VIDEO_ENCODING_THEORA;
			case MVC:
				return VIDEO_ENCODING_MVC;
			default:
				throw new IllegalArgumentException("Cannot unmap video encoding " + palette);
		}
	}
	
	public static ImagePalette mapColorFormatType(int idx) {
		switch (idx) {
			case COLOR_FORMAT_UNUSED:
				return null;
			case COLOR_FORMAT_MONOCHROME:
				break;
			case COLOR_FORMAT_8BITRGB332:
				return ImagePalette.RGB332;
			case COLOR_FORMAT_12BITRGB444:
				break;
			case COLOR_FORMAT_16BITARGB4444:
				return ImagePalette.RGB444;//TODO not perfect mapping
			case COLOR_FORMAT_16BITARGB1555:
				break;
			case COLOR_FORMAT_16BITRGB565:
				return ImagePalette.RGB565;
			case COLOR_FORMAT_16BITBGR565:
				return ImagePalette.BGR565;
			case COLOR_FORMAT_18BITRGB666:
				return ImagePalette.RGB666;
			case COLOR_FORMAT_18BITARGB1665:
				break;
			case COLOR_FORMAT_19BITARGB1666:
				break;
			case COLOR_FORMAT_24BITRGB888:
				return ImagePalette.RGB24;
			case COLOR_FORMAT_24BITBGR888:
				return ImagePalette.BGR24;
			case COLOR_FORMAT_24BITARGB1887:
				break;
			case COLOR_FORMAT_25BITARGB1888:
				break;
			case COLOR_FORMAT_32BITBGRA8888:
				break;
			case COLOR_FORMAT_32BITARGB8888:
				break;
			case COLOR_FORMAT_YUV411PLANAR:
			case COLOR_FORMAT_YUV411PACKEDPLANAR:
				return ImagePalette.YUV411P;
			case COLOR_FORMAT_YUV420PLANAR:
			case COLOR_FORMAT_YUV420PACKEDPLANAR:
				return ImagePalette.YUV420;
			case COLOR_FORMAT_YUV420SEMIPLANAR:
				return ImagePalette.NV12;
			case COLOR_FORMAT_YUV422PLANAR:
			case COLOR_FORMAT_YUV422PACKEDPLANAR:
				return ImagePalette.YUV422P;
			case COLOR_FORMAT_YUV422SEMIPLANAR:
				return ImagePalette.NV16;
			case COLOR_FORMAT_YCBYCR:
				return ImagePalette.YUYV;
			case COLOR_FORMAT_YCRYCB:
				return ImagePalette.YVYU;
			case COLOR_FORMAT_CBYCRY:
				return ImagePalette.UYVY;
			case COLOR_FORMAT_CRYCBY:
				return ImagePalette.VYUY;
			case COLOR_FORMAT_YUV444INTERLEAVED:
				return ImagePalette.YUV444;
			//TODO figure out which bayer types these are
			case COLOR_FORMAT_RAWBAYER8BIT:
			case COLOR_FORMAT_RAWBAYER10BIT:
			case COLOR_FORMAT_RAWBAYER8BITCOMPRESSED:
				break;
			case COLOR_FORMAT_L2:
				break;
			case COLOR_FORMAT_L4:
				return ImagePalette.Y4;
			case COLOR_FORMAT_L8:
				return ImagePalette.Y8;
			case COLOR_FORMAT_L16:
				return ImagePalette.Y16;
			case COLOR_FORMAT_L24:
				return ImagePalette.Y24;
			case COLOR_FORMAT_L32:
				return ImagePalette.Y32;
			case COLOR_FORMAT_YUV420PACKEDSEMIPLANAR:
				return ImagePalette.NV12;
			case COLOR_FORMAT_YUV422PACKEDSEMIPLANAR:
				return ImagePalette.NV16;
			case COLOR_FORMAT_18BITBGR666:
			case COLOR_FORMAT_24BITARGB6666:
			case COLOR_FORMAT_24BITABGR6666:
			default:
				break;
		}
		throw new IllegalArgumentException("Cannot map " + idx);
	}
	
	public static int unmapColorFormatType(ImagePalette palette) {
		if (palette == null)
			return COLOR_FORMAT_UNUSED;
		switch (palette) {
			case RGB332:
				return COLOR_FORMAT_8BITRGB332;
			case RGB444:
				return COLOR_FORMAT_16BITARGB4444;
			case RGB565:
				return COLOR_FORMAT_16BITRGB565;
			case BGR565:
				return COLOR_FORMAT_16BITBGR565;
			case RGB666:
				return COLOR_FORMAT_18BITRGB666;
			case RGB24:
				return COLOR_FORMAT_24BITRGB888;
			case BGR24:
				return COLOR_FORMAT_24BITBGR888;
			case YUV411P:
				return COLOR_FORMAT_YUV411PACKEDPLANAR;
			case YUV420:
				return COLOR_FORMAT_YUV420PACKEDPLANAR;
			case YUV422P:
				return COLOR_FORMAT_YUV422PACKEDPLANAR;
			case YUYV:
				return COLOR_FORMAT_YCBYCR;
			case YVYU:
				return COLOR_FORMAT_YCRYCB;
			case UYVY:
				return COLOR_FORMAT_CBYCRY;
			case VYUY:
				return COLOR_FORMAT_CRYCBY;
			case YUV444:
				return COLOR_FORMAT_YUV444INTERLEAVED:
			case Y4:
				return COLOR_FORMAT_L4;
			case GRAY:
				return COLOR_FORMAT_L8;
			case Y16:
				return COLOR_FORMAT_L16;
			case Y32:
				return COLOR_FORMAT_L32;
			case NV12:
				return COLOR_FORMAT_YUV420PACKEDSEMIPLANAR;
			case NV16:
				return COLOR_FORMAT_YUV422PACKEDSEMIPLANAR;
			default:
				break;
		}
		throw new IllegalArgumentException("Cannot unmap " + palette);
	}
	
	public static Rational decodeQ16(int q16) {
		float fValue = ((float) q16)/32768.0f;
		//TODO finish
		return null;
	}
	
	public static int encodeQ16(Rational r) {
		float fValue = r.floatValue() * 32768.0f;
		int iValue = (int) (fValue + .5f);
		if (iValue > 32767)
			iValue = 32767;
		if (iValue < -32768)
			iValue = -32768;
		return iValue;
	}
}
