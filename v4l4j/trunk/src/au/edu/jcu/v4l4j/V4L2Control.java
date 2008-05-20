/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a   
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

package au.edu.jcu.v4l4j;

/**
 * Objects of this class represent video source controls of any nature. When a video source is attached to a frame grabber,
 * its controls are automatically detected and made available through the <code>getControls()</code> method of the frame grabber. 
 * @author gilles
 *
 */
public class V4L2Control {
	
	/**
	 * If this control has a type equal to BUTTON, it has a value of 0,
	 * and pressing it is done by setting any value using <code>setValue()</code> 
	 */
	public final static int BUTTON=0;
	/**
	 * If this control has a type equal to SLIDER, it accepts a range of value between a minimum (as returned by <code>getMin()</code>) and
	 * a maximum (as returned by <code>getMax()</code>) in increments (as returned by <code>getStep()</code>)
	 */
	public final static int SLIDER=1;
	
	private int id;
	private String name;
	private int max;
	private int min;
	private int step;
	private int type;
	private FrameGrabber fg;
	
	/**
	 * Builds a V4L2 control associated with a frame grabber 
	 * @param id the index of the control
	 * @param name the name of the control
	 * @param min the minimum value it will accept
	 * @param max the maximum value it will accept
	 * @param step the increments
	 * @param fg the frame grabber associated with this control
	 */
	V4L2Control(int id, String name, int min, int max, int step, FrameGrabber fg) {
		this.id = id;
		this.name = new String(name);
		this.min=min;
		this.max=max;
		this.step=step;
		this.type= min==max ? BUTTON : SLIDER;
		this.fg = fg;
	}

	/**
	 * retrieves the current value of this control
	 * @return the value (0 if it is a button)
	 * @throws V4L4JException if something goes wrong
	 */
	public int getValue() throws V4L4JException {
		if(type==BUTTON) return 0;
		return fg.getControlValue(id);
	}

	/**
	 * Sets a new value for this control
	 * @param value the new value
	 * @throws V4L4JException if the value is incorrect
	 */
	public void setValue(int value) throws V4L4JException {
		fg.setControlValue(id, value);
	}

	/**
	 * retrieves the maximum value this control will accept 
	 * @return the maximum value
	 */
	public int getMax() {
		return max;
	}

	/**
	 * retrieves the minimum value this control will accept
	 * @return the minimum value
	 */
	public int getMin() {
		return min;
	}

	/**
	 * Retrieves the name of this control
	 * @return the name of this control
	 */
	public String getName() {
		return name;
	}

	/**
	 * Retrieves the increment to be used when setting a new value for this control.
	 * New values must be equal to <code>getMin() + K*getStep()</code> where K is an integer, and the result is less
	 * or equal to <code>getMax()</code>
	 * @return the increment
	 */
	public int getStep() {
		return step;
	}

	/**
	 * Retrieves the type of this control
	 * @return the type
	 */
	public int getType() {
		return type;
	}
}
