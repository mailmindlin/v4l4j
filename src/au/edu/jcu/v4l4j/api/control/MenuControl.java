package au.edu.jcu.v4l4j.api.control;

import java.util.List;

public interface MenuControl extends Control {
	List<String> getLabels();
	String setValue(String value);
}
