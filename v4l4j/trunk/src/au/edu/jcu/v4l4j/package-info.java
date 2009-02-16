/**
 * <h1>Video4Linux4java Main Package</h1>
 * <h2>Introduction</h2>
 * The Video4Linux (V4L) API provides data structure & methods to access and control
 * video input & output hardware, as well as radio tuners. The API is divided into
 * several interfaces, including video capture, video overlay, video output & 
 * teletext interfaces. Classes in this package provide access to the video capture 
 * interface of V4L only. This interface allows you to setup a video source and grab 
 * images or frames from it.  
 * <h2>Usage</h2>
 * <h3>Video device object</h3>
 * To use v4l4j, the first step is to create a {@link au.edu.jcu.v4l4j.VideoDevice}
 * object, which represents a physical video input hardware. This includes webcams 
 * and capture / tuner cards. The only pre-requisite is that your hardware must be 
 * supported and its driver loaded. Creating a {@link au.edu.jcu.v4l4j.VideoDevice}
 * is simple. You only need the path to a valid device file to create this object:<br>
 * <code>
 * VideoDevice vd = new VideoDevice("/dev/video0");
 * </code><br>
 * As it is the case most of the time when interacting with hardware, you must
 * release resources and data structures when finished. All v4l4j classes follow 
 * this rule (with few exceptions clearly noted in their java documentation),
 * including {@link au.edu.jcu.v4l4j.VideoDevice}s:<br>
 * <code>
 * vd.release();</code><br>
 * Once an object has been <code>release()</code>d, it must not be reused, and 
 * all references to it must be discarded, by explicitly setting them to 
 * <code>null</code> for example. Calling any <code>release()</code> method more 
 * than once will throw a
 *  {@link au.edu.jcu.v4l4j.exceptions.StateException StateException}.
 * <br>
 * {@link au.edu.jcu.v4l4j.VideoDevice} objects give you access to 3 broad 
 * categories:
 * <ul>
 * <li>information about the underlying hardware through 
 * {@link au.edu.jcu.v4l4j.DeviceInfo} objects,</li>
 * <li>a variety of controls through {@link au.edu.jcu.v4l4j.ControlList}
 * objects, and</li>
 * <li>a frame capture facility using {@link au.edu.jcu.v4l4j.FrameGrabber}
 * objects.</li>
 * </ul>
 * Each of these can be obtained independently from the other, by calling 
 * the appropriate method on a {@link au.edu.jcu.v4l4j.VideoDevice} instance.
 * <h3>Video hardware information</h3>
 * With a valid {@link au.edu.jcu.v4l4j.VideoDevice} object, you 
 * can retrieve information about 
 * the underlying hardware and find out, for example how many video inputs the 
 * device has, how many tuners are available or the set of supported video 
 * standards. This kind of information is contained in a 
 * {@link au.edu.jcu.v4l4j.DeviceInfo} object. You can get a reference to a 
 * {@link au.edu.jcu.v4l4j.DeviceInfo} object for a given video device by calling
 * {@link au.edu.jcu.v4l4j.VideoDevice#getDeviceInfo()} on the 
 * {@link au.edu.jcu.v4l4j.VideoDevice} object. See the documentation of the
 * {@link au.edu.jcu.v4l4j.VideoDevice} and {@link au.edu.jcu.v4l4j.DeviceInfo}
 * classes for more information.
 * 
 * <h3>Video controls</h3>
 * Video capture devices usually have a set of controls which influence 
 * various parameters related to the video stream. Examples are brightness, 
 * contrast, gamma & white balance. A single control is represented by
 * {@link au.edu.jcu.v4l4j.Control} object. The set of supported controls for 
 * a given video device hardware-dependent and is embedded into a 
 * {@link au.edu.jcu.v4l4j.ControlList} object, which can be obtained by calling 
 * {@link au.edu.jcu.v4l4j.VideoDevice#getControlList()}. <strong>The 
 * {@link au.edu.jcu.v4l4j.ControlList} object and associated 
 * {@link au.edu.jcu.v4l4j.Control}s must be released when they are no longer 
 * needed.</strong> This is done by calling 
 * {@link au.edu.jcu.v4l4j.VideoDevice#releaseControlList()}. Again, once released,
 * neither the {@link au.edu.jcu.v4l4j.ControlList} nor individual 
 * {@link au.edu.jcu.v4l4j.Control}s must be used, and any reference to them 
 * must be discarded, by explicitly setting them to <code>null</code> for example.
 * For more information, check the documentation of the 
 * {@link au.edu.jcu.v4l4j.Control} and {@link au.edu.jcu.v4l4j.ControlList} classes.
 * 
 * <h3>Video capture</h3>
 * Capture in v4l4j is extremely simple. v4l4j hands out captured frames in a
 * {@link java.nio.ByteBuffer} object, either in a native format (also called 
 * raw format) or JPEG format:
 * <ul>
 * <li>JPEG format capture availability depends on what native formats are available.
 * Most (but not all) native formats can be JPEG-encoded. Calling 
 * {@link au.edu.jcu.v4l4j.VideoDevice#canJPEGEncode()} will tell whether it is
 * available or not. 
 * <li>Native format capture is always available regardless of the video device. 
 * The set of supported {@link au.edu.jcu.v4l4j.ImageFormat}s can be obtained through 
 * the {@link au.edu.jcu.v4l4j.DeviceInfo} object associated with a
 * {@link au.edu.jcu.v4l4j.VideoDevice} object (see paragraph "Video hardware 
 * information" above). When capturing in a native format, v4l4j simply hands out the 
 * captured frame without further processing.</li>
 * </ul>
 *  
 * Frame capture in v4l4j is done using {@link au.edu.jcu.v4l4j.FrameGrabber} or 
 * {@link au.edu.jcu.v4l4j.JPEGFrameGrabber} objects:
 * <ul>
 * <li>First, get a frame grabber object from a video device, by invoking
 * {@link au.edu.jcu.v4l4j.VideoDevice#getRawFrameGrabber(int, int, int, int) 
 * VideoDevice.getRawFrameGrabber()} or
 * {@link au.edu.jcu.v4l4j.VideoDevice#getJPEGFrameGrabber(int, int, int, int, int) 
 * VideoDevice.getJPEGFrameGrabber()}.</li>
 * <li>Second, when ready to capture, start it by calling 
 * {@link au.edu.jcu.v4l4j.FrameGrabber#startCapture()}.</li>
 * <li>Get the last frame using 
 * {@link au.edu.jcu.v4l4j.FrameGrabber#getFrame()} and do something useful with it.</li>
 * <li>When done capturing, stop the capture with 
 * {@link au.edu.jcu.v4l4j.FrameGrabber#stopCapture()}. If you need to start capturing 
 * again later, just call {@link au.edu.jcu.v4l4j.FrameGrabber#startCapture()} again.</li>
 * <li><strong>Again, when finished with the {@link au.edu.jcu.v4l4j.FrameGrabber} 
 * object, you must release it with a call to
 * {@link au.edu.jcu.v4l4j.VideoDevice#releaseFrameGrabber()}.
 * </strong> Do not use the frame grabber subsequently and set any stray reference to null.</li>
 * </ul>
 * More information on video capture is given in the documentation of the 
 * {@link au.edu.jcu.v4l4j.FrameGrabber} class.
 */
package au.edu.jcu.v4l4j;
