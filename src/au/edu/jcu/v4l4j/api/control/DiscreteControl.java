package au.edu.jcu.v4l4j.api.control;

import java.util.List;

public interface DiscreteControl<T> extends Control<T> {
	List<T> getOptions();
}
