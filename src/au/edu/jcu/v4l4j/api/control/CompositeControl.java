package au.edu.jcu.v4l4j.api.control;

import java.util.Map;
import java.util.Set;

public interface CompositeControl extends Control<Map<String, Object>> {
	Set<Control<?>> getChildren();
	
	Control<?> getChildByName(String name);
	
	@Override
	ControlAccessor<Void, Map<String, Object>, Map<String, Object>> get();
	
	@Override
	ControlAccessor<Void, Map<String, Object>, Void> access();
	
	@Override
	default ControlType getType() {
		return ControlType.COMPOSITE;
	}
}
