package au.edu.jcu.v4l4j.api.control;

import java.util.Set;

public interface CompositeControl {
	Set<Control> getChildren();
	Control getChildByName(String name);
}
