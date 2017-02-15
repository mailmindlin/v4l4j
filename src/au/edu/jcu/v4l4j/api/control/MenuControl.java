package au.edu.jcu.v4l4j.api.control;

import java.util.List;

public interface MenuControl<T> extends Control {
	List<T> getOptions();
	void setValue(T value);
}