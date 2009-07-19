package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QWidget;

public class RenumberWidget extends QWidget {

    private Ui_RenumberWidget ui = new Ui_RenumberWidget();
    private BigInteger vnum;
    Signal1<BigInteger> vnumSpecified = new Signal1<BigInteger>();

    public RenumberWidget() {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
    }

    @SuppressWarnings("unused")
    private void on_vnumEdit_textChanged(String str) {
        try {
            vnum = BigInteger.valueOf(Long.parseLong(str));
            ui.acceptButton.setEnabled(vnum.compareTo(BigInteger.ZERO) == 1);
        } catch (NumberFormatException e) {
            ui.acceptButton.setEnabled(false);
        }
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        hide();
        vnumSpecified.emit(vnum);
        close();
    }

    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        close();
    }
}
