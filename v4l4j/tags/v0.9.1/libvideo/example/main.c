	/*
	copyright 2006 Gilles GIGAN (gilles.gigan@gmail.com)

	This file is part of light_cap.

   	light_cap is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    light_cap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with light_cap; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <string.h>			//strstr, memcpy
#include <fcntl.h>			//for file open flags O_*
#include <stdlib.h>
#include <unistd.h>			//for close, write, open
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>			//for atoi and friends
#include <time.h>			//for nanosleep
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> 		//gettimeofday
#include <netinet/in.h>
#include <arpa/inet.h>		//for inet_ntoa (convert strcut in_addr to char*)

#include "libvideo.h"
#include "libvideo-palettes.h"
#include "utils.h"
#include "jpeg.h"
#include "log.h"

#define SUPPORTED_FORMATS		{JPEG, MJPEG, YUV420, RGB24, YUYV}
#define NB_SUPPORTED_FORMATS	5


#ifdef DEBUG
int log_source=DEFAULT_LOG_SOURCE, log_level = DEFAULT_LOG_LEVEL;
#endif

/* thread-safe ints:
 * client_nr: number of clients connected to the webcam stream
 * jpeg_quality: the requested jpeg quality (0-100%)
 * requested_fps: the requested frame rate (1-25)
 * max_fps: the maximum fps achieved during the calibration period, defined as
 * 			the first SHOW_FPS_INTERVAL seconds after the first client connects
 * 			where frames are sent as fast as possible. max_fps is measured at this time
 * keep_going: flag used to know when to terminate the capture
 * */
volatile sig_atomic_t client_nr = 0, jpeg_quality, requested_fps, max_fps = 0, keep_going = 1;

//What to print on terminal
int verbosity = LOG_ALL;

/* fps adjustment variables:
 * fps_nanosleep and fps_secsleep: specify the amount of time(in nanosec and sec resp.) to sleep before
 * 			capturing a frame. This allows control over the frame rate, between
 * 			max_fps (fps_secsleep = fps_nanosecsleep = 0) and 1 (frame/sec)
 * fps_nanosleepstep: specifies an amount of nanosecs used to adjust fps_nanosleep in order to achieve
 * 			requested_fps. fps_nanosleepstep is calculated during the calibration period and is set
 * 			to the time (in ns) required to capture, jpeg-encode and send 1 frame.
 * */
long fps_nanosleep = 0, fps_secsleep = 0, fps_nanosleep_step = 0;;


/* set keep_going to 0 when SIGINT is received
 */
void catch_int (int sig) {
	info(LOG_INFO, "Exiting ...\n");
	keep_going = 0;
	signal (sig, catch_int);
}

int main(int argc, char **argv) {
	/*Assumes args are in order !!!!
	1: V4L2 dev name, 2: height, 3: width, 4: channel
	5: standard, 6: jpeg_quality, 7: verbosity, 8: log source, 9: log_level
	*/
	struct video_device *d;
	struct capture_device *cdev;
	int sockfd, port;
	jpeg_quality = JPEG_QUALITY;
	int fmts[NB_SUPPORTED_FORMATS] = SUPPORTED_FORMATS;

	//catch ctrl-c
	signal (SIGINT, catch_int);
	signal (SIGPIPE, SIG_IGN );

	//Parse cmd line options
	if(argc != NB_ARGS) {
		print_usage();
		exit(1);
	}

	jpeg_quality = atoi(argv[6]);
	requested_fps = atoi(argv[7]);
	verbosity = atoi(argv[8]);
	port = atoi(argv[9]);

#ifdef DEBUG
	log_source = atoi(argv[10]);
	log_level = atoi(argv[11]);
#endif

	d = open_device(argv[1]);
	if(d==NULL){
		info(LOG_ERR, "Cant open device %s",argv[1]);
		exit(1);
	}

	//create capture device
	cdev = init_capture_device(d, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), 5);
	if(cdev==NULL) {
		info(LOG_ERR, "Failed to initialise video device\n");
		info(LOG_ERR, "Recompile libvideo with debugging enabled (see README)\n");
		info(LOG_ERR, "to see why/where the initialisation fails.\n");
		exit(1);
	}

	//Set capture param (image format, color, size, crop...)
	if ((*cdev->actions->set_cap_param)(d, -1, MJPEG)!=0) {
		info(LOG_ERR, "Unable to set capture parameters. It could be due to:\n");
		info(LOG_ERR, " - the chosen width and height,\n - the driver not supporting the image formats libvideo tried\n");
		info(LOG_ERR, "Recompile libvideo with debugging enabled (see README)\n");
		info(LOG_ERR, "to see why/where the setup fails.\n");
		free_capture_device(d);
		close_device(d);
		exit(1);
	}

	info(LOG_INFO, "Capturing at %dx%d\n", cdev->width, cdev->height);

	//Prepare capture:Allocates v4l2 buffers, mmap buffers
	if ((*cdev->actions->init_capture)(d)) {
		info(LOG_ERR, "Failed to setup capture\n");
		free_capture_device(d);
		close_device(d);
		exit(1);
	}

	info(LOG_INFO, "Using palette %s\n", libvideo_palettes[cdev->palette].name);

	//init tcp server
	sockfd = setup_tcp_server_sock(port);

	//main loop
	main_loop(sockfd, d);

	//close tcp socket
	close(sockfd);

	//Deallocates V4l2 buffers
	(*cdev->actions->free_capture)(d);

	//delete cdev
	free_capture_device(d);

	//close device
	close_device(d);

	return 0;
}
/* Sets up a listening (server) socket on port "port"
 * returns the socket descriptor
 */
