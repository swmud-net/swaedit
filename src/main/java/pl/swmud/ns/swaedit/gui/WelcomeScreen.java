package pl.swmud.ns.swaedit.gui;

import com.trolltech.qt.core.QRect;
import com.trolltech.qt.core.QTimer;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QApplication;
import com.trolltech.qt.gui.QBrush;
import com.trolltech.qt.gui.QCloseEvent;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QImage;
import com.trolltech.qt.gui.QMouseEvent;
import com.trolltech.qt.gui.QPaintEvent;
import com.trolltech.qt.gui.QPainter;
import com.trolltech.qt.gui.QWidget;

public class WelcomeScreen extends QWidget {
    
    private QImage img;
    private QRect screen;
    Signal1<Boolean> closed = new Signal1<Boolean>();
    
    public WelcomeScreen() {
        super(SWAEdit.ref);
        setWindowTitle(SWAEdit.ref.windowTitle());
        setWindowIcon(QApplication.windowIcon());
        img = new QImage("images/header.png");
        screen = QApplication.desktop().screenGeometry();
        setFixedSize(screen.size());
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
        setWindowFlags(Qt.WindowType.Dialog);
        setWindowState(Qt.WindowState.WindowFullScreen);
        closed.connect(SWAEdit.ref, "splashScreen_closed()");
    }
    
    public void showNow() {
        if (!img.isNull()) {
            QTimer.singleShot(10000, this, "close()");
            showFullScreen();
        }
        else {
            closed.emit(true);
        }
    }
    
    public void paintEvent(QPaintEvent e) {
        super.paintEvent(e);
        QPainter p = new QPainter(this);
        p.fillRect(screen, new QBrush(QColor.black));
        p.drawImage(screen.size().width()/2-img.size().width()/2,screen.size().height()/2-img.size().height()/2,img);
    }
    
    public void mousePressEvent(QMouseEvent e) {
        close();
    }
    
    public void closeEvent(QCloseEvent e) {
        hide();
        closed.emit(true);
        super.closeEvent(e);
    }
}
