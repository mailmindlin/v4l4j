package au.edu.jcu.v4l4j.impl.omx;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.function.Function;
import java.util.function.IntFunction;

import au.edu.jcu.v4l4j.api.ObjIntFunction;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

public class OMXControlPrototype {

	public static OMXControlPrototypeBuilder builder() {
		return new OMXControlPrototypeBuilder();
	}
	
	protected final int queryIdx;
	protected final String name;
	protected final StructPrototype struct;
	protected final List<FieldInfo> children;
	protected final EnumeratorDefinition enumerator;
	
	public OMXControlPrototype(int queryIdx, String name, StructPrototype struct, List<FieldInfo> children, EnumeratorDefinition enumerator) {
		this.queryIdx = queryIdx;
		this.name = name;
		this.struct = struct;
		this.children = children;
		this.enumerator = enumerator;
	}
	
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public BaseOMXQueryControl build(OMXComponent component, int port) {
		BaseOMXQueryControl base = new BaseOMXQueryControl(component, name, queryIdx, port, struct, buildEnumerator(Type.MAP, enumerator));
		for (FieldInfo childDef : children) {
			
			OMXOptionEnumeratorPrototype enumerator = buildEnumerator(childDef.type, childDef.enumerator);
			
			AbstractOMXQueryControl child;
			switch (childDef.type) {
				case ENUM: {
					child = new EnumChildOMXQueryControl(base, port, childDef.name, (Class<? extends Enum>) childDef.param, childDef.sfName);
					break;
				}
				case NUMBER: {
					child = new NumberOMXQueryControl(base, port, childDef.name, childDef.sfName, enumerator);
					break;
				}
				case MAP: {
					child = null;
					break;
				}
				default:
					//Should be impossible
					child = null;
					break;
			}
			base.registerChild(child);
		}
		return base;
	}
	
	@SuppressWarnings("unchecked")
	protected <T> OMXOptionEnumeratorPrototype<T> buildEnumerator(Type type, EnumeratorDefinition enumDef) {
		if (enumDef == null)
			return null;
		ObjIntFunction<NativeStruct, NativeStruct> queryGenerator = (i, n) -> {
			if (n == null)
				n = new NativeStruct(enumDef.struct);
			if (enumDef.indexSfField != null)
				n.put(enumDef.indexSfField, i);
			return n;
		};
		Function<NativeStruct, T> resultMapper = null;
		switch (type) {
			case MAP: {
				//Return a map of some of the returned fields
				final Map<String, String> fieldMap = (Map<String, String>) enumDef.param;
				resultMapper = query -> {
					Map<String, Object> result = new HashMap<>();
					for (Entry<String, String> e : fieldMap.entrySet())
						result.put(e.getValue(), query.get(e.getKey()));
					return (T) result;
				};
				break;
			}
			default: {
				String fieldName = (String) enumDef.param;
				//Map a single field
				resultMapper = query -> ((T) query.get(fieldName));
			}
		}
		return new OMXOptionEnumeratorPrototype<T>(enumDef.queryIdx, queryGenerator, resultMapper);
	}
	
	public static class OMXControlPrototypeBuilder {
		int queryIdx;
		String name;
		StructPrototype struct;
		List<FieldInfo> fields = new ArrayList<>();
		EnumeratorDefinition enumerator;
		
		public OMXControlPrototypeBuilder setQuery(int index) {
			this.queryIdx = index;
			return this;
		}
		
		public OMXControlPrototypeBuilder setName(String name) {
			this.name = name;
			return this;
		}
		
		public OMXControlPrototypeBuilder setStruct(StructPrototype struct) {
			this.struct = struct;
			return this;
		}
		
		public OMXControlPrototypeBuilder withNumberField(String sfName, String name) {
			FieldInfo field = new FieldInfo();
			field.type = Type.NUMBER;
			field.name = name;
			field.sfName = sfName;
			this.fields.add(field);
			return this;
		}
		
		public <T extends Enum<T>> OMXControlPrototypeBuilder withEnumField(String sfName, String name, Class<T> enumClass) {
			FieldInfo field = new FieldInfo();
			field.type = Type.ENUM;
			field.param = enumClass;
			field.name = name;
			field.sfName = sfName;
			this.fields.add(field);
			return this;
		}
		
		public OMXControlPrototype build() {
			return new OMXControlPrototype(this.queryIdx, this.name, this.struct, this.fields, this.enumerator);
		}
		
		public OMXEnumeratorPrototypeBuilder withEnumerator() {
			this.enumerator = new EnumeratorDefinition();
			//We can (kinda) assume that this enumerator will iterate over the same
			//struct/query
			this.enumerator.queryIdx = this.queryIdx;
			this.enumerator.struct = this.struct;
			return new OMXEnumeratorPrototypeBuilder(this.enumerator);
		}
		
		public OMXEnumeratorPrototypeBuilder enumerateField() {
			FieldInfo field = this.fields.get(this.fields.size() - 1);
			if (field.enumerator == null)
				field.enumerator = new EnumeratorDefinition();
			return new OMXEnumeratorPrototypeBuilder(field.enumerator);
		}
		
		public class OMXEnumeratorPrototypeBuilder {
			final EnumeratorDefinition enumDef;
			
			OMXEnumeratorPrototypeBuilder(EnumeratorDefinition enumDef) {
				this.enumDef = enumDef;
			}
			
			public OMXEnumeratorPrototypeBuilder setQuery(int queryIdx) {
				enumDef.queryIdx = queryIdx;
				return this;
			}
			
			public OMXEnumeratorPrototypeBuilder setStruct(StructPrototype struct) {
				enumDef.struct = struct;
				return this;
			}
			
			public OMXEnumeratorPrototypeBuilder setIndexField(String fieldName) {
				enumDef.indexSfField = fieldName;
				return this;
			}
			
			public OMXEnumeratorPrototypeBuilder setResultField(String fieldName) {
				enumDef.param = fieldName;
				return this;
			}
			
			@SuppressWarnings("unchecked")
			public OMXEnumeratorPrototypeBuilder addResultField(String sfName, String rName) {
				if (enumDef.param == null || !(enumDef.param instanceof Map))
					enumDef.param = new HashMap<>();
				((Map<String, String>) enumDef.param).put(sfName, rName);
				return this;
			}
			
			public OMXControlPrototypeBuilder and() {
				if (enumDef.param == null) {
					if (OMXControlPrototypeBuilder.this.enumerator == enumDef) {
						//We can infer fields from the general control prototype
						Map<String, String> protoFields = new HashMap<>();
						for (FieldInfo field : OMXControlPrototypeBuilder.this.fields) {
							if (field.sfName == enumDef.indexSfField)
								continue;
							protoFields.put(field.sfName, field.name);
						}
						enumDef.param = protoFields;
					} else {
						throw new IllegalStateException("Can't iterate over [none] values");
					}
				}
				return OMXControlPrototypeBuilder.this;
			}
		}
	}
	
	static enum Type {
		NUMBER,
		ENUM,
		MAP;
	}
	
	static class FieldInfo {
		String name;
		String sfName;
		Type type;
		Object param;
		EnumeratorDefinition enumerator;
	}
	
	static class EnumeratorDefinition {
		int queryIdx;
		StructPrototype struct;
		String indexSfField = "nIndex";
		/**
		 * Actually a union of String and Map<String, String> (Java doesn't do unions).
		 * Specifies the field(s) of interest for mapping the result.
		 */
		Object param;
	}
}
