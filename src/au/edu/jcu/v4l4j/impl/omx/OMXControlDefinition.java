package au.edu.jcu.v4l4j.impl.omx;

import java.io.Reader;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.function.BiPredicate;
import java.util.function.Function;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import au.edu.jcu.v4l4j.api.ObjIntFunction;
import au.edu.jcu.v4l4j.impl.jni.AbstractMappingStructFieldType.MappingStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.ArrayStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.PointerStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.PrimitiveStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.QStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.StructFieldType;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype.StructPrototypeBuilder;
import au.edu.jcu.v4l4j.impl.jni.UnionPrototype;
import au.edu.jcu.v4l4j.impl.jni.UnionPrototype.UnionPrototypeBuilder;

/**
 * The definition of an OMX control
 */
public class OMXControlDefinition {

	public static OMXControlDefinitionBuilder builder() {
		return new OMXControlDefinitionBuilder();
	}
	
	protected final int queryIdx;
	protected final String name;
	protected final StructPrototype struct;
	protected final List<FieldInfo> children;
	protected final BiPredicate<OMXComponent, Integer> filter;
	protected final EnumeratorDefinition enumerator;
	
	public OMXControlDefinition(int queryIdx, String name, StructPrototype struct, List<FieldInfo> children, BiPredicate<OMXComponent, Integer> filter, EnumeratorDefinition enumerator) {
		this.queryIdx = queryIdx;
		this.name = name;
		this.struct = struct;
		this.children = children;
		this.filter = filter;
		this.enumerator = enumerator;
	}
	
	/**
	 * Test if this control is applicable for the given port on the component
	 * @param component
	 * @param port
	 * @return if this control is applicable
	 */
	public boolean test(OMXComponent component, int port) {
		if (this.filter == null)
			return false;
		
		return this.filter.test(component, port);
	}
	/**
	 * Build this prototype into a control for the given component and (optional) port.
	 * @param component
	 *     Component to be bound to
	 * @param port
	 *     Port that the control is bound to. {@code -1} if not used.
	 * @return bound control
	 */
	public BaseOMXQueryControl build(OMXComponent component, int port) {
		BaseOMXQueryControl base = new BaseOMXQueryControl(component, name, queryIdx, port, struct, buildEnumerator(Type.MAP, enumerator));
		for (FieldInfo childDef : children)
			base.registerChild(makeChild(port, base, childDef));
		return base;
	}
	
