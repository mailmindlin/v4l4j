package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderResult;
import java.nio.charset.StandardCharsets;

public class StringStructFieldType implements StructFieldType<String> {
	private static final long serialVersionUID = 2703178802386680298L;
	
	/**
	 * String of size 128. Shows up often.
	 */
	public static final StringStructFieldType ML_128 = new StringStructFieldType(128);
	
	protected final int maxLength;
	protected final Charset charset;

	public StringStructFieldType(int maxLength) {
		this(StandardCharsets.UTF_8, maxLength);
	}
	
	public StringStructFieldType(Charset charset, int maxLength) {
		this.charset = charset;
		this.maxLength = maxLength;
	}
	
	public int getLength() {
		return this.maxLength;
	}
	
	@Override
	public boolean expands() {
		return false;
	}
	
	@Override
	public int getAlignment() {
		return PrimitiveStructFieldType.CHAR.getAlignment();
	}
	
	@Override
	public int getSize() {
		return PrimitiveStructFieldType.CHAR.getSize() * this.maxLength;
	}
	
	public Charset getCharset() {
		return this.charset;
	}
	
	@SuppressWarnings({ "unchecked", "rawtypes" })
	public ArrayStructFieldType<Character> asArray() {
		return new ArrayStructFieldType<Character>((StructFieldType) PrimitiveStructFieldType.CHAR, this.maxLength);
	}

	@Override
	public void write(ByteBuffer buffer, String value) {
		// TODO Auto-generated method stub
		//super.write(buffer, value);
		throw new UnsupportedOperationException();
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
		
		CharsetDecoder decoder = this.charset.newDecoder();
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
