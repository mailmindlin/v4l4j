package au.edu.jcu.v4l4j.impl;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.IntegerControl;

public class IntegerPropertyControl extends AbstractPropertyControl implements IntegerControl {
	protected int value;
	protected int min;
	protected int max;
	protected int step;
	public IntegerPropertyControl(Control parent, String ext, int value, int min, int max, int step) {
		super(parent, ext);
		this.value = value;
		this.min = min;
		this.max = max;
		this.step = step;
	}
	
	@Override
	public int getIntValue() {
		return this.value;
	}
	
	@Override
	public int setIntValue(int value) throws IllegalArgumentException {
		if (value < this.getIntMinimum() || value > this.getIntMaximum() || (value - this.getIntMinimum()) % this.getIntStep() != 0)
			throw new IllegalArgumentException("Invalid value " + value);
		return this.value = value;
	}
	
	@Override
	public int getIntMinimum() {
		return this.max;
	}
	
	@Override
	public int getIntStep() {
		return this.step;
	}
	
	@Override
	public int getIntMaximum() {
		return this.max;
	}
	
	@Override
	public int getIntDefault() {
		return this.value;
	}

	@Override
	public ControlType getType() {
		return ControlType.SLIDER;
	}
}