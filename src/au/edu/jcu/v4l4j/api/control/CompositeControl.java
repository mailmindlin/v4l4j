package au.edu.jcu.v4l4j.api.control;

import java.util.Set;

public interface CompositeControl extends Control {
	Set<Control> getChildren();
	Control getChildByName(String name);
	
	@Override
	default ControlType getType() {
		return ControlType.COMPOSITE;
	}
}
