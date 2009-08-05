package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import pl.swmud.ns.swaedit.core.ResetWrapper;

import com.trolltech.qt.core.QEvent;
import com.trolltech.qt.core.QObject;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QMouseEvent;

class ResetVnumEventFilter extends QObject {
    
    private ResetWrapper wrapper;
    
    public ResetVnumEventFilter(ResetWrapper wrapper) {
        this.wrapper = wrapper;
    }

    public boolean eventFilter(QObject o, QEvent e) {
        if (e.type() == QEvent.Type.MouseButtonPress
                && ((QMouseEvent) e).button() == Qt.MouseButton.RightButton) {
            VnumQuestionWidget vqw = new VnumQuestionWidget();
            vqw.vnumSet.connect(this, "vnumSet(BigInteger)");
            vqw.show();
            return true;
        }
        return super.eventFilter(o, e);
    }
    
    @SuppressWarnings("unused")
    private void vnumSet(BigInteger vnum) {
        wrapper.setCurrentValue(vnum);
        SWAEdit.ref.fillResetData(wrapper.getReset());
        SWAEdit.ref.setModified();
    }
}
