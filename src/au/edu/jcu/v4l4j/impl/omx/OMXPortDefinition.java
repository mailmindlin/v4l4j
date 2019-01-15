package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.api.AudioEncodingType;
import au.edu.jcu.v4l4j.api.ImageCodingType;
import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.component.port.AudioPortDefinition;
import au.edu.jcu.v4l4j.api.component.port.IVPortDefinition;
import au.edu.jcu.v4l4j.api.component.port.ImagePortDefinition;
import au.edu.jcu.v4l4j.api.component.port.PortDefinition;
import au.edu.jcu.v4l4j.api.component.port.VideoPortDefinition;

public class OMXPortDefinition implements PortDefinition {
	public static OMXPortDefinition from(String mime, int[] info) {
		StreamType type = getType(info);
		switch (type) {
			case AUDIO:
				return new AudioDefinition(mime, info);
			case VIDEO:
				return new VideoDefinition(mime, info);
			case IMAGE:
				return new ImageDefinition(mime, info);
			default:
				return new OMXPortDefinition(mime, info);
		}
	}
	
	protected final StreamType type;
	protected String mime;
	protected boolean input;
	protected int bufferCountActual;
	protected int bufferCountMin;
	protected int bufferSize;
	protected boolean enabled;
	protected boolean populated;
	protected long bufferAlignment;
	protected boolean contiguous;
	
	protected static StreamType getType(int[] info) {
		switch (info[5]) {
			case 0:
				return StreamType.AUDIO;
			case 1:
				return StreamType.VIDEO;
			case 2:
				return StreamType.IMAGE;
			case 3:
				if (info[0] < 10)
					throw new IllegalArgumentException("'Other' type ports need at least 10 fields");
				switch (info[10]) {
					case 0:
						return StreamType.CLOCK;
					case 3:
						return StreamType.BINARY;
					case 1:// Power management
					case 2:// Stats
					default:
						return StreamType.UNKNOWN;
				}
			default:
				return StreamType.UNKNOWN;
		}
	}
	
	protected OMXPortDefinition(String mime, int[] info) {
		this(mime, info, 9);
	}
	
	protected OMXPortDefinition(String mime, int[] info, int minInfo) {
		if (info[0] < minInfo)
			throw new IllegalArgumentException("Not enough info (expected >=" + minInfo + "; actual " + info[0] + ")");
		this.mime = mime;
		this.type = getType(info);
		this.input =     ((info[1] & 1 << 0) != 0);
		this.enabled =   ((info[1] & 1 << 1) != 0);
		this.populated = ((info[1] & 1 << 2) != 0);
		this.contiguous= ((info[1] & 1 << 3) != 0);
		this.bufferCountActual = info[2];
		this.bufferCountMin = info[3];
		this.bufferSize = info[4];
		System.out.println("Buffer size: " + this.bufferSize);
		this.bufferAlignment = info[5];
	}
	
	@Override
	public StreamType getType() {
		return type;
	}
	
	@Override
	public boolean isInput() {
		return input;
	}

	@Override
	public boolean isOutput() {
		return !input;
	}

	@Override
	public int actualBuffers() {
		return bufferCountActual;
	}

	@Override
	public int minimumBuffers() {
		return bufferCountMin;
	}

	@Override
	public int bufferSize() {
		return bufferSize;
	}

	@Override
	public boolean isEnabled() {
		return enabled;
	}

	@Override
	public boolean isPopulated() {
		return populated;
	}

	@Override
	public String getMIMEType() {
		return mime;
	}

	@Override
	public boolean buffersContiguous() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public long bufferAlignment() {
		return bufferAlignment;
	}
	
	protected static class AudioDefinition extends OMXPortDefinition implements AudioPortDefinition {
		protected int encoding;
		
		protected AudioDefinition(String mime, int[] info) {
			super(mime, info, 9);
			this.encoding = info[8];
		}

		@Override
		public AudioEncodingType getEncoding() {
			return OMXConstants.mapAudioEncodingType(encoding);
		}
		
	}
	
	protected static abstract class IVDefinition extends OMXPortDefinition implements IVPortDefinition {
		protected int frameWidth;
		protected int frameHeight;
		protected int stride;
		protected int sliceHeight;
		protected int compression;
		protected int format;
		
		protected IVDefinition(String mime, int[] info, int minInfo) {
			super(mime, info, minInfo);
			this.frameWidth = info[8];
			this.frameHeight = info[9];
			this.stride = info[10];
			this.sliceHeight = info[11];
			this.compression = info[12];
			this.format = info[13];
		}

		@Override
		public int getFrameWidth() {
			return frameWidth;
		}

		@Override
		public int getFrameHeight() {
			return frameHeight;
		}

		@Override
		public int getStride() {
			return stride;
		}

		@Override
		public int getSliceHeight() {
			return sliceHeight;
		}

		@Override
		public ImagePalette getColorFormat() {
			return OMXConstants.mapColorFormatType(format);
		}
		
	}
	
	protected static class ImageDefinition extends IVDefinition implements ImagePortDefinition {
		
		protected ImageDefinition(String mime, int[] info) {
			super(mime, info, 14);
		}
	
		@Override
		public ImageCodingType getCodingType() {
			return OMXConstants.mapImageCodingType(compression);
		}
	}
	
	protected static class VideoDefinition extends IVDefinition implements VideoPortDefinition {

		protected VideoDefinition(String mime, int[] info) {
			super(mime, info, 16);
			// TODO Auto-generated constructor stub
		}

		@Override
		public ImagePalette getCompression() {
			return OMXConstants.mapVideoEncodingType(compression);
		}
	}
}
