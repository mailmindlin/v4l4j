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
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;

/**
 * Objects of this class represent a video source control of any nature.
 * <code>Control</code> objects are not directly instantiated. Instead a
 * list of <code>Control</code>s associated with a <code>VideoDevice</code>
 * can be obtained by calling {@link VideoDevice#getControlList() getControlList()}.
 * Once the controls are no longer used, they must be released by calling 
 * {@link VideoDevice#releaseControlList() releaseControlList()}. Any attempt 
 * to use a control after it has been released will result in a 
 * {@link StateException}.<br> A control can be either of the following types:<br>
 * <ul>
 * <li><code>V4L4JConstants.BUTTON</code>: Such controls take only 2 different
 * values: 0 and 1</li>
 * <li><code>V4L4JConstants.SLIDER</code>: Controls of this type take a range
 * of values between a minimum value (returned by {@link #getMinValue()}) and a
 * maximum value (returned by {@link #getMaxValue()}) with an increment value 
 * (returned by {@link #getStepValue()})</li>
 * <li><code>V4L4JConstants.SWITCH</code>: Switch controls do not take any
 * specific value, and attempting to read its current value will always return 
 * 0. Setting any value will activate the switch.</li>
 * <li><code>V4L4JConstants.DISCRETE</code>: These controls accept only specific
 * discrete values, which can be retrieved using {@link #getDiscreteValues()}.
 * Each discrete value may be accompanied by a description, which is returned by
 * {@link #getDiscreteValueNames()}.</li>
 * </ul>
 * @author gilles
 *
 */
public class Control {

	/**
	 * This JNI method returns the value of a control given its id.
	 * @param o a C pointer to a struct v4l4j_device
	 * @param id the id of the control
	 * @return the value
	 * @throws ControlException if the value cant be retrieved.
	 */
	private native int doGetValue(long o, int id) throws ControlException;
	
