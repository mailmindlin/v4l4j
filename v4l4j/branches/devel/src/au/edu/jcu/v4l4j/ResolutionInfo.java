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

import java.util.List;
import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;

/**
 * This class encapsulates information about the supported capture resolutions
 * for a video device. The first step is to determine how the supported 
 * resolutions are expressed by calling {@link #getType()}. This method returns
 * either:
 * <ul>
 * <li>{@link Type#UNSUPPORTED}: no resolution information can be obtained from
 * the driver. Calling any methods (except {@link #getType()}) will throw a
 * {@link UnsupportedMethod} exception.</li>
 * <li>{@link Type#DISCRETE}: supported resolutions are returned as a list
 * of {@link DiscreteResolution} objects, which encapsulate the width and height
 * for this resolution. Calling {@link #getDiscreteResolutions()} returns this
 * list.</li>
 * <li>{@link Type#STEPWISE}: supported width and height values can be anywhere
 * within a given minimum and maximum using a step value. The minimum, maximum &
 * step values for width and height are obtained using the similarly named 
 * methods.</li>
 * </ul>
 * {@link ResolutionInfo} objects are not instantiated directly. Instead, each 
 * {@link ImageFormat} carries a reference to a {@link ResolutionInfo} object, 
 * describing the supported resolutions for this format. A list of supported
 * image formats is obtained by calling {@link DeviceInfo#getFormatList()} on 
 * a {@link VideoDevice}. 
 * @author gilles
 *
 */
public class ResolutionInfo {
	/**
	 * The Type enumeration defines how the supported resolutions are expressed.
	 * If DISCRETE, then supported resolutions are reported as a list of 
	 * {@link DiscreteResolution} objects. If STEPWISE, then minimum, maximum 
	 * and step values for width and height are reported. If UNSUPPORTED, then
	 * resolution information is not available.
	 * @author gilles
	 *
	 */
	public enum Type{
		/**
		 * An UNSUPPORTED type means supported resolutions cannot be obtained
		 * from the driver. Calling any methods on the {@link ResolutionInfo}
		 * object (except {@link ResolutionInfo#getType()} will throw an
		 * {@link UnsupportedMethod} exception. 
		 */
		UNSUPPORTED,
		/**
		 * A DISCRETE type means resolutions are reported as a list of 
		 * {@link DiscreteResolution} objects, using 
		 * {@link ResolutionInfo#getDiscreteResolutions()}
		 */
		DISCRETE,
		/**
		 * A STEPWISE type means that resolutions are reported as minimum,
		 * maximum and step values for width and height, using
		 * {@link ResolutionInfo#getMinWidth()}, 
		 * {@link ResolutionInfo#getMaxWidth()},
		 * {@link ResolutionInfo#getWidthStep()},
		 * {@link ResolutionInfo#getMinHeight()}, 
		 * {@link ResolutionInfo#getMaxHeight()},
		 * {@link ResolutionInfo#getHeightStep()}.  
		 */
		STEPWISE
	};
	
	/**
	 * The resolution information type
	 */
	private final Type type;
	
	/**
	 * The minimum, maximum and step values for both width and height
	 * Valid only if type==STEPWISE
	 */
	private int minWidth, maxWidth, stepWidth, minHeight, maxHeight, stepHeight;
	
	/**
	 * A list of {@link DiscreteResolution} object if type==DISCRETE
	 */
	private List<DiscreteResolution> discreteValues;
	
	/**
	 * This native method returns the type of the supported resolutions.
	 * @param o a C pointer to a struct v4l4j_device
	 * @return 0: unsupported, 1: discrete, 2: continuous 
	 */
	private native int doGetType(int index, long o);
	
	/**
	 * This native method sets the stepwise attributes (min, max & step width 
	 * & height)
	 * @param index the image format index
	 * @param o a C pointer to a struct v4l4j_device 
	 */
	private native void doGetStepwise(int index, long o);
	
	/**
	 * This native method sets the discrete resolution list (discreteValues)
	 * @param index the image format index
	 * @param o a C pointer to a struct v4l4j_device 
	 */
	private native void doGetDiscrete(int index, long o);
	
	/**
	 * This method builds a new resolution information object. It MUST be called
	 * while the device info interface of libvideo is acquired.
	 * @param index the libvideo index of the image format for which this
	 * resolution info object is to be constructed 
	 * @param o a C pointer to a struct v4l4j_device
	 */
	ResolutionInfo(int index, long o){
		int t = doGetType(index, o);
		try {
			if(t==1){
				discreteValues = new Vector<DiscreteResolution>();
				doGetDiscrete(index,o);
			} else if(t==2){
				doGetStepwise(index,o);
			}
		} catch (Exception e){
			//error checking supported resolutions
			e.printStackTrace();
			System.err.println("There was an error checking the supported"
					+" resolutions.\nPlease report this error to the"
					+"v4l4j mailing list.\nSee REAME file for "
					+"information on reporting bugs");
			type = Type.UNSUPPORTED;
			return;
		}
		
		if(t==0)
			type = Type.UNSUPPORTED;
		else if(t==1){
			type = Type.DISCRETE;
		} else {
			type = Type.STEPWISE;
		}
	}
	
