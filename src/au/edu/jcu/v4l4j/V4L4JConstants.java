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
	public static final int STANDARD_WEBCAM = 0;
	/**
	 * Video standard value for PAL sources
	 */
	public static final int STANDARD_PAL = 1;
	/**
	 * Video standard value for SECAM sources
	 */
	public static final int STANDARD_SECAM = 2;
	/**
	 * Video standard value for NTSC sources
	 */
	public static final int STANDARD_NTSC = 3;
	/**
	 * Setting the capture width to this value will set the actual width to the
	 * maximum width supported by the hardware
	 */
	public static final int MAX_WIDTH = 0;
	
	/**
	 * Setting the capture height to this value will set the actual height to
	 * the maximum height supported by the hardware
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
	 * and pressing it is done by setting any value using
	 * {@link Control#setValue(int)}.
	 */
	public final static int CTRL_TYPE_BUTTON = 0;
	
	/**
	 * If a control has a type equal to CTRL_TYPE_SLIDER, it accepts a range of
	 * values between a minimum (as returned by {@link Control#getMinValue()})
	 * and a maximum (as returned by {@link Control#getMaxValue()}) in
	 * increments (as returned by {@link Control#getStepValue()})
	 */
	public final static int CTRL_TYPE_SLIDER = 1;
	
	/**
	 * If a control has a type equal to CTRL_TYPE_SWITCH, it accepts two
	 * different values: 0 (as returned by {@link Control#getMinValue()}) and 1
	 * (as returned by {@link Control#getMaxValue()}.
	 */
	public final static int CTRL_TYPE_SWITCH = 2;
	
	/**
	 * If a control has a type equal to CTRL_TYPE_DISCRETE, it only accepts a
	 * set of values (discrete values). This set of acceptable values is
	 * returned by {@link Control#getDiscreteValues()}. These discrete values
	 * may have string descriptions (returned by
	 * {@link Control#getDiscreteValueNames()}) and are in the range
	 * {@link Control#getMinValue()} and {@link Control#getMaxValue()}. The step
	 * value as returned by {@link Control#getStepValue()} is not applicable.
	 */
	public final static int CTRL_TYPE_DISCRETE = 3;
	
	/**
	 * If a control has a type equal to CTRL_TYPE_STRING, it only accepts /
	 * returns strings. {@link Control#getMinValue()} and
	 * {@link Control#getMaxValue()} return the minimum and maximum string
	 * length. The actual value is set / retrieved with
	 * {@link Control#setStringValue(String)} and
	 * {@link Control#getStringValue()}.
	 */
	public final static int CTRL_TYPE_STRING = 4;
	
	/**
	 * If a control has a type equal to CTRL_TYPE_LONG, it only accepts long
	 * values, between {@link Long#MIN_VALUE} and {@link Long#MAX_VALUE} with a
	 * step value of 1. The actual value is set / retrieved with
	 * {@link Control#setLongValue(long)} and {@link Control#getLongValue()}.
	 */
	public final static int CTRL_TYPE_LONG = 5;
	
	/**
	 * If a control has a type equal to CTRL_TYPE_BITMASK, it only accepts
	 * integer values, between 0 and {@link Integer#MAX_VALUE} with a step value
	 * of 1. The actual value is set / retrieved with
	 * {@link Control#setValue(int)} and {@link Control#getValue()}.
	 */
	public final static int CTRL_TYPE_BITMASK = 6;
	
	/**
	 * A tuner with a type (as returned by {@link TunerInfo#getType()}) equal to
	 * TUNER_TYPE_RADIO is a radio tuner
	 */
	public static final int TUNER_TYPE_RADIO = 1;
	/**
	 * A tuner with a type (as returned by {@link TunerInfo#getType()}) equal to
	 * TUNER_TV_TYPE is a TV tuner
	 */
	public static final int TUNER_TYPE_TV = 2;
	/**
	 * Frequencies of a tuner (as returned by {@link TunerInfo#getUnit()}) are
	 * expressed in KHz
	 */
	public static final int FREQ_KHZ = 1;
	/**
	 * Frequencies of a tuner (as returned by {@link TunerInfo#getUnit()}) are
	 * expressed in MHz
	 */
	public static final int FREQ_MHZ = 2;
	
	/**
	 * String to display for errors that should be reported. Should only appear
	 * in error messages if we're reasonably sure that the exception would only
	 * be thrown if there's a problem with V4L4J, <i>NOT</i> the user's code.
	 */
	public static final String REPORT_ERROR_MSG = "Please report this problem to github.com/mailmindlin/v4l4j\n"
			+ "See README for information on reporting bugs.";
}
