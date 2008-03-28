package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import pl.swmud.ns.swmud._1_0.area.Area;
import pl.swmud.ns.swmud._1_0.area.Head;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Head.Economy;
import pl.swmud.ns.swmud._1_0.area.Head.Ranges;
import pl.swmud.ns.swmud._1_0.area.Head.Vnums;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QMessageBox;
import com.trolltech.qt.gui.QWidget;

class NewAreaWidget extends QWidget {

    private Ui_NewAreaWidget ui = new Ui_NewAreaWidget();
    Signal0 areaCreated = new Signal0();

    public NewAreaWidget() {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);
    }
    
    private Area newArea(BigInteger lvnum, BigInteger uvnum) {
        ObjectFactory of = new ObjectFactory();
        Area area = of.createArea();
        Head head = of.createHead();
        head.setAuthors("");
        head.setBuilders("");
        Economy economy = of.createHeadEconomy();
        economy.setHigh(0);
        economy.setLow(0);
        head.setEconomy(economy);
        Ranges ranges = of.createHeadRanges();
        ranges.setHigh((short)100);
        ranges.setLow((short)1);
        head.setRanges(ranges);
        pl.swmud.ns.swmud._1_0.area.Head.Reset reset = of.createHeadReset();
        reset.setFrequency(120);
        reset.setMessage("");
        head.setReset(reset);
        Vnums vnums = of.createHeadVnums();
        vnums.setLvnum(lvnum);
        vnums.setUvnum(uvnum);
        head.setVnums(vnums);
        head.setFlags(0);
        head.setName("");
        area.setHead(head);
        area.setMobiles(of.createMobiles());
        area.setObjects(of.createObjects());
        area.setRepairs(of.createRepairs());
        area.setResets(of.createResets());
        area.setRooms(of.createRooms());
        area.setShops(of.createShops());
        area.setSpecials(of.createSpecials());
        
        return area;
    }

    void on_cancelButton_clicked() {
        close();
    }

    void on_acceptButton_clicked() {
        int ammount = 100;
        if (ui.vnums50Radio.isChecked()) {
            ammount = 50;
        }
        else if (ui.vnums100Radio.isChecked()) {
            ammount = 100;
        }
        else if (ui.vnums150Radio.isChecked()) {
            ammount = 150;
        }
        else {
            ammount = ui.vnumsOtherBox.value();
        }
        if (ammount < 50 || ammount % 50 != 0) {
            QMessageBox.critical(null, "Invalid Vnum Ammount", "Vnum ammount must be greater than 49 and divisible by 50!");
            return;
        }
        SWAEdit.ref.area = newArea(BigInteger.ONE,BigInteger.valueOf(ammount));
        hide();
        areaCreated.emit();
        close();
    }
    
    @SuppressWarnings("unused")
    private void on_vnumsOtherRadio_toggled(boolean checked) {
        ui.vnumsOtherBox.setEnabled(checked);
    } 
}
