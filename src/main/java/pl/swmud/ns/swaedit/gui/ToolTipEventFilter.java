package pl.swmud.ns.swaedit.gui;

import com.trolltech.qt.core.QEvent;
import com.trolltech.qt.core.QObject;
import com.trolltech.qt.gui.QHelpEvent;
import com.trolltech.qt.gui.QWidget;

class ToolTipEventFilter extends QObject {

    public boolean eventFilter(QObject o, QEvent e) {
        if (e.type() == QEvent.Type.ToolTip && o instanceof QWidget) {
            QHelpEvent he = (QHelpEvent)e;
            QWidget w = (QWidget)o;
            if (!w.toolTip().isEmpty() && BalloonWidget.canShowToolTip(w, he.globalPos())) {
                new BalloonWidget(he.globalPos()).showToolTip(w);
            }
            e.accept();
            return true;
        }
        else {
            return super.eventFilter(o, e);
        }

    }

}
