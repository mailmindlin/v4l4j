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

package au.edu.jcu.v4l4j.examples.videoViewer;

import java.util.List;
import java.util.Vector;

import javax.swing.JOptionPane;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class create a graphical interface to capture frames 
 * from a video device and display it. The interface also gives access to the 
 * video controls. Frames are captured from a video device in a format that
 * v4l4j can encode in JPEG. If the given video device does not support
 * such format, 
 * 
 * @author gilles
 *
 */
public class JPEGViewer extends AbstractVideoViewer{
	private int width, height, qty, std, channel;
	
	/**
	 * Builds a WebcamViewer object
	 * @param v the video device 
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param s the capture standard
	 * @param c the capture channel
	 * @param q the JPEG compression quality
	 * @throws V4L4JException if any parameter if invalid,  or if the video device 
	 * does not support an image format that can be converted to JPEG
	 */
    public JPEGViewer(VideoDevice v, int w, int h, int s, int c, int q) 
    	throws V4L4JException{
    	super(v);

    	List<ImageFormat> fmts;

		if(!vd.supportJPEGConversion()){
			String msg = "Image from this video device cannot be converted\n"
				+ "to JPEG. If no other application is currently using\n"
				+ "the device, please submit a bug report about this issue,\n"
				+"so support for your device can be added to v4l4j.\n"
				+"See README file in the v4l4j/ directory for directions.";
			JOptionPane.showMessageDialog(null, msg);
			fmts=new Vector<ImageFormat>();
		} else
			fmts = vd.getDeviceInfo().getFormatList().getJPEGEncodableFormats();

		initGUI(fmts.toArray(),w,h,"JPEG");
		
		width = w;
		height = h;
		std = s;
		channel = c;
		qty = q;
    	      
    }
    

	@Override
	protected FrameGrabber getFrameGrabber(ImageFormat i) throws V4L4JException {
		return vd.getJPEGFrameGrabber(width, height, channel, std, qty, i);
	}
 }
