package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Resets.Reset;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QWidget;

class NewResetWidget extends QWidget {
    
    private Ui_NewResetWidget ui = new Ui_NewResetWidget();
    Signal0 resetCreated = new Signal0();

    
    public NewResetWidget() {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);

        for (pl.swmud.ns.swaedit.resets.Reset reset : SWAEdit.ref.resetsInfo.getReset()) {
            if (requirementMet(reset)) {
                ui.typeBox.addItem(reset.getName(), reset.getValue());
            }
        }
        
        ui.typeBox.setCurrentIndex(0);
    }

    private Reset newReset(String command) {
        ObjectFactory of = new ObjectFactory();
        Reset reset = of.createResetsReset();
        reset.setExtra(BigInteger.ZERO);
        reset.setArg1(BigInteger.ZERO);
        reset.setArg2(BigInteger.ZERO);
        reset.setArg3(BigInteger.ZERO);
        reset.setArg4(BigInteger.ZERO);
        reset.setCommand(command);

        return reset;
    }

    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        close();
    }

    boolean requirementMet(pl.swmud.ns.swaedit.resets.Reset reset) {
        if (reset.getRequires().equals("")) {
            return true;
        }

        for (Reset r : SWAEdit.ref.area.getResets().getReset()) {
            if (reset.getRequires().equals(r.getCommand())) {
                return true;
            }
        }
        
        return false;
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        SWAEdit.ref.area.getResets().getReset().add(newReset((String)ui.typeBox.itemData(ui.typeBox.currentIndex())));
        hide();
        resetCreated.emit();
        close();
    }

}
