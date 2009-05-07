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

#ifndef H_VIDEODEV_ADDITIONS
#define H_VIDEODEV_ADDITIONS

//
// Marks V4L2 controls created by v4l4j as "fake" controls
// representing private IOCTLs....
// after probing the underlying driver, v4l4j creates fake v4l2_ctrl for each private ioctl detected 
// these fake v4l2_controls are marked with the following value (in the reserved[0] field) so set/get
// can be intercepted by v4l4j and dealt with properly.
//
#define V4L2_PRIV_IOCTL	0x54E83


#endif /*H_VIDEODEV_ADDITIONS*/
