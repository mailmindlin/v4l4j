package au.edu.jcu.v4l4j;

import java.awt.Point;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;

/**
 * We need our own implementation of a WritableRaster because we cannot create one
 * with {@link Raster#createInterleavedRaster(DataBuffer, int, int, int, int, int[], Point)} as
 * we cannot pass a {@link V4L4JDataBuffer} to it, it will throw an exception because
 * it expects a {@link DataBufferByte} object.
 * @author gilles
 *
 */
public class V4L4JRaster extends WritableRaster {
	
	public V4L4JRaster(SampleModel sampleModel, DataBuffer dataBuffer,
			Point origin) {
		super(sampleModel, dataBuffer, origin);
		// TODO Auto-generated constructor stub
	}
}
