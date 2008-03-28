package pl.swmud.ns.swaedit.gui;
import java.util.List;

import pl.swmud.ns.swaedit.core.FlagsWrapper;
import pl.swmud.ns.swaedit.flags.*;
import pl.swmud.ns.swaedit.gui.Ui_FlagsWidget;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.*;

public class FlagsWidget extends QWidget{
    
    private Ui_FlagsWidget ui = new Ui_FlagsWidget();
    private List<Flag> flagList;
    private Long flagsValue;
    private FlagsWrapper flagsWrapper;
    private QCheckBox[] flagBox;
    
    public FlagsWidget(List<Flag> flagList, FlagsWrapper flagsWrapper, String flagsName){
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        this.flagList = flagList;
        this.flagsWrapper = flagsWrapper;
        flagsValue = flagsWrapper.getFlagsValue();
        setWindowTitle(flagsName);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
        ui.flagsValueEdit.setText(flagsValue.toString());
        ui.flagGroupBox.setTitle(flagsName);
        flagBox = new QCheckBox[flagList.size()];
        int i = 0, j = 0, k = 0;
        QGridLayout gl = new QGridLayout(ui.flagGroupBox);
        for (Flag flag : this.flagList) {
            flagBox[k] = new QCheckBox(flag.getName());
            flagBox[k].setChecked((flagsValue.longValue() & flag.getValue().longValue()) == flag.getValue().longValue());
            flagBox[k].stateChanged.connect(this, "flagBoxStateChanged(Integer)");
            flagBox[k].setObjectName(flag.getValue().toString());
            gl.addWidget(flagBox[k], i, j);
            if( i >= 15 ) {
                i = 0;
                j++;
            } 
            else {
                i++;
            }
            k++;
        }
        ui.cancelButton.clicked.connect(this, "close()");
        ui.acceptButton.setEnabled(false);
    }
    
    @SuppressWarnings("unused")
    private void flagBoxStateChanged(Integer i) {
        QWidget w = (QWidget)signalSender();
        if( Qt.CheckState.resolve(i) == Qt.CheckState.Checked ) {
            flagsValue = flagsValue.longValue() | Long.parseLong(w.objectName());
        }
        else {
            flagsValue = flagsValue.longValue() & ~Long.parseLong(w.objectName());
        }
        ui.flagsValueEdit.setText(flagsValue.toString());
        ui.acceptButton.setEnabled(!flagsValue.equals(flagsWrapper.getFlagsValue()));
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        flagsWrapper.setFlagsValue(flagsValue);
        close();
    }
    
    @SuppressWarnings("unused")
    private void on_flagsValueEdit_textChanged(String str) {
        try {
            flagsValue = Long.parseLong(str);
            checkFlagBoxes();
            ui.acceptButton.setEnabled(!flagsValue.equals(flagsWrapper.getFlagsValue()));
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Flags Value", "All flags must have number value!");
                ui.flagsValueEdit.setText(flagsValue.toString());
            }
        }
    }
    
    private void checkFlagBoxes() {
        if( flagBox == null )
            return;
        for (int i = 0; i < flagBox.length; i++) {
            long flag = Long.parseLong(flagBox[i].objectName());
            flagBox[i].setChecked((flagsValue.longValue() & flag) == flag);
        }
    }
}
