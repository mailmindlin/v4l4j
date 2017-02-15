package au.edu.jcu.v4l4j.impl.omx;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import au.edu.jcu.v4l4j.api.control.CompositeControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

/**
 * A root composite control that when push or pull is called, executes a 
 * @author mailmindlin
 */
public class OMXQueryControl implements CompositeControl {
	protected final OMXComponent component;
	protected final Set<Control> children = new HashSet<>();
	protected transient Map<String, Control> childMap;
	protected final String rootName;
	protected final int queryId;
	protected final StructPrototype struct;
	
	protected OMXQueryControl(OMXComponent component, String rootName, int queryId, StructPrototype struct) {
		this.component = component;
		this.rootName = rootName;
		this.queryId = queryId;
		this.struct = struct;
	}
	
	@Override
	public String getName() {
		return this.rootName;
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
	public void increase() {
		
	}

	@Override
	public void decrease() {
		
	}

	@Override
	public Set<Control> getChildren() {
		return this.children;
	}

	@Override
	public void close() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public ControlType getType() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Control getChildByName(String name) {
		if (childMap == null) {
			for (Control child : this.children)
				addToMap("", child);
		}
		
		return childMap.get(name);
	}
	
	protected void addToMap(String prefix, Control child) {
		
	}

}
