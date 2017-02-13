package au.edu.jcu.v4l4j.impl.omx;

import java.util.HashSet;

import au.edu.jcu.v4l4j.impl.AbstractCompositeControl;
import au.edu.jcu.v4l4j.impl.IntegerPropertyControl;
import au.edu.jcu.v4l4j.impl.MenuPropertyControl;

public class OMXBitrateControl extends AbstractCompositeControl {
	IntegerPropertyControl targetBitrateControl;
	MenuPropertyControl controlRate;
	protected OMXBitrateControl(String name) {
		super(name, new HashSet<>());
		this.targetBitrateControl = new IntegerPropertyControl(this, "targetBitrate", 0, 0, Integer.MAX_VALUE, 1);
		this.children.add(this.targetBitrateControl);
		
		this.controlRate = new MenuPropertyControl(this, "controlRate", OMXConstants.BITRATE_CONTROLS);
		this.children.add(this.controlRate);
	}
	
	@Override
	public void push() {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void pull() {
		// TODO Auto-generated method stub
		
	}
	
}