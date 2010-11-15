package au.edu.jcu.v4l4j;

import java.awt.image.DataBuffer;

/**
 * This class represents a {@link DataBuffer} containing an image
 * obtained though v4l4j.
 * @author gilles
 *
 */
class V4L4JDataBuffer extends DataBuffer {
	private byte byteArray[];
	
	V4L4JDataBuffer(byte array[]) {
		super(TYPE_BYTE, array.length);
		byteArray = array;
	}
	
	public void setNewFrameSize(int s){
		size = s;
	}

	@Override
	public int getElem(int bank, int i) {
		if (bank != 0)
			throw new IndexOutOfBoundsException("Only one bank in this data buffer");
		
		return (int)(byteArray[i]) & 0xff;
	}

	@Override
	public void setElem(int bank, int i, int val) {
		// Do nothing
	}

}
