package au.edu.jcu.v4l4j.api.control;

import java.util.concurrent.Future;

import au.edu.jcu.v4l4j.api.CloseableIterator;

public interface DiscreteControl<T> extends Control<T> {
	
	Future<CloseableIterator<T>> options();
	
	@Override
	default boolean isDiscrete() {
		return true;
	}
	
	@Override
	default ControlType getType() {
		return ControlType.DISCRETE;
	}
}
