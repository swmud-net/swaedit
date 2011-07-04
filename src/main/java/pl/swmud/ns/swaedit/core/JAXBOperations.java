package pl.swmud.ns.swaedit.core;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.StringReader;

import javax.xml.XMLConstants;
import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import pl.swmud.ns.swaedit.exits.Exits;
import pl.swmud.ns.swaedit.flags.Flags;
import pl.swmud.ns.swaedit.gui.SWAEdit;
import pl.swmud.ns.swaedit.highlighter.Highlighter;
import pl.swmud.ns.swaedit.itemtypes.Itemtypes;
import pl.swmud.ns.swaedit.lastupdate.Lastupdate;
import pl.swmud.ns.swaedit.names.Names;
import pl.swmud.ns.swaedit.resets.Resets;
import pl.swmud.ns.swaedit.types.Types;
import pl.swmud.ns.swaedit.usmessages.Messages;
import pl.swmud.ns.swaedit.usprotocol.Usprotocol;
import pl.swmud.ns.swmud._1_0.area.Area;

import com.sun.xml.internal.bind.marshaller.CharacterEscapeHandler;
import com.trolltech.qt.core.QByteArray;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QTextCodec;

@SuppressWarnings("restriction")
public final class JAXBOperations {

	private static final long MAX_FILE_SIZE = 8388608; /* 8M */