int setup_tcp_server_sock(int port) {
	int sockfd, oldflags;
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
    	info(LOG_ERR, "Cant open a socket\n");
		keep_going = 0;
	}

    oldflags = 1;
    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &oldflags, sizeof( oldflags ) );

    //oldflags = fcntl (sockfd, F_GETFL, 0);
	//fcntl(sockfd, F_SETFL, oldflags | O_NONBLOCK);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		info(LOG_ERR,  "Cant bind to if/port\n");
		keep_going = 0;
	}
	listen(sockfd,5);
	info(LOG_INFO, "Listening on TCP port %d\n", port);
	return sockfd;
}

/* main loop: waits for incoming TCP connections on "serv_sockfd", and service them
 * cdev is the initialised V4L2 device
 */
void main_loop(int serv_sockfd, struct video_device *d) {
	int clilen, newsockfd;
	struct sockaddr_in cli_addr;
	fd_set rfds;

	clilen = sizeof(cli_addr);
	info(LOG_INFO, "Waiting for incoming connections...\n");
	while(keep_going) {
		FD_ZERO(&rfds);
		FD_SET(serv_sockfd, &rfds);

		//select is used so we can sleep and be interrupted while we wait (accept) for an incoming cx
		//using non blocking socket was not good because the while loop would use 100% CPU while accept'ing
		if (select(serv_sockfd+1, &rfds, NULL, NULL, NULL)>0) {
			newsockfd = accept(serv_sockfd, (struct sockaddr *) &cli_addr, (unsigned int *) &clilen);
			//info(LOG_INFO, "New connection from %s:%d on socket %d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, newsockfd );
			if(newsockfd > 0) {
				switch(get_action(newsockfd, d)){
					case ACTION_CAPTURE:
						if(client_nr < MAX_CLIENTS) {
			    	 		//send stream
			     			start_thread_client(newsockfd, d);
						}
			 			else {
							info(LOG_ERR, "Cant accept anymore clients.try increasing MAX_CLIENT in utils.h and recompile\n");
							close(newsockfd);
						}
						break;
					case ACTION_LIST:
						//output current capture params
						list_cap_param(newsockfd, d);
						info(LOG_INFO, "closing connection on socket %d\n", newsockfd );
						close(newsockfd);
						break;
					default:
						break;
				} //end switch
			} //end if new connection
		} //end if(select>0)
	} //while(keep_going)
}

/* send an HTMl page over the "newsockfd" socket with a list of V4L2 controls (brightness, contrast, ...)
 * as well as JPEG and fps controls
 */
