package au.edu.jcu.v4l4j.impl.omx;

import java.io.Reader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.function.Function;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import au.edu.jcu.v4l4j.api.ObjIntFunction;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructFieldType;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype.StructPrototypeBuilder;
import au.edu.jcu.v4l4j.impl.omx.OMXControlPrototype.OMXControlPrototypeBuilder.OMXEnumeratorPrototypeBuilder;

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
	
	/**
	 * Build this prototype into a control for the given component and (optional) port.
	 * @param component
	 *     Component to be bound to
	 * @param port
	 *     Port that the control is bound to. {@code -1} if not used.
	 * @return bound control
	 */
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
	
	/**
	 * Control type
	 */
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
	
	public static class OMXControlPrototypeRegistry {
		HashMap<String, StructFieldType> typeRegistry = new HashMap<>();
		HashMap<String, OMXControlPrototype> controlRegistry = new HashMap<>();
		
		private StructFieldType lookupType(String name) {
			if (name == null || name.isEmpty())
				return null;
			return null;//TODO fin
		}
		
		private boolean addField(StructPrototypeBuilder builder, String fieldTypeName, String fieldName) {
			return false;
		}
		
		protected void readTypedef(JSONObject typedef, boolean continueOnError) {
			StructPrototypeBuilder typeBuilder = StructPrototype.builder();
			
			String typeName = typedef.getString("name");
			String kind = typedef.getString("kind");//TODO support unions
			
			JSONArray fields = typedef.getJSONArray("fields");
			final int numFields = fields.length();
			for (int j = 0; j < numFields; j++) {
				JSONObject field = fields.getJSONObject(j);
				String fieldName = field.optString("name", "unknown$" + j);//TODO validate names
				String fieldKindName = field.getString("type");
				if (!addField(typeBuilder, fieldKindName, fieldName)) {
					//We failed to add the field
					if (continueOnError) {
						//Try as int field
						if (addField(typeBuilder, "int32", fieldName)) {
							System.err.print("Warning: Downgraded field '" + typeName + "::" + fieldName + "' from unknown type '" + fieldKindName + "' to type 'int32'");
							continue;
						}
					}
					//TODO: replace with better exception type
					throw new RuntimeException("Unknown type '" + fieldKindName + "' for field '" + fieldName + "' in typedef '" + typeName + "'");
				}
			}
			
			typeRegistry.put(typeName, typeBuilder.build());
		}
		
		protected void readTypes(JSONArray typedefs, boolean continueOnError) {
			if (typedefs == null)
				return;
			final int numTypedefs = typedefs.length();
			for (int i = 0; i < numTypedefs; i++)
				readTypedef(typedefs.getJSONObject(i), continueOnError);
		}
		
		protected void readQuery(JSONObject querydef) {
			OMXControlPrototypeBuilder queryBuilder = OMXControlPrototype.builder();
			
			String queryName = querydef.getString("name");
			queryBuilder.setName(queryName);
			
			String queryIdx = querydef.getString("query");
			//TODO set on builder
			
			String queryTypeName = querydef.getString("type");
			StructFieldType queryType = lookupType(queryTypeName);
			//TODO better exception types
			if (queryType == null)
				throw new RuntimeException("Unknown type '" + queryTypeName + "' in query '" + queryName + "'");
			
			//TODO support primitive base types (boolean control shouldn't need to have a subcontrol for its value).
			if (!(queryType instanceof StructPrototype))
				throw new RuntimeException("Illegal type '" + queryTypeName + "' in query '" + queryName + "'");
			queryBuilder.setStruct((StructPrototype) queryType);
			
			//Parse query fields
			JSONArray queryFieldDefs = querydef.optJSONArray("fields");
			if (queryFieldDefs != null) {
				final int numQueryFieldDefs = queryFieldDefs.length();
				for (int j = 0; j < numQueryFieldDefs; j++) {
					JSONObject queryFieldDef = queryFieldDefs.getJSONObject(j);
					
					String fieldName = queryFieldDef.getString("name");
					String fieldKind = queryFieldDef.getString("kind");
					String sfName = queryFieldDef.getString("sfName");
					
					switch (fieldKind.toLowerCase()) {
						case "number":
							queryBuilder.withNumberField(sfName, fieldName);
							break;
						case "enum":
							try {
								queryBuilder.withEnumField(sfName, fieldName, (Class<? extends Enum>) Class.forName(queryFieldDef.getString("enumClass")));
							} catch (ClassNotFoundException | JSONException e) {
								throw new RuntimeException(e);
							}
							break;
						default:
							throw new RuntimeException("Unknwon field type");
					}
					
					JSONObject enumeratorDef = queryFieldDef.optJSONObject("enumerator");
					if (enumeratorDef != null) {
						OMXEnumeratorPrototypeBuilder enumeratorBuilder = queryBuilder.enumerateField();
						//TODO finish
						enumeratorBuilder.and();
					}
				}
			}
		}
		
		protected void readQueries(JSONArray querydefs, boolean continueOnError) {
			if (querydefs == null)
				return;
			final int numQueries = querydefs.length();
			for (int i = 0; i < numQueries; i++)
				readQuery(querydefs.getJSONObject(i));//TODO store
		}
		
		void read(Reader reader, boolean continueOnError) {
			JSONObject base = new JSONObject(reader);
			//First read in types
			readTypes(base.optJSONArray("types"), continueOnError);
			
			readQueries(base.optJSONArray("queries"), continueOnError);
		}
	}
}
