package au.edu.jcu.v4l4j.api.control;

public interface IntegerControl extends Control {
	int getIntValue();
	int setIntValue(int value);
	int getIntMinimum();
	int getIntStep();
	int getIntMaximum();
	int getIntDefault();
}
