package au.edu.jcu.v4l4j.impl.omx;

public class OMXComponent extends Component {
	private final String name;
	private final long pointer;
	private final OMXComponentProvider provider;
	
	protected OMXComponent(OMXComponentProvider provider, String name) {
		this.provider = provider;
		this.name = name;
		this.pointer = pointer;
	}
	
	private final native long getComponentHandle(String name);
	
	@Override
	public String getName() {
		return this.name;
	}
}