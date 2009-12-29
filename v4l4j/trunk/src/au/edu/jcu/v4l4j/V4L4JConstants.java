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
	public final static int CTRL_TYPE_BUTTON=0;
	/**
	 * If a control has a type equal to CTRL_TYPE_SLIDER, it accepts a range of 
	 * values between a minimum (as returned by {@link Control#getMinValue()})
	 * and a maximum (as returned by {@link Control#getMaxValue()}) in 
	 * increments (as returned by {@link Control#getStepValue()})
	 */
	public final static int CTRL_TYPE_SLIDER=1;
	/**
	 * If a control has a type equal to CTRL_TYPE_SWITCH, it accepts two 
	 * different values: 
	 * 0 (as returned by {@link Control#getMinValue()}) and
	 * 1 (as returned by {@link Control#getMaxValue()}.
	 */
	public final static int CTRL_TYPE_SWITCH=2;	
	/**
	 * If a control has a type equal to CTRL_TYPE_DISCRETE, it only accepts 
	 * a set of values (discrete values). This set of acceptable values is  
	 * returned by {@link Control#getDiscreteValues()}.
	 * These discrete values may have string descriptions (returned by 
	 * {@link Control#getDiscreteValueNames()}) and are in the range 
	 * {@link Control#getMinValue()} and {@link Control#getMaxValue()}. The 
	 * step value as returned by {@link Control#getStepValue()} is not 
	 * applicable.
	 */
	public final static int CTRL_TYPE_DISCRETE=3;
	
	/**
	 * A tuner with a type (as returned by {@link TunerInfo#getType()}) equal to
	 * TUNER_TYPE_RADIO is a radio tuner
	 */
	public static final int TUNER_TYPE_RADIO = 1;
	/**
	 * A tuner with a type (as returned by {@link TunerInfo#getType()}) equal to
	 * TUNER_TV_TYPE is a  TV tuner
	 */
	public static final int TUNER_TYPE_TV = 2;
	/**
	 * Frequencies of a tuner (as returned by 
	 * {@link TunerInfo#getUnit()}) are expressed in KHz
	 */
	public static final int FREQ_KHZ = 1;
	/**
	 * Frequencies of a tuner (as returned by 
	 * {@link TunerInfo#getUnit()}) are expressed in MHz
	 */
	public static final int FREQ_MHZ = 2;
	
	// RGB image formats
	/**
	 * RGB332 image format
	 */
	public static final int IMF_RGB332=0;
	/**
	 * RGB444 image format
	 */
	public static final int IMF_RGB444=1;
	/**
	 * RGB555 image format
	 */
	public static final int IMF_RGB555=2;
	/**
	 * RGB565 image format
	 */
	public static final int IMF_RGB565=3;
	/**
	 * RGB555X image format
	 */
	public static final int IMF_RGB555X=4;
	/**
	 * RGB565X image format
	 */
	public static final int IMF_RGB565X=5;
	/**
	 * BGR24 image format
	 */
	public static final int IMF_BGR24=6;
	/**
	 * RGB24 image format
	 */
	public static final int IMF_RGB24=7;
	/**
	 * BGR32 image format
	 */
	public static final int IMF_BGR32=8;
	/**
	 * RGB32 image format
	 */
	public static final int IMF_RGB32=9;
	
	// Grey image formats
	/**
	 * GREY image format
	 */
	public static final int IMF_GREY=10;
	/**
	 * Y10 image format
	 */
	public static final int IMF_Y10=11;	
	/**
	 * Y16 image format
	 */
	public static final int IMF_Y16=12;
	
	// PAL 8 format
	/**
	 * PAL8 image format
	 */
	public static final int IMF_PAL8=13;
	
	// YUV formats
	/**
	 * YVU410 image format
	 */
	public static final int IMF_YVU410=14;
	/**
	 * YVU420 image format
	 */
	public static final int IMF_YVU420=15;	
	/**
	 * YUYV image format
	 */
	public static final int IMF_YUYV=16;
	/**
	 * YYUV image format
	 */
	public static final int IMF_YYUV=17;
	/**
	 * YVYU image format
	 */
	public static final int IMF_YVYU=18;
	/**
	 * UYVY image format
	 */
	public static final int IMF_UYVY=19;
	/**
	 * VYUY image format
	 */
	public static final int IMF_VYUY=20;
	/**
	 * YUV422 planar image format
	 */
	public static final int IMF_YUV422P=21;
	/**
	 * YUV411 planar image format
	 */
	public static final int IMF_YUV411P=22;
	/**
	 * Y41P image format
	 */
	public static final int IMF_Y41P=23;
	/**
	 * YUV444 image format
	 */
	public static final int IMF_YUV444=24;
	/**
	 * YUV555 image format
	 */
	public static final int IMF_YUV555=25;
	/**
	 * YUV565 image format
	 */
	public static final int IMF_YUV565=26;
	/**
	 * YUV32 image format
	 */
	public static final int IMF_YUV32=27;
	/**
	 * YUV410 planar image format
	 */
	public static final int IMF_YUV410=28;
	/**
	 * YUV420 planar image format index
	 */
	public static final int IMF_YUV420=29;
	/**
	 * HI240 image format
	 */
	public static final int IMF_HI240=30;
	/**
	 * HM12 image format
	 */
	public static final int IMF_HM12=31;
	
	// two planes - Y and Cb/Cr interleaved
	/**
	 * NV12 image format
	 */
	public static final int IMF_NV12=32;
	/**
	 * NV21 image format
	 */
	public static final int IMF_NV21=33;
	/**
	 * NV16 image format
	 */
	public static final int IMF_NV16=34;
	/**
	 * NV61 image format
	 */
	public static final int IMF_NV61=35;
	
	// Bayer formats
	/**
	 * SBGGR8 bayer image format
	 */
	public static final int IMF_SBGGR8=36;
	/**
	 * SGBRG8 image format
	 */
	public static final int IMF_SGBRG8=37;
	/**
	 * SGRBG8 bayer image format
	 */
	public static final int IMF_SGRBG8=38;
	/**
	 * SRGGB8 bayer image format
	 */
	public static final int IMF_SRGGB8=39;
	/**
	 * SBGGR10 bayer image format
	 */
	public static final int IMF_SBGGR10=40;
	/**
	 * SGBRG10 bayer image format
	 */
	public static final int IMF_SGBRG10=41;
	/**
	 * SGRBG10 bayer image format
	 */
	public static final int IMF_SGRBG10=42;
	/**
	 * SRGGB10 bayer image format
	 */
	public static final int IMF_SRGGB10=43;
	/**
	 * SGRBG10_DPCM8 bayer image format
	 */
	public static final int IMF_SGRBG10DPCM8=44;
	/**
	 * SBGGR16 bayer image format
	 */
	public static final int IMF_SBGGR16=45;

	// compressed formats
	/**
	 * MJPEG image format
	 */
	public static final int IMF_MJPEG=46;
	/**
	 * JPEG image format
	 */
	public static final int IMF_JPEG=47;
	/**
	 * DV image format
	 */
	public static final int IMF_DV=48;
	/**
	 * MPEG image format
	 */
	public static final int IMF_MPEG=49;
	
	// vendor specific
	/**
	 * WNVA image format
	 */
	public static final int IMF_WNVA=50;	
	/**
	 * SN9C10X image format
	 */
	public static final int IMF_SN9C10X=51;
	/**
	 * SN9C20X_I420 image format
	 */
	public static final int IMF_SN9C20X_I420=52;
	/**
	 * PWC1 image format
	 */
	public static final int IMF_PWC1=53;
	/**
	 * PWC2 image format
	 */
	public static final int IMF_PWC2=54;
	/**
	 * ET61X251 image format
	 */
	public static final int IMF_ET61X251=55;
	/**
	 * SPCA501 image format
	 */
	public static final int IMF_SPCA501=56;
	/**
	 * SPCA505 image format
	 */
	public static final int IMF_SPCA505=57;
	/**
	 * SPCA508 image format
	 */
	public static final int IMF_SPCA508=58;
	/**
	 * SPCA561 image format
	 */
	public static final int IMF_SPCA561=59;
	/**
	 * PAC207 image format
	 */
	public static final int IMF_PAC207=60;
	/**
	 * MR97310A image format
	 */
	public static final int IMF_MR97310A=61;
	/**
	 * SQ905C image format
	 */
	public static final int IMF_SQ905C=62;
	/**
	 * PJPG image format
	 */
	public static final int IMF_PJPG=63;
	/**
	 * OV511 image format
	 */
	public static final int IMF_OV511=64;
	/**
	 * OV518 image format
	 */
	public static final int IMF_OV518=65;
	/**
	 * STV0680 image format
	 */
	public static final int IMF_STV0680=66;
	/**
	 * TM6000 image format
	 */
	public static final int IMF_TM6000=67;
}
