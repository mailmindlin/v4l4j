package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;
import java.nio.charset.StandardCharsets;

public class StringStructFieldType extends ArrayStructFieldType {
	private static final long serialVersionUID = 2703178802386680298L;
	
	/**
	 * String of size 128. Shows up often.
	 */
	public static final StringStructFieldType ML_128 = new StringStructFieldType(128);

	public StringStructFieldType(int maxLength) {
		super(PrimitiveStructFieldType.INT8, maxLength);
	}

	@Override
	public void write(ByteBuffer buffer, Object value) {
		// TODO Auto-generated method stub
		super.write(buffer, value);
	}
	
	protected int strlen(ByteBuffer buffer) {
		//Fast strlen (looks for null terminator)
		//see http://www.stdlib.net/~colmmacc/strlen.c.html
		//TODO testme
		final int lim = buffer.limit();
		int i;
		for (i = buffer.position(); i < lim; i+= 4) {
			int v = buffer.getInt(i);
			if (buffer.order() == ByteOrder.LITTLE_ENDIAN)
				v = Integer.reverseBytes(v);
			if ((v & 0xFF000000) == 0)
				return i;
			else if ((v & 0x00FF0000) == 0)
				return i + 1;
			else if ((v & 0x0000FF00) == 0)
				return i + 2;
			else if ((v & 0x000000FF) == 0)
				return i + 3;
		}
		switch (lim - (i - 4)) {
			case 3:
				if (buffer.get(i - 3) == 0)
					return i - 3;
			case 2:
				if (buffer.get(i - 2) == 0)
					return i - 2;
			case 1:
			case 0:
		}
		return lim;
	}

	@Override
	public String read(ByteBuffer buffer, StructReadingContext parentContext) {
		//Find the null terminator
		buffer.limit(strlen(buffer));
		
		CharsetDecoder decoder = StandardCharsets.UTF_8.newDecoder();
		CharBuffer out = CharBuffer.allocate((int) (buffer.remaining() * decoder.averageCharsPerByte()));
		
		//Decode bytes 
		CoderResult decodeResult = decoder.decode(buffer, out, true);
		
		StringBuffer result = new StringBuffer(out.capacity());
		out.flip();
		result.append(out);
		
		while (decodeResult == CoderResult.UNDERFLOW) {
			out.clear();
			decodeResult = decoder.flush(out);
			out.flip();
			result.append(out);
		}
		
		return result.toString();
	}
	
	
	
}