void list_cap_param(int newsockfd,struct video_device *d) {
	char *page, *ptr;
	struct control_list *l = get_control_list(d);
	int i, v;

	XMALLOC(page, char *, PARAM_PAGE_SIZE);
	ptr = page;
	ptr += sprintf(ptr,"<html><body>\n");

	//outputs JPEG quality control setting
	ptr += sprintf(ptr,"<form method=\"get\"><h4>JPEG quality</h4>Value: %d (min: 0, max: 100, step: 1)<br>\n", jpeg_quality);
	ptr += sprintf(ptr,"<input type=\"text\" name=\"val\" value=\"%d\" size=\"5\"> &nbsp; <input type=\"submit\" name=\"-1\" value=\"update\"></form>\n", jpeg_quality);
	write(newsockfd, page, strlen(page));
	ptr = page;
	memset(page, 0, PARAM_PAGE_SIZE);

	//outputs frame rate control setting
	ptr += sprintf(ptr,"<form method=\"get\"><h4>Frame rate</h4>Value: %d (min: 1, max: 25, step: 1)<br>\n", requested_fps);
	ptr += sprintf(ptr,"<input type=\"text\" name=\"val\" value=\"%d\" size=\"3\"> &nbsp; <input type=\"submit\" name=\"-2\" value=\"update\"></form>\n", requested_fps);
	write(newsockfd, page, strlen(page));
	ptr = page;
	memset(page, 0, PARAM_PAGE_SIZE);

	//outputs controls
	l = d->control;
	for(i = 0; i< l->count; i++) {
		if(get_control_value(d, l->controls[i].v4l2_ctrl, &v, sizeof(v)) != 0) {
			info(LOG_ERR, "Error getting value for control %s\n", l->controls[i].v4l2_ctrl->name);
			v = 0;
		}
		ptr += sprintf(ptr,"<form method=\"get\"><h4>%s</h4>Value: %d (min: %d, max: %d", l->controls[i].v4l2_ctrl->name, v, l->controls[i].v4l2_ctrl->minimum, l->controls[i].v4l2_ctrl->maximum);
		if(l->controls[i].count_menu!=0) {
			int j;
			ptr += sprintf(ptr,"\n)<select name=\"val\">\n");
			for(j=0; j<l->controls[i].count_menu; j++){
				ptr += sprintf(ptr, "<option value=\"%d\"%s>%d</option>\n",l->controls[i].v4l2_menu[j].index, v == l->controls[i].v4l2_menu[j].index ? " selected" : "", l->controls[i].v4l2_menu[j].index);
			}
			ptr += sprintf(ptr, "</select>\n");
		} else {
			ptr += sprintf(ptr,", step: %d)<br>\n", l->controls[i].v4l2_ctrl->step);
			ptr += sprintf(ptr,"<input type=\"text\" name=\"val\" value=\"%d\" size=\"5\"> ", v);
		}
		ptr += sprintf(ptr, "&nbsp; <input type=\"submit\" name=\"%d\" value=\"update\"></form>\n", i);
		write(newsockfd, page, strlen(page));
		ptr = page;
		memset(page, 0, PARAM_PAGE_SIZE);
	}

	//output end of HTML body
	ptr += sprintf(ptr,"</body></html>\n");
	write(newsockfd, page, strlen(page));
	XFREE(page);
	release_control_list(d);
}
/* Reads the first few bytes of "sock" socket and decides what to do (send webcam stream or list of controls)
 * In the latter case (list of controls), more bytes are parsed to see whether we should also set a new value to one
 * of the controls
 */
