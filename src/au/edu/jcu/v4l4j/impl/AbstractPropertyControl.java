package au.edu.jcu.v4l4j.impl;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;

public abstract class AbstractPropertyControl implements Control {
	protected final String name;
	protected final Control parent;
	
	protected AbstractPropertyControl(Control parent, String name) {
		this.parent = parent;
		this.name = parent.getName() + "." + name;
	}
	
	@Override
	public String getName() {
		return name;
	}



	@Override
	public void push() {
		if (this.parent != null)
			this.parent.push();
	}

	@Override
	public void pull() {
		if (this.parent != null)
			this.parent.pull();
	}
	
}
