package au.edu.jcu.v4l4j;

/**
 * Object of this class encapsulate information about a tuner:
 * <ul>
 * <li>its type: V4L4JConstants.RADIO or V4L4JConstants.TV,</li>
 * <li>the frequency unit: V4L4JConstants.KHZ or V4L4JConstants.MHZ,</li>
 * <li>the name of the tuner,</li>
 * <li>the index of the tuner,</li>
 * <li>and the lowest and highest tunable frequencies.</li>  
 * </ul>
 * @author gilles
 *
 */
public class TunerInfo {
	/**
	 * The type of this tuner (RADIO or TV)
	 */
	private int type;
	
	/**
	 * Frequency unit (MHZ or KHZ)
	 */
	private int unit;
	
	/**
	 * The lowest & highest tunable frequencies
	 */
	private long rangeLow, rangeHigh;
	
	/**
	 * The name of this tuner
	 */
	private String name;
	
	/**
	 * The V4L index of this tuner 
	 */
	private int index;
	
	
	public TunerInfo(String n, int u, int t, int id, long low, long high){
		type = t;
		unit = u;
		rangeLow = low & 0xffffffff;
		rangeHigh = high & 0xffffffff;
		name = n;
		index = id;
		
	}

	/**
	 * This method returns the type of this tuner (V4L4JConstants.RADIO or V4L4JConstants.TV)
	 * @return the type
	 */
	public int getType() {
		return type;
	}

	/**
	 * This method returns the unit used by this tuner (V4L4JConstants.MHZ or V4L4JConstants.KHZ)
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
	
	/**
	 * This method returns the V4L index of this tuner.
	 * @return the V4L index of this tuner.
	 */
	int getIndex(){
		return index;
	}
}
