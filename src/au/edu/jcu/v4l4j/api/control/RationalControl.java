package au.edu.jcu.v4l4j.api.control;

import java.util.Set;

import au.edu.jcu.v4l4j.api.Rational;

public interface RationalControl extends Control {
	Rational getValue();
	Rational setValue(Rational value);
	
	Rational minValue();
	Rational maxValue();
	
	public static interface DiscreteRationalControl extends RationalControl {
		Set<Rational> getValues();
	}
	
	public static interface ContinuousRationalControl extends RationalControl {
		Rational getStep();
	}
}
