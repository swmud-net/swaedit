package pl.swmud.ns.swaedit.core;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.StringWriter;
import java.math.BigInteger;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.validation.SchemaFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import com.trolltech.qt.core.QPoint;
import com.trolltech.qt.gui.QApplication;

import pl.swmud.ns.swaedit.gui.BalloonWidget;
import pl.swmud.ns.swaedit.gui.MessagesWidget;
import pl.swmud.ns.swaedit.lastupdate.Lastupdate;
import pl.swmud.ns.swaedit.usmessages.Message;
import pl.swmud.ns.swaedit.usmessages.Messages;
import pl.swmud.ns.swaedit.usmessages.ObjectFactory;
import pl.swmud.ns.swaedit.usprotocol.Usprotocol;

public class FileServer extends Thread {

    private final String ADDRESS = "swmud.pl";
    private final int PORT = 4011;
    private final int BUF_LEN = 1024;
    private ServerSocket server;
    private BalloonWidget msgBalloon;
    private BalloonWidget dlBalloon;
    private Lastupdate lastUpdate;
    private long bytes;

    public FileServer(final QPoint pos) {
        msgBalloon = new BalloonWidget(new QPoint(pos.x(),pos.y()-70));
        dlBalloon = new BalloonWidget(pos);
        msgBalloon.moveToThread(QApplication.instance().thread());
        dlBalloon.moveToThread(QApplication.instance().thread());
        lastUpdate = JAXBOperations.unmarshallLastUpdate("data/lastupdate.xml");
        try {
            server = new ServerSocket(8011);
            start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void run() {
        Socket sock;
        int b;
        try {
            sock = new Socket(ADDRESS,PORT);
            BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            bw.write(createRequest());
            bw.flush();
            
            StringWriter sw = new StringWriter();
            InputStream is = sock.getInputStream();
            while( (b = is.read()) > 0 ) {
                sw.write((char)b);
            }

            Usprotocol usprotocol = JAXBOperations.unmarshallUSProtocol(sw.toString());
            appendMessages(usprotocol);
            
            long progress = 0;
            for (pl.swmud.ns.swaedit.usprotocol.File file : usprotocol.getResponse().getFiles().getFile()) {
                progress += file.getLength().intValue();
            }

            if (progress > 0) {
                setProgress(progress);   
            }

            while ((sock = server.accept()) != null) {
                is = sock.getInputStream();
                for (pl.swmud.ns.swaedit.usprotocol.File file : usprotocol.getResponse().getFiles().getFile()) {
                    retrFile(file,is,progress);
                }
                is.close();
                break;
            }
            sock.close();
            server.close();
            lastUpdate.setTimestamp(BigInteger.valueOf(new Date().getTime()/1000));
            JAXBOperations.marshall(lastUpdate,"data/lastupdate.xml","schemas/lastupdate.xsd");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private void appendMessages(final Usprotocol usprotocol) {
        if (usprotocol.getResponse().getMessages().getMessage().size() > 0) {
            Messages messages = JAXBOperations.unmarshallMessages("data/usmessages.xml");
            ObjectFactory of = new ObjectFactory();
            Message message;
            for (pl.swmud.ns.swaedit.usprotocol.Message msg : usprotocol.getResponse().getMessages().getMessage()) {
                message = of.createMessage();
                message.setAuthor(msg.getAuthor());
                message.setBody(msg.getBody());
                message.setTimestamp(msg.getTimestamp());
                message.setTitle(msg.getTitle());
                messages.getMessage().add(message);
            }
            JAXBOperations.marshall(messages, "data/usmessages.xml", "schemas/usmessages.xsd");
            msgBalloon.balloonClicked.connect(this, "showMessagesWidget()");
            showMessages(usprotocol.getResponse().getMessages().getMessage().size());
        }
    }
    
    private void retrFile(pl.swmud.ns.swaedit.usprotocol.File file, InputStream is, long total) {
        int nread = -1;
        byte[] buf = new byte[BUF_LEN];
        long localBytes = 0;
        long len = BUF_LEN;
        int prcnt = 0;
        try {
            FileOutputStream fos = new FileOutputStream(file.getName());
            while ((nread = is.read(buf,0,(int)len)) > -1) {
                fos.write(buf,0,nread);
                bytes += nread;
                localBytes += nread;
                if ((len = file.getLength().longValue()-localBytes) > BUF_LEN) {
                    len = BUF_LEN;
                }
                if ((bytes*100)/total > prcnt) {
                    prcnt = (int)((bytes*100)/total);
                    showProgress(file.getName(),bytes);
                }
                if (localBytes == file.getLength().intValue() || len < 1) {
                    break;
                }
            }
            showProgress(file.getName(),bytes);
            fos.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private String createRequest() {
        Document doc = null;
        try {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setNamespaceAware(true);
            dbf.setValidating(true);
            dbf.setSchema(SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI).newSchema(new File("schemas/usprotocol.xsd")));
            DocumentBuilder db = dbf.newDocumentBuilder();
            doc = db.newDocument();
            Element root = doc.createElementNS("http://swmud.pl/ns/swaedit/usprotocol", "usprotocol");
            doc.appendChild(root);
            Element e = doc.createElement("request");
            e.setAttribute("timestamp", lastUpdate.getTimestamp().toString());
            root.appendChild(e);
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
        } catch (SAXException e) {
            e.printStackTrace();
        }
        
        StringWriter sw = new StringWriter();
        StreamResult sr = new StreamResult(sw);
        DOMSource ds = new DOMSource(doc);
        Transformer transformer = null;
        try {
            transformer = TransformerFactory.newInstance().newTransformer();
            transformer.transform(ds, sr);
        } catch (TransformerConfigurationException e) {
            e.printStackTrace();
        } catch (TransformerFactoryConfigurationError e) {
            e.printStackTrace();
        } catch (TransformerException e) {
            e.printStackTrace();
        }
        
        return sw.toString();
    }
    
    private void setProgress(final double progress) {
        QApplication.invokeLater(new Runnable() {
            public void run() {
                dlBalloon.setProgress("File download", "Downloading...", progress);
            }
        });
    }

    private void showProgress(final String filename, final double progress) {
        QApplication.invokeLater(new Runnable() {
            public void run() {
                if (dlBalloon.canShowProgress()) {
                    dlBalloon.showProgress("File download", "Downloading: "+filename, progress);
                }
            }
        });
    }

    private void showMessages(final int count) {
        QApplication.invokeLater(new Runnable() {
            public void run() {
                if (msgBalloon.canShowMessage()) {
                    msgBalloon.showMessage("New Messages", "New messages: "+count+".");
                }
            }
        });
    }

    @SuppressWarnings("unused")
    private void showMessagesWidget() {
        QApplication.invokeLater(new Runnable() {
            public void run() {
                new MessagesWidget().show();
            }
        });
    }
}
