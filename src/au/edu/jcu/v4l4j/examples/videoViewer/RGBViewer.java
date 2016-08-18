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

import java.util.Collections;
import java.util.List;

import javax.swing.JOptionPane;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class RGBViewer extends AbstractVideoViewer {
	private int width, height, std, channel;

	/**
	 * Builds a WebcamViewer object
	 * 
	 * @param v
	 *            the video device
	 * @param width
	 *            the desired capture width
	 * @param height
	 *            the desired capture height
	 * @param std
	 *            the capture standard
	 * @param channel
	 *            the capture channel
	 * @throws V4L4JException
	 *             if the device file is not readable
	 */
	public RGBViewer(VideoDevice v, int width, int height, int std, int channel) throws V4L4JException {
		super(v);

		List<ImageFormat> fmts;
		if (!vd.supportRGBConversion()) {
			String msg = "Image from this video device cannot be converted\n"
					+ "to RGB. If no other application is currently using the\n"
					+ "device, please submit a bug report about this issue,\n"
					+ "so support for your device can be added to v4l4j.\n"
					+ "See README file in the v4l4j/ directory for directions.";
			JOptionPane.showMessageDialog(null, msg);
			fmts = Collections.emptyList();
		} else
			fmts = vd.getDeviceInfo().getFormatList().getRGBEncodableFormats();

		this.width = width;
		this.height = height;
		this.std = std;
		this.channel = channel;

		initGUI(fmts.toArray(), width, height, "RGB");

	}

	@Override
	protected FrameGrabber getFrameGrabber(ImageFormat i) throws V4L4JException {
		return vd.getRGBFrameGrabber(width, height, channel, std, i);
	}
}
