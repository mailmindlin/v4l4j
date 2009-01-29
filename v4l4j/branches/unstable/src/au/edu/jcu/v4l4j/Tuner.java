package au.edu.jcu.v4l4j;

public class Tuner {
	/**
	 * Radio tuner
	 */
	public static int RADIO = 1;
	
	/**
	 * TV tuner
	 */
	public static int TV = 2;
	/**
	 * The type of this tuner (RADIO or TV)
	 */
	private int type;
	
	/**
	 * Frequencies in KHz
	 */
	public static int KHZ = 1;
	
	/**
	 * Frequencies in MHz
	 */
	public static int MHZ = 2;
	/**
	 * Frequency unit (MHZ or KHZ)
	 */
	private int unit;
	
	/**
	 * The lowest & highest tunable frequencies
	 */
	private long rangeLow, rangeHigh;
	
	/**
	 * The name fo this tuner
	 */
	private String name;
	
	public Tuner(String n,int u, int t, long low, long high){
		type = t;
		unit = u;
		rangeLow = low & 0xffffffff;
		rangeHigh = high & 0xffffffff;
		name = n;		
	}

	/**
	 * This method returns the type of this tuner (Tuner.RADIO or Tuner.TV)
	 * @return the type
	 */
	public int getType() {
		return type;
	}

	/**
	 * This method returns the unit used by this tuner (Tuner.MHZ or Tuner.KHZ)
	 * @return the unit
	 */
	public int getUnit() {
		return unit;
	}


	/**
	 * This method returns the lowest tunable frequency supported by this tuner
	 * @return the rangeLow
	 */
	public long getRangeLow() {
		return rangeLow;
	}


	/**
	 * This method returns the highest tunable frequency supported by this tuner
	 * @return the rangeHigh
	 */
	public long getRangeHigh() {
		return rangeHigh;
	}


	/**
	 * This method returns the name of this tuner
	 * @return the name
	 */
	public String getName() {
		return name;
	}
}
