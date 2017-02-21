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
	
	public Rational reduced() {
		//Slightly modified binary GCD algorithm
		int num = this.numerator;
		int den = this.denominator;
		
		if (num == 0)
			return new Rational(0, 1);
		
		//Remove all all divisors that are 2 from both
		while (((num | den) & 1) == 0) {
			num >>= 1;
			den >>= 1;
		}
		
		//Find the GCD of the remaining numbers
		int u = num;
		int v = den;
		
		while ((u & 1) == 0)
			u >>= 1;
			
		do {
			while ((v & 1) == 0)
				v >>= 1;
			
			if (u > v) {
				int tmp = u;
				u = v;
				v = tmp;
			}
			
			v -= u;
		} while (v != 0);
		
		//'u' is the GCD
		return new Rational(num / u, den / u);
	}
}
