package au.edu.jcu.v4l4j.impl;

public class IntegerPropertyControl implements IntegerControl {
	protected int value;
	protected final int min;
	protected final int max;
	protected final String name;
	public IntegerPropertyControl(int value, int min, int max, String name) {
		this.value = value;
		this.min = min;
		this.max = max;
		this.name = name;
	}
	
	@Override
	public String getName() {
		return this.name;
	}
	
	@Override
	public int getIntValue() {
		return this.value;
	}
	
	@Override
	public int setIntValue(int value) throws IllegalArgumentException {
		if (value < this.getIntMinimum() || value > this.getIntMaximum() || (value - this.getIntMinimum()) & this.getIntStep() != 0)
			throw new IllegalArgumentException("Invalid value " + value);
		this.value = value;
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
}