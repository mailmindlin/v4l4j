package au.edu.jcu.v4l4j.api.component.port;

public interface VideoPort extends IVPort {
	@Override
	default VideoPortDefinition getDefinition() {
		return getDefinition(false);
	}
	
	@Override
	VideoPortDefinition getDefinition(boolean flush);
}
