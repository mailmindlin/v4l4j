/*
* Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
package au.edu.jcu.v4l4j.examples.server;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Map;
import java.util.StringTokenizer;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.ControlList;
import au.edu.jcu.v4l4j.JPEGFrameGrabber;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.ControlException;

public class ClientConnection{
	private static String mjpegHeader = "HTTP/1.0 200 OK\r\n"
		+ "Expires: 0\r\n"
		+ "Pragma: no-cache\r\n"
		+ "Cache-Control: no-cache\r\n"
		+ "Content-Type: multipart/x-mixed-replace;boundary=\"boundary\"\r\n\r\n";
	
	private static String mjpegFrameheader = "--boundary\r\n"
		+ "Content-Type: image/jpeg\r\n"
		+ "Content-Length: ";

	private static String mainPageHTML = "HTTP/1.0 200 OK\r\n"
		+ "Location: http://v4l4j_mini_server\r\n"
		+ "Expires: 0\r\n"
		+ "Content-Type: text/html\r\n"
		+ "\r\n"
			+ "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\" http://www.w3.org/TR/html4/frameset.dtd>"
			+ "<html>\n"
				+ "<head>\n"
					+ "<title>v4l4j mini server</title>\n"
				+ "</head>\n"
				+ "<frameset cols='4*,6*'>\n"
					+ "<frame src='control' name='control list'>\n"
					+ "<frame src='webcam' name='video stream'>\n"
				+ "</frameset>\n"
			+ "</html>";

	private static String webcamPageHTML= "HTTP/1.0 200 OK\r\n"
		+ "Location: http://v4l4j_mini_server\r\n"
		+ "Expires: 0\r\n"
		+ "Content-Type: text/html\r\n\r\n" +
	"<html>\n" +
	"<body>\n" +
	"<table>\n" +
	"<td>\n" +
	"<tr>\n" +
	"<img src=\"stream.jpg\">\n" +
	"</tr>\n" +
	"</td>\n" +
	"</table>\n" +
	"</body>\n" +
	"</html>";

	private static String controlPageHTMLHeader = "HTTP/1.0 200 OK\r\nLocation: http://v4l4j_mini_server\r\nExpires: 0\r\nContent-Type: text/html\r\n\r\n" +
	"<html>\n" +
	"<body>\n" +
	"<table>\n";

	private static String controlPageHTMLFooter=  "</table>\n" +
	"</body>\n" +
	"</html>\n";


	private Socket 				clientSocket;
	private BufferedReader 		inStream;
	private DataOutputStream 	outStream;


	/**
	 * Builds an object handling a tcp connection to one client. Sends the MJPEG
	 * header straight away
	 * @param client the client who just connected to us
	 * @param in the input stream
	 * @param out the ouput stream
	 * @throws IOException if there is an error get in/output streams
	 */
	public ClientConnection(Socket client, BufferedReader in, DataOutputStream out) throws IOException{
		if ((client == null) || (in == null) || (out == null))
			throw new NullPointerException("client, in and out cannot be null");

		clientSocket = client;
		inStream = in;
		outStream = out;

		// send mjpeg header
		outStream.writeBytes(mjpegHeader);
	}

	/**
	 * Close the input and output streams and closes the socket
	 */
	public void stop() {
		try {
			System.out.println("Disconnected from " + clientSocket.getInetAddress().getHostAddress() + ":" + clientSocket.getPort());
			
			inStream.close();
			outStream.close();
			clientSocket.close();
		} catch (IOException e) {
			// error closing connection with client
			e.printStackTrace();
		}
	}

	/**
	 * Send the given frame in an mjpeg frame header
	 * @param frame the frame to be send
	 * @throws IOException if there is an error writing over the socket
	 */
	public void sendNextFrame(VideoFrame frame) throws IOException{
		outStream.writeBytes(mjpegFrameheader + Integer.toString(frame.getFrameLength()) + "\r\n\r\n");
		outStream.write(frame.getBytes(), 0, frame.getFrameLength());
	}	

	/**
	 * Send the main page in html over the given output stream
	 * @param out the output stream
	 * @throws IOException if the stream is closed
	 */
	public static void sendMainPage(DataOutputStream out) throws IOException {
		out.writeBytes(mainPageHTML);
	}


	public static void sendWebcamPage(DataOutputStream out) throws IOException {
		out.writeBytes(webcamPageHTML);
	}

	/**
	 * Send a basic HTML table containing a form per control allowing the
	 * user to view the current value and it update it.
	 * @param ctrlList the list of control for which the HTML form should be created
	 * @param out the output stream
	 * @throws IOException  if there is an error writing out the stream
	 */
	public static void sendControlListPage(ControlList ctrlList, int jpegQuality, DataOutputStream out) throws IOException {
		out.writeBytes(controlPageHTMLHeader);


		// add a fake control to adjust the jpeg quality
		out.writeBytes("<tr>\n");
		out.writeBytes("<td>JPEG Quality</td>\n");
		out.writeBytes("<td><form action=\"update\">\n");
		out.writeBytes("<input type=\"hidden\" name=\"id\" value=\"-1\">\n");
		writeSliderControl(out, jpegQuality, 0, 100, 1);
		out.writeBytes("</form></td></tr>\n");

		// for each control, create an entry in the table
		for(Control control : ctrlList.getList()) {
			out.writeBytes("<form action=\"update\"><tr>\n");
			out.writeBytes("<td>"+control.getName()+"</td>\n");
			out.writeBytes("<td>\n");
			out.writeBytes("<input type=\"hidden\" name=\"id\" value=\"" + ctrlList.getList().indexOf(control) + "\">\n");

			// Select the best HTML element to represent the control based
			// on its type
			switch (control.getType()) {
				case V4L4JConstants.CTRL_TYPE_BUTTON:
					out.writeBytes("<input type=\"hidden\" name=\"val\" value=\"0\">");
					out.writeBytes("</td>\n<td>\n<input type=\"submit\" name=\"Activate\" value=\"Activate\">");
					break;

				case V4L4JConstants.CTRL_TYPE_SLIDER:
					writeSliderControl(out, control);
					break;

				case V4L4JConstants.CTRL_TYPE_DISCRETE:
					writeDiscreteControl(out, control);
					break;
				case V4L4JConstants.CTRL_TYPE_SWITCH:
					out.writeBytes("<input type='checkbox' name='val' value='");
					try {
						if(control.getValue() == 1)
							out.writeBytes("0' checked='checked'>\n");
						else
							out.writeBytes("1'>\n");
					} catch (Exception e) {}
					out.writeBytes("</td>\n<td data-ctrl-type='switch'><input type='submit' name='set' value='set'>");
					break;
				default:
					System.out.println("Unknown control type: " + control.getType());

			}

			out.writeBytes("</td>\n");
			out.writeBytes("</tr></form>\n");			
		}

		out.writeBytes(controlPageHTMLFooter);
	}

	/**
	 * Parses the given http line, expecting to find something along the lines of
	 * <code>GET /control?id=ID&val=VAL&submit=set HTTP/1.1</code> where ID and 
	 * VAL are integers.
	 * @param ctrlList the control list
	 * @param httpLine the http line to be parsed
	 * @throws ControlException if there is an error setting the new value
	 */
	public static void updateControlValue(ControlList ctrlList, JPEGFrameGrabber fg, String httpLine) throws ControlException {
		boolean	hasValue = false;
		boolean	hasID = false;
		int 	controlID = 0;
		int 	value = 0;

		// parse the http line to find out the control index and
		// its new value. Expected line:
		// "GET /control?id=ID&val=VAL&submit=set HTTP/1.1"
		StringTokenizer tokens = new StringTokenizer(httpLine, "?=&", false);

		while(tokens.hasMoreTokens()){
			String next = tokens.nextToken();

			if ((next.equalsIgnoreCase("id")) && tokens.hasMoreTokens())
			{
				try {
					controlID = Integer.parseInt(tokens.nextToken());
					hasID = true;
				} catch (NumberFormatException e) {
					// control id is not a number, ignore 
				}
			} else if ((next.equalsIgnoreCase("val")) && tokens.hasMoreTokens())
			{
				try {
					value = Integer.parseInt(tokens.nextToken());
					hasValue = true;
				} catch (NumberFormatException e) {
					// control value is not a number, ignore
				}
			}
		}

		// HTML checkboxes dont return a value if unchecked, which means
		// hasValue is false in this case. Check if ID is of type SWICTH
		// and if hasValues == false, in which case, set it to true,
		// and use default value of 0
		if (hasID && !hasValue && (ctrlList.getList().get(controlID).getType() == V4L4JConstants.CTRL_TYPE_SWITCH)) {
			hasValue = true;
			value = 0;
		}

		// Set new value
		if (hasValue && hasID) {
			// catch the jpeg quality control which is not a real control
			if (controlID == -1)
				fg.setJPGQuality(value);
			else
				ctrlList.getList().get(controlID).setValue(value);
		}
	}
	protected void writeDiscreteControl(DataOutputStream out, Control control) throws IOException {
		out.writeBytes("<select name='val' size='1'>");
		Map<String, Integer> valueMap = control.getDiscreteValuesMap();
		for(Map.Entry<String, Integer> entry: valueMap.entrySet()) {
			out.writeBytes("<option value='" + entry.getValue() + "'");
			try {
				if (control.getValue() == entry.getValue().intValue())
					out.writeBytes(" selected='selected'");
			} catch (Exception e) {}
			out.writeBytes(" >");
			out.writeBytes(entry.getKey());
			out.writeBytes("</option>");
		}
		out.writeBytes("</select>\n");
		out.writeBytes("</td>\n<td data-ctrl-type='discrete'><input type='submit' name='set' value='set'>");
	}
	protected void writeSliderControl(DataOutputStream out, Control control) throws IOException {
		int min, max, step, value;
		try {
			min = control.getMinValue();
			max = control.getMaxValue();
			value = control.getValue();
			step = control.getStepValue();
		} catch (Exception e) {
			e.printStackTrace();
		}
		writeSliderControl(out, value, min, max, step);
	}
	protected void writeSliderControl(DataOutputStream out, int value, int min, int max, int step) throws IOException {
		out.writeBytes("<input type='number' name='val' size='10' maxlength='10' value='");
		out.writeBytes("" + value + "' min='" + min + "' max='" + max + "' step='" + step + "'/>");
		out.writeBytes("\n</td>\n<td data-ctrl-type='slider'>\n<input type='submit' name='set' value='set'/>");
	}
}
