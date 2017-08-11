libvideo is a GPL-ed C wrapper library around the capture facility provided by
the Video4Linux (V4L) API. It offers a simplified API to capture frames from
video devices. Both V4L1 and V4L2 devices are supported. libvideo also provides
access to all V4L controls reported by the underlying V4L driver. This includes
the usual controls found on every video device such as brightness & contrast,
as well as private V4L controls which are device and driver-dependent, such as
'Save' & 'Restore' settings. A neat feature of libvideo is that it also tries 
to detect the underlying driver, and create pseudo controls for driver-specific,
private ioctls. This provides a uniform way to access and control video
devices. That way, control of the video capture device is done seamlessly
through normal V4L2 controls exposed by libvideo. A concrete example:
The PWC driver supports webcams with pan/tilt capabilities. Control of the pan
and tilt however, requires first checking with the driver if the attached device
supports these (done through driver-specific ioctls, which means you have to 
detect which driver the device is using first). If supported, pan/tilt control
is then done via driver-specific ioctls. libvideo will probe the driver. If it
is PWC, it will probe the device and if the device supports Pan/Tilt features,
libvideo will create V4L controls to provide access to Pan & tilt.

## Documentation
The latest version of libvideo and associated documentation (Howtos, API, ...) can
always be found at [github.com/mailmindlin/v4l4j](https://github.com/mailmindlin/v4l4j).

## Tested Devices
See TestedHardware file

## Features
livideo allows:
- frame capture from V4L devices in their native image formats. For V4L2
  devices, extra formats (RGB24, BGR24, YUV420 and YVU420) are also available.
  The conversion is done transparently by libvideo (which uses libv4l convert to
  do the conversion internally). This ensures libvideo works with exotic & chip-
  specific image formats.
- access to V4L controls
- access to driver-private ioctls through "fake" V4L2 controls created by 
  libvideo
- hardware acceleration for some video conversions, depending on platform

## Requirements
- a V4L compatible video source (capture card, webcam)

## Compiling
- run `make` in the directory.

## Usage
libvideo is a library and is meant to be linked to an application. However, 
three test programs are shipped:
- `test-capture`, which runs a capture from a given video device at a given
  resolution. It then prints the frame rate.
- `dump-capture` captures frames and writes them to files.
- `list-caps` which lists information about a given video device.

To compile the test programs, run `make test`.
The "example" directory contains light_cap, a lighweight application which 
captures frames and sends them out as a multipart JPEG stream suitable for
viewing in any standard-compliant web browser. See example/README for more
details.

## Debugging
Run `DEBUG=1 make clean test` to recompile libvideo with debug statements.
Extra output will be produced to allows troubleshooting.

## Feedback
Feedback is is greatly appreciated to expand the list of platforms and video
devices v4l4j has been successfully tested with. Feedback, issues and queries 
can be submitted as an issue at [https://github.com/mailmindlin/v4l4j/issues](https://github.com/mailmindlin/v4l4j/issues).

This library was exported from [v4l4j.googlecode.com](code.google.com/archive/p/v4l4j).
