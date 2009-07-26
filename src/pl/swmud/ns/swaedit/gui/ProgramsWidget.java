package pl.swmud.ns.swaedit.gui;
import javax.xml.bind.JAXBElement;

import pl.swmud.ns.swaedit.core.Cloner;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Programs;
import pl.swmud.ns.swmud._1_0.area.Programs.Program;
import pl.swmud.ns.swaedit.gui.Ui_ProgramsWidget;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.*;

public class ProgramsWidget extends QWidget{

    private Ui_ProgramsWidget ui = new Ui_ProgramsWidget();
    private Programs originalPrograms;
    private Programs programs;
    private boolean canEdit = false;
    private int currentProgram;
    private boolean modified = false;
    
    public ProgramsWidget(Programs programs){
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
        ui.cancelButton.clicked.connect(this, "close()");
        originalPrograms = programs;
        this.programs = Cloner.clone(programs);
        for (JAXBElement<String> type : SWAEdit.ref.progtypes.getName()) {
            ui.typeComboBox.addItem(type.getValue());
        }
        try {
            new ProgramsHighlighter(ui.programEdit.document(),SWAEdit.ref.highlighter);
        } catch (Exception e) {
            e.printStackTrace();
        }
        fillProgram(0);
        canEdit = this.programs.getProgram().size() > 0;
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        originalPrograms.getProgram().clear();
        originalPrograms.getProgram().addAll(programs.getProgram());
        SWAEdit.ref.setModified();
        close();
    }

    @SuppressWarnings("unused")
    private void on_firstButton_clicked() {
        fillProgram(0);
    }

    private void on_prevButton_clicked() {
        fillProgram(currentProgram-1);
    }

    private void on_nextButton_clicked() {
        fillProgram(currentProgram+1);
    }

    private void on_lastButton_clicked() {
        fillProgram(programs.getProgram().size()-1);
    }
    
    private void fillProgram(int no) {
        enable(programs.getProgram().size() > 0);
        if (no < 0 || no > programs.getProgram().size()-1) {
            return;
        }
        ui.progLcdNumber.display(no+1);
        currentProgram = no;
        Program program = programs.getProgram().get(no);
        canEdit = false;
        ui.triggerEdit.setText(program.getArgs());
        ui.programEdit.setText(program.getComlist());
        ui.typeComboBox.setCurrentIndex(ui.typeComboBox.findText(program.getType()));
        ui.wholePhraseCheckBox.setChecked(program.getArgs().trim().startsWith("p "));
        canEdit = true;
    }
    
    @SuppressWarnings("unused")
    private void on_triggerEdit_textChanged(String str) {
        if (!canEdit)
            return;
        Program program = programs.getProgram().get(currentProgram);
        program.setArgs(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Trigger", "Trigger cannot be empty!");
            ui.triggerEdit.setFocus();
        }
        canEdit = false;
        ui.wholePhraseCheckBox.setChecked(program.getArgs().trim().startsWith("p "));
        canEdit = true;
        modified = true;
        enable(true);
    }

    @SuppressWarnings("unused")
    private void on_programEdit_textChanged() {
        if (!canEdit)
            return;
        Program program = programs.getProgram().get(currentProgram);
        program.setComlist(ui.programEdit.toPlainText());
        if (ui.programEdit.toPlainText().isEmpty()) {
            QMessageBox.warning(null, "Invalid Program", "Program cannot be empty!");
            ui.programEdit.setFocus();
        }
        modified = true;
        enable(true);
    }
    
    @SuppressWarnings("unused")
    private void on_addButton_clicked() {
        ObjectFactory of = new ObjectFactory();
        Program program = of.createProgramsProgram();
        program.setArgs("witaj");
        program.setComlist("say Witaj $n");
        program.setType("speech_prog");
        programs.getProgram().add(program);
        modified = true;
        on_lastButton_clicked();
        ui.typeComboBox.setFocus();
    }

    @SuppressWarnings("unused")
    private void on_deleteButton_clicked() {
        if (programs.getProgram().size() < 1) {
            return;
        }
        programs.getProgram().remove(currentProgram);
        if (programs.getProgram().size() > 0) {
            if (currentProgram > 0) {
                on_prevButton_clicked();
            }
            else {
                currentProgram--;
                on_nextButton_clicked();
            }
            modified = true;
            enable(true);
        }
        else {
            currentProgram = -1;
            canEdit = false;
            ui.typeComboBox.setCurrentIndex(-1);
            ui.triggerEdit.setText("");
            ui.programEdit.setText("");
            ui.progLcdNumber.display(0);
            ui.wholePhraseCheckBox.setChecked(false);
            modified = originalPrograms.getProgram().size() != 0;
            enable(false);
        }
    }
    
    @SuppressWarnings("unused")
    private void on_typeComboBox_currentIndexChanged(int idx) {
        if (!canEdit)
            return;
        programs.getProgram().get(currentProgram).setType(ui.typeComboBox.itemText(idx));
        modified = true;
    }

    @SuppressWarnings("unused")
    private void on_wholePhraseCheckBox_stateChanged(int checked) {
        if (!canEdit)
            return;
        if (Qt.CheckState.resolve(checked) == Qt.CheckState.Checked) {
            programs.getProgram().get(currentProgram).setArgs("p "+programs.getProgram().get(currentProgram).getArgs().trim());
        }
        else {
            programs.getProgram().get(currentProgram).setArgs(programs.getProgram().get(currentProgram).getArgs().trim().replaceFirst("p ", ""));
        }
        ui.triggerEdit.setText(programs.getProgram().get(currentProgram).getArgs());
        modified = true;
    }
    
    private void enable(boolean enabled) {
        ui.deleteButton.setEnabled(enabled);
        ui.acceptButton.setEnabled(modified);
        ui.firstButton.setEnabled(enabled);
        ui.lastButton.setEnabled(enabled);
        ui.nextButton.setEnabled(enabled);
        ui.prevButton.setEnabled(enabled);
        ui.triggerEdit.setEnabled(enabled);
        ui.programEdit.setEnabled(enabled);
        ui.typeComboBox.setEnabled(enabled);
        ui.wholePhraseCheckBox.setEnabled(enabled);
    }
}
