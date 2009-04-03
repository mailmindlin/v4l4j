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

import java.io.IOException;

import javax.swing.JOptionPane;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class RGBViewer implements ImageProcessor{
	private int width, height, std, channel;
	private VideoDevice vd;
	private VideoViewer viewer;
	
	/**
	 * Builds a WebcamViewer object
	 * @param dev the video device file to capture from
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param s the capture standard
	 * @param c the capture channel
	 * @throws V4L4JException if the device file is not readable, or does 
	 * not support an image format that can be converted to RGB24
	 */
    public RGBViewer(String dev, int w, int h, int s, int c)
    	throws V4L4JException{
    	viewer = new VideoViewer(dev, this);
    	vd = viewer.getVideoDevice();
		if(!vd.supportRGBConversion()){
			String msg = "Image from this video device cannot be converted\n"
				+ "to RGB. Please submit a bug report about this,\n"
				+"so support for your device can be added to v4l4j.\n"
				+"See README file in the v4l4j/ directory for directions.";
			JOptionPane.showMessageDialog(null, msg);
			throw new V4L4JException(msg);
		}
		
		width = w;
		height = h;
		std = s;
		channel = c;

        viewer.initGUI(
        			vd.getDeviceInfo()
        			.getFormatList().getRGBEncodableFormats().toArray(), 
        			w, 
        			h,
        			"RGB"
        		);
      
    }

	@Override
	public FrameGrabber getGrabber(ImageFormat i) throws V4L4JException {
		return vd.getRGBFrameGrabber(width, height, channel, std, i);
	}

	@Override
	public void processImage(byte[] b) {
		viewer.setImageRaster(b);
	}
  

	public static void main(String[] args) throws V4L4JException, IOException {

		String dev;
		int w, h, std, channel;

		//Check if we have the required args
		//otherwise put sensible values in
		try {
			dev = args[0];
		} catch (Exception e){
			dev = "/dev/video0";
		}
		try {
			w = Integer.parseInt(args[1]);
		} catch (Exception e){
			w = 640;
		}
		try{			
			h = Integer.parseInt(args[2]);
		} catch  (Exception e) {
			h = 480;
		}
		try {
			std = Integer.parseInt(args[3]);
		} catch (Exception e) {
			std = 0;
		}
		try {
			channel = Integer.parseInt(args[4]);
		} catch (Exception e){
			channel = 0;
		}
		
		new RGBViewer(dev,w,h,std,channel);
	}
}
