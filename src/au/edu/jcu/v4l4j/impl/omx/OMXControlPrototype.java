package au.edu.jcu.v4l4j.impl.omx;

import java.util.List;
import java.util.Set;
import java.util.function.Function;
import java.util.function.IntFunction;

import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

public class OMXControlPrototype {
	protected int queryIdx;
	protected String name;
	protected StructPrototype struct;
	protected List<FieldInfo> children;
	
	@SuppressWarnings({ "rawtypes", "unchecked" })
	public BaseOMXQueryControl build(OMXComponent component, int port) {
		BaseOMXQueryControl base = new BaseOMXQueryControl(component, name, queryIdx, port, struct);
		for (FieldInfo childDef : children) {
			
			OMXOptionEnumeratorPrototype enumerator = null;
			if (childDef.enumerator != null) {
				EnumeratorDefinition enumDef = childDef.enumerator;
				IntFunction<NativeStruct> queryGenerator = i -> {
					NativeStruct n = new NativeStruct(enumDef.struct);
					if (enumDef.indexSfField != null)
						n.put(enumDef.indexSfField, i);
					return n;
				};
				Function<NativeStruct, ?> resultMapper = null;
				//TODO finish
				enumerator = new OMXOptionEnumeratorPrototype(enumDef.queryIdx, queryGenerator, resultMapper);
			}
			
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
	
	public static class OMXControlPrototypeBuilder {
		
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
		String indexSfField;
		Set<String> fields;
	}
}
