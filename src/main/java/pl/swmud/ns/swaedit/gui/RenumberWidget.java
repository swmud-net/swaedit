package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import pl.swmud.ns.swaedit.core.Renumberer;

import com.trolltech.qt.core.QByteArray;
import com.trolltech.qt.core.QFile;
import com.trolltech.qt.core.QTextCodec;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QWidget;

public class RenumberWidget extends QWidget {

    private Ui_RenumberWidget ui = new Ui_RenumberWidget();
    private BigInteger vnum;
    Signal2<BigInteger,Integer> paramsSpecified = new Signal2<BigInteger,Integer>();

    public RenumberWidget() {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
        loadWarning("data/renumberWarning.html");
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
        paramsSpecified.emit(vnum,ui.programsButton.isChecked()
                ? Renumberer.RENUMBER_MUDPROGS : Renumberer.RENUMBER_RELIABLE);
        close();
    }

    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        close();
    }
    
    private void loadWarning(String path) {
        QByteArray buf = null;
        QFile file = new QFile(path);
        if (file.open(QFile.OpenModeFlag.ReadOnly)) {
            buf = file.readAll();
            file.close();
            ui.warningBrowser.setText(QTextCodec.codecForName(SWAEdit.FILE_ENCODING).toUnicode(buf));
        }
    }
}