	/**
	 * This method returns the resolution information type. See {@link Type} 
	 * enumeration.
	 * @return the resolution information type. See {@link Type}.
	 */
	public Type getType(){
		return type;
	}
	
	/**
	 * This method returns a list of {@link DiscreteResolution}s, or throws a 
	 * {@link UnsupportedMethod} exception if this resolution info object
	 * is not of type {@link Type#DISCRETE}.
	 * @return a list of {@link DiscreteResolution}s
	 * @throws UnsupportedMethod if this resolution info object
	 * is not of type {@link Type#DISCRETE}.
	 */
	public List<DiscreteResolution> getDiscreteResolutions()
		throws UnsupportedMethod{
		if(type!=Type.DISCRETE)
			throw new UnsupportedMethod("Supported resolutions are not discrete");
		return new Vector<DiscreteResolution>(discreteValues);
	}
	
	/**
	 * This method returns the minimum width, or throws an 
	 * {@link UnsupportedMethod} exception if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 * @return the minimum width
	 * @throws UnsupportedMethod if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 */
	public int getMinWidth() throws UnsupportedMethod{
		if(type!=Type.STEPWISE)
			throw new UnsupportedMethod("Supported resolutions are not stepwise");
		return minWidth;
	}
	
	/**
	 * This method returns the maximum width, or throws an 
	 * {@link UnsupportedMethod} exception if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 * @return the maximum width
	 * @throws UnsupportedMethod if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 */
	public int getMaxWidth() throws UnsupportedMethod{
		if(type!=Type.STEPWISE)
			throw new UnsupportedMethod("Supported resolutions are not stepwise");
		return maxWidth;
	}
	
	/**
	 * This method returns the width step, or throws an 
	 * {@link UnsupportedMethod} exception if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 * @return the width step
	 * @throws UnsupportedMethod if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 */
	public int getWidthStep() throws UnsupportedMethod{
		if(type!=Type.STEPWISE)
			throw new UnsupportedMethod("Supported resolutions are not stepwise");
		return stepWidth;
	}
	
	/**
	 * This method returns the minimum height, or throws an 
	 * {@link UnsupportedMethod} exception if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 * @return the minimum height
	 * @throws UnsupportedMethod if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 */
	public int getMinHeight() throws UnsupportedMethod{
		if(type!=Type.STEPWISE)
			throw new UnsupportedMethod("Supported resolutions are not stepwise");
		return minHeight;
	}
	
	/**
	 * This method returns the maximum height, or throws an 
	 * {@link UnsupportedMethod} exception if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 * @return the maximum height
	 * @throws UnsupportedMethod if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 */
	public int getMaxHeight() throws UnsupportedMethod{
		if(type!=Type.STEPWISE)
			throw new UnsupportedMethod("Supported resolutions are not stepwise");
		return maxHeight;
	}
	
	/**
	 * This method returns the height step, or throws an 
	 * {@link UnsupportedMethod} exception if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 * @return the height step
	 * @throws UnsupportedMethod if this resolution info object is not
	 * of type {@link Type#STEPWISE}
	 */
	public int getHeightStep() throws UnsupportedMethod{
		if(type!=Type.STEPWISE)
			throw new UnsupportedMethod("Supported resolutions are not stepwise");
		return stepHeight;
	}
	
	@Override
	public String toString(){
		String s;
		if(type==Type.STEPWISE){
			s = "(min/max/step) Width: "+minWidth+"/"+maxWidth+"/"+stepWidth+
				" - Height: "+minHeight+"/"+maxHeight+"/"+stepHeight;
		} else if(type == Type.DISCRETE){
			s = "";
			for(DiscreteResolution d: discreteValues)
				s += d+" - ";
		} else {
			s = "no resolution information";
		}
		return s;
	}
	
	/**
	 * This class represents a possible resolution supported by a video device.
	 * @author gilles
	 */
	public static class DiscreteResolution{
		
		/**
		 * The resolution width 
		 */
		public final int width;
		
		/**
		 * The resolution height
		 */
		public final int height;
		
		private DiscreteResolution(int w, int h){
			width = w;
			height = h;
		}
		
		/**
		 * This method returns the resolution width
		 * @return the resolution width
		 */
		public int getWidth(){
			return width;
		}
		
		/**
		 * This method returns the resolution height
		 * @return the resolution height
		 */
		public int getHeight(){
			return height;
		}
		
		@Override
		public String toString(){
			return width+"x"+height;
		}
	}
}
