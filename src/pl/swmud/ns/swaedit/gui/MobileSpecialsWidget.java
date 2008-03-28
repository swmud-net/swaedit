package pl.swmud.ns.swaedit.gui;

import javax.xml.bind.JAXBElement;

import pl.swmud.ns.swaedit.core.Cloner;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Mobiles.Mobile;
import pl.swmud.ns.swmud._1_0.area.Specials.Special;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QWidget;

class MobileSpecialsWidget extends QWidget {

    private Ui_MobileSpecialsWidget ui = new Ui_MobileSpecialsWidget();
    private Special spec;
    private Special originalSpec;
    private boolean canEdit = false;
    
    public MobileSpecialsWidget(Mobile mob) {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
        
        for (Special s : SWAEdit.ref.area.getSpecials().getSpecial()) {
            if (s.getVnum().equals(mob.getVnum())) {
                originalSpec = s;
                spec = Cloner.clone(s);
                break;
            }
        }
        
        if (spec == null) {
            ObjectFactory of = new ObjectFactory();
            spec = of.createSpecialsSpecial();
            spec.setVnum(mob.getVnum());
            spec.setFunction("");
            spec.setFunction2("");
        }
        
        fillSpecs();
        canEdit = true;
    }
    
    private void fillSpecs() {
        boolean tmpCanEdit = canEdit;
        canEdit = false;
        ui.spec1Box.clear();
        ui.spec2Box.clear();
        ui.spec1Box.addItem("");
        ui.spec2Box.addItem("");
        for (JAXBElement<String> jxbe : SWAEdit.ref.mobileSpecFunctions.getName()) {
            ui.spec1Box.addItem(jxbe.getValue());
            if (!spec.getFunction().equals(jxbe.getValue())) {
                ui.spec2Box.addItem(jxbe.getValue());
            }
        }
        ui.spec1Box.setCurrentIndex(ui.spec1Box.findText(spec.getFunction()));
        ui.spec2Box.setCurrentIndex(ui.spec2Box.findText(spec.getFunction2()));
        ui.spec2Box.setEnabled(!spec.getFunction().isEmpty());
        canEdit = tmpCanEdit;
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        if (!canEdit) {
            return;
        }
        if (originalSpec == null) {
            SWAEdit.ref.area.getSpecials().getSpecial().add(spec);
        }
        else {
            originalSpec.setVnum(spec.getVnum());
            originalSpec.setFunction(spec.getFunction());
            originalSpec.setFunction2(spec.getFunction2());
        }
        SWAEdit.ref.setModified();
        close();
    }

    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        if (!canEdit) {
            return;
        }
        close();
    }
    
    @SuppressWarnings("unused")
    private void on_spec1Box_currentIndexChanged(int idx) {
        if (!canEdit) {
            return;
        }
        if (idx < 1) {
            ui.spec2Box.setEnabled(false);
            ui.spec2Box.setCurrentIndex(0);
        }
        else {
            ui.spec2Box.setEnabled(true);
            spec.setFunction(ui.spec1Box.itemText(idx));
            if (spec.getFunction().equals(spec.getFunction2())) {
                spec.setFunction2("");
            }
            fillSpecs();
        }
    }

    @SuppressWarnings("unused")
    private void on_spec2Box_currentIndexChanged(int idx) {
        if (!canEdit) {
            return;
        }
        spec.setFunction2(ui.spec2Box.itemText(idx));
    }
}