	@SuppressWarnings({ "unchecked", "rawtypes" })
	protected AbstractOMXQueryControl<?> makeChild(int port, AbstractOMXQueryControl<?> base, FieldInfo childDef) {
		OMXOptionEnumeratorPrototype<?> enumerator = buildEnumerator(childDef.type, childDef.enumerator);
		
		switch (childDef.type) {
			case ENUM:
				return new EnumChildOMXQueryControl<>(base, port, childDef.name, (Class<? extends Enum>) childDef.param, childDef.sfName);
			case NUMBER:
				return new NumberOMXQueryControl(base, port, childDef.name, childDef.sfName, (OMXOptionEnumeratorPrototype<Number>) enumerator);
			case ENDPOINT:
				return new ChildEndpointOMXQueryControl<>(base, port, childDef.name, childDef.sfName, enumerator);
			case MAP: {
				CompositeOMXQueryControl child = new CompositeOMXQueryControl(base, port, childDef.name, childDef.sfName, (OMXOptionEnumeratorPrototype<Map<String, Object>>) enumerator);
				for (FieldInfo grandchild : ((List<FieldInfo>) childDef.param))
					child.registerChild(makeChild(port, child, grandchild));
				return child;
			}
			default:
				//Should be impossible
				return null;
		}
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
	
	public Object toJSON() {
		JSONObject result = new JSONObject();
		result.put("name", this.name);
		result.put("enumerator", this.enumerator == null ? null : this.enumerator.toJSON());
		return result;
	}
	
	
	public static class OMXCompositeDefinitionBuilder<T extends OMXCompositeDefinitionBuilder<?>> {
		protected final T owner;
		protected final List<FieldInfo> fields = new ArrayList<>();
		protected EnumeratorDefinition enumerator;
		protected final String sfName;
		protected final String name;
		
		protected OMXCompositeDefinitionBuilder() {
			this(null, null, null);
		}
		
		protected OMXCompositeDefinitionBuilder(T owner, String sfName, String name) {
			this.owner = owner;
			this.name = name;
			this.sfName = sfName;
		}

		public OMXCompositeDefinitionBuilder<T> withNumberField(String sfName, String name) {
			FieldInfo field = new FieldInfo();
			field.type = Type.NUMBER;
			field.name = name;
			field.sfName = sfName;
			this.fields.add(field);
			return this;
		}
		
		public OMXCompositeDefinitionBuilder<T> withField(String sfName, String name) {
			FieldInfo field = new FieldInfo();
			field.type = Type.ENDPOINT;
			field.name = name;
			field.sfName = sfName;
			this.fields.add(field);
			return this;
		}
		
		public OMXCompositeDefinitionBuilder<? extends OMXCompositeDefinitionBuilder<T>> withCompositeField(String sfName, String name) {
			return new OMXCompositeDefinitionBuilder<>(this, sfName, name);
		}
		
		public <E extends Enum<E>> OMXCompositeDefinitionBuilder<T> withEnumField(String sfName, String name, Class<E> enumClass) {
			FieldInfo field = new FieldInfo();
			field.type = Type.ENUM;
			field.param = enumClass;
			field.name = name;
			field.sfName = sfName;
			this.fields.add(field);
			return this;
		}
		
		public OMXEnumeratorPrototypeBuilder<? extends OMXCompositeDefinitionBuilder<T>> withEnumerator() {
			this.enumerator = new EnumeratorDefinition();
			return new OMXEnumeratorPrototypeBuilder<>(this, this.enumerator);
		}
		
		public OMXEnumeratorPrototypeBuilder<? extends OMXCompositeDefinitionBuilder<T>> enumerateField() {
			FieldInfo field = this.fields.get(this.fields.size() - 1);
			if (field.enumerator == null)
				field.enumerator = new EnumeratorDefinition();
			return new OMXEnumeratorPrototypeBuilder<>(this, field.enumerator);
		}
		
		public T and() {
			FieldInfo field = new FieldInfo();
			field.enumerator = this.enumerator;
			field.type = Type.MAP;
			field.name = this.name;
			field.sfName = this.sfName;
			field.param = this.fields;
			this.owner.fields.add(field);
			return this.owner;
		}
	}
	
	public static class OMXEnumeratorPrototypeBuilder<U extends OMXCompositeDefinitionBuilder<?>> {
		final U owner;
		final EnumeratorDefinition enumDef;
		
		protected OMXEnumeratorPrototypeBuilder(U owner, EnumeratorDefinition enumDef) {
			this.owner = owner;
			this.enumDef = enumDef;
		}
		
		public OMXEnumeratorPrototypeBuilder<U> setQuery(int queryIdx) {
			enumDef.queryIdx = queryIdx;
			return this;
		}
		
		public OMXEnumeratorPrototypeBuilder<U> setStruct(StructPrototype struct) {
			enumDef.struct = struct;
			return this;
		}
		
		public OMXEnumeratorPrototypeBuilder<U> setIndexField(String fieldName) {
			enumDef.indexSfField = fieldName;
			return this;
		}
		
		public OMXEnumeratorPrototypeBuilder<U> setResultField(String fieldName) {
			enumDef.param = fieldName;
			return this;
		}
		
		@SuppressWarnings("unchecked")
		public OMXEnumeratorPrototypeBuilder<U> addResultField(String sfName, String rName) {
			if (enumDef.param == null || !(enumDef.param instanceof Map))
				enumDef.param = new HashMap<>();
			((Map<String, String>) enumDef.param).put(sfName, rName);
			return this;
		}
		
		public U and() {
			if (enumDef.param == null) {
				if (owner.enumerator == enumDef) {
					//We can infer fields from the general control prototype
					Map<String, String> protoFields = new HashMap<>();
					for (FieldInfo field : owner.fields) {
						if (field.sfName == enumDef.indexSfField)
							continue;
						protoFields.put(field.sfName, field.name);
					}
					enumDef.param = protoFields;
				} else {
					throw new IllegalStateException("Can't iterate over [none] values");
				}
			}
			
			return owner;
		}
	}
	
	public static class OMXControlDefinitionBuilder extends OMXCompositeDefinitionBuilder<OMXControlDefinitionBuilder> {
		int queryIdx;
		String name;
		StructPrototype struct;
		BiPredicate<OMXComponent, Integer> filter;
		
		public OMXControlDefinitionBuilder setQuery(int index) {
			this.queryIdx = index;
			return this;
		}
		
		public OMXControlDefinitionBuilder setName(String name) {
			this.name = name;
			return this;
		}
		
		public OMXControlDefinitionBuilder setStruct(StructPrototype struct) {
			this.struct = struct;
			return this;
		}
		
		public OMXControlDefinitionBuilder setFilter(BiPredicate<OMXComponent, Integer> filter) {
			this.filter = filter;
			return this;
		}
		
		@Override
		public OMXControlDefinitionBuilder withNumberField(String sfName, String name) {
			super.withNumberField(sfName, name);
			return this;
		}
		
		@Override
		public OMXControlDefinitionBuilder withField(String sfName, String name) {
			super.withField(sfName, name);
			return this;
		}

		@Override
		@SuppressWarnings("unchecked")
		public OMXCompositeDefinitionBuilder<OMXControlDefinitionBuilder> withCompositeField(String sfName, String name) {
			return (OMXCompositeDefinitionBuilder<OMXControlDefinitionBuilder>) super.withCompositeField(sfName, name);
		}
		
		@Override
		public <T extends Enum<T>> OMXControlDefinitionBuilder withEnumField(String sfName, String name, Class<T> enumClass) {
			super.withEnumField(sfName, name, enumClass);
			return this;
		}
		
		@Override
		public OMXEnumeratorPrototypeBuilder<OMXControlDefinitionBuilder> withEnumerator() {
			this.enumerator = new EnumeratorDefinition();
			//We can (kinda) assume that this enumerator will iterate over the same
			//struct/query
			this.enumerator.queryIdx = this.queryIdx;
			this.enumerator.struct = this.struct;
			return new OMXEnumeratorPrototypeBuilder<>(this, this.enumerator);
		}

		@Override
		@SuppressWarnings("unchecked")
		public OMXEnumeratorPrototypeBuilder<OMXControlDefinitionBuilder> enumerateField() {
			return (OMXEnumeratorPrototypeBuilder<OMXControlDefinitionBuilder>) super.enumerateField();
		}
		
		@Override
		public OMXControlDefinitionBuilder and() {
			return this;
		}
		
		public OMXControlDefinition build() {
			return new OMXControlDefinition(this.queryIdx, this.name, this.struct, this.fields, this.filter, this.enumerator);
		}
	}
	
	/**
	 * Control type
	 */
	static enum Type {
		NUMBER,
		ENDPOINT,
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
		public Object toJSON() {
			JSONObject result = new JSONObject();
			result.put("query", this.queryIdx);
			result.put("struct", this.struct.toString());
			result.put("indexSfField", this.indexSfField);
			result.put("param", "" + this.param);
			return result;
		}
	}
	
	public static class OMXControlDefinitionRegistry {
		HashMap<String, StructFieldType<?>> typeRegistry = new HashMap<>();
		HashMap<String, OMXControlDefinition> controlRegistry = new HashMap<>();
		
		public BaseOMXQueryControl makeControl(OMXComponent component, int port, String controlName) {
			OMXControlDefinition definition = controlRegistry.get(controlName);
			//System.out.println(((JSONObject)definition.toJSON()).toString(2));
			return definition.build(component, port);
		}
		
		/**
		 * Register definition under its own name
		 * @param definition
		 */
		public void registerDefinition(OMXControlDefinition definition) {
			this.controlRegistry.put(definition.name, definition);
		}
		
		/**
		 * Look up a {@link StructFieldType} from a string identifier.
		 * <p>
		 * Valid identifiers consist of:
		 * <ul>
		 * <li>A primitive name (e.g., {@code bool}, {@code unsigned long long int})</li>
		 * <li>A previously defined type</li>
		 * <li>A pointer to a type (a pointer to some type {@code T} is written as {@code T*})</li>
		 * <li>An array of a type (an array of some type {@code T} of size {@code n} is written as {@code T[n]}</li>
		 * </ul>
		 * </p>
		 * @param type Type name to resolve
		 * @return StructFieldType for the given name, else {@code null} if the name cannot be resolved.
		 */
		private StructFieldType<?> lookupType(String type) {
			if (type == null || type.isEmpty())
				return null;
			
			return typeRegistry.computeIfAbsent(type, name -> {
				//Compute type
				
				//Check if it's an alias for one of the primitives
				switch (name) {
					case "bool":
					case "boolean":
						return PrimitiveStructFieldType.BOOL;
					case "char":
						return PrimitiveStructFieldType.CHAR;
					case "signed char":
						return PrimitiveStructFieldType.SCHAR;
					case "unsigned char":
						return PrimitiveStructFieldType.UCHAR;
					case "short":
					case "short int":
					case "signed short":
					case "signed short int":
						return PrimitiveStructFieldType.SHORT;
					case "unsigned short":
					case "unsigned short int":
						return PrimitiveStructFieldType.USHORT;
					case "int":
					case "signed":
					case "signed int":
						return PrimitiveStructFieldType.INT;
					case "unsigned":
					case "unsigned int":
						return PrimitiveStructFieldType.UINT;
					case "long":
					case "long int":
					case "signed long":
					case "signed long int":
						return PrimitiveStructFieldType.LONG;
					case "unsigned long":
					case "unsigned long int":
						return PrimitiveStructFieldType.ULONG;
					case "long long":
					case "long long int":
					case "signed long long":
					case "signed long long int":
						return PrimitiveStructFieldType.LLONG;
					case "unsigned long long":
					case "unsigned long long int":
						return PrimitiveStructFieldType.ULLONG;
					case "float":
						return PrimitiveStructFieldType.FLOAT;
					case "double":
						return PrimitiveStructFieldType.DOUBLE;
					case "long double":
						return PrimitiveStructFieldType.LONG_DOUBLE;
					case "i8":
					case "int8":
						return PrimitiveStructFieldType.INT8;
					case "u8":
					case "uint8":
						return PrimitiveStructFieldType.UINT8;
					case "i16":
					case "int16":
						return PrimitiveStructFieldType.INT16;
					case "u16":
					case "uint16":
						return PrimitiveStructFieldType.UINT16;
					case "i32":
					case "int32":
						return PrimitiveStructFieldType.INT32;
					case "u32":
					case "uint32":
						return PrimitiveStructFieldType.UINT32;
					case "i64":
					case "int64":
						return PrimitiveStructFieldType.UINT64;
					case "u64":
					case "uint64":
						return PrimitiveStructFieldType.INT64;
					case "f32":
					case "float32":
						return PrimitiveStructFieldType.FLOAT32;
					case "f64":
					case "float64":
						return PrimitiveStructFieldType.FLOAT64;
					case "void*":
						return PrimitiveStructFieldType.RAW_POINTER;
					case "q16":
					case "q16.16":
						return QStructFieldType.Q16;
					default:
						break;
				}
				if (name.endsWith("]")) {
					//Is array type
					int openBracketIdx = name.lastIndexOf('[');
					if (openBracketIdx < 0)
						return null;
					//TODO check indicies
					//TODO support vararrays
					int arraySize = Integer.parseInt(name.substring(openBracketIdx + 1, name.length() - 1));
					StructFieldType<?> baseType = lookupType(name.substring(0, openBracketIdx));
					if (baseType == null)
						return null;
					return new ArrayStructFieldType<>(baseType, arraySize);
				} else if (name.endsWith("*")) {
					//Pointer
					StructFieldType<?> baseType = lookupType(name.substring(0, name.length() - 1));
					if (baseType == null)
						return null;
					return new PointerStructFieldType<>(baseType);
				}
				return null;
			});
		}
		
		/**
		 * Reads struct/query type definition in form of:
		 * <pre>
		 * interface StructTypeDefinition extends TypeDefinition {
		 *     kind: 'struct';
		 *     fields: ({name: string, type: string} | [string, string]) [];
		 * }
		 * </pre>
		 * @param typedef
		 * @param continueOnError
		 * @return
		 */
		protected StructPrototypeBuilder readStructTypedef(JSONObject typedef, String typeName, boolean continueOnError) {
			StructPrototypeBuilder typeBuilder = StructPrototype.builder();
			JSONArray fields;
			try {
				fields = typedef.getJSONArray("fields");
			} catch (JSONException e) {
				throw new IllegalArgumentException("Cannot get fields for " + typeName, e);
			}
			final int numFields = fields.length();
			for (int j = 0; j < numFields; j++) {
				String fieldName, fieldKindName;
				
				//Field definition is either an object or an array (for shorthand)
				JSONObject fieldDefObj = fields.optJSONObject(j);
				if (fieldDefObj != null) {
					fieldName = fieldDefObj.optString("name", "unknown$" + j);
					fieldKindName = fieldDefObj.getString("type");
				} else {
					//Shorthand array used
					JSONArray fieldDefArr = fields.getJSONArray(j);
					fieldName = fieldDefArr.getString(0);
					fieldKindName = fieldDefArr.getString(1);
				}
				
				//TODO validate names
				
				StructFieldType<?> fieldType = lookupType(fieldKindName);
				if (fieldType == null) {
					//We failed to add the field
					if (continueOnError) {
						//Try as int field
						fieldType = PrimitiveStructFieldType.INT;
						System.err.print("Warning: Downgraded field '" + typedef.getString("name") + "::" + fieldName + "' from unknown type '" + fieldKindName + "' to type 'int32'");
						continue;
					}
					//TODO: replace with better exception type
					throw new RuntimeException("Unknown type '" + fieldKindName + "' for field '" + fieldName + "' in typedef '" + typedef.getString("name") + "'");
				}
				
				typeBuilder.add(fieldType, fieldName);
			}
			
			return typeBuilder;
		}
		
		protected UnionPrototype readUnionTypedef(JSONObject typedef, boolean continueOnError) {
			UnionPrototypeBuilder typeBuilder = UnionPrototype.builder();
			JSONObject fieldMap = typedef.optJSONObject("fields");
			JSONArray fields;
			if (fieldMap != null) {
				for (Entry<String, Object> e : fieldMap.toMap().entrySet()) {
					String fieldName = e.getKey();
					
					String fieldKindName = e.getValue().toString();
					StructFieldType<?> fieldType = lookupType(fieldKindName);
					if (fieldType == null) {
						//We failed to add the field
						if (continueOnError) {
							//Try as int field
							fieldType = PrimitiveStructFieldType.INT;
							System.err.print("Warning: Downgraded field '" + typedef.getString("name") + "::" + fieldName + "' from unknown type '" + fieldKindName + "' to type 'int32'");
							continue;
						}
						//TODO: replace with better exception type
						throw new RuntimeException("Unknown type '" + fieldKindName + "' for field '" + fieldName + "' in typedef '" + typedef.getString("name") + "'");
					}
					typeBuilder.add(fieldType, fieldName);
				}
			} else if ((fields = typedef.optJSONArray("fields")) != null) {
				final int numFields = fields.length();
				for (int j = 0; j < numFields; j++) {
					JSONObject field = fields.getJSONObject(j);
					String fieldName = field.optString("name", "unknown$" + j);//TODO validate names
					
					String fieldKindName = field.getString("type");
					StructFieldType<?> fieldType = lookupType(fieldKindName);
					if (fieldType == null) {
						//We failed to add the field
						if (continueOnError) {
							//Try as int field
							fieldType = PrimitiveStructFieldType.INT;
							System.err.print("Warning: Downgraded field '" + typedef.getString("name") + "::" + fieldName + "' from unknown type '" + fieldKindName + "' to type 'int32'");
							continue;
						}
						//TODO: replace with better exception type
						throw new RuntimeException("Unknown type '" + fieldKindName + "' for field '" + fieldName + "' in typedef '" + typedef.getString("name") + "'");
					}
					
					typeBuilder.add(fieldType, fieldName);
				}
			}
			return typeBuilder.build();
		}
		
		/**
		 * Read type definition in form of:
		 * <pre>
		 * interface TypeDefinition {
		 *     name: string;
		 *     kind: 'alias' | 'enum' | 'query' | 'struct' | 'union';
		 * }
		 * interface AliasDefinition extends TypeDefinition {
		 *     kind: 'alias';
		 *     alias: string;
		 * }
		 * interface EnumTypeDefinition extends TypeDefinition {
		 *     kind: 'enum';
		 *     values: string[];
		 * }
		 * </pre>
		 * See {@link #readStructTypedef(JSONObject, boolean)} for struct & query type definitions.
		 * See {@link #readUnionTypedef(JSONObject, boolean)} for union type definitions.
		 * @param typedef
		 * @param continueOnError
		 */
		protected void readTypedef(JSONObject typedef, boolean continueOnError) {			
			String typeName = typedef.getString("name");
			String kind = typedef.getString("kind");
			
			switch (kind) {
				case "alias": {
					String targetName = typedef.getString("alias");
					StructFieldType<?> target = lookupType(targetName);
					if (target == null) {
						if (continueOnError) {
							target = lookupType("i32");
							System.err.print("Warning: Downgraded alias '" + typeName + "' from unknown type '" + targetName + "' to type 'int32'");
						} else {
							throw new RuntimeException("Unknown type '" + targetName + "' for alias '" + typeName + "'");
						}
					}
					typeRegistry.put(typeName, target);
					break;
				}
				case "enum": {
					JSONArray values = typedef.getJSONArray("values");
					final List<String> names = new ArrayList<>(values.length());
					for (int i = 0; i < values.length(); i++)
						names.add(i, values.getString(i));
					//TODO generate (better) mapping
					MappingStructFieldType<?> field = new MappingStructFieldType<>(names::get, names::indexOf);
					typeRegistry.put(typeName, field);
					break;
				}
				case "query":
					typeRegistry.put(typeName, readStructTypedef(typedef, typeName, continueOnError)
							.add(0, OMXConstants.VERSION_TYPE, "version")
							.add(0, PrimitiveStructFieldType.INT32, "size")
							//Add port?
							.build());
					break;
				case "struct":
					typeRegistry.put(typeName, readStructTypedef(typedef, typeName, continueOnError).build());
					break;
				case "union":
					typeRegistry.put(typeName, readUnionTypedef(typedef, continueOnError));
					break;
				default:
					throw new IllegalArgumentException("Unknown typedef kind: " + kind);
			}
		}
		
		protected void readTypes(JSONArray typedefs, boolean continueOnError) {
			if (typedefs == null)
				return;
			final int numTypedefs = typedefs.length();
			for (int i = 0; i < numTypedefs; i++)
				readTypedef(typedefs.getJSONObject(i), continueOnError);
		}
		
		protected int lookupQueryIndex(String queryIdxName) {
			try {
				//Try to discover radix by looking for prefixes
				int radix = 10;
				if (queryIdxName.startsWith("0x") || queryIdxName.startsWith("0X"))
					radix = 16;
				else if (queryIdxName.startsWith("0o") || queryIdxName.startsWith("0O"))
					radix = 8;
				else if (queryIdxName.startsWith("0b") || queryIdxName.startsWith("0B"))
					radix = 2;
				
				//Remove prefix
				if (radix != 10)
					queryIdxName = queryIdxName.substring(2);
				
				return Integer.parseInt(queryIdxName, radix);
			} catch (NumberFormatException e) {
				
			}
			
			try {
				Field f = OMXConstants.class.getDeclaredField("INDEX_" + queryIdxName);
				return f.getInt(null);
			} catch (IllegalAccessException | IllegalArgumentException | SecurityException | NoSuchFieldException e) {
				
			}
			
			throw new IllegalArgumentException("Unable to resolve query index by name: " + queryIdxName);
		}
		
		/**
		 * Read query in form of:
		 * <pre>
		 * interface QueryDefinition {
		 *     name: string;
		 *     query: number;
		 *     type: string;
		 *     fields?: FieldMappingDefinition[];
		 * }
		 * </pre>
		 * Where:
		 * <dl>
		 * <dt>name</dt>
		 *     <dd>The name of the query</dd>
		 * <dt>query</dt>
		 *     <dd>The query index</dd>
		 * <dt>type</dt>
		 *     <dd>The name of the type of the query's data structure</dd>
		 * <dt>fields</dt>
		 *     <dd>Info on how to map fields from the query's data structure to visible controls</dd>
		 * </dl>
		 * 
		 * <p>
		 * FieldMappingDefinition is further defined as:
		 * <pre>
		 * interface FieldMappingDefinition {
		 *     name: string;
		 *     kind: 'number' | 'enum';
		 *     sfName: string;
		 * }
		 * </pre>
		 * <dl>
		 * <dt>name</dt>
		 *     <dd>The exported name of the field</dd>
		 * <dt>kind<dt>
		 * <dt>sfName</dt>
		 * </dl>
		 * </p>
		 * @param querydef
		 */
		@SuppressWarnings("unchecked")
		protected void readQuery(JSONObject querydef) {
			OMXControlDefinitionBuilder queryBuilder = OMXControlDefinition.builder();
			
			String queryName = querydef.getString("name");
			queryBuilder.setName(queryName);
			
			queryBuilder.setQuery(this.lookupQueryIndex(querydef.getString("query")));
			
			String queryTypeName = querydef.getString("type");
			StructFieldType<?> queryType = lookupType(queryTypeName);
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
								@SuppressWarnings({ "rawtypes" })
								Class<? extends Enum> enumClass = (Class<? extends Enum>) Class.forName(queryFieldDef.getString("enumClass"));
								queryBuilder.withEnumField(sfName, fieldName, enumClass);
							} catch (ClassNotFoundException | JSONException e) {
								throw new RuntimeException(e);
							}
							break;
						default:
							throw new RuntimeException("Unknwon field type");
					}
					
					//Generate enumerator for field
					JSONObject enumeratorDef = queryFieldDef.optJSONObject("enumerator");
					if (enumeratorDef != null) {
						OMXEnumeratorPrototypeBuilder<?> enumeratorBuilder = queryBuilder.enumerateField();
						//TODO finish
						enumeratorBuilder.and();
					}
				}
			}
			
			//Register enumerator for entire query
			if (querydef.has("enumerator")) {
				OMXEnumeratorPrototypeBuilder<?> enumeratorBuilder = queryBuilder.withEnumerator();
				
				//Check if the enumerator has any properties
				JSONObject enumeratorDefinition = querydef.optJSONObject("enumerator");
				
				//TODO refactor to own method with field enumerator definitions
				if (enumeratorDefinition != null) {
					if (enumeratorDefinition.has("query"))
						enumeratorBuilder.setQuery(this.lookupQueryIndex(enumeratorDefinition.getString("query")));
					
					if (enumeratorDefinition.has("index"))
						enumeratorBuilder.setIndexField(enumeratorDefinition.getString("index"));
					
					if (enumeratorDefinition.has("result"))
						enumeratorBuilder.setResultField(enumeratorDefinition.getString("result"));
					
					if (enumeratorDefinition.has("type"))
						enumeratorBuilder.setStruct((StructPrototype) this.lookupType(enumeratorDefinition.getString("type")));
				}
				
				enumeratorBuilder.and();
			}
			
			this.controlRegistry.put(queryName, queryBuilder.build());
		}
		
		protected void readQueries(JSONArray querydefs, boolean continueOnError) {
			if (querydefs == null)
				return;
			final int numQueries = querydefs.length();
			for (int i = 0; i < numQueries; i++)
				readQuery(querydefs.getJSONObject(i));//TODO store
		}
		
		public void read(JSONObject base, boolean continueOnError) {
			//First read in types
			readTypes(base.optJSONArray("types"), continueOnError);
			
			readQueries(base.optJSONArray("queries"), continueOnError);
		}
		
		public void read(String json, boolean continueOnError) {
			read(new JSONObject(json), continueOnError);
		}
		
		public void read(Reader reader, boolean continueOnError) {
			read(new JSONObject(reader), continueOnError);
		}
		
		@Override
		public String toString() {
			JSONObject json = new JSONObject();
			json.put("types", new JSONObject(this.typeRegistry));
			json.put("controls", new JSONObject(this.controlRegistry));
			return json.toString(4);
			/*return new StringBuilder()
					.append(super.toString())
					.append(this.typeRegistry)
					.append(this.controlRegistry)
					.toString();*/
		}
	}
}
