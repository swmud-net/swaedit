package pl.swmud.ns.swaedit.map;

import com.trolltech.qt.core.QTimer;
import com.trolltech.qt.gui.QWidget;

public class Animator extends QTimer {
	public Animator(QWidget parent, int fps) {
		super(parent);
		setInterval(1000/fps);
		timeout.connect(parent, "updateGL()");
    }
}
