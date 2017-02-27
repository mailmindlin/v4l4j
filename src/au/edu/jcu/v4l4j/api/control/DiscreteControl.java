package au.edu.jcu.v4l4j.api.control;

import java.util.Iterator;
import java.util.concurrent.Future;

public interface DiscreteControl<T> extends Control<T> {
	
	Future<Iterator<T>> options();
	
	@Override
	default boolean isDiscrete() {
		return true;
	}
	
	@Override
	default ControlType getType() {
		return ControlType.DISCRETE;
	}
}
