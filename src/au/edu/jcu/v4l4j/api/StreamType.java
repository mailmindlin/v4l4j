package au.edu.jcu.v4l4j.api;

/**
 * An enum of data types that can be in a stream.
 * @author mailmindlin
 */
public enum StreamType {
	/**
	 * An unknown stream.
	 * This differs from {@link #OTHER}, because OTHER indicates that the
	 * stream type is known, but not one of the values in this enum, while
	 * UNKNOWN indicates that the driver does not know what type the stream is.
	 */
	UNKNOWN,
	/**
	 * A stream of video data.
	 * @see #IMAGE
	 */
	VIDEO,
	/**
	 * A stream of audio data.
	 */
	AUDIO,
	/**
	 * A stream of image data
	 * @see #VIDEO
	 */
	IMAGE,
	/**
	 * A stream of text data
	 */
	TEXT,
	/**
	 * A stream of clock tick events
	 */
	CLOCK,
	/**
	 * A stream of arbitrary binary data
	 */
	BINARY,
	/**
	 * A stream of data not listed in this enum, but is known to the driver.
	 * @see #UNKNOWN
	 */
	OTHER;
}
