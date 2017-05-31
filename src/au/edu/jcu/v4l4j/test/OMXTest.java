package au.edu.jcu.v4l4j.test;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.ComponentState;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.PrimitiveStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;
import au.edu.jcu.v4l4j.impl.omx.BaseOMXQueryControl;
import au.edu.jcu.v4l4j.impl.omx.OMXComponent;
import au.edu.jcu.v4l4j.impl.omx.OMXComponentProvider;
import au.edu.jcu.v4l4j.impl.omx.OMXConstants;
import au.edu.jcu.v4l4j.impl.omx.OMXControlDefinition.OMXControlDefinitionRegistry;
import au.edu.jcu.v4l4j.impl.omx.OMXQuery;
import au.edu.jcu.v4l4j.impl.omx.OMXVideoPort;

public class OMXTest {
	public static void main(String...fred) throws Exception {
		for (PrimitiveStructFieldType<?> type : PrimitiveStructFieldType.values())
			System.out.println(type.name() + ";" + type.getAlignment() + ";" + type.getSize());
		
		testLoadDefs();
		
		OMXComponentProvider provider = OMXComponentProvider.getInstance();
		for (String name : provider.availableNames(Paths.get("/"), null))
			System.out.println(name);
		OMXComponent encoder = (OMXComponent) provider.get("OMX.broadcom.video_encode");
		System.out.println("Encoder name: " + encoder.getName());
		System.out.println("Encoder state: " + encoder.getState());
		
		testAccess(encoder.getPointer());
		
		testQueryPorts(encoder);
		
		Set<? extends ComponentPort> ports = encoder.getPorts();
		System.out.println("Ports: " + ports);
		
		for (ComponentPort port : ports) {
			System.out.println("Port " + port.getIndex());
			System.out.println("\t=>MIME:\t" + port.getMIMEType());
			System.out.println("\t=>Type:\t" + port.getPortType());
			System.out.println("\t=>MinB:\t" + port.minimumBuffers());
			System.out.println("\t=>ActB:\t" + port.actualBuffers());
			System.out.println("\t=>szBf:\t" + port.bufferSize());
			System.out.println("\t=>Dir: \t" + (port.isInput() ? "INPUT" : "OUTPUT"));
			System.out.println("\t=>Enbl:\t" + (port.isEnabled() ? "ENABLED" : "DISABLED"));
		}

		VideoPort inputPort = (VideoPort) encoder.getPort(200);
		VideoPort outputPort = (VideoPort) encoder.getPort(201);
		
		inputPort.setEnabled(false);
		outputPort.setEnabled(false);
		

		OMXTest.testApi(outputPort);
		//testQueryPortdef(encoder);
		
		System.out.println("Transitioned to state " + encoder.setState(ComponentState.IDLE));
		
		((OMXVideoPort)inputPort).pull();
		((OMXVideoPort)outputPort).pull();
		
		inputPort.setEnabled(true);
		Thread.sleep(200);
		((OMXVideoPort)inputPort).pull();
		System.out.println(inputPort.isEnabled());
		outputPort.setEnabled(true);
		Thread.sleep(200);
		((OMXVideoPort)outputPort).pull();
		System.out.println(outputPort.isEnabled());
		

		ByteBuffer buffer = ByteBuffer.allocateDirect(inputPort.bufferSize());
		FrameBuffer inBuffer = inputPort.useBuffer(buffer);
//		FrameBuffer inBuffer = inputPort.allocateBuffer(inputPort.bufferSize());
		FrameBuffer outBuffer = outputPort.allocateBuffer(outputPort.bufferSize());
		Thread.sleep(200);
		
		System.out.println("IBsz" + inBuffer.getCapacity());
		System.out.println(inBuffer.asByteBuffer());
		System.out.println("OBsz" + outBuffer.getCapacity());
		System.out.println(outBuffer.asByteBuffer().position());
		
		encoder.setState(ComponentState.EXECUTING);
		
		Thread.sleep(200);
		
		//outputPort.fill(outBuffer);
		
		System.out.println("Putting random stuff in buffer");
		ByteBuffer image = ByteBuffer.allocate(inBuffer.getCapacity());
		for (int i = 0; i < inBuffer.getCapacity();i+=4)
			image.putInt(0xFF00FF00);
		image.flip();
		
		inBuffer.asByteBuffer().put(image.duplicate()).flip();
		
		
		AtomicInteger gSeq = new AtomicInteger(0);
		AtomicBoolean doQueueInput = new AtomicBoolean(false);
		AtomicBoolean doQueueOutput = new AtomicBoolean(false);
		Object lock = new Object();
		
		
		File outFile = new File("out.h264").getAbsoluteFile();
		System.out.println("Writing out to " + outFile);
		final FileChannel outChannel = new FileOutputStream(outFile, false).getChannel();

		outputPort.onBufferFill(frame->{
			System.out.println("Buffer fill called in Java");
			System.out.println("" + frame.asByteBuffer().remaining() + " bytes in buffer");
			System.out.println("Seq " + frame.getSequenceNumber());
			System.out.println("Time " + frame.getTimestamp());
			synchronized (outChannel) {
				try {
					outChannel.write(frame.asByteBuffer());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			doQueueOutput.set(true);
			synchronized (lock) {
				lock.notifyAll();
			}
		});
		
		inputPort.onBufferEmpty(frame->{
			System.out.println("Buffer empty called in Java");
			int seq = gSeq.incrementAndGet();
			if (seq > 25)
				return;
			System.out.println("Writing frame " + seq);
			ByteBuffer buf = frame.asByteBuffer();
			buf.clear();
			buf.put(image.duplicate());
			buf.flip();
			frame.setSequenceNumber(seq);
			frame.setTimestamp(System.nanoTime());
			doQueueInput.set(true);
			synchronized (lock) {
				lock.notifyAll();
			}
		});
		
		inputPort.empty(inBuffer);
		
		outputPort.fill(outBuffer);
		
		while (!Thread.interrupted()) {
			if (doQueueInput.compareAndSet(true, false))
				inputPort.empty(inBuffer);
			if (doQueueOutput.compareAndSet(true, false))
				outputPort.fill(outBuffer);
			if (gSeq.get() > 25)
				break;
			synchronized (lock) {
				lock.wait(50);
			}
		}
		outChannel.close();
		System.out.println("Done");
	}
	
	public static void testLoadDefs() {
		String[] paths = {
				"omx.component.json",
				"omx.types.json",
				"omx.other.json",
				"omx.ivcommon.json"
		};
		OMXControlDefinitionRegistry registry = new OMXControlDefinitionRegistry();
		
		for (String path : paths) {
			try (BufferedReader br = new BufferedReader(new InputStreamReader(ClassLoader.getSystemClassLoader().getResourceAsStream(path)))) {
				StringBuilder sb = new StringBuilder();
				char[] buf = new char[4096];
				while (br.ready()) {
					int len = br.read(buf);
					sb.append(buf, 0, len);
				}
				registry.read(sb.toString(), true);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		System.out.println(registry);
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
	
	public static void testQueryPortdef(OMXComponent component) throws Exception {
		final int FRAME_WIDTH = 1920/4;
		final int FRAME_HEIGHT = 1080/4;
		final int FRAMERATE = 25;
		final int BITRATE = 10000000;
		try (NativeStruct query = new NativeStruct(OMXConstants.PARAM_PORTDEFINITIONTYPE)) {
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
		}
		
		try (NativeStruct bitrateQuery = new NativeStruct(OMXConstants.PARAM_BITRATETYPE)) {
			bitrateQuery.clear();
			bitrateQuery.put("nPortIndex", 201);
			bitrateQuery.put("eControlRate", 1);
			bitrateQuery.put("nTargetBitrate", BITRATE);
			component.setConfig(false, OMXConstants.INDEX_ParamVideoBitrate, bitrateQuery.buffer());
		}
		try (NativeStruct formatQuery = new NativeStruct(OMXConstants.PARAM_VIDEO_PORTFORMATTYPE)) {
			formatQuery.clear();
			formatQuery.put("nPortIndex", 201);
			formatQuery.put("eCompressionFormat", OMXConstants.VIDEO_ENCODING_MJPEG);
			formatQuery.put("nIndex", 0);
			formatQuery.put("eColorFormat", 0);
			formatQuery.put("xFramerate", 0);
			component.setConfig(false, OMXConstants.INDEX_ParamVideoPortFormat, formatQuery.buffer());
		}
	}
	
	public static void testApi(ComponentPort port) throws IllegalStateException, Exception {
		/*BaseOMXQueryControl formatControl = new BaseOMXQueryControl((OMXComponent) port.getComponent(), "format", OMXConstants.INDEX_ParamVideoPortFormat, port.getIndex(), OMXConstants.PARAM_VIDEO_PORTFORMATTYPE, null);
		formatControl.registerChild(new NumberOMXQueryControl(formatControl, port.getIndex(), "index", "nIndex", null));
		formatControl.registerChild(new NumberOMXQueryControl(formatControl, port.getIndex(), "framerate", "xFramerate", null));
		formatControl.registerChild(new EnumChildOMXQueryControl<VideoCompressionType>(formatControl, port.getIndex(), "compression", VideoCompressionType.class, "eCompressionFormat"));
		formatControl.registerChild(new EnumChildOMXQueryControl<ImagePalette>(formatControl, port.getIndex(), "color", ImagePalette.class, "eColorFormat"));*/
		BaseOMXQueryControl formatControl = OMXConstants.CTRL_VIDEO_FORMAT.build((OMXComponent) port.getComponent(), port.getIndex());
		Iterator<Map<String, Object>> i = formatControl.options().get();
		List<Object> formats = new ArrayList<>();
		while (i.hasNext())
			formats.add(i.next().get("compression"));
		System.out.println(formats);
		formatControl.access()
			.<Integer>withChild("index")
				.set(0)
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
