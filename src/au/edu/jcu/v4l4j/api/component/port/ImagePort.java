package au.edu.jcu.v4l4j.api.component.port;

public interface ImagePort extends IVPort {
	@Override
	default ImagePortDefinition getDefinition() {
		return getDefinition(false);
	}
	
	@Override
	ImagePortDefinition getDefinition(boolean flush);
}
