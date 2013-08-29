#!/usr/bin/python

import sys
import re


def fill_v4l_pixfmt_dict(d):
	# Open videodev2.h and parse all lines
	videodev_file = "videodev2.h"
	file = open(videodev_file)
	lines = file.readlines()

	# Keep all the lines that start with #define V4L2_PIX_FMT_xxx such as
	# #define V4L2_PIX_FMT_RGB332  v4l2_fourcc('R', 'G', 'B', '1') /*  8  RGB-3-3-2     */
	regexp = re.compile('^#define V4L2_PIX_FMT_\w*')

	for line in lines:
		match = regexp.match(line)
		if match == None:
			continue

		# Keep only the part after #define V4L2_PIX_FMT_ : 
		# when we get #define V4L2_PIX_FMT_RGB332, keep RGB332 
		pixelfmt = match.group()
		tokens = pixelfmt.split('#define V4L2_PIX_FMT_')
		pixfmt_name = tokens[-1] 		# RGB332

		# Get each of the chars in the fourcc and translate to an int and string version
		tokens = line.split('\'')
		#								 RGB1
		fourcc_int = ord(tokens[1]) + (ord(tokens[3]) << 8) + (ord(tokens[5]) << 16) + (ord(tokens[7]) << 24)
		fourcc_str = tokens[1] + tokens[3] + tokens[5] + tokens[7] 

		# Get the v4l2 pix fmt macro name:
		# V4L2_PIX_FMT_RGB332
		tokens = line.split(' ')
		v4l2_macro_name = tokens[1]

		# store elements in dictionary
		# RGB332			123456		RGB1		V4L2_PIXFMT_RGB332
		d[pixfmt_name] = [ fourcc_int, fourcc_str, v4l2_macro_name ]

	file.close()

def fill_v4l4j_pixfmt_dict(d):
	# Open V4L4JConstants.java and parse all lines
	v4l4j_file = "../src/au/edu/jcu/v4l4j/V4L4JConstants.java"
	file = open(v4l4j_file)
	lines = file.readlines()

	# Keep all the lines that start with public static final int IMF_ such as
	regexp = re.compile('^\s*public static final int IMF_.*')

	for line in lines:
		match = regexp.match(line)
		if match == None:
			continue

		# Keep only the part after 'public static final int IMF_' : 
		# when we get 'public static final int IMF_RGB332 = 123456; /* RGB1 */', keep 'RGB332 = 123456; /* RGB1 */'
		pixelfmt = match.group()
		
		tokens = pixelfmt.split('public static final int IMF_')
		
		pixfmt_def = tokens[-1]
		# Extract pixfmt name and fourcc_int :
		# when we get 'RGB332 = 123456;', extract RGB332 and 123456
		tokens = pixfmt_def.split(' ')
		
		pixfmt_name = tokens[0] 		# RGB331
		fourcc_int = tokens[2][:-1]  	# 123456
		if len(tokens) == 6:
			fourcc_str = tokens[4] 		# RGB1
		else:
			fourcc_str = ""

		#  RGB331			123456		RGB1
		d[pixfmt_name] = [ fourcc_int, fourcc_str ]

	file.close()


v4l_pixfmt_dict = dict()
v4l4j_pixfmt_dict = dict()
fill_v4l_pixfmt_dict(v4l_pixfmt_dict)
fill_v4l4j_pixfmt_dict(v4l4j_pixfmt_dict)

print "in v4l4j"
for p in v4l4j_pixfmt_dict.keys():
	print p, " " ,v4l4j_pixfmt_dict[p] 

print "in v4l"
for p in v4l_pixfmt_dict.keys():
	print p, " ", v4l_pixfmt_dict[p]

# First check if any existing pixel format in v4l4j has been removed from v4l
for p in v4l4j_pixfmt_dict.keys():
	if p not in v4l_pixfmt_dict.keys():
		print "Pixel format present in v4l4j has been removed from v4l"
		print p
		print "FIXME: this script does not handle this case"
		sys.exit(1)


print "in v4l but not in v4l4j"
for p in v4l_pixfmt_dict.keys():
	if p not in v4l4j_pixfmt_dict.keys():
		print p, " " , v4l_pixfmt_dict[p]


