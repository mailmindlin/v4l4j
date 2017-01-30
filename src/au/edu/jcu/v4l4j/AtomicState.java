package au.edu.jcu.v4l4j;


import java.io.Closeable;
import java.lang.reflect.Field;
import java.util.concurrent.TimeUnit;

import sun.misc.Unsafe;

@SuppressWarnings("deprecation")
public class AtomicState {
	private static final Unsafe unsafe;
	private static final int usersFieldOffset;
	private static final int currentFieldOffset;
	
	static {
		Unsafe tmp = null;
		try {
			Field unsafeInstanceField = Unsafe.class.getDeclaredField("theUnsafe");
			unsafeInstanceField.setAccessible(true);
			tmp = (Unsafe) unsafeInstanceField.get(null);
		} catch (IllegalArgumentException | IllegalAccessException | NoSuchFieldException | SecurityException e) {
			throw new Error(e);
		} finally {
			unsafe = tmp;
		}
		try {
			usersFieldOffset = unsafe.fieldOffset(AtomicState.class.getDeclaredField("users"));
			currentFieldOffset = unsafe.fieldOffset(AtomicState.class.getDeclaredField("currentState"));
		} catch (Exception e) {
			throw new Error("Unable to lookup field offsets", e);
		}
	}
	
	private volatile int users = 0;
	private volatile int currentState = 0;
	private volatile boolean allowNonExclusiveLock = true;
	
	public AtomicState(State initialValue) {
		this.currentState = initialValue == null ? -1 : initialValue.ordinal();
	}
	
	public AtomicStateUser getIfNot(State unexpected) {
		AtomicStateUser user = new AtomicStateUser();
		//TODO finish
		return user;
	}
	
	public AtomicStateUser getUser(State expected, State update) {
		AtomicStateUser user = new AtomicStateUser();
		if (!unsafe.compareAndSwapInt(this, AtomicState.currentFieldOffset, expected.ordinal(), update.ordinal()))
			user.close();
		return user;
	}
	
	public void set(State value) {
		this.currentState = value.ordinal();
	}
	
	public void lazySet(State value) {
		unsafe.putOrderedInt(this, AtomicState.currentFieldOffset, value.ordinal());
	}
	
	public State get() {
		int value = this.currentState;
		State[] states = State.values();
		if (value < 0 || value > states.length)
			return null;
		return states[value];
	}
	
	public int getUsers() {
		return this.users;
	}
	
	public AtomicStateUser getSelfish() {
		return getSelfish(TimeUnit.NANOSECONDS, 0);
	}
	
	public AtomicStateUser getSelfish(TimeUnit unit, long duration) {
		return null;
	}
	
	class AtomicStateUser implements Closeable {
		boolean isValid;
		
		AtomicStateUser() {
			this(true);
		}
		
		AtomicStateUser(boolean valid) {
			this.isValid = valid;
			unsafe.getAndAddInt(AtomicState.this, AtomicState.usersFieldOffset, 1);
		}
		
		@Override
		public void close() {
			if (!isValid)
				return;
			isValid = false;
			unsafe.getAndAddInt(AtomicState.this, AtomicState.usersFieldOffset, -1);
		}
	}
	
	public static enum State {
		UNINITIALIZED,
		INIT,
		STARTED,
		STOPPED,
		RELEASED;
	}
}
