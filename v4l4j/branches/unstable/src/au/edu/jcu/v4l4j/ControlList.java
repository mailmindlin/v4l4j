package au.edu.jcu.v4l4j;

import java.util.Hashtable;

import au.edu.jcu.v4l4j.exceptions.StateException;

/**
 * Objects of this class encapsulate a list of video source controls. This class can not be directly
 * instantiated. Instead, to retrieve a list of controls from a <code>VideoDevice</code>, use its
 * <code>getControlList()</code> method instead. Once the control list and associated controls are no longer
 * needed, they must be released by calling <code>releaseControlList()</code>. After that, neither the list
 * nor the controls themselves must be used. If any attempt to use them is made, a <code>StateException</code> will be raised.
 * @author gilles
 *
 */
public class ControlList {
	private Hashtable<String,Control> controls;
	private boolean released;
	
	/**
	 * This constructor builds a control list from the given list. (no copy is made)
	 * @param c the control list used to initialise this object.
	 */
	ControlList(Hashtable<String,Control> c){
		controls = c;
		released = false;
	}
	
	/**
	 * This constructor builds a control list from the given list. (no copy is made)
	 * @param c the control list used to initialise this object.
	 */
	ControlList(Control[] c){
		controls = new Hashtable<String, Control>();
		for(Control ctrl: c)
			controls.put(ctrl.getName(), ctrl);

		released = false;
	}
	
	/**
	 * This method returns a copy of the control list.
	 * @return a copy of the control list.
	 * @throws StateException if this control list has been released and must no be used anymore
	 */
	public synchronized Hashtable<String,Control> getList(){
		checkReleased();
		return new Hashtable<String,Control>(controls);
	}
	
	/**
	 * This method returns a control given its name
	 * @return the control matching the given name, null otherwise
	 * @throws StateException if this control list has been released and must no be used anymore
	 */
	public synchronized Control getControl(String n){
		checkReleased();
		return controls.get(n);
	}
	
	/**
	 * This method released the control list, and all controls in it.
	 */
	synchronized void release(){
		released = true;
		for(Control c: controls.values())
			c.release();
	}

	private void checkReleased(){
		if(released)
			throw new StateException("The control list has been released and must not be used");
	}
}
