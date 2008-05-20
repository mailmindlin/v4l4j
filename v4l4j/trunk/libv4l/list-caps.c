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


// This program list capture parameters from /dev/video0


#include <stdio.h>

#include "libv4l.h"

int main(int argc, char** argv) {
	struct capture_device *c;
	
	if(argc!=2) {
		printf("This program requires the path to the video device file to be tested.\n");
		printf("Usage: %s /dev/video0\n", argv[0]);
		return -1;
	}
	
	c = init_libv4l(argv[1], 320,240 ,0,0,2);
	
	if(c==NULL) {
		printf("Error initialising device.");
		return -1;
	}
	c->capture.list_cap(c);
	
	del_libv4l(c);

	return 0;
}
