package pl.swmud.ns.swaedit.gui;

import java.util.List;

import pl.swmud.ns.swaedit.core.FlagsWrapper;
import pl.swmud.ns.swaedit.core.IFlagsSetter;
import pl.swmud.ns.swaedit.core.ResetWrapper;
import pl.swmud.ns.swaedit.flags.Flag;

import com.trolltech.qt.gui.QMessageBox;
import com.trolltech.qt.gui.QWidget;

class ResetFlagsWidget extends QWidget {
    
    private Ui_ValueFlagsWidget ui = new Ui_ValueFlagsWidget();
    private boolean canEdit = false;
    private List<Flag> flagsTree;
    private Long flags;
    private ResetWrapper wrapper;
    private String title;
    public Signal1<String> textChanged = new Signal1<String>();
    public Signal0 clicked = new Signal0();
    
    public ResetFlagsWidget(ResetWrapper wrapper) {
        super(SWAEdit.ref);
        ui.setupUi(this);
        this.wrapper = wrapper;
        canEdit = true;
    }
    
    @SuppressWarnings("unused")
    private void on_edit_textChanged(String str) {
        if (!canEdit) {
            return;
        }

        try {
            setFWFlags(Long.parseLong(str));
            wrapper.setCurrentValue(flags);
            SWAEdit.ref.updateResetNavigatorText(wrapper);
            SWAEdit.ref.setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", title+" must have number value!");
                setFWFlags(wrapper.getCurrentValue());
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_button_clicked() {
        if (!canEdit) {
            return;
        }
        clicked.emit();
    }
    
    Long getFlags() {
        return flags;
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

    public void setTitle(String title) {
        this.title = title;
    }
    
    void showFlagsWidget() {
        new FlagsWidget(flagsTree,new FlagsWrapper(flags,
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        setFWFlags(flagsValue);
                        wrapper.setCurrentValue(flagsValue);
                        SWAEdit.ref.updateResetNavigatorText(wrapper);
                        SWAEdit.ref.setModified();
                    }
        }),title).show();
    }

}
