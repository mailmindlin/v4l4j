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

import au.edu.jcu.v4l4j.api.ImagePalette;

/**
 * Objects of this class represent an image format (YUV, RGB, GREY, BGR, ...)
 * and its supported capture resolutions and frame intervals.
 * <code>ImageFormat</code> objects have three members: a name, a unique index,
 * and a {@link ResolutionInfo} object which provides information on supported
 * capture resolutions and frame intervals for this format. A list of known
 * image format indexes can be found in {@link V4L4JConstants}.IMF_*.
 * <code>ImageFormat</code>s are not directly instantiated. Instead, they can be
 * enumerated by instantiating a {@link VideoDevice}, and checking the
 * {@link DeviceInfo} object associated with it: <br>
 * <code>
 * <br>VideoDevice vd = new VideoDevice("/dev/video0");
 * <br>vd.init();
 * <br>List<ImageFormat> fmts = vd.getDeviceInfo().getFormatList().getNativeFormats();
 * <br>for(ImageFormat im: fmts)
 * <br>&nbsp;&nbsp; System.out.println("Format name: "+im);
 * <br>vd.release();
 * </code><br>
 * See the {@link ImageFormatList} documentation for more information.
 * 
 * @see ImageFormatList
 * @see ImagePalette
 * @author gilles, mailmindlin
 *
 */
public class ImageFormat {
	/**
	 * The name of this format
	 */
	private final ImagePalette palette;
	
	/**
	 * Info on supported resolutions for this image format
	 */
	private final ResolutionInfo resolutions;

	/**
	 * This method builds a new Image format with the given name and index. It
	 * also invokes a JNI method to retrieve the list of supported resolutions
	 * Therefore, it MUST be called while the device info interface of libvideo
	 * is checked out
	 * 
	 * @param name
	 *            the name of this image format
	 * @param paletteId
	 *            the index of this image format
	 * @param object
	 *            a C pointer to a struct v4l4j_device
	 */
	ImageFormat(String name, int paletteId, long object) {
		this.palette = ImagePalette.lookup(paletteId);
		this.resolutions = new ResolutionInfo(paletteId, object);
	}

	/**
	 * This method returns the {@link ResolutionInfo} object associated with
	 * this image format. {@link ResolutionInfo} objects provide information on
	 * supported capture resolutions and frame intervals.
	 * 
	 * @return the {@link ResolutionInfo} object associated with this image
	 *         format.
	 */
	public ResolutionInfo getResolutionInfo() {
		return resolutions;
	}

	/**
	 * This method returns the name of this image format.
	 * 
	 * @return the name of this format
	 */
	public String getName() {
		return this.palette.toString();
	}
	
	/**
	 * Get the palette for this format
	 * @return the palette for this ImageFormat
	 */
	public ImagePalette getPalette() {
		return this.palette;
	}

	/**
	 * This method returns the index for this format. A list of all known format
	 * indexes can be found in {@link V4L4JConstants} (V4L4JConstants.IMF_*).
	 * 
	 * @return the index of this format
	 */
	public int getIndex() {
		return this.palette.getIndex();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((palette == null) ? 0 : palette.getIndex());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (!(obj instanceof ImageFormat))
			return false;
		ImageFormat other = (ImageFormat) obj;
		if (this.palette != other.palette)
			return false;
		if (palette == null) {
			if (other.palette != null)
				return false;
		} else if (!palette.equals(other.palette))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return palette + " - " + this.palette;
	}

	public String toNiceString() {
		return palette + " - " + this.palette + " - " + resolutions;
	}
}
