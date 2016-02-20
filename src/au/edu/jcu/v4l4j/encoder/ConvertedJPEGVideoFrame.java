package au.edu.jcu.v4l4j.encoder;

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;

import javax.imageio.ImageIO;

import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;

public class ConvertedJPEGVideoFrame extends AbstractConvertedVideoFrame {

	protected ConvertedJPEGVideoFrame(int bufferSize) {
		super(bufferSize);
	}
	
	@Override
	protected BufferedImage refreshBufferedImage() {
		try (ByteArrayInputStream bais = new ByteArrayInputStream(frameBuffer, 0, frameLength)) {
			return ImageIO.read(bais);
		} catch (IOException e) {
			System.err.println("It seems your JVM is unable to decode this image.");

			// print supported image types and mime types
			System.err.println("Supported image types:");
			String supportedTypes[] = ImageIO.getReaderFormatNames();
			for (String name : supportedTypes)
				System.err.println(name);
			System.err.println("Supported MIME types:");
			String supportedMimeTypes[] = ImageIO.getReaderMIMETypes();
			for (String name : supportedMimeTypes)
				System.err.println(name);

			e.printStackTrace();
			throw new UnsupportedMethod("Unable to decode the image", e);
		}
	}
}