int get_action(int sock, struct video_device *d) {
	int c, ctrl_index = 0, value = 0, ret = ACTION_CAPTURE;
	char *buf, *sptr, *fptr;
	struct control_list *l = get_control_list(d);

	XMALLOC(buf, char *, INPUT_BLOCK_SIZE);
	c = read(sock, buf, INPUT_BLOCK_SIZE - 1);
	buf[c] = '\0';

	if(strstr(buf, "webcam") != NULL) {
		ret = ACTION_CAPTURE;
		info(LOG_INFO, "going for capture\n");
	}
	else if (strstr(buf, "list") != NULL){
		ret = ACTION_LIST;
		if((sptr = strstr(buf, "?")) != NULL) {
			fptr = strstr(sptr, " HTTP");
			*fptr = '\0';
			if(sscanf(++sptr, "val=%6d&%3d=update", &value, &ctrl_index) == 2) {
				//catch the jpeg control setting
				if(ctrl_index==-1) {
					info(LOG_INFO, "Setting JPEG quality to %d\n", value);
					if((1 <= value) && (value <= 100)) jpeg_quality=value;
					else info(LOG_ERR, "Invalid jpeg quality value %d\n", value);
				} else if(ctrl_index==-2) {
				//catch the frame ratio control
					info(LOG_INFO, "Setting frame ratio to %d\n", value);
					if((1 <= value) && (value <= 25)) {
						requested_fps = value; set_fps(requested_fps);
					} else info(LOG_ERR, "Invalid frame rate %d\n", value);
				} else {
					assert(ctrl_index < l->count);
					info(LOG_INFO, "Setting %s to %d\n", l->controls[ctrl_index].v4l2_ctrl->name, value);
					set_control_value(d, l->controls[ctrl_index].v4l2_ctrl, &value, sizeof(value));
					info(LOG_INFO, "New value: %d\n", value);
				}
			} else
				info(LOG_ERR, "Error parsing URL. Unable to set new value\n");
		}
	}
	XFREE(buf);
	release_control_list(d);
	return ret;
}

/* Starts a thread to handle a webcam connection on "sock" socket
 */
void start_thread_client(int sock, struct video_device *d) {
	pthread_attr_t attr;
	pthread_t tid;
	struct thread_data *td;

	XMALLOC(td, struct thread_data *, sizeof(struct thread_data));
	if (td != NULL) {
		td->sock = sock;
		td->vdev = d;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		//if 1st client
		if(client_nr==0) {
			//Start capture
			dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "First client, starting capture \n");
			if ((*d->capture->actions->start_capture)(d) !=0 ) {
				info(LOG_ERR, "Cant initiate capture...\n");
				keep_going = 0;
				XFREE(td);
			}
		}

		//create new thread
		if (pthread_create(&tid, &attr, send_stream_to, (void *)td) == 0) {
			client_nr++;
			dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Created new thread \n");
		}
		else {
			(*d->capture->actions->stop_capture)(d);
			info(LOG_ERR, "Cant create a thread to handle the connection...\n");
		}

		pthread_attr_destroy(&attr);
	}
	else
			info(LOG_ERR, "No more memory :(\n");
}

/* Webcam stream thread: v is a ptr to a struct thread_data which packages a socket and a cdev
 * this function is meant to run as a separate thread and handles a webcam stream connection
 * on "sock" socket: It sends and HTTP header and then continuously captures a frame from cdev,
 * jpeg-encodes it and sends it. The loop is terminated when keep_going is false or when the
 * socket is closed from the other end
 */