	/**
	 * This JNI method sets the value of a control given its id.
	 * @param o a C pointer to a struct v4l4j_device
	 * @param id the id of the control
	 * @param v the new value
	 * @throws ControlException if the value cant be set.
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
	private int defaultValue;
	private State state;
	
	/**
	 * Builds a V4L2 control.
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
		//this.middleValue = (int) Math.round((max - min) / 2.0) + min;
		this.defaultValue = 0;
		v4l4jObject = o;
		state = new State();
	}
	
	/**
	 * This method retrieves the current value of this control. Some controls
	 * (for example relative values like pan or tilt) are read-only and getting
	 * their value does not make sense. Invoking this method on this kind of
	 * controls will trigger a ControlException.   
	 * @return the current value of this control (0 if it is a button)
	 * @throws ControlException if the value cannot be retrieved
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public int getValue() throws ControlException{
		int v = 0;
		state.get();
		if(type==V4L4JConstants.BUTTON) return 0;
		try {v = doGetValue(v4l4jObject, id);}
		catch (ControlException ce){
			state.put();
			throw ce;
		}
		state.put();
		return v;
	}

	/**
	 * This method sets a new value for this control.
	 * @param value the new value
	 * @throws ControlException if the value can not be set
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public void setValue(int value) throws ControlException {
		state.get();
		try { doSetValue(v4l4jObject,id, validateValue(value));}
		catch (ControlException ce){
			state.put();
			throw ce;
		}
		state.put();
	}
	
	/**
	 * This method increases this control's current value by its step (as returned
	 * by {@link #getStepValue()})</code>.
	 * @throws ControlException if the value cannot be increased
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public void increaseValue() throws ControlException {
		int old = 0;
		/**
		 * the following try statement is here so that write-only 
		 * controls (Relative Pan for instance) that return a ControlException
		 * when read can still have their value increase()d.
		 */
		state.get();
		try { old = doGetValue(v4l4jObject,id);} catch (ControlException e) {}
		try {doSetValue(v4l4jObject,id, validateValue(old+step));}
		catch (ControlException ce){
			state.put();
			throw ce;
		}
		state.put();
	}
	
	/**
	 * This method decreases this control's current value by its step (as returned
	 * by {@link #getStepValue()}. 
	 * @throws ControlException if the value can not be increased
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public void decreaseValue() throws ControlException {
		int old = 0;
		/**
		 * the following try statement is here so that write-only 
		 * controls (Relative Pan for instance) that return a ControlException
		 * when read can still have their value decrease()d
		 */
		state.get();
		try { old = doGetValue(v4l4jObject,id);} catch (ControlException e) {}
		try {doSetValue(v4l4jObject,id, validateValue(old-step));}
		catch (ControlException ce){
			state.put();
			throw ce;
		}
		state.put();
	}

	/**
	 * This method retrieves the maximum value this control will accept.
	 * @return the maximum value
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public int getMaxValue() {
		synchronized(state){
			if(state.isNotReleased())
				return max;
			else
				throw new StateException("This control has been released and must not be used");
		}
	}

	/**
	 * This method retrieves the minimum value this control will accept.
	 * @return the minimum value
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public int getMinValue() {
		synchronized(state){
			if(state.isNotReleased())
				return min;
			else
				throw new StateException("This control has been released and must not be used");
		}
	}

	/**
	 * This method retrieves the name of this control.
	 * @return the name of this control
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public String getName() {
		synchronized(state){
			if(state.isNotReleased())
				return name;
			else
				throw new StateException("This control has been released and must not be used");
		}
	}

	/**
	 * This method retrieves the increment to be used when setting a new value
	 * for this control. New values must be equal to <code>getMin() + K*getStep()</code>
	 * where K is an integer, and the result is less or equal to {@link #getMaxValue()}.
	 * @return the increment
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public int getStepValue() {
		synchronized(state){
			if(state.isNotReleased())
				return step;
			else
				throw new StateException("This control has been released and must not be used");
		}
	}

	/**
	 * This method retrieves the type of this control. Values are
	 * <code>V4L4JConstants.BUTTON</code>, <code>V4L4JConstants.SLIDER</code>,
	 * <code>V4L4JConstants.SWITCH</code> and <code>V4L4JConstants.DISCRETE</code>.
	 * @return the type of this controls
	 * @throws StateException if this control has been released and must not be used anymore.
	 */
	public int getType() {
		synchronized(state){
			if(state.isNotReleased())
				return type;
			else
				throw new StateException("This control has been released and must not be used");
		}
	}
	
	/**
	 * This method returns the default value for this control
	 * @return the default value for this control
	 * @throws StateException if this control has been released and must not be used anymore
	 */
	public int getDefaultValue(){
		synchronized(state){
			if(state.isNotReleased())
				return defaultValue;
			else
				throw new StateException("This control has been released and must not be used");
		}
	}
	
	/**
	 * This method returns a list of the discrete values accepted by this control
	 * @return a list of the discrete values accepted by this control
	 * @throws UnsupportedMethod if this control does not support discrete values. Instead,
	 * any values between {@link #getMinValue()} and {@link #getMaxValue()} with a step of
	 * {@link #getStepValue()} may be used.
	 * @throws StateException if this control has been released and must not be used anymore
	 */
	public Vector<Integer> getDiscreteValues(){
		if(type!=V4L4JConstants.DISCRETE && values!=null)
			throw new UnsupportedMethod("This control does not accept discrete values");
		state.get();
		Vector<Integer> v = new Vector<Integer>();
		for(int i: values)
			v.add(new Integer(i));
		state.put();
		return v;
			
	}
	
	/**
	 * This method returns a description for each of the supported discrete values.
	 * @return a description for each of the supported discrete values.
	 * @throws UnsupportedMethod if this control does not support discrete values. Instead,
	 * any values between {@link #getMinValue()} and {@link #getMaxValue()} with a step of
	 * {@link #getStepValue()} may be used.
	 * @throws StateException if this control has been released and must not be used anymore
	 */
	public Vector<String> getDiscreteValueNames(){
		if(type!=V4L4JConstants.DISCRETE && names!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		state.get();
		Vector<String> v = new Vector<String>(names);
		state.put();
		return v;
			
	}
	
	/**
	 * This method returns the index of a given discrete value in the list of discrete values,
	 * as returned by {@link #getDiscreteValues()}.
	 * @param v the discrete value whose index is needed
	 * @return the index of a given discrete value, or -1 if the given value is not found
	 * @throws UnsupportedMethod if this control doesnt support discrete values
	 * @throws StateException if this control has been released and must not be used anymore
	 */
	public int getDiscreteValueIndex(int v){
		if(type!=V4L4JConstants.DISCRETE && values!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		
		int ret = -1;
		state.get();
		for(int i=0; i<values.length;i++)
			if(values[i]==v) {
				ret = i;
				break;
			}
		state.put();
		return ret;
	}
	
	/**
	 * This method returns the index of a given discrete value's description in the list of discrete values,
	 * as returned by {@link #getDiscreteValues()}.
	 * @param n the discrete value's description whose index is needed
	 * @return the index of a given discrete value, or -1 if the given value is not found
	 * @throws UnsupportedMethod if this control doesnt support discrete values
	 * @throws StateException if this control has been released and must no be used anymore
	 */
	public int getDiscreteNameIndex(String n){
		if(type!=V4L4JConstants.DISCRETE && names!=null)
			throw new UnsupportedMethod("This control does not have discrete values");
		
		state.get();
		int ret = names.indexOf(n);
		state.put();
		return ret;
	}
	
	/**
	 * This method returns the discrete value matching a given description.
	 * @param n the description whose value is to be looked up
	 * @return the value matching the description
	 * @throws UnsupportedMethod if this control doesnt support discrete values
	 * @throws IndexOutOfBoundsException if the given description doesnt match anything
	 * @throws StateException if this control has been released and must not be used anymore
	 */
	public int getDiscreteValueFromName(String n){
		if(type!=V4L4JConstants.DISCRETE && values!=null)
			throw new UnsupportedMethod("This control does not have discrete values");

		int ret;
		state.get();
		try {ret = values[getDiscreteNameIndex(n)];}
		catch (IndexOutOfBoundsException e){
			state.put();
			throw e;
		}
		state.put();
		return ret;
	}
	
	/**
	 * This method releases this control. Any attempt to use the control afterwards will
	 * raise a <code>StateException</code>
	 */
	void release(){
		state.release();
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
	
	private static class State {

		private int state;
		private int temp;
		private int users;
		
		private int INIT=0;
		private int RELEASED=1;

		public State() {
			state = INIT;
			temp = INIT;
			users = 0;
		}
		
		public synchronized void get(){
			if(state==INIT  && temp!=RELEASED) 
				users++;
			else
				throw new StateException("This Control has been released and must not be used");
		}
		
		public synchronized void put(){
			if(state==INIT) {
				System.out.println("PUT("+users+"->"+(users-1)+")");
				if(--users==0  && temp==RELEASED)
					notify();
			} else
				throw new StateException("This Control has been released and must not be used");
		}
		
		/**
		 * Must be called with the lock on this object(State) held
		 * @return
		 */
		public boolean isNotReleased(){
			return state==INIT && temp!=RELEASED;
		}
		
		
		public synchronized void release(){
			if(state==INIT && temp!=RELEASED) {
				temp=RELEASED;
				while(users!=0)
					try {
						wait();
					} catch (InterruptedException e) {
						System.err.println("Interrupted while waiting for Control users to complete");
						e.printStackTrace();
						throw new StateException("There are remaining users of this Control and it can not be stopped");
					}
			} else
				throw new StateException("This Control has been released");
		}
		
		public synchronized void commit(){
			state=temp;
		}
	}
}
