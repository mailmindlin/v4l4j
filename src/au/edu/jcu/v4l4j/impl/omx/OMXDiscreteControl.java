package au.edu.jcu.v4l4j.impl.omx;

import java.util.Set;

import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.DiscreteControl;

public class OMXDiscreteControl<T> implements DiscreteControl<T> {

	@Override
	public ControlType getType() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getName() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void close() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public au.edu.jcu.v4l4j.api.control.Control.ControlAccessor<Void, T, Void> access() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Set<au.edu.jcu.v4l4j.api.control.DiscreteControl.DiscreteOption<T>> options() {
		// TODO Auto-generated method stub
		return null;
	}
	
}
