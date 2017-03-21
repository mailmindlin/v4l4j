package au.edu.jcu.v4l4j.impl.jni;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectInputStream;
import java.io.ObjectOutput;
import java.io.Serializable;

public class StructField implements Serializable {
	private static final long serialVersionUID = 1L;

	protected static StructField read(ObjectInputStream in) throws ClassNotFoundException, IOException {
		String name = in.readUTF();
		StructFieldType type = (StructFieldType) in.readObject();
		int offset = in.readInt();
		
		return new StructField(type, name, offset);
	}
	
	protected final String name;
	protected final StructFieldType type;
	protected final int offset;

	public StructField(StructFieldType type, String name, int offset) {
		this.type = type;
		this.name = name;
		this.offset = offset;
	}

	public String getName() {
		return name;
	}

	public StructFieldType getType() {
		return type;
	}

	public int getSize() {
		return type.getSize();
	}
	
	public int getAlignment() {
		return type.getAlignment();
	}
	
	public int getOffset() {
		return this.offset;
	}
	
	public StructField withOffset(int offset) {
		return new StructField(this.type, this.name, offset);
	}

	@Override
	public String toString() {
		return "StructField{name=" + getName() + ";type=" + getType() +";alignment=" + getAlignment() + ";size=" + getSize() + "}";
	}

	@Deprecated
	public static class PointerStructField extends StructField {
		/**
		 * 
		 */
		private static final long serialVersionUID = 1L;
		protected final boolean dereference;
		protected final StructFieldType remote;

		public PointerStructField(boolean dereference, StructFieldType remote, String name, int offset) {
			super(PrimitiveStructFieldType.RAW_POINTER, name, offset);
			this.dereference = dereference;
			this.remote = remote;
		}
		
		@Override
		public PointerStructField withOffset(int offset) {
			return new PointerStructField(this.dereference, this.remote, this.name, offset);
		}

	}

	private Object writeReplace() {
		return new StructFieldProxy(this);
	}
	
	/**
	 * Proxy class for StructField to let us serialize it without making it mutable
	 * @see {@link www.ibm.com/developerworks/library/j-5things1/}
	 * @author mailmindlin
	 */
	private static class StructFieldProxy implements Externalizable {
		String name;
		StructFieldType type;
		int offset;
		
		/**
		 * Constructor for deserialization
		 */
		@SuppressWarnings("unused")
		private StructFieldProxy() {
			
		}
		
		StructFieldProxy(StructField origin) {
			this.name = origin.name;
			this.type = origin.type;
			this.offset = origin.offset;
		}
		
		private Object readResolve() {
			return new StructField(type, name, offset);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			this.name = in.readUTF();
			this.type = (StructFieldType) in.readObject();
			this.offset = in.readInt();
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeUTF(this.name);
			out.writeObject(type);
			out.writeInt(offset);
		}
	}
}
