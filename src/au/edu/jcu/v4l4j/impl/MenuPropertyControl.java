package au.edu.jcu.v4l4j.impl;

import java.util.List;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.MenuControl;

public class MenuPropertyControl extends AbstractPropertyControl implements MenuControl {
	protected final List<String> labels;
	protected int value;
	
	public MenuPropertyControl(Control parent, String name, List<String> labels) {
		super(parent, name);
		this.labels = labels;
	}

	@Override
	public ControlType getType() {
		return ControlType.MENU;
	}

	@Override
	public List<String> getLabels() {
		return this.labels;
	}

	@Override
	public String setValue(String value) {
		this.value = labels.indexOf(value);
		return value;
	}
	
}
