package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QWidget;

class VnumQuestionWidget extends QWidget {
    
    private Ui_VnumQuestionWidget ui = new Ui_VnumQuestionWidget();
    private BigInteger vnum;
    Signal1<BigInteger> vnumSet = new Signal1<BigInteger>();
    

    public VnumQuestionWidget() {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
    }
    
    @SuppressWarnings("unused")
    private void on_vnumEdit_textChanged(String str) {
        try {
            vnum = BigInteger.valueOf(Long.parseLong(str));
            if (vnum.compareTo(BigInteger.ZERO) == 1) {
                ui.acceptButton.setEnabled(true);
            }
        } catch (NumberFormatException e) {
            ui.acceptButton.setEnabled(false);
        }
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        hide();
        vnumSet.emit(vnum);
        close();
    }

    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        close();
    }


}
