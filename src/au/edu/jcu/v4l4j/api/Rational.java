package au.edu.jcu.v4l4j.api;

/**
 * A datatype representing a rational number (a number that can be represented as a fraction).
 */
public class Rational extends Number implements Comparable<Number> {
	
	protected static long gcd(long a, long b) {
		int sign = Math.sign(a) * Math.sign(b);
		return -1;//TODO finish
	}
	public static Rational reduced(int numerator, int denominator) {
		//Slightly modified binary GCD algorithm
		int sign = Math.sign(numerator) * Math.sign(denominator);
		int num = Math.abs(numerator);
		int den = Math.abs(denominator);
		
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
		return new Rational(sign * num / u, den / u);
	}
	
	/**
	 * Returns the closest approximation of the given rational with 32-bit integer precision.
	 */
	protected static Rational reducedApprox(long numerator, long denominator) {
		//Slightly modified binary GCD algorithm
		int sign = Math.sign(numerator) * Math.sign(denominator);
		long num = Math.abs(numerator);
		long den = Math.abs(denominator);
		
		if (num == 0)
			return new Rational(0, 1);
		
		//Remove all all divisors that are 2 from both
		while (((num | den) & 1) == 0) {
			num >>= 1;
			den >>= 1;
		}
		
		//Find the GCD of the remaining numbers
		long u = num;
		long v = den;
		
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
		
		//Now we have the GCD of (num, den) (it's u), return a rational
		num /= u;
		den /= u;
		if (num > Integer.MAX_VALUE) {
			//We can't cast to integers without loss of precision
			
			if (((num + (den - 1))/den) > Integer.MAX_VALUE)//Approximate with Integer.MAX_VALUE/1
				return new Rational(sign == 1 ? Integer.MAX_VALUE : Integer.MIN_VALUE, 1);
			
			//See http://stackoverflow.com/a/4357555
			//We have to find a fraction that's pretty close
			Rational low = new Rational(0, 1);
			Rational high = new Rational((num + (den - 1)) / den, 1);
			if (num < 0) {
				Rational tmp = low;
				low = high;
				high = tmp;
			}
			
			float tolerance = 1.0e-1;
			while (true) {
				Rational middle = 
			}
		}
		return new Rational((int) (num * sign), (int) den);
	}
	
	private static final long serialVersionUID = 3257202508717804844L;
	protected final int numerator;
	protected final int denominator;
	
	public Rational(int numerator, int denominator) {
		if (denominator == 0)
			throw new IllegalArgumentException("Denominator may not be 0");
		if (denominator < 0) {
			//TODO catch overflows (abs(INT_MIN) > INT_MAX)
			numerator *= -1;
			denominator *= -1;
		}
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
	
	public Rational add(Rational other) {
		if (this.getDenominator() == other.getDenominator())
			//We don't have to scale
			return Rational.reduced(this.getNumerator() + other.getNumerator(), this.getDenominator());
		long numerator = ((long)this.getNumerator()) * ((long)other.getDenominator()) + ((long)other.getNumerator()) * ((long)this.getDenominator());
		long denominator = ((long)this.getDenominator()) * ((long)other.getDenominator());
		return Rational.reduced(numerator, denominator);
	}
	
	public Rational subtract(Rational other) {
		if (this.getDenominator() == other.getDenominator()) {
			//No scaling
			return Rational.reduced(this.getNumerator() - other.getNumerator(), this.getDenominator());
		}
	}
	
	public Rational reduced() {
		return Rational.reduced(this.getNumerator(), this.getDenominator());
	}
	
	@Override
	public int compareTo(Number other) {
		if (other instanceof Rational) {
			Rational r = (Rational) other;
			//Scale the numerators to same 
			//Convert to longs to prevent overflows
			long myScaledNumerator = ((long) this.getNumerator()) * ((long) other.getDenominator());
			long otherScaledNumerator = ((long) other.getNumerator()) * ((long) other.getDenominator());
			if (myScaledNumerator < otherScaledNumerator)
				return -1;
			if (myScaledNumerator > otherScaledNumerator)
				return 1;
			return 0;
		}
		double myValue = this.doubleValue();
		double otherValue = other.doubleValue();
		if (myValue < otherValue)
			return -1;
		if (myValue > otherValue)
			return 1;
		return 0;
	}
}
