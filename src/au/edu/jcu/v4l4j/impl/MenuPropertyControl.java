package au.edu.jcu.v4l4j.impl;

import java.util.List;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.MenuControl;

public class MenuPropertyControl extends AbstractPropertyControl implements MenuControl<String> {
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
	public List<String> getOptions() {
		return this.labels;
	}

	@Override
	public void setValue(String value) {
		this.value = labels.indexOf(value);
	}

	@Override
	public void increase() {
		if (value < labels.size() - 1)
			value++;
	}

	@Override
	public void decrease() {
		if (value > 0)
			value--;
	}

	@Override
	public void close() {
		
	}
}
