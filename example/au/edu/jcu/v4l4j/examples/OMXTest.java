package au.edu.jcu.v4l4j.examples;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.CloseableIterator;
import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.ComponentState;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;
import au.edu.jcu.v4l4j.impl.jni.MemoryUtils;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;
import au.edu.jcu.v4l4j.impl.omx.BaseOMXQueryControl;
import au.edu.jcu.v4l4j.impl.omx.OMXComponent;
import au.edu.jcu.v4l4j.impl.omx.OMXComponentPort;
import au.edu.jcu.v4l4j.impl.omx.OMXComponentProvider;
import au.edu.jcu.v4l4j.impl.omx.OMXConstants;
import au.edu.jcu.v4l4j.impl.omx.OMXControlDefinition.OMXControlDefinitionRegistry;
import au.edu.jcu.v4l4j.impl.omx.OMXQuery;
import au.edu.jcu.v4l4j.impl.omx.OMXVideoPort;

public class OMXTest {
	public static int VIDEO_WIDTH = 1920/4;
	public static int VIDEO_HEIGHT = 1080/4;
	public static int VIDEO_FRAMERATE = 25;
	public static int VIDEO_BITRATE = 10_000_000;
	
	public static void main(String...fred) throws Exception {
//		for (PrimitiveStructFieldType<?> type : PrimitiveStructFieldType.values())
//			System.out.println(type.name() + "[align=" + type.getAlignment() + "; size=" + type.getSize() + "]");
		
		OMXControlDefinitionRegistry registry = testLoadDefs();
		registry.registerDefinition(OMXConstants.CTRL_VIDEO_PARAM_FORMAT);
		
		OMXComponentProvider provider = OMXComponentProvider.getInstance();
		
		System.out.println("Available components:");
		for (String name : provider.availableNames(Paths.get("/"), null))
			System.out.println("\t=> " + name);
		
		OMXComponent encoder = (OMXComponent) provider.get("OMX.broadcom.video_encode");
		System.out.println("Encoder name: " + encoder.getName());
		System.out.println("Encoder state: " + encoder.getState());
		
		OMXTest.testAccess(encoder.getPointer());
		
		//Test that we can query where ports are
		OMXTest.testQueryPorts(encoder);
		
		Set<? extends ComponentPort> ports = encoder.getPorts();
		System.out.println("Ports: " + ports);
		
		for (ComponentPort port : ports)
			printPort(port);
		
		
		VideoPort inputPort = (VideoPort) encoder.getPort(200);
		VideoPort outputPort = (VideoPort) encoder.getPort(201);
		
		System.out.println("Preparing ports...");
		inputPort.setEnabled(false);
		outputPort.setEnabled(false);
		
		//List available codecs
		OMXTest.testApi(outputPort, registry);
		
		//Magic
		OMXTest.testQueryPortdef(encoder);
		
		blockUntilStateChange(encoder, ComponentState.IDLE);
		
		((OMXVideoPort)inputPort).pull();
		((OMXVideoPort)outputPort).pull();
		
		System.out.println("Enabling ports...");
		inputPort.setEnabled(true);
		Thread.sleep(200);
		((OMXVideoPort)inputPort).pull();
		System.out.println("Input enabled: " + inputPort.isEnabled());
		outputPort.setEnabled(true);
		Thread.sleep(200);
		((OMXVideoPort)outputPort).pull();
		System.out.println("Output enabled: " + outputPort.isEnabled());
		
		
		System.out.println("Allocating buffers...");
//		ByteBuffer buffer = ByteBuffer.allocateDirect(inputPort.bufferSize());
//		FrameBuffer inBuffer = inputPort.useBuffer(buffer);
		FrameBuffer inBuffer = inputPort.allocateBuffer(inputPort.bufferSize());
		FrameBuffer outBuffer = outputPort.allocateBuffer(outputPort.bufferSize());
		Thread.sleep(200);
		
		printFB("Input buffer", inBuffer);
		printFB("Output buffer", outBuffer);
		
		blockUntilStateChange(encoder, ComponentState.EXECUTING);
		
		//outputPort.fill(outBuffer);
		
		System.out.println("Putting random stuff in buffer...");
		ByteBuffer image = ByteBuffer.allocate(inBuffer.getCapacity());
		for (int i = 0; i < inBuffer.getCapacity();i+=8)
			image.putLong(0);
		image.putLong(0, -1L);
		image.flip();
		
		inBuffer.asByteBuffer().put(image.duplicate()).flip();
		
		
		AtomicInteger gSeq = new AtomicInteger(0);
		AtomicBoolean doQueueInput = new AtomicBoolean(false);
		AtomicBoolean doQueueOutput = new AtomicBoolean(false);
		
		
		File outFile = new File("out.h264").getAbsoluteFile();
		System.out.println("Writing out to " + outFile);
		@SuppressWarnings("resource")
		final FileChannel outChannel = new FileOutputStream(outFile, false).getChannel();
		
		final int NUM_FRAMES = 5000;
		
		outputPort.onBufferFill(frame -> {
//			System.out.println("Buffer fill called in Java");
//			System.out.println("" + frame.asByteBuffer().remaining() + " bytes in buffer");
//			System.out.println("Seq " + frame.getSequenceNumber());
//			System.out.println("Time " + frame.getTimestamp());
//			System.out.println("Flags " + ((OMXFrameBuffer) frame).getFlags());
			synchronized (outChannel) {
				try {
					outChannel.write(frame.asByteBuffer());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
//			System.out.println("...done reading frame");
			doQueueOutput.set(true);
//			synchronized (lock) {
//				lock.notifyAll();
//			}
//			System.out.println("Bye!0");
		});
		
		inputPort.onBufferEmpty(frame->{
//			System.out.println("Buffer empty called in Java");
			int seq = gSeq.incrementAndGet();
			if (seq > NUM_FRAMES)
				return;
//			System.out.println("Writing frame " + seq);
			ByteBuffer buf = frame.asByteBuffer();
			buf.clear();
//			for (int i = 0; i < inBuffer.getCapacity();i+=8)
//				buf.putLong(System.nanoTime());
			buf.put(image.duplicate());
			buf.flip();
			frame.setSequenceNumber(seq);
			frame.setTimestamp(System.nanoTime());
			doQueueInput.set(true);
//			System.out.println("...done writing frame " + seq);
//			synchronized (lock) {
//				lock.notifyAll();
//			}
//			System.out.println("Bye!1");
		});
		
		long startTime = System.nanoTime();
		
		inputPort.empty(inBuffer);
		
		outputPort.fill(outBuffer);
		
		while (!Thread.interrupted()) {
			if (doQueueInput.compareAndSet(true, false))
				inputPort.empty(inBuffer);
			if (doQueueOutput.compareAndSet(true, false))
				outputPort.fill(outBuffer);
			if (gSeq.get() >= NUM_FRAMES || (System.in.available() > 0 && System.in.read() == 'x'))
				break;
//			synchronized (lock) {
//				lock.wait(10);
//			}
//			System.out.println("Spinning!");
		}
		
		long endTime = System.nanoTime();
		long duration = endTime - startTime, frames = gSeq.get();
		System.out.format("Encoding %d frames to H.264 took %,dns (%f FPS)%n", frames, duration, ((double) frames) / duration * 1e9);
		System.out.println("Shutting down...");
		
		outChannel.close();
		
		System.out.println("Flushing buffers...");
		inputPort.flush();
		outputPort.flush();
		
		System.out.println("Closing ports...");
		inputPort.setEnabled(false);
		outputPort.setEnabled(false);
		
		System.out.println("Releasing buffers...");
		inputPort.releaseBuffer(inBuffer);
		outputPort.releaseBuffer(outBuffer);
		
		System.out.println("Shutting down encoder...");
		blockUntilStateChange(encoder, ComponentState.IDLE);
		blockUntilStateChange(encoder, ComponentState.LOADED);
		
		System.out.println("Releasing encoder...");
		encoder.close();
		
		System.out.println("Closing provider...");
		provider.close();
		
		System.out.println("Done");
	}
	
	public static void blockUntilStateChange(OMXComponent component, ComponentState state) throws Exception {
		System.out.println("Attempting to change state to " + state + "...");
		if (component.setState(state) == state)
			return;
		do {
			System.out.print("...");
			Thread.sleep(100);
		} while (component.getState() != state);
	}
	
	public static OMXControlDefinitionRegistry testLoadDefs() {
		String[] paths = {
				"omx.types.json",
				"omx.audio.json",
				"omx.other.json",
				"omx.ivcommon.json",
				"omx.image.json",
				"omx.video.json",
				"omx.component.json",
				"omx.example.json"
		};
		OMXControlDefinitionRegistry registry = new OMXControlDefinitionRegistry();
		
		for (String path : paths) {
			System.out.println(path);
			try (BufferedReader br = new BufferedReader(new InputStreamReader(ClassLoader.getSystemClassLoader().getResourceAsStream(path)))) {
				StringBuilder sb = new StringBuilder();
				char[] buf = new char[4096];
				while (br.ready()) {
					int len = br.read(buf);
					sb.append(buf, 0, len);
				}
				registry.read(sb.toString(), true);
			} catch (RuntimeException | IOException e) {
				e.printStackTrace();
			}
			try {
				System.out.flush();
				System.err.flush();
				Thread.sleep(100);
				System.out.flush();
				System.err.flush();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
//		System.out.println(registry);
		return registry;
	}
	
	public static void testAccess(long pointer) {
		System.out.println("Pointer: 0x" + Long.toHexString(pointer));
		
		StructPrototype proto = StructPrototype.builder()
				.addInt32("nSize")
				.addStruct(OMXQuery.VERSION_TYPE, "version")
				.build();
		
		StructPrototype adProto = StructPrototype.builder()
				.addPointer(proto, "component")
				.addPointer("callbacks")
				.build();
		
		NativeStruct data = new NativeStruct(adProto, pointer, 16);
		System.out.println("Component pointer: " + Long.toHexString(((Number)data.get("component")).longValue()));
		data.wrapChildRemote("component");
		NativeStruct component = data.getStruct("component");
		System.out.println("component struct size: " + ((Number)component.get("nSize")).intValue());
		NativeStruct componentVsn = component.getStruct("version");
		System.out.println("Version major " + componentVsn.get("nVersionMajor"));
		System.out.println("Version minor " + componentVsn.get("nVersionMinor"));
		System.out.println("Version revis " + componentVsn.get("nRevision"));
		System.out.println("Version step  " + componentVsn.get("nStep"));
		try {
			data.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public static void testQueryPorts(OMXComponent component) throws Exception {
		StructPrototype portQueryProto = StructPrototype.builder()
			.addInt32("size")
			.addStruct(OMXQuery.VERSION_TYPE, "version")
			.addInt32("length")
			.addInt32("startPortNumber")
			.build();
		try (NativeStruct query = new NativeStruct(portQueryProto)) {
			component.getConfig(false, OMXConstants.INDEX_ParamVideoInit, query.buffer());
			System.out.println("Struct size: " + query.get("size"));
			NativeStruct componentVsn = query.getStruct("version");
			System.out.println("Version major " + componentVsn.get("nVersionMajor"));
			System.out.println("Version minor " + componentVsn.get("nVersionMinor"));
			System.out.println("Version revis " + componentVsn.get("nRevision"));
			System.out.println("Version step  " + componentVsn.get("nStep"));
			System.out.println("Size: " + query.get("length"));
			System.out.println("Start: " + query.get("startPortNumber"));
		}
	}
	
	public static void printPort(ComponentPort port) {
		System.out.println("Port " + port.getIndex());
		System.out.println("\t=>MIME:\t" + port.getMIMEType());
		System.out.println("\t=>Type:\t" + port.getPortType());
		System.out.println("\t=>MinB:\t" + port.minimumBuffers());
		System.out.println("\t=>ActB:\t" + port.actualBuffers());
		System.out.println("\t=>szBf:\t" + port.bufferSize());
		System.out.println("\t=>Dir: \t" + (port.isInput() ? "INPUT" : "OUTPUT"));
		System.out.println("\t=>Enbl:\t" + (port.isEnabled() ? "ENABLED" : "DISABLED"));
	}
	
	public static void printFB(String bufName, FrameBuffer buffer) {
		System.out.println(bufName + ":");
		System.out.println("\t=>Capacity:\t" + buffer.getCapacity());
		System.out.println("\t=>" + buffer.asByteBuffer());
	}
	
	public static void testQueryPortdef(OMXComponent component) throws Exception {
		BaseOMXQueryControl inPortdefControl = OMXConstants.CTRL_PARAM_PORTDEFINITION.build(component, 200);
		
		int bufferAlignment = inPortdefControl.<Integer>getChildByName("bufferAlignment").get().call();
		System.out.println("Buffer alignment: " + bufferAlignment);
		
		inPortdefControl.access()
			.read()
			.withChild("format", "video")
				.get(System.out::println)
				.setChild(VIDEO_WIDTH, "frameWidth")
				.setChild(VIDEO_HEIGHT, "frameHeight")
				.setChild(VIDEO_FRAMERATE, "framerate")
				.setChild(ImagePalette.YUV420, "colorFormat")
				.setChild(0, "compressionFormat")
				.setChild(VIDEO_BITRATE, "bitrate")
				.setChild(MemoryUtils.align(bufferAlignment, VIDEO_WIDTH), "stride")
				.get(System.out::println)
				.and()
			.writeAndRead()
			.withChild("format", "video")
				.get(x -> System.out.println("In(act): " + x))
				.and()
			.call();
		
		BaseOMXQueryControl outPortdefControl = OMXConstants.CTRL_PARAM_PORTDEFINITION.build(component, 201);
		
		outPortdefControl.access()
			.read()
			.get(say("In(raw)"))
			.withChild("format")
				.withChild("video")
					.setChild(VIDEO_WIDTH, "frameWidth")
					.setChild(VIDEO_HEIGHT, "frameHeight")
					.setChild(VIDEO_FRAMERATE, "framerate")
					.setChild(OMXConstants.COLOR_FORMAT_UNUSED, "colorFormat")
					.setChild(ImagePalette.AVC, "compressionFormat")
					.setChild(VIDEO_BITRATE, "bitrate")
					.and()
				.and()
			.get(say("In(set)"))
			.write()
			.read()
//			.withChild("format")
//				.withChild("video")
					.get(say("In(act)"))
//					.and()
//				.and()
			.call();
		
		BaseOMXQueryControl outBitrateControl = OMXConstants.CTRL_VIDEO_PARAM_BITRATE.build(component, 201);
		
		outBitrateControl.access()
			.setChild(1, "controlRate")
			.setChild(BITRATE, "targetBitrate")
			.write()
			.call();
		
		/*try (NativeStruct query = new NativeStruct(OMXConstants.PARAM_PORTDEFINITIONTYPE)) {
			query.clear();
			query.put("nPortIndex", 200);
			NativeStruct videoDef = query.getUnion("format").getStruct("video");
			component.getConfig(false, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
			for (String key : query.keySet())
				System.out.println("\t=>" + key + ":" + query.get(key));
			videoDef.put("nFrameWidth", FRAME_WIDTH);
			videoDef.put("nFrameHeight", FRAME_HEIGHT);
			videoDef.put("xFramerate", FRAMERATE << 16);
			//videoDef.put("nBitrate", BITRATE);
			//videoDef.put("nStride", MemoryUtils.align(16, FRAME_WIDTH));
			videoDef.put("eColorFormat", 20);
			videoDef.put("eCompressionFormat", 0);
			for (String key : query.keySet())
				System.out.println("\t->" + key + ":" + query.get(key));
			component.setConfig(false, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
			
			
			System.out.println("Setting port 201");
			query.put("nPortIndex", 201);
			component.getConfig(false, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
			for (String key : query.keySet())
				System.out.println("\t@>" + key + ":" + query.get(key));
			videoDef.put("nFrameWidth", FRAME_WIDTH);
			videoDef.put("nFrameHeight", FRAME_HEIGHT);
			videoDef.put("xFramerate", FRAMERATE << 16);
			videoDef.put("eColorFormat", 0);
			videoDef.put("eCompressionFormat", OMXConstants.VIDEO_ENCODING_MJPEG);
			videoDef.put("nBitrate", BITRATE);
			component.setConfig(false, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
		}*/
		
		/*try (NativeStruct bitrateQuery = new NativeStruct(OMXConstants.PARAM_BITRATETYPE)) {
			bitrateQuery.clear();
			bitrateQuery.put("nPortIndex", 201);
			bitrateQuery.put("eControlRate", 1);
			bitrateQuery.put("nTargetBitrate", BITRATE);
			component.setConfig(false, OMXConstants.INDEX_ParamVideoBitrate, bitrateQuery.buffer());
		}*/
		
		try (NativeStruct formatQuery = new NativeStruct(OMXConstants.PARAM_VIDEO_PORTFORMATTYPE)) {
			formatQuery.clear();
			formatQuery.put("nPortIndex", 201);
			formatQuery.put("eCompressionFormat", OMXConstants.VIDEO_ENCODING_AVC);
			formatQuery.put("nIndex", 0);
			formatQuery.put("eColorFormat", 0);
			formatQuery.put("xFramerate", 0);
			component.setConfig(false, OMXConstants.INDEX_ParamVideoPortFormat, formatQuery.buffer());
		}
	}
	
	protected static void doPrint(Map<String, Object> data, String prefix) {
		for (Map.Entry<String, Object> e : data.entrySet()) {
			if (e.getValue() != null && e.getValue() instanceof Map) {
				System.out.println(prefix + "-> " + e.getKey());
				doPrint((Map<String, Object>) e.getValue(), prefix + "\t");
			} else {
				System.out.println(prefix + "-> " + e.getKey() + " = " + e.getValue());
			}
		}
	}
	
	public static <T> Consumer<T> say(String prefix) {
		return value -> {
			System.out.println(prefix);
			doPrint((Map<String, Object>) value, "\t");
		};
	}
	
	public static void testApi(ComponentPort port, OMXControlDefinitionRegistry registry) throws IllegalStateException, Exception {
		System.out.println("==========Querying available codecs===========");
		BaseOMXQueryControl formatControl = registry.makeControl((OMXComponentPort) port, "omx.video.format");
		//System.out.println(((JSONObject)OMXConstants.CTRL_VIDEO_FORMAT.toJSON()).toString(2));
		
		//Get formats
		List<Object> formats = new ArrayList<>();
		formatControl.options().get().forEachRemaining(v -> formats.add(v));
		System.out.println(formats);
		
		int j = 0;
		for (Object format : formats) {
			formatControl.access()
				.<Integer>withChild("index")
					.set(j++)
					.and()
				.read()
				.withChild("framerate")
					.get(v->System.out.println("Framerate: " + v))
					.and()
				.withChild("compression")
					.get(v->System.out.println("Compression: " + v))
					.and()
				.withChild("color")
					.get(v->System.out.println("Color: " + v))
					.and()
				.call();
		}
	}
}
