package au.edu.jcu.v4l4j.api.control;

import java.util.Set;

public interface DiscreteControl<T> extends Control<T> {
	
	Set<DiscreteOption> options();
	
	public static interface DiscreteOption<T> {
		T get();
		void select();
	}
}
