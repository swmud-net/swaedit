package pl.swmud.ns.swaedit.gui;

import java.math.BigInteger;
import java.util.HashMap;

import pl.swmud.ns.swaedit.exits.Exit;
import pl.swmud.ns.swaedit.exits.Exits;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Rooms.Room;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QGridLayout;
import com.trolltech.qt.gui.QToolButton;
import com.trolltech.qt.gui.QWidget;


public class NewExitWidget extends QWidget {

    private Ui_NewExitWidget ui = new Ui_NewExitWidget();
    private HashMap<Short,QToolButton> buttons = new HashMap<Short,QToolButton>();
    private Room room;
    private short direction = -1;
    
    public NewExitWidget(Room room, Exits exits) {
        ui.setupUi(this);
        SWAEdit.setChildPosition(this);
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        this.room = room;
        int columns = exits.getGridColumns();
        QGridLayout gl = new QGridLayout();
        gl.setContentsMargins(2, 2, 2, 2);
        QWidget w = new QWidget();
        QToolButton b;
        w.setFixedSize(25, 25);
        int r = 0;
        for (Exit exit : exits.getExit()) {
            if (exit.isEmpty()) {
                gl.addWidget(w, r/columns, r%columns);
            }
            else {
                b = new QToolButton();
                b.setText(exit.getAbbreviation());
                b.setToolTip(exit.getName());
                b.setFixedSize(25, 25);
                b.setProperty("exit", exit);
                b.installEventFilter(new ExitEventFilter());
                b.pressed.connect(this, "buttonPressed()");
                for (pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit ex : room.getExits().getExit()) {
                    if (ex.getDirection() == exit.getValue() && !SWAEdit.ref.exitsMap.get(exit.getValue()).getName().equalsIgnoreCase("somewhere")) {
                        b.setEnabled(false);
                    }
                }
                buttons.put(exit.getValue(), b);
                gl.addWidget(b, r/columns, r%columns);
            }
            r++;
        }
        for (Room destRoom : SWAEdit.ref.area.getRooms().getRoom()) {
            ui.destinationVnumBox.addItem(destRoom.getVnum()+" - "+destRoom.getName(), destRoom);
        }
        ui.destinationVnumBox.setCurrentIndex(-1);
        ui.directionBox.setLayout(gl);
    }
    
    @SuppressWarnings("unused")
    private void buttonPressed() {
        QToolButton button = (QToolButton)signalSender(); 
        Exit exit = (Exit)button.property("exit");
        if (direction == -1) {
            direction = exit.getValue();
            button.setDown(true);
            ui.destinationVnumBox.setEnabled(true);
            ui.oneWayButton.setEnabled(false);
            ui.twoWayButton.setEnabled(false);
            fillExits();
        }
        else if (direction == exit.getValue()) {
            button.setDown(false);
            direction = -1;
            ui.destinationVnumBox.setEnabled(false);
            ui.acceptButton.setEnabled(false);
            ui.oneWayButton.setEnabled(true);
            ui.twoWayButton.setEnabled(true);
        }
    }

    private pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit newExit() {
        ObjectFactory of = new ObjectFactory();
        pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit exit = of.createRoomsRoomExitsExit();
        exit.setVnum(BigInteger.ZERO);
        exit.setDescription("");
        exit.setKeyword("");
        exit.setKey(new BigInteger("-1"));
        return exit;
    }
    
    @SuppressWarnings("unused")
    private void on_acceptButton_clicked() {
        pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit exit = newExit();
        Room destRoom = (Room)ui.destinationVnumBox.itemData(ui.destinationVnumBox.currentIndex());
        exit.setDirection(direction);
        exit.setVnum(destRoom.getVnum());
        room.getExits().getExit().add(exit);
        if (ui.twoWayButton.isChecked()) {
            exit = newExit();
            short opposite = SWAEdit.ref.exitsMap.get(direction).getOpposite();
            if (opposite < 0) {
                exit.setDirection(direction);
            }
            else {
                exit.setDirection(opposite);
            }
            exit.setVnum(room.getVnum());
            destRoom.getExits().getExit().add(exit);
        }
        SWAEdit.ref.fillExitData(room);
        SWAEdit.ref.setLastExitIndex();
        SWAEdit.ref.setModified();
        close();
    }

    @SuppressWarnings("unused")
    private void on_cancelButton_clicked() {
        close();
    }
    
    @SuppressWarnings("unused")
    private void on_destinationVnumBox_currentIndexChanged(Integer idx) {
        if (idx.intValue() < 0) {
            return;
        }
    }
    
    private void fillExits() {
        if (direction < 0) {
            return;
        }

        Exit exit = SWAEdit.ref.exitsMap.get(direction);
        ui.destinationVnumBox.clear();
        if (ui.twoWayButton.isChecked()) {
            outer:
                for (Room destRoom : SWAEdit.ref.area.getRooms().getRoom()) {
                    for (pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit ex : destRoom.getExits().getExit()) {
                        if (ex.getDirection() == exit.getOpposite() && exit.getOpposite() >= 0) {
                            continue outer;
                        }
                    }
                    ui.destinationVnumBox.addItem(destRoom.getVnum()+" - "+destRoom.getName(), destRoom);
                }
        }
        else {
            for (Room destRoom : SWAEdit.ref.area.getRooms().getRoom()) {
                ui.destinationVnumBox.addItem(destRoom.getVnum()+" - "+destRoom.getName(), destRoom);
            }
        }
        
        if (ui.destinationVnumBox.count() > 0) {
            ui.acceptButton.setEnabled(true);
        }
    }
}