void *send_stream_to(void *v) {
	struct timeval start, now;
	struct timespec sleep_length, sleep_rem;
	struct jpeg j;
	unsigned int buf_index;
	struct thread_data *td = (struct thread_data *)v;
	struct video_device *d = td->vdev;
	struct capture_device *cdev = d->capture;
	int jpeg_len, yuv_len, f_nr = 0, retval = 0, sock = td->sock; //, calibrated = 1;
	void *yuv_data, *jpeg_data;
	float ts, last_fps;

	CLEAR(j);
	XMALLOC(jpeg_data, void *, (cdev->width * cdev->height * 3));
	dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "New thread starting sock: %d, jpeg_quality: %d\n", sock, jpeg_quality);

	//setup the jpeg encoder based on the image palette
	if(setup_jpeg_encoder(cdev, &j)!=0) {
		info(LOG_ERR, "Error setting the JPEG encoder\n");
		goto end;
	}


	//send mjpeg header
	if (send_mjpeg_header(sock) != 0 )
		goto end;

	gettimeofday(&start, NULL);
	sleep_rem.tv_sec = sleep_rem.tv_nsec = 0;

	while((retval>=0) && keep_going) {

		gettimeofday(&now, NULL);
		if((now.tv_sec>=start.tv_sec + SHOW_FPS_INTERVAL)) {
			ts = (now.tv_sec - start.tv_sec) + ((float) (now.tv_usec - start.tv_usec)/1000000);
			last_fps = (f_nr / ts);
			//fprintf(stderr, "%d frames in %.3f sec (fps=%.1f, req_fps = %d)\n",f_nr, ts, last_fps, requested_fps);
			info(LOG_INFO, "%d frames in %.3f sec (fps=%.1f)\n",f_nr, ts, last_fps);

			//check whether the calibration was done
			if(fps_nanosleep_step == 0) {
				//no it wasnt, compute fps_nanosleep_step, the time it takes to capture,
				//jpeg-encode and send a single frame
				fps_nanosleep_step = (1000000000 / last_fps);
				dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1,"Calibrating loop: set nanosleep step to %09ld\n",fps_nanosleep_step);
				max_fps = last_fps;
				set_fps(requested_fps);
			} else {
				//calibration already completed
				//is current_fps = req_fps +- 0.5 ?
				if(last_fps<(requested_fps - 0.5 ))
					decr_nanosleep();
				else if (last_fps >(requested_fps + 0.5 ))
				 	incr_nanosleep();
				else
					dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1,"Current fps_nanosleep (%ld.%09ld) achieves the desired framerate\n", fps_secsleep, fps_nanosleep);
			}

			f_nr = 0;
			gettimeofday(&start, NULL);
		}

		//sleep to adjust the fps to correct value
		if(((fps_nanosleep != 0) || (fps_secsleep != 0))) {
			sleep_length.tv_nsec = fps_nanosleep;
			sleep_length.tv_sec = fps_secsleep;
			nanosleep(&sleep_length, &sleep_rem);
		}

		//get frame from v4l2
		if((yuv_data = (*cdev->actions->dequeue_buffer)(d, &yuv_len, &buf_index, NULL, NULL)) != NULL) {

			//encode in JPEG
			jpeg_len = (*j.jpeg_encode)(yuv_data,yuv_len, cdev, &j, jpeg_data);

			//return buffer to v4l2
			(*cdev->actions->enqueue_buffer)(d, buf_index);

			//send in multipart_jpeg stream
			retval = send_frame(sock, jpeg_data, jpeg_len);

			f_nr++;
		}
        else
        {
            info(LOG_ERR, "Error dequeing buffer\n");
        }
	}

	end:

	XFREE(jpeg_data);
	release_jpeg_encoder(cdev, &j);
	//not thread safe!!
	//if multiple clients enabled, needs locking
	//concurrent paths with start_client_thread() could lead to bad thinds
	//FIXME if MAX_CLIENT > 1
	if(--client_nr==0) {
		dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG2, "Last client, stopping capture \n");
		//Stop capture
		(*cdev->actions->stop_capture)(d);
	}

	//close socket
	info(LOG_INFO, "closing connection on socket %d\n", sock );
	close(sock);

	XFREE(v);

	return NULL;
}
/* Sends a motion JPEG frame at "d" of size "len" over socket "sock"
 * returns 0 (success) or (-1) failure because of a TCP connection problem (ie, close)
 */
int send_frame(int sock, void *d, int len) {
	int  bs = 0, retval = 0;
	char c[14];

	//send the header
	if(write(sock, MP_JPEG_FRAME_HEADER, strlen(MP_JPEG_FRAME_HEADER)) < 0) {
		dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1, "Error sending the MJPEG frame header\n");
		retval = -1;
		goto end;
	}

	sprintf(c, "%d\r\n\r\n",len);

	if(write(sock, c, strlen(c))<0) {
		dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1, "Error sending the MJPEG frame header\n");
		retval = -1;
		goto end;
	}

	//sends the frame itself
	while(len > 0) {
		bs = (len < OUTPUT_BLOCK_SIZE) ? len : OUTPUT_BLOCK_SIZE;
		if(write(sock, d, bs) <0) {
			dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1, "Error sending the MJPEG frame\n");
			retval = -1;
			break;
		}
		len -= bs;
		d += bs;
	}

	end:
	return retval;
}

/* Sends an MJPEG HTTP header over socket "sock"
 * returns 0(success) or -1(failure)
 */
