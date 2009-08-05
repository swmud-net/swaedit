package pl.swmud.ns.swaedit.gui;


import pl.swmud.ns.swaedit.core.Cloner;
import pl.swmud.ns.swmud._1_0.area.Extradescs;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Extradescs.Extradesc;
import pl.swmud.ns.swaedit.gui.Ui_ExtraDescWidget;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.*;

public class ExtraDescWidget extends QWidget{

    private Ui_ExtraDescWidget ui = new Ui_ExtraDescWidget();
    private Extradescs originalExtraDescs;
    private Extradescs extraDescs;
    private boolean modified = false;
    private boolean canEdit = false;

    
    public ExtraDescWidget(Extradescs extraDescs){
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
        originalExtraDescs = extraDescs;
        this.extraDescs = Cloner.clone(extraDescs);
        fillExtraDescs();
        ui.cancelButton.clicked.connect(this,"close()");
        try {
            setExtradesc(extraDescs.getExtradesc().get(0));
            canEdit = true;
        } catch (IndexOutOfBoundsException e) {
        }
    }
    
    private Extradesc getCurrentExtraDesc() {
        return (Extradesc)ui.navigationComboBox.itemData(ui.navigationComboBox.currentIndex());
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        originalExtraDescs.getExtradesc().clear();
        originalExtraDescs.getExtradesc().addAll(extraDescs.getExtradesc());
        SWAEdit.ref.setModified();
        close();
    }
    
    @SuppressWarnings("unused")
    private void on_keywordsEdit_textChanged(String str) {
        if (getCurrentExtraDesc() == null || !canEdit)
            return;
        getCurrentExtraDesc().setKeyword(str);
        ui.navigationComboBox.setItemText(ui.navigationComboBox.currentIndex(), str);
        modified = true;
        enable(true);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Keyword", "Keyword cannot be empty!");
            ui.keywordsEdit.setFocus();
        }
    }
    
    @SuppressWarnings("unused")
    private void on_descriptionEdit_textChanged() {
        if (getCurrentExtraDesc() == null || !canEdit)
            return;
        getCurrentExtraDesc().setDescription(ui.descriptionEdit.toPlainText());
        modified = true;
        enable(true);
    }

    @SuppressWarnings("unused")
    private void on_navigationComboBox_currentIndexChanged(int idx) {
        setExtradesc((Extradesc)ui.navigationComboBox.itemData(idx));
    }
    
    private void setExtradesc(Extradesc ed) {
        if( ed == null )
            return;
        canEdit = false;
        ui.keywordsEdit.setText(ed.getKeyword());
        ui.descriptionEdit.setText(ed.getDescription());
        canEdit = true;
    }
    
    private void fillExtraDescs() {
        canEdit = false;
        enable(extraDescs.getExtradesc().size() > 0);
        ui.navigationComboBox.clear();
        for (Extradesc ed : this.extraDescs.getExtradesc()) {
            ui.navigationComboBox.addItem(ed.getKeyword(), ed);
        }
        ui.navigationComboBox.setCurrentIndex(0);
        canEdit = extraDescs.getExtradesc().size() > 0;
    }

    @SuppressWarnings("unused")
    private void on_addButton_clicked() {
        ObjectFactory of = new ObjectFactory();
        Extradesc ed = of.createExtradescsExtradesc();
        short no = 0;
        for (Extradesc edd : extraDescs.getExtradesc()) {
            if (edd.getKeyword().equals("newExtraDescription"+no)) {
                no++;
            }
        }
        ed.setKeyword("newExtraDescription"+no);
        ed.setDescription("");
        extraDescs.getExtradesc().add(ed);
        modified = true;
        enable(true);
        fillExtraDescs();
        ui.navigationComboBox.setCurrentIndex(ui.navigationComboBox.count()-1);
        ui.keywordsEdit.selectAll();
        ui.keywordsEdit.setFocus();
    }

    @SuppressWarnings("unused")
    private void on_deleteButton_clicked() {
        if (ui.navigationComboBox.count() < 1)
            return;
        canEdit = false;
        extraDescs.getExtradesc().remove(getCurrentExtraDesc());
        ui.keywordsEdit.setText("");
        ui.descriptionEdit.setText("");
        if (extraDescs.getExtradesc().size() > 0) {
            modified = true;
            enable(true);
        }
        else {
            modified =  originalExtraDescs.getExtradesc().size() != 0;
            enable(false);
        }
        fillExtraDescs();
    }
    
    private void enable(boolean enabled) {
        ui.keywordsEdit.setEnabled(enabled);
        ui.descriptionEdit.setEnabled(enabled);
        ui.navigationComboBox.setEnabled(enabled);
        ui.deleteButton.setEnabled(enabled);
        ui.acceptButton.setEnabled(modified);
    }
}
