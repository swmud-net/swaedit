package pl.swmud.ns.swaedit.core;

import java.io.IOException;
import java.io.Writer;

import com.sun.xml.internal.bind.marshaller.CharacterEscapeHandler;

@SuppressWarnings("restriction")
public class XmlEscapeHandler implements CharacterEscapeHandler {

	@Override
	public void escape(char[] ch, int start, int length, boolean isAttrVal, Writer out) throws IOException {
		String s = new String(ch, start, length);
		s = s.replace("&", "&amp;");
		s = s.replace("\"", "&quot;");
		s = s.replace("'", "&apos;");
		s = s.replace(">", "&gt;");
		s = s.replace("<", "&lt;");
		s = s.replace("\r", "&#13;");
		out.write(s);
	}
}
