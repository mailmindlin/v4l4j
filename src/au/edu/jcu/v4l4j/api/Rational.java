package au.edu.jcu.v4l4j.api;

public class Rational extends Number {
	private static final long serialVersionUID = 3257202508717804844L;
	protected final int numerator;
	protected final int denominator;
	
	public Rational(int numerator, int denominator) {
		if (denominator == 0)
			throw new IllegalArgumentException("Denominator may not be 0");
		this.numerator = numerator;
		this.denominator = denominator;
	}
	
	public int getNumerator() {
		return this.numerator;
	}
	
	public int getDenominator() {
		return this.denominator;
	}

	@Override
	public double doubleValue() {
		return ((double)getNumerator()) / ((double) getDenominator());
	}

	@Override
	public float floatValue() {
		return ((float)getNumerator()) / ((float) getDenominator());
	}

	@Override
	public int intValue() {
		int num = this.getNumerator();
		int den = this.getDenominator();
		//Check for overflow, and saturate, because Integer.MAX_VALUE < -Integer.MIN_VALUE
		if (num == Integer.MIN_VALUE && den == -1)
			return Integer.MAX_VALUE;
		return num / den;
	}

	@Override
	public long longValue() {
		double d = doubleValue();
		if (d > Long.MAX_VALUE)
			return Long.MAX_VALUE;
		if (d < Long.MIN_VALUE)
			return Long.MIN_VALUE;
		return (long) d;
	}
}
