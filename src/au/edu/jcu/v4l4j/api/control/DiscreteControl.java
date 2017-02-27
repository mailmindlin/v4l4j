package au.edu.jcu.v4l4j.api.control;

import java.util.Set;
import java.util.concurrent.Future;

public interface DiscreteControl<T> extends Control<T> {
	
	Future<Set<DiscreteOption<T>>> options();
	
	@Override
	default ControlType getType() {
		return ControlType.DISCRETE;
	}

	public static interface DiscreteOption<T> {
		T get();
		void select();
	}
}
