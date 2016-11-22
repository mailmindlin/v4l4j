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

#ifndef V4L1TUNER_H_
#define V4L1TUNER_H_

#include "libvideo.h"

/**
 * Set v4l1 device tuner frequency
 * @param device Device to query
 * @param idx Unused
 * @param frequency Value to set
 * @return 0 on success, else LIBVIDEO_ERR_IOCTL
 */
LIBVIDEO_LOCAL int set_tuner_freq_v4l1(struct video_device *device, int idx, unsigned int frequency) __attribute__((nonnull (1)));

/**
 * Get the v4l1 device's tuner's frequency
 * @param device Device to query
 * @param idx TODO figure out
 * @param frequency Output to which the frequency is written
 * @return 0 on success, else LIBVIDEO_ERR_IOCTL
 */
LIBVIDEO_LOCAL int get_tuner_freq_v4l1(struct video_device *device, int idx, unsigned int *frequency) __attribute__((nonnull (1, 3)));

/**
 * Get RSSI info for device
 * @param device Device to query
 * @param idx Unused
 * @param r Signal value
 * @param a TODO figure out
 * @return 0 on success, else LIBVIDEO_ERR_IOCTL
 */
LIBVIDEO_LOCAL int get_rssi_afc_v4l1(struct video_device *device, int idx, int *r, int *a) __attribute__((nonnull (1, 3, 4)));

#endif /* V4L1TUNER_H_ */
