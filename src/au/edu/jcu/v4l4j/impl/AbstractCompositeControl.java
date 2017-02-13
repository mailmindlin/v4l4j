package au.edu.jcu.v4l4j.impl;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import au.edu.jcu.v4l4j.api.control.CompositeControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;

public abstract class AbstractCompositeControl implements CompositeControl {
	protected final String name;
	protected final Set<Control> children;
	
	protected AbstractCompositeControl(String name, Control...children) {
		this(name, new HashSet<>(Arrays.asList(children)));
	}
	
	protected AbstractCompositeControl(String name, Set<Control> children) {
		this.name = name;
		this.children = children;
	}
	
	@Override
	public String getName() {
		return this.name;
	}
	
	@Override
	public Set<Control> getChildren() {
		return this.children;
	}
	
	@Override
	public Control getChildByName(String name) {
		return children.stream()
				.filter(child->child.getName().equals(name))
				.findAny()
				.orElse(null);
	}
	

	@Override
	public ControlType getType() {
		return ControlType.COMPOSITE;
	}
	
}
