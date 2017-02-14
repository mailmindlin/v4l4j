package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.util.Set;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.StreamType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.control.Control;

public class OMXComponentPort implements ComponentPort {
	protected final int id;
	protected final OMXComponent component;
	protected final String mime;
	protected final StreamType type;
	protected boolean input;
	protected int bufferCountActual;
	protected int bufferCountMin;
	protected int bufferSize;
	protected boolean enabled;
	protected boolean populated;
	
	protected OMXComponentPort(OMXComponent component, int id, String mime, StreamType type) {
		this.component = component;
		this.id = id;
		this.mime = mime;
		this.type = type;
	}
	
	protected OMXComponentPort(OMXComponent component, int id, String mime, int[] info) {
		this.component = component;
		this.id = id;
		this.mime = mime;
		
		if (info[0] < 9)
			throw new IllegalArgumentException("Minimum info length is 9");
		this.input = info[1] != 0;
		this.bufferCountActual = info[2];
		this.bufferCountMin = info[3];
		this.bufferSize = info[4];
		this.enabled = info[5] != 0;
		this.populated = info[6] != 0;
		switch (info[7]) {
			case 0:
				this.type = StreamType.AUDIO;
				break;
			case 1:
				this.type = StreamType.VIDEO;
				break;
			case 2:
				this.type = StreamType.IMAGE;
				break;
			case 3:
				//Other type; let's try to guess from the otherDomain field
				if (info[0] < 10)
					throw new IllegalArgumentException("'Other' type ports need at least 10 fields");
				switch (info[10]) {
					case 0:
						this.type = StreamType.CLOCK;
						break;
					case 3:
						this.type = StreamType.BINARY;
						break;
					case 1://Power management
					case 2://Stats
					default:
						this.type = StreamType.UNKNOWN;
						break;
				}
				break;
			default:
				//Is some proprietary thing
				this.type = StreamType.UNKNOWN;
		}
		//We don't use the other stuff ATM
	}

	@Override
	public int getIndex() {
		return this.id;
	}

	@Override
	public OMXComponent getComponent() {
		return this.component;
	}
	
	@Override
	public boolean isInput() {
		return this.input;
	}

	@Override
	public boolean isOutput() {
		return !isInput();
	}

	@Override
	public boolean isEnabled() {
		return this.enabled;
	}

	@Override
	public boolean setEnabled(boolean aflag) {
		this.getComponent().setPortEnabled(this.getIndex(), aflag);
		return this.enabled = aflag;
	}

	@Override
	public boolean isPopulated() {
		return this.populated;
	}

	@Override
	public OMXFrameBuffer allocateBuffer(int length) {
		OMXFrameBuffer result = this.getComponent().allocateBufferOnPort(this.getIndex(), length);
		this.bufferCountActual++;
		return result;
	}

	@Override
	public int minimumBuffers() {
		return this.bufferCountMin;
	}

	@Override
	public int actualBuffers() {
		return this.bufferCountActual;
	}

	@Override
	public int bufferSize() {
		return this.bufferSize;
	}

	@Override
	public FrameBuffer useBuffer(ByteBuffer buffer) {
		this.getComponent().useBufferOnPort(this.getIndex(), buffer);
		return null;
	}

	@Override
	public void empty(FrameBuffer buffer) {
		this.getComponent().emptyThisBuffer((OMXFrameBuffer) buffer);
	}

	@Override
	public void fill(FrameBuffer buffer) {
		this.getComponent().fillThisBuffer((OMXFrameBuffer) buffer);
	}

	@Override
	public Set<Control> getChildren() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Control getChildByName(String name) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getName() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void push() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void pull() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public StreamType getPortType() {
		return this.type;
	}

	@Override
	public String getMIMEType() {
		return this.mime;
	}
}