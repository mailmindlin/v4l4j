package au.edu.jcu.v4l4j.test;

import java.nio.file.Paths;
import java.util.Set;

import au.edu.jcu.v4l4j.api.FrameBuffer;
import au.edu.jcu.v4l4j.api.ImagePalette;
import au.edu.jcu.v4l4j.api.VideoCompressionType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;
import au.edu.jcu.v4l4j.api.component.ComponentState;
import au.edu.jcu.v4l4j.api.component.port.VideoPort;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.PrimitiveStructFieldType;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;
import au.edu.jcu.v4l4j.impl.omx.BaseOMXQueryControl;
import au.edu.jcu.v4l4j.impl.omx.EnumChildOMXQueryControl;
import au.edu.jcu.v4l4j.impl.omx.NumberOMXQueryControl;
import au.edu.jcu.v4l4j.impl.omx.OMXComponent;
import au.edu.jcu.v4l4j.impl.omx.OMXComponentProvider;
import au.edu.jcu.v4l4j.impl.omx.OMXConstants;
import au.edu.jcu.v4l4j.impl.omx.OMXQuery;
import au.edu.jcu.v4l4j.impl.omx.OMXVideoPort;

public class OMXTest {
	public static void main(String...fred) throws Exception {
		for (PrimitiveStructFieldType type : PrimitiveStructFieldType.values())
			System.out.println(type.name() + ";" + type.getAlignment() + ";" + type.getSize());
		OMXComponentProvider provider = OMXComponentProvider.getInstance();
		for (String name : provider.availableNames(Paths.get("/"), null))
			System.out.println(name);
		OMXComponent encoder = (OMXComponent) provider.get("OMX.broadcom.video_encode");
		System.out.println("Encoder name: " + encoder.getName());
		System.out.println("Encoder state: " + encoder.getState());
		
		testAccess(encoder.getPointer());
		
		testQueryPorts(encoder);
		
		Set<ComponentPort> ports = encoder.getPorts();
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
		
		testQueryPortdef(encoder);
		
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
		

		FrameBuffer inBuffer = inputPort.allocateBuffer(inputPort.bufferSize());
		FrameBuffer outBuffer = outputPort.allocateBuffer(outputPort.bufferSize());
		Thread.sleep(200);
		
		
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
			component.accessConfig(false, true, OMXConstants.INDEX_ParamVideoInit, query.buffer());
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
			component.accessConfig(false, true, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
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
			component.accessConfig(false, false, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
			
			
			System.out.println("Setting port 201");
			query.put("nPortIndex", 201);
			component.accessConfig(false, true, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
			for (String key : query.keySet())
				System.out.println("\t@>" + key + ":" + query.get(key));
			videoDef.put("nFrameWidth", FRAME_WIDTH);
			videoDef.put("nFrameHeight", FRAME_HEIGHT);
			videoDef.put("xFramerate", FRAMERATE << 16);
			videoDef.put("eColorFormat", 0);
			videoDef.put("eCompressionFormat", OMXConstants.VIDEO_ENCODING_AVC);
			videoDef.put("nBitrate", BITRATE);
			component.accessConfig(false, false, OMXConstants.INDEX_ParamPortDefinition, query.buffer());
		}
		
		try (NativeStruct bitrateQuery = new NativeStruct(OMXConstants.PARAM_BITRATETYPE)) {
			bitrateQuery.clear();
			bitrateQuery.put("nPortIndex", 201);
			bitrateQuery.put("eControlRate", 1);
			bitrateQuery.put("nTargetBitrate", BITRATE);
			component.accessConfig(false, false, OMXConstants.INDEX_ParamVideoBitrate, bitrateQuery.buffer());
		}
		try (NativeStruct formatQuery = new NativeStruct(OMXConstants.PARAM_PORTFORMATTYPE)) {
			formatQuery.clear();
			formatQuery.put("nPortIndex", 201);
			formatQuery.put("eCompressionFormat", OMXConstants.VIDEO_ENCODING_AVC);
			formatQuery.put("nIndex", 0);
			formatQuery.put("eColorFormat", 0);
			formatQuery.put("xFramerate", 0);
			component.accessConfig(false, false, OMXConstants.INDEX_ParamVideoPortFormat, formatQuery.buffer());
		}
	}
	
	@SuppressWarnings("unchecked")
	public static void testApi(ComponentPort port) {
		BaseOMXQueryControl formatControl = new BaseOMXQueryControl((OMXComponent) port.getComponent(), "format", OMXConstants.INDEX_ParamVideoPortFormat, OMXConstants.PARAM_PORTFORMATTYPE);
		Set<Control<?>> children = ((Set<Control<?>>)formatControl.getChildren());
		children.add(new NumberOMXQueryControl(formatControl, "nPortIndex"));
		children.add(new NumberOMXQueryControl(formatControl, "nIndex"));
		children.add(new NumberOMXQueryControl(formatControl, "xFramerate"));
		children.add(new EnumChildOMXQueryControl<VideoCompressionType>(formatControl, "compression", VideoCompressionType.class, "eCompressionFormat"));
		children.add(new EnumChildOMXQueryControl<ImagePalette>(formatControl, "color", ImagePalette.class, "eColorFormat"));
		formatControl.access();
	}
}
