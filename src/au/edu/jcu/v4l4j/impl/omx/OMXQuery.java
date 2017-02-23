package au.edu.jcu.v4l4j.impl.omx;

import java.util.HashMap;
import java.util.Map;

import au.edu.jcu.v4l4j.impl.jni.StructField;
import au.edu.jcu.v4l4j.impl.jni.PrimitiveStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

public class OMXQuery extends StructPrototype {
	public static final StructPrototype VERSION_TYPE = StructPrototype.builder()
			.addInt8("nVersionMajor")
			.addInt8("nVersionMinor")
			.addInt8("nRevision")
			.addInt8("nStep")
			.build();
	
	public static final StructPrototype CONFIG_BOOL = OMXQuery.builder()
			.addBoolean("bEnabled")
			.build();
	
	protected static StructField[] injectHeader(StructField...fields) {
		StructField[] result = new StructField[fields.length + 2];
		result[0] = new StructField(PrimitiveStructFieldType.INT32, "nSize", -1);
		result[1] = new StructField(VERSION_TYPE, "nVersion", -1);
		System.arraycopy(fields, 0, result, 2, fields.length);
		return result;
	}
	
	public static StructPrototypeBuilder builder() {
		return new StructPrototypeBuilder() {
			@Override
			public StructPrototype build() {
				return new OMXQuery(this.fields.toArray(new StructField[this.fields.size()]));
			}
		};
	}
	
	protected Map<String, Object> bindings;
	
	public OMXQuery(StructField...fields) {
		super(fields);
		this.bindings = new HashMap<>();
	}
	
	protected OMXQuery(StructField[] fields, Map<String, StructField> fieldMap, Map<String, Object> bindings) {
		super(fields, fieldMap);
		this.bindings = bindings;
	}
	
	public OMXQuery duplicate() {
		return new OMXQuery(this.fields, this.fieldMap, new HashMap<>(bindings));
	}
	
	public OMXQuery bind(String key, Object value) {
		this.bindings.put(key, value);
		return this;
	}
	
	
}
