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

public class V4L4JConstants {

	/**
	 * Input from a tuner
	 */
	public static final short INPUT_TYPE_TUNER = 1;
	/**
	 * Camera-type input
	 */
	public static final short INPUT_TYPE_CAMERA = 2;
	/**
	 * Video standard value for webcams
	 */
	public static final int STANDARD_WEBCAM=0;
	/**
	 * Video standard value for PAL sources
	 */
	public static final int STANDARD_PAL=1;
	/**
	 * Video standard value for SECAM sources
	 */
	public static final int STANDARD_SECAM=2;
	/**
	 * Video standard value for NTSC sources
	 */
	public static final int STANDARD_NTSC=3;
	/**
	 * Setting the capture width to this value will set the actual width to the
	 * maximum width supported by the hardware  
	 */
	public static final int MAX_WIDTH = 0;
	/**
	 * Setting the capture height to this value will set the actual height to the
	 * maximum height supported by the hardware  
	 */
	public static final int MAX_HEIGHT = 0;
	/**
	 * This value represents the maximum value of the JPEG quality setting
	 */
	public static final int MAX_JPEG_QUALITY = 100;
	/**
	 * This value represents the minimum value of the JPEG quality setting
	 */
	public static final int MIN_JPEG_QUALITY = 0;
	/**
	 * If a control has a type equal to CTRL_TYPE_BUTTON, its value is always 0,
	 * and pressing it is done by setting any value using {@link Control#setValue(int)}.
	 */
	public final static int CTRL_TYPE_BUTTON=0;
	/**
	 * If a control has a type equal to CTRL_TYPE_SLIDER, it accepts a range of value between a 
	 * minimum (as returned by {@link Control#getMinValue()}) and
	 * a maximum (as returned by {@link Control#getMaxValue()}) in increments 
	 * (as returned by {@link Control#getStepValue()})
	 */
	public final static int CTRL_TYPE_SLIDER=1;
	/**
	 * If a control has a type equal to CTRL_TYPE_SWITCH, it accepts two different values: 
	 * 0 (as returned by {@link Control#getMinValue()}) and
	 *  1 (as returned by {@link Control#getMaxValue()}.
	 */
	public final static int CTRL_TYPE_SWITCH=2;	
	/**
	 * If a control has a type equal to CTRL_TYPE_DISCRETE, it accepts discrete values as 
	 * returned by {@link Control#getDiscreteValues()}.
	 * These discrete values may have string descriptions (returned by 
	 * {@link Control#getDiscreteValueNames()} )are in the range 
	 * <code>Control.getMin()</code> and <code>Control.getMax()</code>. The step value as returned by <code>Control.getStep()</code> is not applicable.
	 */
	public final static int CTRL_TYPE_DISCRETE=3;
	
	/**
	 * Radio tuner type
	 */
	public static final int TUNER_TYPE_RADIO = 1;
	/**
	 * TV tuner type
	 */
	public static final int TUNER_TYPE_TV = 2;
	/**
	 * Frequencies in KHz
	 */
	public static final int FREQ_KHZ = 1;
	/**
	 * Frequencies in MHz
	 */
	public static final int FREQ_MHZ = 2;

}
