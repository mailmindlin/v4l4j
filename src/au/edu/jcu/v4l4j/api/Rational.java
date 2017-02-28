package au.edu.jcu.v4l4j.api;

/**
 * A datatype representing a rational number (a number that can be represented as a fraction).
 */
public class Rational extends Number implements Comparable<Number> {
	
	protected static int sign(long value) {
		if (value > 0)
			return 1;
		if (value < 0)
			return -1;
		return 0;
	}
	
	protected static long gcd(long a, long b) {
		int shift = Long.numberOfTrailingZeros(a | b);
		a >>= Long.numberOfTrailingZeros(a);
		
		do {
			b >>= Long.numberOfLeadingZeros(b);
			if (a > b) {
				long tmp = a;
				a = b;
				b = tmp;
			}
			
			b -= a;
		} while (b != 0);
		
		return a << shift;
	}
	
	public static Rational reduced(int numerator, int denominator) {
		//Slightly modified binary GCD algorithm
		int sign = sign(numerator) * sign(denominator);
		int num = Math.abs(numerator);
		int den = Math.abs(denominator);
		
		if (num == 0)
			return new Rational(0, 1);
		
		//Remove all all divisors that are 2 from both
		int shift = Integer.numberOfTrailingZeros(num | den);
		num >>= shift;
		den >>= shift;
		
		//Find the GCD of the remaining numbers
		int u = num;
		int v = den;
		
		//Remove powers of 2 from u
		u >>= Integer.numberOfTrailingZeros(u);
			
		do {
			//Remove powers of 2 from v
			v >>= Integer.numberOfLeadingZeros(v);
			
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
		int sign = sign(numerator) * sign(denominator);
		long num = Math.abs(numerator);
		long den = Math.abs(denominator);
		
		if (num == 0)
			return new Rational(0, 1);
		
		{
			final long gcd = Rational.gcd(num, den);
			num /= gcd;
			den /= gcd;
		}
		
		if (num > Integer.MAX_VALUE) {
			//We can't cast to integers without loss of precision
			
			if (((num + (den - 1))/den) >= Integer.MAX_VALUE)//Approximate with Integer.MAX_VALUE/1
				return new Rational(sign == 1 ? Integer.MAX_VALUE : Integer.MIN_VALUE, 1);
			
			long iPart = num / den;
			double fPart = 1.0 * (num % den) / den;
			//Maximum denominator that we can use for our approximation and still have the
			//numerator within the range of int32
			int maxApproxDenominator = (int) Math.abs(Integer.MAX_VALUE / iPart);
			//See http://stackoverflow.com/a/4357555
			//We have to find a fraction that's pretty close
			int loNum = 0, loDen = 1;
			int hiNum = 1, hiDen = 1;
			
			final double tolerance = 1.0e-4;
			while (true) {
				//TODO fixme; I think that this part needs some work.
				long midNum = hiNum * loDen + loNum * hiDen;
				long midDen = hiDen * loDen;
				final long gcd = gcd(midNum, midDen);
				midNum /= gcd;
				midDen /= gcd;
				
				double d = 1.0 * midNum / midDen;
				if (Math.abs(fPart - d) < tolerance)
					return new Rational((int) (midNum + (num % den) * midDen / den), (int) midDen);
				if (fPart > d) {
					loNum = (int) midNum;
					loDen = (int) midDen;
				} else {
					hiNum = (int) midNum;
					hiDen = (int) midDen;
				}
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
		return Rational.reducedApprox(numerator, denominator);
	}
	
	public Rational subtract(Rational other) {
		if (this.getDenominator() == other.getDenominator()) {
			//No scaling
			return Rational.reduced(this.getNumerator() - other.getNumerator(), this.getDenominator());
		}
		long numerator = ((long)this.getNumerator()) * ((long)other.getDenominator()) - ((long)other.getNumerator()) * ((long)this.getDenominator());
		long denominator = ((long)this.getDenominator()) * ((long)other.getDenominator());
		return Rational.reducedApprox(numerator, denominator);
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
			long myScaledNumerator = ((long) this.getNumerator()) * ((long) r.getDenominator());
			long otherScaledNumerator = ((long) r.getNumerator()) * ((long) this.getDenominator());
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
