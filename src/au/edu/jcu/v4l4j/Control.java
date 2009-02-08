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

import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.ControlException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;

/**
 * Objects of this class represent video source controls of any nature. <code>Control</code>s are not directly
 * instantiated, instead a list of <code>Control</code>s associated with a <code>VideoDevice</code> can be obtained
 * by calling <code>VideoDevice.getControlList()</code>. Once the controls are no longer used, they must be released
 * by calling <code>VideoDevice.releaseControlList()</code>
 * @author gilles
 *
 */
public class Control {

	/**
	 * This JNI method returns the value of a control given its id
	 * @param o a C pointer to a struct v4l4j_device
	 * @param id the id of the control
	 * @return the value
	 * @throws ControlException if the value cant be retrieved
	 */
	private native int doGetValue(long o, int id) throws ControlException;
	
	/**
	 * This JNI method sets the value of a control given its id
	 * @param o a C pointer to a struct v4l4j_device
	 * @param id the id of the control
	 * @param v the new value
	 * @throws ControlException if the value cant be set
	 */
	private native int doSetValue(long o, int id, int v)  throws ControlException;
	
	
	private int id;
	private String name;
	private int max;
	private int min;
	private int step;
	private int type;
	private Vector<String> names;
	private int[] values;
	private long v4l4jObject;
	private int middleValue;
	
	/**
	 * Builds a V4L2 control associated with a frame grabber 
	 * @param id the index of the control
	 * @param name the name of the control
	 * @param min the minimum value it will accept
	 * @param max the maximum value it will accept
	 * @param step the increments
	 * @param type the type of this control
	 * @param names the names of the discrete values (if any), otherwise null
	 * @param values the discrete values if any. Otherwise null
	 * @param o A C pointer to a struct v4l4j_device
	 */
	Control(int id, String name, int min, int max, int step, int type, String[] names, int[] values, long o) {
		this.id = id;
		this.name = new String(name);
		this.min=min;
		this.max=max;
		this.step=step;
		this.type= type;
		if(names!=null) {
			this.names = new Vector<String>();
			for(String s: names)
					this.names.add(s);
		}
		this.values = values;
		this.middleValue = (int) Math.round((max - min) / 2.0) + min;
		v4l4jObject = o;
	}
	
	/**
	 * retrieves the current value of this control
	 * @return the value (0 if it is a button)
	 * @throws ControlException if the value cant be retrieved
	 */
	public int getValue() throws ControlException{
		if(type==V4L4JConstants.BUTTON) return 0;
		return doGetValue(v4l4jObject, id);
	}

	/**
	 * Sets a new value for this control
	 * @param value the new value
	 * @throws ControlException if the value can not be set
	 */
	public void setValue(int value) throws ControlException {
		doSetValue(v4l4jObject,id, validateValue(value));
	}
	
	/**
	 * This method increases this control's current value by its step (as returned
	 * by <code>getStep()</code>. 
	 * @throws ControlException if the value can not be increased
	 */
	public void increaseValue() throws ControlException {
		int old = 0;
		/**
		 * the following try statement is here so that write-only 
		 * controls (Relative Pan for instance) that return a ControlException
		 * when read can still have their value increase()d.
		 */
		try { old = doGetValue(v4l4jObject,id);} catch (ControlException e) {}
		doSetValue(v4l4jObject,id, validateValue(old+step));
	}
	
	/**
	 * This method decreases this control's current value by its step (as returned
	 * by <code>getStep()</code>. 
	 * @throws ControlException if the value can not be increased
	 */
	public void decreaseValue() throws ControlException {
		int old = 0;
		/**
		 * the following try statement is here so that write-only 
		 * controls (Relative Pan for instance) that return a ControlException
		 * when read can still have their value decrease()d
		 */
		try { old = doGetValue(v4l4jObject,id);} catch (ControlException e) {}
		doSetValue(v4l4jObject,id, validateValue(old-step));
	}

	/**
	 * retrieves the maximum value this control will accept 
	 * @return the maximum value
	 */
	public int getMaxValue() {
		return max;
	}

	/**
	 * retrieves the minimum value this control will accept
	 * @return the minimum value
	 */
	public int getMinValue() {
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
	public int getStepValue() {
		return step;
	}

	/**
	 * Retrieves the type of this control
	 * @return the type
	 */
	public int getType() {
		return type;
	}
	
	public int getMiddleValue(){
		return middleValue;
	}
	
	/**
	 * 
	 * @return
	 * @throws UnsupportedMethod
	 */
	public Vector<Integer> getDiscreteValues(){
		if(type!=V4L4JConstants.DISCRETE && values!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		Vector<Integer> v = new Vector<Integer>();
		for(int i: values)
			v.add(new Integer(i));
		return v;
			
	}
	
	/**
	 * 
	 * @return
	 * @throws UnsupportedMethod
	 */
	public Vector<String> getDiscreteValueNames(){
		if(type!=V4L4JConstants.DISCRETE && names!=null)
			throw new UnsupportedMethod("This control does not have discrete values");

		return new Vector<String>(names);
			
	}
	
	/**
	 * 
	 * @param v
	 * @return
	 * @throws UnsupportedMethod
	 */
	public int getDiscreteValueIndex(int v){
		if(type!=V4L4JConstants.DISCRETE && values!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		for(int i=0; i<values.length;i++)
			if(values[i]==v)
				return i;
		
		return -1;
	}
	
	/**
	 * 
	 * @param v
	 * @return
	 * @throws UnsupportedMethod
	 */
	public int getDiscreteNameIndex(String n){
		if(type!=V4L4JConstants.DISCRETE && names!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		return names.indexOf(n);
	}
	
	/**
	 * 
	 * @param v
	 * @return
	 * @throws UnsupportedMethod
	 */
	public int getDiscreteValueFromName(String n){
		if(type!=V4L4JConstants.DISCRETE && values!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		return values[getDiscreteNameIndex(n)];
	}
	/**
	 * This method validates the given value, ie it checks that it is between
	 * the allowed minimum & maximum. If it is, the given value is returned.
	 * Otherwise, it throws a ControlException  
	 * @param val the value to be checked
	 * @return the value
	 * @throws ControlException if the value is off-limit
	 */
	private int validateValue(int val) throws ControlException{
		if(val < min || val > max)
			throw new ControlException("The value '"+val+"' is outside the allowed range");
		
		return val;
	}
}
