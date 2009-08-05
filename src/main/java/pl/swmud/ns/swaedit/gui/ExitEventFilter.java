package pl.swmud.ns.swaedit.gui;

import com.trolltech.qt.core.QEvent;
import com.trolltech.qt.core.QObject;
import com.trolltech.qt.gui.QMouseEvent;
import com.trolltech.qt.gui.QToolButton;

class ExitEventFilter extends QObject {

    public boolean eventFilter(QObject o, QEvent e) {
        QToolButton button = (QToolButton)o; 
        if (e.type() == QEvent.Type.MouseButtonPress && button.isEnabled()) {
            button.pressed.emit();
            return true;
        }
        if (e instanceof QMouseEvent) {
            return true;
        }
        return super.eventFilter(o, e);
    }

}
