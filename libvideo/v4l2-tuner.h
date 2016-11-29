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

#ifndef V4L2TUNER_H_
#define V4L2TUNER_H_

#include "libvideo.h"

/**
 * Set the v4l2 device's tuner's frequency
 * @param device Device to set frequency
 * @param idx Index of tuner
 * @param frequency Frequency value to set
 * @return 0 on success, else LIBVIDEO_ERR_IOCTL
 */
LIBVIDEO_LOCAL int set_tuner_freq_v4l2(struct video_device *device, unsigned int idx, unsigned int frequency) __attribute__((nonnull (1)));
/**
 * Get the v4l2 device's tuner's frequency
 * @param device Device to query
 * @param idx Index of tuner
 * @param frequency Output for frequency
 * @return 0 on success, else LIBVIDEO_ERR_IOCTL
 */
LIBVIDEO_LOCAL int get_tuner_freq_v4l2(struct video_device *device, unsigned int idx, unsigned int *frequency) __attribute__((nonnull (1, 3)));
/**
 * Get the v4l2 device's tuner's RSSI and AFC info
 * @param Device device to query
 * @param idx Tuner index
 * @param rssi Output for tuner's RSSI info
 * @param afc Output for tuner's AFC info
 * @return 0 on success, else LIBVIDEO_ERR_IOCTL
 */
LIBVIDEO_LOCAL int get_rssi_afc_v4l2(struct video_device *device, unsigned int idx, int *rssi, int *afc) __attribute__((nonnull (1, 3, 4)));

#endif /* V4L2TUNER_H_ */