int send_mjpeg_header(int sock) {
	char *header = NULL;
	int retval = 0;

	XMALLOC(header, char *, MP_JPEG_HTTP_HEADER_MAX_LENGTH + 1);
	sprintf(header, MP_JPEG_HTTP_HEADER, MP_JPEG_HTTP_BOUNDARY);

	if(write(sock, header, strlen(header)) < 0) {
		dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1, "Error sending the MJPEG header\n");
		retval = -1;
	}
	XFREE(header);
	return retval;
}

/*
 * Set fps_nanosleep and fps_secsleep according to a requested fps (int f)
 */
void set_fps (int f) {
	if(max_fps - requested_fps <= 0) {
		fps_nanosleep = 0;
		fps_secsleep = 0;
	} else {
		fps_secsleep = 0;
		fps_nanosleep = (1000000000 / requested_fps) - fps_nanosleep_step;
	}
	dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1,"Setting fps_nanosleep to %ld.%09ld\n", fps_secsleep, fps_nanosleep);
}

/* Increase fps{nano,sec}sleep by a fraction of fps_nanosleep_step
 * to slightly decrease the current fps
 */
void incr_nanosleep() {
	if((fps_nanosleep += (fps_nanosleep_step/8)) > 999999999) {
		fps_secsleep +=1;
		fps_nanosleep -= 1000000000;
	}
	dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1,"Incrementing fps_nanosleep %ld.%09ld\n", fps_secsleep, fps_nanosleep);
}

/* Decrease fps{nano,sec}sleep by a fraction of fps_nanosleep_step
 * to slightly decrease the current fps
 */
void decr_nanosleep() {
	if((fps_nanosleep -= (fps_nanosleep_step/8)) < 0) {
		fps_secsleep -=1;
		fps_nanosleep -= 1000000000;
	}
	if(fps_secsleep <0) {
		fps_secsleep = 0;
		fps_nanosleep = 0;
	}
	dprint(LOG_SOURCE_HTTP, LOG_LEVEL_DEBUG1,"Decrementing fps_nanosleep %ld.%09ld\n", fps_secsleep, fps_nanosleep);
}

void init_destination( j_compress_ptr cinfo ){}

boolean empty_output_buffer( j_compress_ptr cinfo ){return TRUE;}

void term_destination( j_compress_ptr cinfo ){}

void print_usage() {
	fprintf(stdout, "light_cap version %d.%d\n", VER_MAJ, VER_MIN);
	fprintf(stdout, "Usage: light_cap <device> <height> <width> <channel> <standard> <jpeg_quality> <fps> <verbosity> <port>");
#ifdef DEBUG
	fprintf(stdout, " <log_source> <log_level>\n");
#else
	fprintf(stdout, "\n");
#endif
	fprintf(stdout, "All options are mandatory.\n");
	fprintf(stdout, "<device>: full path to the video device file\n");
	fprintf(stdout, "<height>: requested height of captured frames.\n");
	fprintf(stdout, "<width>: requested width of captured frames.\n");
	fprintf(stdout, "(The V4L2 layer will use the closest height and width to the ones requested if they are not available)\n");
	fprintf(stdout, "<channel>: capture channel to be used (0 for webcams).\n");
	fprintf(stdout, "<standard>: standard of the video device (0: webcam, 1: PAL, 2: SECAM, 3:NTSC).\n");
	fprintf(stdout, "<jpeg_quality>: quality (0 - 100) of generated JPEG images\n");
	fprintf(stdout, "<fps>: required frame rate (1 - 25)\n");
	fprintf(stdout, "<verbosity>: what to print on terminal: 0: nothing; 1: info; 2: errors; 3: both\n");
        fprintf(stdout, "<port>: the TCP port on which light_cap should listen\n");
#ifdef DEBUG
	fprintf(stdout, "<log_source>: source of debug info (see log.h)\n");
	fprintf(stdout, "<log_level>: level of debug info (see log.h)\n");
#endif
	fprintf(stdout, "\nlight_cap is a lightweight video capture program. It grabbs frames from a V4L2 device and sends them\n");
	fprintf(stdout, "as a motion JPEG stream, which is accessible at the URL \"http://host:8080/webcam\"\n");
	fprintf(stdout, "light_cap also allows you to change capture parameters on the run via a set of web pages,\n");
	fprintf(stdout, "accessible at the URL \"http://host:8080/list\"\n");
}
