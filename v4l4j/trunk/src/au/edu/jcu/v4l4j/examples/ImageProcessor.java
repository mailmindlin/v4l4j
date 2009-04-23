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
package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects implementing this interface can produce a {@link FrameGrabber} 
 * , and can handle images as they are captured.
 * @author gilles
 *
 */
public interface ImageProcessor {
	/**
	 * This method is called on this processor whenever a {@link FrameGrabber}
	 * (or one of its subclasses) must be created for a given {@link ImageFormat}
	 * @param i the Image format the frame grabber must be created for
	 * @return the frame grabber
	 * @throws V4L4JException if there is an error creating the frame grabber
	 */
	public FrameGrabber getGrabber(ImageFormat i)  throws V4L4JException;
	
	/**
	 * this method is called on this image processor whenever a new image is
	 * available for processing
	 * @param b the image data as a byte array
	 */
	public void processImage(byte[] b);
}
