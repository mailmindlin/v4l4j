package au.edu.jcu.v4l4j.stream;

public enum ImageStartCodeType {
	NONE,
	/**
	 * SPS Image Start Code
	 */
	SPS,
	/**
	 * PPS Image Start Code
	 */
	PPS,
	/**
	 * I Image Start Code
	 */
	I,
	/**
	 * P Image Start Code
	 */
	P
}
