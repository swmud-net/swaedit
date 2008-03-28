package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;

import pl.swmud.ns.swmud._1_0.area.Mobiles.Mobile;
import pl.swmud.ns.swmud._1_0.area.Repairs.Repair;
import pl.swmud.ns.swmud._1_0.area.Shops.Shop;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QWidget;

class NewShopWidget extends QWidget {
    
    enum Type {
        SHOP,
        REPAIR
    }

    private Ui_NewShopWidget ui = new Ui_NewShopWidget();
    private boolean found;
    Signal1<BigInteger> vnumChosen = new Signal1<BigInteger>();
    
    public NewShopWidget(Type type) {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowModality(Qt.WindowModality.ApplicationModal);

        if (type == Type.REPAIR) {
            for (Mobile mob : SWAEdit.ref.area.getMobiles().getMobile()) {
                found = false;
                for (Repair repair : SWAEdit.ref.area.getRepairs().getRepair()) {
                    if (mob.getVnum().equals(repair.getKeeper())) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ui.keeperBox.addItem(mob.getVnum()+" - "+mob.getName(), mob.getVnum());
                }
            }
        }
        else
        {
            for (Mobile mob : SWAEdit.ref.area.getMobiles().getMobile()) {
                found = false;
                for (Shop shop : SWAEdit.ref.area.getShops().getShop()) {
                    if (mob.getVnum().equals(shop.getKeeper())) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ui.keeperBox.addItem(mob.getVnum()+" - "+mob.getName(), mob.getVnum());
                }
            }
        }
        
        ui.keeperBox.setCurrentIndex(0);
    }
    
    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        close();
    }

    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        hide();
        vnumChosen.emit((BigInteger)ui.keeperBox.itemData(ui.keeperBox.currentIndex()));
        close();
    }
}
