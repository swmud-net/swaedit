package pl.swmud.ns.swaedit.gui;
import java.util.List;

import pl.swmud.ns.swaedit.core.FlagsWrapper;
import pl.swmud.ns.swaedit.core.IFlagsSetter;
import pl.swmud.ns.swaedit.flags.Flag;
import pl.swmud.ns.swmud._1_0.area.Objects.Object.Values;
import pl.swmud.ns.swaedit.gui.Ui_ValueFlagsWidget;

import com.trolltech.qt.gui.*;

class ValueFlagsWidget extends QWidget{
    
    private Ui_ValueFlagsWidget ui = new Ui_ValueFlagsWidget();
    private boolean canEdit = false;
    private List<Flag> flagsTree;
    private Long flags;
    private int valueNo;
    private Values values;
    private String title;
    public Signal1<String> textChanged = new Signal1<String>();
    public Signal1<Boolean> clicked = new Signal1<Boolean>();
    
    public ValueFlagsWidget(int valueNo){
        super(SWAEdit.ref);
        ui.setupUi(this);
        this.valueNo = valueNo;
        canEdit = true;
    }
    
    @SuppressWarnings("unused")
    private void on_edit_textChanged(String str) {
        if (!canEdit) {
            return;
        }
        switch (valueNo) {
        case 0:
            try {
                values.setValue0(Integer.parseInt(str));
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                    setFWFlags(Long.parseLong(((Integer)values.getValue0()).toString()));
                }
            }
            break;
        case 1:
            try {
                values.setValue1(Integer.parseInt(str));
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                    setFWFlags(Long.parseLong(((Integer)values.getValue1()).toString()));
                }
            }
            break;
        case 2:
            try {
                values.setValue2(Integer.parseInt(str));
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                    setFWFlags(Long.parseLong(((Integer)values.getValue2()).toString()));
                }
            }
            break;
        case 3:
            try {
                Integer.parseInt(str);
                values.setValue3(str);
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                    setFWFlags(Long.parseLong(values.getValue3()));
                }
            }
            break;
        case 4:
            try {
                Integer.parseInt(str);
                values.setValue4(str);
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                    setFWFlags(Long.parseLong(values.getValue4()));
                }
            }
            break;
        case 5:
            try {
                Integer.parseInt(str);
                values.setValue5(str);
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                    setFWFlags(Long.parseLong(values.getValue5()));
                }
            }
            break;
        }
    }

    @SuppressWarnings("unused")
    private void on_button_clicked() {
        if (!canEdit) {
            return;
        }
        clicked.emit(true);
    }
    
    Long getFlags() {
        return Long.parseLong(ui.edit.text());
    }

    void setFWFlags(Long flags) {
        canEdit = false;
        this.flags = flags;
        ui.edit.setText(flags.toString());
        canEdit = true;
    }

    List<Flag> getFlagsTree() {
        return flagsTree;
    }

    void setFlagsTree(List<Flag> flagsTree) {
        this.flagsTree = flagsTree;
    }

    public void setValues(Values values) {
        this.values = values;
    }
    
    public void setTitle(String title) {
        this.title = title;
    }
    
    void showFlagsWidget() {
        new FlagsWidget(flagsTree,new FlagsWrapper(flags,
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        setFWFlags(flagsValue);
                        switch (valueNo) {
                        case 0:
                            values.setValue0(Integer.parseInt(flagsValue.toString()));
                            break;
                        case 1:
                            values.setValue1(Integer.parseInt(flagsValue.toString()));
                            break;
                        case 2:
                            values.setValue2(Integer.parseInt(flagsValue.toString()));
                            break;
                        case 3:
                            values.setValue3(flagsValue.toString());
                            break;
                        case 4:
                            values.setValue4(flagsValue.toString());
                            break;
                        case 5:
                            values.setValue5(flagsValue.toString());
                            break;
                        }
                        SWAEdit.ref.setModified();
                    }
        }),title).show();
    }
}