	/* deletes &#13; entities (caret return) & manages encoding */
	private static InputSource filter(String path) {
		InputSource result = null;
		try {
			QByteArray buf = null;
			QFile file = new QFile(path);
			if (file.open(QFile.OpenModeFlag.ReadOnly)) {
				if (file.size() > MAX_FILE_SIZE) {
					file.close();
					throw new IOException("swaedit supports areas up to " + MAX_FILE_SIZE + "B large");
					/*
					 * it is stupid to use IOException here, find and use a
					 * better one
					 */
				}
				buf = file.readAll();
				file.close();
			} else {
				throw new IOException("cannot open file: " + path);
			}
			String str = QTextCodec.codecForName(SWAEdit.FILE_ENCODING).toUnicode(buf);
			result = new InputSource(new StringReader(str));
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return result;
	}

	/* unmarshalling Area */
	public static Area unmarshallArea(String xmlPath, String xsdPath) {
		Area area = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Area.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			area = (Area) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return area;
	}

	public static Area unmarshallArea(String xmlPath) {
		return unmarshallArea(xmlPath, "schemas/area.xsd");
	}

	/* marshalling Area */
	public static void marshall(Area area, String xmlPath, String xsdPath) {
		try {
			JAXBContext jc = JAXBContext.newInstance(Area.class);
			Marshaller m = jc.createMarshaller();
			m.setSchema(SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI).newSchema(new File(xsdPath)));
			m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
			m.setProperty(Marshaller.JAXB_ENCODING, SWAEdit.FILE_ENCODING);
			m.setProperty(CharacterEscapeHandler.class.getName(), new XmlEscapeHandler());
			m.marshal(area, new FileOutputStream(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public static void marshall(Area area, String xmlPath) {
		marshall(area, xmlPath, "schemas/area.xsd");
	}

	/* unmarshalling Itemtypes */
	public static Itemtypes unmarshallItemtypes(String xmlPath, String xsdPath) {
		Itemtypes itemtypes = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Itemtypes.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			itemtypes = (Itemtypes) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return itemtypes;
	}

	public static Itemtypes unmarshallItemtypes(String xmlPath) {
		return unmarshallItemtypes(xmlPath, "schemas/itemtypes.xsd");
	}

	/* marshalling Itemtypes (currently no need for that) */
	public static void marshall(Itemtypes itemtypes, String xmlPath, String xsdPath) {
		try {
			JAXBContext jc = JAXBContext.newInstance(Itemtypes.class);
			Marshaller m = jc.createMarshaller();
			m.setSchema(SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI).newSchema(new File(xsdPath)));
			m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
			m.setProperty(Marshaller.JAXB_ENCODING, SWAEdit.FILE_ENCODING);
			m.marshal(itemtypes, new FileOutputStream(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public static void marshall(Itemtypes itemtypes, String xmlPath) {
		marshall(itemtypes, xmlPath, "itemtypes.xsd");
	}

	/* unmarshalling Flags */
	public static Flags unmarshallFlags(String xmlPath, String xsdPath) {
		Flags flags = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Flags.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			flags = (Flags) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return flags;
	}

	public static Flags unmarshallFlags(String xmlPath) {
		return unmarshallFlags(xmlPath, "schemas/flags.xsd");
	}

	/* unmarshalling Names */
	@SuppressWarnings("unchecked")
	public static Names unmarshallNames(String xmlPath, String xsdPath) {
		Names names = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Names.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			names = (Names) ((JAXBElement<Names>) u.unmarshal(filter(xmlPath))).getValue();
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return names;
	}

	public static Names unmarshallNames(String xmlPath) {
		return unmarshallNames(xmlPath, "schemas/names.xsd");
	}

	/* unmarshalling highlighting words */
	@SuppressWarnings("unchecked")
	public static Highlighter unmarshallHighlighter(String xmlPath, String xsdPath) {
		Highlighter highlighter = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Highlighter.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			highlighter = (Highlighter) ((JAXBElement<Highlighter>) u.unmarshal(new File(xmlPath))).getValue();
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return highlighter;
	}

	public static Highlighter unmarshallHighlighter(String xmlPath) {
		return unmarshallHighlighter(xmlPath, "schemas/highlighter.xsd");
	}

	/* unmarshalling Exits */
	public static Exits unmarshallExits(String xmlPath, String xsdPath) {
		Exits exits = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Exits.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			exits = (Exits) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return exits;
	}

	public static Exits unmarshallExits(String xmlPath) {
		return unmarshallExits(xmlPath, "schemas/exits.xsd");
	}

	/* unmarshalling Types */
	public static Types unmarshallTypes(String xmlPath, String xsdPath) {
		Types types = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Types.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			types = (Types) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return types;
	}

	public static Types unmarshallTypes(String xmlPath) {
		return unmarshallTypes(xmlPath, "schemas/types.xsd");
	}

	/* unmarshalling Types */
	public static pl.swmud.ns.swaedit.stringtypes.Types unmarshallStringTypes(String xmlPath, String xsdPath) {
		pl.swmud.ns.swaedit.stringtypes.Types types = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(pl.swmud.ns.swaedit.stringtypes.Types.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			types = (pl.swmud.ns.swaedit.stringtypes.Types) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return types;
	}

	public static pl.swmud.ns.swaedit.stringtypes.Types unmarshallStringTypes(String xmlPath) {
		return unmarshallStringTypes(xmlPath, "schemas/stringtypes.xsd");
	}

	/* unmarshalling Resets */
	public static Resets unmarshallResets(String xmlPath, String xsdPath) {
		Resets resets = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Resets.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			resets = (Resets) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return resets;
	}

	public static Resets unmarshallResets(String xmlPath) {
		return unmarshallResets(xmlPath, "schemas/resets.xsd");
	}

	/* unmarshalling Lastupdate */
	public static Lastupdate unmarshallLastUpdate(String xmlPath, String xsdPath) {
		Lastupdate lastUpdate = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Lastupdate.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			lastUpdate = (Lastupdate) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return lastUpdate;
	}

	public static Lastupdate unmarshallLastUpdate(String xmlPath) {
		return unmarshallLastUpdate(xmlPath, "schemas/lastupdate.xsd");
	}

	/* marshalling Lastupdate */
	public static void marshall(Lastupdate lastUpdate, String xmlPath, String xsdPath) {
		try {
			JAXBContext jc = JAXBContext.newInstance(Lastupdate.class);
			Marshaller m = jc.createMarshaller();
			m.setSchema(SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI).newSchema(new File(xsdPath)));
			m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
			m.setProperty(Marshaller.JAXB_ENCODING, SWAEdit.FILE_ENCODING);
			m.marshal(lastUpdate, new FileOutputStream(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public static void marshall(Lastupdate lastUpdate, String xmlPath) {
		marshall(lastUpdate, xmlPath, "schemas/lastupdate.xsd");
	}

	/* unmarshalling Usprotocol */
	@SuppressWarnings("unchecked")
	public static Usprotocol unmarshallUSProtocol(String xmlString, String xsdPath) {
		Usprotocol usprotocol = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Usprotocol.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			usprotocol = (Usprotocol) ((JAXBElement<Usprotocol>) u.unmarshal(new StringReader(xmlString))).getValue();
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return usprotocol;
	}

	public static Usprotocol unmarshallUSProtocol(String xmlString) {
		return unmarshallUSProtocol(xmlString, "schemas/usprotocol.xsd");
	}

	/* unmarshalling Lastupdate */
	public static Messages unmarshallMessages(String xmlPath, String xsdPath) {
		Messages messages = null;
		try {
			JAXBContext jc = JAXBContext.newInstance(Messages.class);
			Unmarshaller u = jc.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = sf.newSchema(new File(xsdPath));
			u.setSchema(schema);
			messages = (Messages) u.unmarshal(filter(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
		return messages;
	}

	public static Messages unmarshallMessages(String xmlPath) {
		return unmarshallMessages(xmlPath, "schemas/usmessages.xsd");
	}

	/* marshalling Lastupdate */
	public static void marshall(Messages messages, String xmlPath, String xsdPath) {
		try {
			JAXBContext jc = JAXBContext.newInstance(Messages.class);
			Marshaller m = jc.createMarshaller();
			m.setSchema(SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI).newSchema(new File(xsdPath)));
			m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
			m.setProperty(Marshaller.JAXB_ENCODING, SWAEdit.FILE_ENCODING);
			m.marshal(messages, new FileOutputStream(xmlPath));
		} catch (JAXBException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}
}
