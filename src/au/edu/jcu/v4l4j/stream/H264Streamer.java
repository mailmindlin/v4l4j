package au.edu.jcu.v4l4j.stream;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;

public class H264Streamer implements Runnable {
	public static final short RTP_PORT = 18888;
	public static final short RTPC_PORT = RTP_PORT + 1;
	public static final short ttl = 255;

	String url;
	int port;
	ExecutorService executor = Executors.newCachedThreadPool();
	ByteBuffer imageBuffer = ByteBuffer.allocateDirect(1 * 1024 * 1024);

	public H264Streamer(String url, int port) {
		this.url = url;
		this.port = port;
	}

	public void start() {
		try {
			// TODO add portability
			Runtime.getRuntime().exec("sudo ip route add 224.0.0.0/4 dev eth0").waitFor();
		} catch (InterruptedException | IOException e) {
			e.printStackTrace();
			return;
		}
		executor.submit(this);
	}

	ImageStartCodeType getH264ImageType(ByteBuffer buffer) {
		if (buffer.limit() < 5)
			return ImageStartCodeType.NONE;

		byte[] tmp = new byte[5];
		buffer.get(tmp, 0, 5);

		if (tmp[0] == 0x00 && tmp[1] == 0x00 && tmp[2] == 0x00 && tmp[3] == 0x01) {
			switch (tmp[4]) {
			case 0x27:
				return ImageStartCodeType.SPS;
			case 0x28:
				return ImageStartCodeType.PPS;
			case 0x25:
				return ImageStartCodeType.I;
			case 0x21:
				return ImageStartCodeType.P;
			}
		}
		return ImageStartCodeType.NONE;
	}

	public void feedStreamerAndRecordingManager(ByteBuffer buffer) {
		final int imageBufferCapacity = imageBuffer.capacity();
		int imageBufferSize = imageBuffer.limit();
		if (getH264ImageType(buffer) != ImageStartCodeType.NONE) {
			// Image start
			// Image Start
			if (imageBufferSize > 0) { // New image comes, Send last image
//				LiveVideoSource.feedStreamer(imageBuffer, imageBufferSize);
				imageBuffer.limit(imageBufferSize = 0);
//				CaptureDone();
//				WaitStreamingDone();
			}
		} else {
			// Image fragment
		}

		if (imageBufferCapacity >= imageBufferSize + buffer.limit()) {
			imageBuffer.put(buffer);
		} else {
			throw new BufferOverflowException();
		}
	}

	@Override
	public void run() {
		try (DatagramSocket rtpSocket = new DatagramSocket(RTP_PORT);
				DatagramSocket rtpcSocket = new DatagramSocket(RTPC_PORT)) {
			rtpSocket.setSendBufferSize(16 * 1024 * 1024);//16MB
			InetAddress localhost = InetAddress.getLocalHost();
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
