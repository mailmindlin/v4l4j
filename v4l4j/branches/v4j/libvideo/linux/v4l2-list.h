/*
* Copyright (C) 2010 Gilles Gigan (gilles.gigan@gmail.com)
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

#ifndef V4L2LIST_H_
#define V4L2LIST_H_

/*
 * Query and list methods (printf to stdout, use to debug)
 * Must be called after init_capture_device2 and before free_capture_device2
 */
void list_cap_v4l2(int);	//lists all supported image formats
							//prints capabilities
							//print current settings for v4l2
							//print controls


#endif /*V4L2LIST_H_*/
