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
 * Objects of this class represent an image format (YUV, RGB, GREY, BGR, ...).
 * <code>ImageFormat</code> objects have two members: a name and a unique index.
 * <code>ImageFormat</code>s
 * are not directly instantiated. Instead, they can be enumerated by instantiating
 * a {@link VideoDevice}, and checking the {@link DeviceInfo} object associated
 * with it:
 * <br><code>
 * <br>VideoDevice vd = new VideoDevice("/dev/video0");
 * <br>vd.init();
 * <br>List<ImageFormat> fmts = vd.getDeviceInfo().getFormats();
 * <br>for(ImageFormat im: fmts)
 * <br>&nbsp;&nbsp; System.out.println("Format name: "+im.getName()+" - Index: "+im.getIndex());
 * <br>vd.release();
 * </code>
 * 
 * @author gilles
 *
 */
public class ImageFormat {
	
	/**
	 * The name of this format
	 */
	private String name;

	/**
	 * the libv4l Id for this format
	 */
	private int libv4lID;
	
	
	ImageFormat(String n, int i) {
		name = n;
		libv4lID = i;
	}

	/**
	 * This method returns the name of this image format.
	 * @return the name of this format
	 */
	public String getName() {
		return name;
	}

	/**
	 * This method returns the index for this format.  
	 * @return the index of this format
	 */
	public int getIndex() {
		return libv4lID;
	}
	
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + libv4lID;
		result = prime * result + ((name == null) ? 0 : name.hashCode());
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
		if (libv4lID != other.libv4lID)
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}
}
