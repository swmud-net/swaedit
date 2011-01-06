package pl.swmud.ns.swaedit.gui;


import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.math.BigInteger;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import javax.xml.bind.JAXBElement;

import pl.swmud.ns.swaedit.core.FileServer;
import pl.swmud.ns.swaedit.core.FlagsWrapper;
import pl.swmud.ns.swaedit.core.IFlagsSetter;
import pl.swmud.ns.swaedit.core.JAXBOperations;
import pl.swmud.ns.swaedit.core.Renumberer;
import pl.swmud.ns.swaedit.core.ResetWrapper;
import pl.swmud.ns.swaedit.exits.Exits;
import pl.swmud.ns.swaedit.flags.Flag;
import pl.swmud.ns.swaedit.flags.Flags;
import pl.swmud.ns.swaedit.highlighter.Highlighter;
import pl.swmud.ns.swaedit.itemtypes.Itemtype;
import pl.swmud.ns.swaedit.itemtypes.Itemtypes;
import pl.swmud.ns.swaedit.itemtypes.Subvalue;
import pl.swmud.ns.swaedit.itemtypes.Value;
import pl.swmud.ns.swaedit.map.Mapper;
import pl.swmud.ns.swaedit.names.Names;
import pl.swmud.ns.swaedit.types.Type;
import pl.swmud.ns.swaedit.types.Types;
import pl.swmud.ns.swmud._1_0.area.Area;
import pl.swmud.ns.swmud._1_0.area.Head;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Sectiona;
import pl.swmud.ns.swmud._1_0.area.Sectionr;
import pl.swmud.ns.swmud._1_0.area.Sections;
import pl.swmud.ns.swmud._1_0.area.Sectiont;
import pl.swmud.ns.swmud._1_0.area.Sectionv;
import pl.swmud.ns.swmud._1_0.area.Sectionx;
import pl.swmud.ns.swmud._1_0.area.Short;
import pl.swmud.ns.swmud._1_0.area.Mobiles.Mobile;
import pl.swmud.ns.swmud._1_0.area.Objects.Object;
import pl.swmud.ns.swmud._1_0.area.Repairs.Repair;
import pl.swmud.ns.swmud._1_0.area.Resets.Reset;
import pl.swmud.ns.swmud._1_0.area.Rooms.Room;
import pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit;
import pl.swmud.ns.swmud._1_0.area.Shops.Shop;

import com.trolltech.qt.core.QAbstractEventDispatcher;
import com.trolltech.qt.core.QEventLoop;
import com.trolltech.qt.core.QObject;
import com.trolltech.qt.core.QPoint;
import com.trolltech.qt.core.QRect;
import com.trolltech.qt.core.QTimer;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QAbstractButton;
import com.trolltech.qt.gui.QAbstractSpinBox;
import com.trolltech.qt.gui.QAction;
import com.trolltech.qt.gui.QApplication;
import com.trolltech.qt.gui.QCloseEvent;
import com.trolltech.qt.gui.QComboBox;
import com.trolltech.qt.gui.QDialog;
import com.trolltech.qt.gui.QFileDialog;
import com.trolltech.qt.gui.QHBoxLayout;
import com.trolltech.qt.gui.QIcon;
import com.trolltech.qt.gui.QLabel;
import com.trolltech.qt.gui.QLineEdit;
import com.trolltech.qt.gui.QMainWindow;
import com.trolltech.qt.gui.QMenu;
import com.trolltech.qt.gui.QMessageBox;
import com.trolltech.qt.gui.QSpinBox;
import com.trolltech.qt.gui.QSystemTrayIcon;
import com.trolltech.qt.gui.QTextBrowser;
import com.trolltech.qt.gui.QTextEdit;
import com.trolltech.qt.gui.QWidget;

public class SWAEdit extends QMainWindow {
    
    public static final String FILE_ENCODING = "ISO-8859-2";
    public static final int MAX_RESET_ARGS = 5;
    
    static SWAEdit ref;
    private Ui_SWAEdit ui = new Ui_SWAEdit();
    private Flags areaFlags;
    private Flags itemWearFlags;
    private Flags itemExtraFlags;
    private Flags mobileActFlags;
    private Flags mobileAffectedFlags;
    private Flags roomFlags;
    private Flags exitFlags;
    private Flags xFlags;
    private Flags resistFlags;
    private Flags attackFlags;
    private Flags defenseFlags;
    private Flags shopFlags;
    private Itemtypes itemtypes;
    Area area;
    Highlighter highlighter;
    Exits exits;
    Names progtypes;
    Names mobileSpecFunctions;
    private Types roomSectorTypes;
    pl.swmud.ns.swaedit.resets.Resets resetsInfo;
    private Names races;
    private Names languages;
    private Names planets;
    private Types positions;
    private Types repairTypes;
    String currentFileName;
    private QLabel[] valueLabels = new QLabel[6];
    private QWidget[] valueWidgets = new QWidget[6];
    private QHBoxLayout[] valueLayouts = new QHBoxLayout[6];
    private HashMap<String,QLabel> resetLabels = new HashMap<String,QLabel>();
    private HashMap<String,QWidget> resetWidgets = new HashMap<String,QWidget>();
    private HashMap<String,QHBoxLayout> resetLayouts = new HashMap<String,QHBoxLayout>();
    private boolean modified = false;
    private boolean itemCanChange = false;
    private boolean headCanChange = false;
    private boolean mobCanChange = false;
    private boolean roomCanChange = false;
    private boolean resetCanChange = false;
    private boolean shopCanChange = false;
    private boolean repairCanChange = false;
    HashMap<java.lang.Short, pl.swmud.ns.swaedit.exits.Exit> exitsMap
        = new HashMap<java.lang.Short,pl.swmud.ns.swaedit.exits.Exit>();
    HashMap<String, pl.swmud.ns.swaedit.resets.Reset> resetsMap
    = new HashMap<String,pl.swmud.ns.swaedit.resets.Reset>();
    private int keyValue = -1;
    private QSystemTrayIcon sysTray = new QSystemTrayIcon(new QIcon("images/icon.png"),this);
    private QTimer backupTimer = new QTimer(this);
    private MapWidget mapWidget;
    private boolean mapSelection;
    
    public static void main(String[] args) {
        try {
            System.setErr(new PrintStream(new FileOutputStream("swaedit_err.log",true)));
        } catch (FileNotFoundException e) {
        }
        try {
            System.setOut(new PrintStream(new FileOutputStream("swaedit_out.log",true)));
        } catch (FileNotFoundException e) {
        }

        QApplication.initialize(args);
        
        QApplication.setWindowIcon(new QIcon("images/icon.png"));
        new SWAEdit();
        new WelcomeScreen().showNow();

        QApplication.exec();
    }
    
    public SWAEdit(){
        ui.setupUi(ref = this);
        itemtypes = JAXBOperations.unmarshallItemtypes("data/itemtypes.xml");
        highlighter = JAXBOperations.unmarshallHighlighter("data/highlighter.xml");
        exits = JAXBOperations.unmarshallExits("data/exits.xml");
        areaFlags = JAXBOperations.unmarshallFlags("data/areaflags.xml");
        itemWearFlags = JAXBOperations.unmarshallFlags("data/itemwearflags.xml");
        itemExtraFlags = JAXBOperations.unmarshallFlags("data/itemextraflags.xml");
        mobileActFlags = JAXBOperations.unmarshallFlags("data/mobileactflags.xml");
        mobileAffectedFlags = JAXBOperations.unmarshallFlags("data/mobileaffectedflags.xml");
        roomFlags = JAXBOperations.unmarshallFlags("data/roomflags.xml");
        exitFlags = JAXBOperations.unmarshallFlags("data/exitflags.xml");
        xFlags = JAXBOperations.unmarshallFlags("data/xflags.xml");
        resistFlags = JAXBOperations.unmarshallFlags("data/resistflags.xml");
        attackFlags = JAXBOperations.unmarshallFlags("data/attackflags.xml");
        defenseFlags = JAXBOperations.unmarshallFlags("data/defenseflags.xml");
        shopFlags = JAXBOperations.unmarshallFlags("data/shopflags.xml");
        progtypes = JAXBOperations.unmarshallNames("data/progtypes.xml");
        races = JAXBOperations.unmarshallNames("data/races.xml");
        languages = JAXBOperations.unmarshallNames("data/languages.xml");
        planets = JAXBOperations.unmarshallNames("data/planets.xml");
        mobileSpecFunctions = JAXBOperations.unmarshallNames("data/mobilespecfunctions.xml");
        positions = JAXBOperations.unmarshallTypes("data/positions.xml");
        repairTypes = JAXBOperations.unmarshallTypes("data/repairtypes.xml");
        roomSectorTypes = JAXBOperations.unmarshallTypes("data/roomsectortypes.xml");
        resetsInfo = JAXBOperations.unmarshallResets("data/resetsinfo.xml");
        
        setUpItemValueLabels();
        fillItemTypes();
        createRoomConstants();
        resetCreateConstants();
        fillMobileSex();
        fillShopConstants();
        installEventFilter();
        setSystemTray();
        backupTimer.timeout.connect(this, "timerBackup()");
        modified = false;
        
        //FIXME: temporary for map testing - remove afterwards
//        {
//            String fileName = "/root/workspace/swaedit/quarren.xml";
//            area = JAXBOperations.unmarshallArea(fileName);
//            currentFileName = fileName;
//            fillAll();
//            setNotModified();
//            on_actionShow_Map_triggered();
//        }
    }
    
    public void showNow() {
        /* processEvents() must be called here, to create all widgets before QApplication.exec(),
         * because sysTray.geometry() is required here */
        processEvents();
        QRect g = sysTray.geometry();
        /* FIXME: uncomment the follofing line */
        new FileServer(new QPoint(g.x()+g.width()/2,g.y()+g.height()/2));
    }
    
    /* System tray handling */
    private void setSystemTray() {
        sysTray.activated.connect(this, "sysTrayActivated(QSystemTrayIcon$ActivationReason)");
        QMenu m = new QMenu(windowTitle());
        QAction a = m.addAction("Quit");
        a.setShortcut(ui.actionQuit.shortcut());
        a.triggered.connect(this, "close()");
        sysTray.setContextMenu(m);
        sysTray.show();
    }
    
    @SuppressWarnings("unused")
    private void sysTrayActivated(QSystemTrayIcon.ActivationReason reason) {
        switch (reason) {
        case Context:
            break;

        case DoubleClick:
            break;

        case Trigger:
            setVisible(!isVisible());
            break;

        case MiddleClick:
            break;

        default: /* Unknown */
            break;
        }
    }
    
    /* Event handling & events */
    protected void closeEvent(QCloseEvent e) {
        if (canLeaveCurrent()) {
        	sysTray.hide();
            e.accept();
        }
        else {
            e.ignore();
        }
    }
    
    private static void installEventFilter() {
        ToolTipEventFilter ttef = new ToolTipEventFilter();
        for (QWidget w : QApplication.allWidgets()) {
            if (w instanceof QAbstractButton
                    || w instanceof QLabel
                    || w instanceof QLineEdit
                    || w instanceof QTextEdit
                    || w instanceof QTextBrowser
                    || w instanceof QAbstractSpinBox
                    || w instanceof QComboBox
                    ) {
                w.installEventFilter(ttef);
            }
        }
    }
    
    private void processEvents() {
        QEventLoop.ProcessEventsFlags pef = new QEventLoop.ProcessEventsFlags(QEventLoop.ProcessEventsFlag.ExcludeSocketNotifiers);
        pef.set(QEventLoop.ProcessEventsFlag.ExcludeUserInputEvents);
        QAbstractEventDispatcher.instance().processEvents(pef);
    }

    /* Splash Screen handling */
    void splashScreen_closed() {
        show();
        openArea();
        showNow();
    }
    
    /* Action handling */
    @SuppressWarnings("unused")
    private void on_actionQuit_triggered() {
        close();
    }
    
    @SuppressWarnings("unused")
    private void on_actionOpen_Area_triggered() {
        if (canLeaveCurrent()) {
            openArea();
        }
    }

    @SuppressWarnings("unused")
    private void on_actionSave_Area_triggered() {
        saveArea();
    }
    
    @SuppressWarnings("unused")
    private void on_actionSave_Area_As_triggered() {
        saveAreaAs();
    }

    @SuppressWarnings("unused")
    private void on_actionCreate_New_Area_triggered() {
        if (canLeaveCurrent()) {
            NewAreaWidget newAreaWidget = new NewAreaWidget();
            newAreaWidget.areaCreated.connect(this, "areaCreated()");
            newAreaWidget.show();
        }
    }

    @SuppressWarnings("unused")
    private void on_actionRenumber_triggered() {
        if (area == null) {
            QMessageBox.critical(null, "Renumber", "No Area. Create one first.");
            return;
        }
        RenumberWidget renumberWidget = new RenumberWidget();
        renumberWidget.paramsSpecified.connect(this, "renumber(BigInteger,Integer)");
        renumberWidget.show();
    }

    @SuppressWarnings("unused")
    private void on_actionCreate_New_Item_triggered() {
        Object item = newItem();
        if (item != null) {
            area.getObjects().getObject().add(item);
            fillItemData(item);
            setModified();
            statusBar().showMessage("New item created.", 5000);
        }
    }
    
    @SuppressWarnings("unused")
    private void on_actionCreate_New_Mobile_triggered() {
        Mobile mob = newMobile();
        if (mob != null) {
            area.getMobiles().getMobile().add(mob);
            fillMobileData(mob);
            setModified();
            statusBar().showMessage("New mobile created.", 5000);
        }
    }

    @SuppressWarnings("unused")
    private void on_actionCreate_New_Room_triggered() {
        Room room = newRoom();
        if (room != null) {
            area.getRooms().getRoom().add(room);
            fillRoomData(room);
            setModified();
            statusBar().showMessage("New room created.", 5000);
        }
    }

    @SuppressWarnings("unused")
    private void on_actionShow_Map_triggered() {
        if (area == null) {
            QMessageBox.critical(null, "Show Map", "No Area. Create one first.");
            return;
        }
        Mapper mapper = new Mapper(area);
        mapWidget = mapper.makeMap();
        statusBar().showMessage("Map prepared.", 5000);
    }
    
    @SuppressWarnings("unused")
    private void mapClosed() {
    	mapWidget = null;
    }

    private void on_actionCreate_New_Reset_triggered() {
        NewResetWidget newResetWidget = new NewResetWidget();
        newResetWidget.resetCreated.connect(this, "resetCreated()");
        newResetWidget.show();
    }

    private void on_actionCreate_New_Shop_triggered() {
        NewShopWidget nsw = new NewShopWidget(NewShopWidget.Type.SHOP);
        nsw.vnumChosen.connect(this,"shopVnumChosen(BigInteger)");
        nsw.show();
    }

    private void on_actionCreate_New_Repair_triggered() {
        NewShopWidget nsw = new NewShopWidget(NewShopWidget.Type.REPAIR);
        nsw.vnumChosen.connect(this,"repairVnumChosen(BigInteger)");
        nsw.show();
    }

    @SuppressWarnings("unused")
    private void on_actionDelete_Current_Item_triggered() {
        Object item = getCurrentObject();
        if (item == null) {
            return;
        }
        QMessageBox.StandardButton button =
            QMessageBox.question(null, "Deleting Item", "Are you sure you want to delete the current item?",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) );
        switch (button) {
        case Yes:
            break;
        default: /* no */
            return;
        }
        int idx = area.getObjects().getObject().indexOf(item);
        area.getObjects().getObject().remove(item);
        if (--idx >= 0) {
            fillItemData(area.getObjects().getObject().get(idx));
        }
        else {
            fillItemData(area.getObjects().getObject().size() > 0 ? area.getObjects().getObject().get(0) : null);
        }
        setModified();
        statusBar().showMessage("Item deleted.", 5000);
    }

    @SuppressWarnings("unused")
    private void on_actionDelete_Current_Mobile_triggered() {
        Mobile mob = getCurrentMob();
        if (mob == null) {
            return;
        }
        QMessageBox.StandardButton button =
            QMessageBox.question(null, "Deleting Mobile", "Are you sure you want to delete the current mobile?",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) );
        switch (button) {
        case Yes:
            break;
        default: /* no */
            return;
        }
        int idx = area.getMobiles().getMobile().indexOf(mob);
        area.getMobiles().getMobile().remove(mob);
        if (--idx >= 0) {
            fillMobileData(area.getMobiles().getMobile().get(idx));
        }
        else {
            fillMobileData(area.getMobiles().getMobile().size() > 0 ? area.getMobiles().getMobile().get(0) : null);
        }
        setModified();
        statusBar().showMessage("Mobile deleted.", 5000);
    }

    @SuppressWarnings("unused")
    private void on_actionDelete_Current_Room_triggered() {
        Room room = getCurrentRoom();
        if (room == null) {
            return;
        }
        QMessageBox.StandardButton button =
            QMessageBox.question(null, "Deleting Room", "Are you sure you want to delete the current room?",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) );
        switch (button) {
        case Yes:
            break;
        default: /* no */
            return;
        }
        int idx = area.getRooms().getRoom().indexOf(room);
        area.getRooms().getRoom().remove(room);
        if (--idx >= 0) {
            fillRoomData(area.getRooms().getRoom().get(idx));
        }
        else {
            fillRoomData(area.getRooms().getRoom().size() > 0 ? area.getRooms().getRoom().get(0) : null);
        }
        setModified();
        statusBar().showMessage("Room deleted.", 5000);
    }

    private void on_actionDelete_Current_Reset_triggered() {
        Reset reset = getCurrentReset();
        if (reset == null) {
            return;
        }
        QMessageBox.StandardButton button =
            QMessageBox.question(null, "Deleting Reset", "Are you sure you want to delete the current reset?",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) );
        switch (button) {
        case Yes:
            break;
        default: /* no */
            return;
        }
        int idx = area.getResets().getReset().indexOf(reset);
        area.getResets().getReset().remove(reset);
        if (--idx >= 0) {
            fillResetData(area.getResets().getReset().get(idx));
        }
        else {
            fillResetData(area.getResets().getReset().size() > 0 ? area.getResets().getReset().get(0) : null);
        }
        setModified();
        statusBar().showMessage("Reset deleted.", 5000);
    }

    private void on_actionDelete_Current_Shop_triggered() {
        Shop shop = getCurrentShop();
        if (shop == null) {
            return;
        }
        QMessageBox.StandardButton button =
            QMessageBox.question(null, "Deleting Shop", "Are you sure you want to delete the current shop?",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) );
        switch (button) {
        case Yes:
            break;
        default: /* no */
            return;
        }
        int idx = area.getShops().getShop().indexOf(shop);
        area.getShops().getShop().remove(shop);
        if (--idx >= 0) {
            fillShopData(area.getShops().getShop().get(idx));
        }
        else {
            fillShopData(area.getShops().getShop().size() > 0 ? area.getShops().getShop().get(0) : null);
        }
        setModified();
        statusBar().showMessage("Shop deleted.", 5000);
    }

    private void on_actionDelete_Current_Repair_triggered() {
        Repair repair = getCurrentRepair();
        if (repair == null) {
            return;
        }
        QMessageBox.StandardButton button =
            QMessageBox.question(null, "Deleting Repair", "Are you sure you want to delete the current repair?",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No) );
        switch (button) {
        case Yes:
            break;
        default: /* no */
            return;
        }
        int idx = area.getRepairs().getRepair().indexOf(repair);
        area.getRepairs().getRepair().remove(repair);
        if (--idx >= 0) {
            fillRepairData(area.getRepairs().getRepair().get(idx));
        }
        else {
            fillRepairData(area.getRepairs().getRepair().size() > 0 ? area.getRepairs().getRepair().get(0) : null);
        }
        setModified();
        statusBar().showMessage("Repair deleted.", 5000);
    }

    @SuppressWarnings("unused")
    private void on_actionMessages_triggered() {
        new MessagesWidget().show();
    }

    /* common section */
    void setModified() {
        if (!modified) {
            modified = true;
            if (!windowTitle().contains(" *")) {
                setWindowTitle(windowTitle()+" *");
            }
        }
    }

    void setNotModified() {
        if (modified) {
            modified = false;
            setWindowTitle(windowTitle().replace(" *", ""));
        }
    }

    void openArea() {
        QFileDialog fd = new QFileDialog(null,"Open an area");
        fd.setFilter("swmud 1.0 area files (*.xml)");
        fd.setFileMode(QFileDialog.FileMode.ExistingFile);
        if (QDialog.DialogCode.resolve(fd.exec()) == QDialog.DialogCode.Accepted) {
            String fileName = fd.selectedFiles().get(0);
            area = JAXBOperations.unmarshallArea(fileName);
            currentFileName = fileName;
            fillAll();
            setNotModified();
            backupTimer.start(300000); /* 5 minutes */
        }
    }
    
    void saveArea() {
        if (area != null) {
            if (currentFileName == null) {
                QFileDialog fd = new QFileDialog(null,"Save an area");
                fd.setFilter("swmud 1.0 area files (*.xml)");
                fd.setFileMode(QFileDialog.FileMode.AnyFile);
                fd.setAcceptMode(QFileDialog.AcceptMode.AcceptSave);
                if (QDialog.DialogCode.resolve(fd.exec()) == QDialog.DialogCode.Accepted) {
                    String fileName = fd.selectedFiles().get(0);
                    if (!fileName.endsWith(".xml")) {
                        fileName += ".xml";
                    }
                    currentFileName = fileName;
                }
            }
    
            if (currentFileName != null) {
                JAXBOperations.marshall(area,currentFileName);
                new File(currentFileName+"~").delete();
                setNotModified();
            }
        }
    }
    
    void saveAreaAs() {
        if (area != null) {
            QFileDialog fd = new QFileDialog(null,"Save an area");
            fd.setFilter("swmud 1.0 area files (*.xml)");
            fd.setFileMode(QFileDialog.FileMode.AnyFile);
            fd.setAcceptMode(QFileDialog.AcceptMode.AcceptSave);
            if (QDialog.DialogCode.resolve(fd.exec()) == QDialog.DialogCode.Accepted) {
                String fileName = fd.selectedFiles().get(0);
                if (!fileName.endsWith(".xml")) {
                    fileName += ".xml";
                }
                currentFileName = fileName;
                JAXBOperations.marshall(area,currentFileName);
                new File(currentFileName+"~").delete();
                setNotModified();
            }
        }
    }

    void timerBackup() {
        if (currentFileName != null) {
            JAXBOperations.marshall(area, currentFileName+"~");
        }
    }
    
    boolean canLeaveCurrent() {
        if (modified) {
            QMessageBox.StandardButton button =
            QMessageBox.question(null, "Leaving swaedit", "Area was modified. Are you sure you want to exit?"
                    +"\nYou will loose your changes if you say Ok now!\n",
                    new QMessageBox.StandardButtons(QMessageBox.StandardButton.Ok, QMessageBox.StandardButton.Cancel,
                    QMessageBox.StandardButton.Save) );
            switch (button) {
            case Ok:
                return true;
            case Cancel:
                return false;
            case Save:
                saveArea();
                return true;
            default:
                return false;
            }
        }
        return true;
    }

    @SuppressWarnings("unused")
    private void areaCreated() {
        currentFileName = null;
        fillAll();
        setModified();
        ui.tabWidget.setCurrentIndex(0);
        ui.nameEdit.setFocus();
        statusBar().showMessage("New area created.", 5000);
    }
    
    @SuppressWarnings("unused")
    private void renumber(BigInteger newFirstVnum, Integer optionsFlags) {
        Renumberer r = new Renumberer(area, newFirstVnum, optionsFlags, resetsMap); 
        r.renumber();
        fillAll();
        setModified();
        List<String> warnings = r.getWarnings();
        if (warnings.size() > 0) {
            statusBar().showMessage("Area vnum range changed (with: " + warnings.size() + " warnings).", 5000);
            if (currentFileName != null) {
                String warningsName = currentFileName.replaceFirst("\\.xml$", "_renumberWarnings");
                File f = null;
                String zeros = null;
                for (int i = 0; i < 100; i++) {
                    if (i < 10) {
                        zeros = "0";
                    } else {
                        zeros = "";
                    }
                    if (!(f = new File(warningsName+zeros + i + ".txt")).exists()) {
                        break;
                    }
                }
                if (!r.saveWarnings(f.getAbsolutePath())) {
                    QMessageBox.warning(null, "Warnings not saved", "Renumber warnings could not be saved to file for some reason.");
                }
            }
            new RenumberWarningsWidget(warnings).show();
        } else {
            statusBar().showMessage("Area vnum range changed.", 5000);
        }
    }
    
    private Short newShort(ObjectFactory of) {
        Short shortDesc = of.createShort();
        shortDesc.setInflect0("");
        shortDesc.setInflect1("");
        shortDesc.setInflect2("");
        shortDesc.setInflect3("");
        shortDesc.setInflect4("");
        shortDesc.setInflect5("");
        return shortDesc;
    }

    private void fillAll() {
        if (area != null) {
            fillAreaData();
            if (area.getObjects().getObject().size() > 0) {
                fillItemData(area.getObjects().getObject().get(0));
            }
            else {
                clearItemData();
            }
            if (area.getRooms().getRoom().size() > 0) {
                fillRoomData(area.getRooms().getRoom().get(0));
            }
            else {
                clearRoomData();
            }
            if (area.getMobiles().getMobile().size() > 0) {
                fillMobileData(area.getMobiles().getMobile().get(0));
            }
            else {
                clearMobileData();
            }
            if (area.getResets().getReset().size() > 0) {
                fillResetData(area.getResets().getReset().get(0));
            }
            else {
                clearResetData();
            }
            if (area.getShops().getShop().size() > 0) {
                fillShopData(area.getShops().getShop().get(0));
            }
            else {
                clearShopData();
            }
            if (area.getRepairs().getRepair().size() > 0) {
                fillRepairData(area.getRepairs().getRepair().get(0));
            }
            else {
                clearRepairData();
            }
        }
    }
    
    static void setChildPosition(QWidget child) {
        child.move(SWAEdit.ref.pos().add(new QPoint(ref.width()/2,ref.height()/2))
                .subtract(new QPoint(child.width()/2,child.height()/2)));
    } 
    
    /* Area Data section */
    private void fillAreaData() {
        headCanChange = false;
        Head head = area.getHead();
        ui.nameEdit.setText(head.getName());
        ui.authorsEdit.setText(head.getAuthors());
        ui.buildersEdit.setText(head.getBuilders());
        ui.securitySpinBox.setValue(head.getSecurity());
        ui.lvnumEdit.setText(head.getVnums().getLvnum().toString());
        ui.uvnumEdit.setText(head.getVnums().getUvnum().toString());
        ui.flagsEdit.setText(String.valueOf(head.getFlags()));
        ui.leconomyEdit.setText(String.valueOf(head.getEconomy().getLow()));
        ui.heconomyEdit.setText(String.valueOf(head.getEconomy().getHigh()));
        ui.resetFreqEdit.setText(String.valueOf(head.getReset().getFrequency()));
        ui.resetMsgEdit.setText(head.getReset().getMessage());
        ui.lrangeSpinBox.setValue(head.getRanges().getLow());
        ui.hrangeSpinBox.setValue(head.getRanges().getHigh());
        if (!head.getName().isEmpty()) {
            statusBar().showMessage("'"+head.getName()+"' loaded with: "+area.getObjects().getObject().size()
                    +" objects, "+area.getMobiles().getMobile().size()+" mobiles, "
                    +area.getRooms().getRoom().size()+" rooms.");
        }
        headCanChange = true;
    }

//    private void clearAreaData() {
//        headCanChange = false;
//        ui.nameEdit.setText("");
//        ui.authorsEdit.setText("");
//        ui.buildersEdit.setText("");
//        ui.securitySpinBox.setValue(2);
//        ui.lvnumEdit.setText("");
//        ui.uvnumEdit.setText("");
//        ui.flagsEdit.setText("0");
//        ui.leconomyEdit.setText("");
//        ui.heconomyEdit.setText("");
//        ui.resetFreqEdit.setText("");
//        ui.resetMsgEdit.setText("");
//        ui.lrangeEdit.setText("");
//        ui.hrangeEdit.setText("");
//        statusBar().showMessage("");
//    }
    
    @SuppressWarnings("unused")
    private void on_flagsButton_clicked() {
        if( !headCanChange )
            return;
        new FlagsWidget(areaFlags.getFlag(),new FlagsWrapper(area.getHead().getFlags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        area.getHead().setFlags(flagsValue);
                        ui.flagsEdit.setText(flagsValue.toString());
                        setModified();
                    }
        }),"Area Flags").show();
    }
    
    @SuppressWarnings("unused")
    private void on_flagsEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        try {
            area.getHead().setFlags(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Area Flags must have number value!");
                ui.flagsEdit.setText(String.valueOf(area.getHead().getFlags()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_nameEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        area.getHead().setName(str);
        setModified();
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Area Name", "Area Name cannot be empty!");
            ui.nameEdit.setFocus();
        }
    }

    @SuppressWarnings("unused")
    private void on_authorsEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        area.getHead().setAuthors(str);
        setModified();
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Authors", "Authors cannot be empty!");
            ui.authorsEdit.setFocus();
        }
    }

    @SuppressWarnings("unused")
    private void on_buildersEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        area.getHead().setBuilders(str);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_securitySpinBox_valueChanged(int value) {
        if( !headCanChange )
            return;
        area.getHead().setSecurity((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_lvnumEdit_textChanged(String str) {
        if( !headCanChange )
            return;
//        setModified();
    }

    @SuppressWarnings("unused")
    private void on_uvnumEdit_textChanged(String str) {
        if( !headCanChange )
            return;
//        setModified();
    }

    @SuppressWarnings("unused")
    private void on_leconomyEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        try {
            area.getHead().getEconomy().setLow(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Low Economy must have number value!");
                ui.leconomyEdit.setText(String.valueOf(area.getHead().getEconomy().getLow()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_heconomyEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        try {
            area.getHead().getEconomy().setHigh(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "High Economy must have number value!");
                ui.heconomyEdit.setText(String.valueOf(area.getHead().getEconomy().getHigh()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_lrangeSpinBox_valueChanged(int val) {
        if( !headCanChange )
            return;

        area.getHead().getRanges().setLow((short)val);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_hrangeSpinBox_valueChanged(int val) {
        if( !headCanChange )
            return;

        area.getHead().getRanges().setHigh((short)val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_resetFreqEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        try {
            area.getHead().getReset().setFrequency(java.lang.Short.parseShort(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Reset Frequency must have number value!");
                ui.resetFreqEdit.setText(String.valueOf(area.getHead().getReset().getFrequency()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_resetMsgEdit_textChanged(String str) {
        if( !headCanChange )
            return;
        area.getHead().getReset().setMessage(str);
        setModified();
    }

    /* Item Data section */
    private void setUpItemValueLabels() {
        valueLabels[0] = ui.itemValue0;
        valueLabels[1] = ui.itemValue1;
        valueLabels[2] = ui.itemValue2;
        valueLabels[3] = ui.itemValue3;
        valueLabels[4] = ui.itemValue4;
        valueLabels[5] = ui.itemValue5;
        clearItemLabelValues();
        byte i = 0;
        for (QObject child : ui.itemValueBox.layout().children()) {
            if (child instanceof QHBoxLayout) {
                valueLayouts[i++] = (QHBoxLayout)child;
            }
        }
    }
    
    private void clearItemLabelValues() {
        for (int i = 0; i < valueLabels.length; i++) {
            valueLabels[i].setText("value"+i+":");
        }
    }
    
    /* also fills gender */
    private void fillItemTypes() {
        ui.itemTypeComboBox.clear();
        for (Itemtype item : itemtypes.getItemtype()) {
            ui.itemTypeComboBox.addItem(item.getName(), item.getValue());
        }
        ui.itemGenderComboBox.addItem("neutral", Integer.valueOf(0));
        ui.itemGenderComboBox.addItem("male", Integer.valueOf(1));
        ui.itemGenderComboBox.addItem("female", Integer.valueOf(2));
        ui.itemGenderComboBox.addItem("plural", Integer.valueOf(3));
    }
    
    private Object newItem() {
        if (area == null) {
            QMessageBox.critical(null, "Item Creation", "No Area. Create one first.");
            return null;
        }
        ObjectFactory of = new ObjectFactory();
        Object item = of.createObjectsObject();
        BigInteger vnum;
        boolean found;
        for (vnum = new BigInteger(area.getHead().getVnums().getLvnum().toString());
        vnum.compareTo(area.getHead().getVnums().getUvnum()) < 1; vnum = vnum.add(BigInteger.ONE)) {
            found = false;
            for (Object it : area.getObjects().getObject()) {
                if (it.getVnum().equals(vnum)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        if (vnum.compareTo(area.getHead().getVnums().getUvnum()) > 0) {
            QMessageBox.critical(null, "Item Creation", "Out of item vnums. Item not created.");
            return null;
        }
        item.setActiondesc("");
        item.setAffects(of.createObjectsObjectAffects());
        item.setCost(0);
        item.setDescription("");
        item.setExtradescs(of.createExtradescs());
        item.setExtraflags(0);
        item.setGender(0);
        item.setLayers(0);
        item.setLevel((short)0);
        item.setName("i"+vnum);
        item.setPrograms(of.createPrograms());
        item.setRequirements(of.createObjectsObjectRequirements());
        item.setShort(newShort(of));
        item.setType(0);
        pl.swmud.ns.swmud._1_0.area.Objects.Object.Values values = of.createObjectsObjectValues();
        values.setValue0(0);
        values.setValue1(0);
        values.setValue2(0);
        values.setValue3("0");
        values.setValue4("0");
        values.setValue5("0");
        item.setValues(values);
        item.setVnum(vnum);
        item.setWearflags(0);
        item.setWeight(0);
        
        return item;
    }
    
    private void fillItemData(Object item) {
        if (item == null) {
            clearItemData();
            return;
        }
        itemCanChange = false;
        ui.itemVnumEdit.setText(item.getVnum().toString());
        ui.itemNameEdit.setText(item.getName());
        ui.itemDescription.setText(item.getDescription());
        ui.itemActionDescription.setText(item.getActiondesc());
        ui.itemInflect0Edit.setText(item.getShort().getInflect0());
        ui.itemInflect1Edit.setText(item.getShort().getInflect1());
        ui.itemInflect2Edit.setText(item.getShort().getInflect2());
        ui.itemInflect3Edit.setText(item.getShort().getInflect3());
        ui.itemInflect4Edit.setText(item.getShort().getInflect4());
        ui.itemInflect5Edit.setText(item.getShort().getInflect5());
        int idx = -1;
        int i = 0;
        ui.itemNavigatorComboBox.clear();
        for (Object it : area.getObjects().getObject()) {
            ui.itemNavigatorComboBox.addItem(it.getVnum()+" - "+it.getName(), it);
            if (it == item) {
                idx = i;
            }
            i++;
        }
        ui.itemNavigatorComboBox.setCurrentIndex(idx);
        ui.itemTypeComboBox.setCurrentIndex(ui.itemTypeComboBox.findData(item.getType()));
        setItemValues(item.getType());
        ui.itemExtraFlagsEdit.setText(String.valueOf(item.getExtraflags()));
        ui.itemWearFlagsEdit.setText(String.valueOf(item.getWearflags()));
        ui.itemLayersEdit.setText(String.valueOf(item.getLayers()));
        ui.itemWeightEdit.setText(String.valueOf(item.getWeight()));
        ui.itemCostEdit.setText(String.valueOf(item.getCost()));
        ui.itemLevelSpinBox.setValue(item.getLevel());
        ui.itemGenderComboBox.setCurrentIndex(ui.itemGenderComboBox.findData(item.getGender()));
        itemCanChange = ui.itemNavigatorComboBox.count() > 0;
    }

    private void clearItemData() {
        itemCanChange = false;
        ui.itemVnumEdit.setText("");
        ui.itemNameEdit.setText("");
        ui.itemDescription.setText("");
        ui.itemActionDescription.setText("");
        ui.itemInflect0Edit.setText("");
        ui.itemInflect1Edit.setText("");
        ui.itemInflect2Edit.setText("");
        ui.itemInflect3Edit.setText("");
        ui.itemInflect4Edit.setText("");
        ui.itemInflect5Edit.setText("");
        ui.itemNavigatorComboBox.clear();
        ui.itemTypeComboBox.setCurrentIndex(-1);
        ui.itemExtraFlagsEdit.setText("0");
        ui.itemWearFlagsEdit.setText("0");
        ui.itemLayersEdit.setText("0");
        ui.itemWeightEdit.setText("0");
        ui.itemCostEdit.setText("0");
        ui.itemLevelSpinBox.setValue(0);
        ui.itemGenderComboBox.setCurrentIndex(-1);
    }

    private void setItemValues(int type) {
        for (Itemtype itype : itemtypes.getItemtype()) {
            if( itype.getValue() == type ) {
                clearItemLabelValues();
                boolean filled[] = new boolean[6];
                for (Value value : itype.getValues().getValue()) {
                    filled[value.getNo()] = true;
                    valueLabels[value.getNo()].setText(value.getName()+":");
                    if( value.getSubvalues().getSubvalue().size() > 0 ) {
                        String contentType = value.getSubvalues().getType(); 
                        if( contentType.equals("auto") || contentType.equals("types") ) {
                            createItemValueTypeEdit(value);
                        }
                        else if( contentType.equals("flags") ) {
                            createItemValueFlagsWidget(value);
                        }
                        else {
                            createItemValueEdit(value.getNo());
                        }
                    }
                    else {
                        createItemValueEdit(value.getNo());
                    }
                }
                
                for (int i = 0; i < filled.length; i++) {
                    if( !filled[i] ) {
                        createItemValueEdit(i);
                    }
                }
                
                break;
            }
        }
    }
    
    private int getItemValueIndex(QComboBox cbox,int no) {
        int idx = 0;
        switch (no) {
        case 0:
            idx = cbox.findData(getCurrentObject().getValues().getValue0());
            break;
        case 1:
            idx = cbox.findData(getCurrentObject().getValues().getValue1());
            break;
        case 2:
            idx = cbox.findData(getCurrentObject().getValues().getValue2());
            break;
        case 3:
            idx = cbox.findData(getCurrentObject().getValues().getValue3());
            break;
        case 4:
            idx = cbox.findData(getCurrentObject().getValues().getValue4());
            break;
        case 5:
            idx = cbox.findData(getCurrentObject().getValues().getValue5());
            break;
        }
        return idx;
    }

    private void createItemValueEdit(int no) {
        QLineEdit widget;
        if( valueWidgets[no] == null || !valueWidgets[no].getClass().getName().equals(QLineEdit.class.getName()) ) {
            widget = new QLineEdit();
            widget.textChanged.connect(this, "itemValueEditChanged(String)");
            itemAddValueWidget(no,widget);
        }
        widget = (QLineEdit)valueWidgets[no];
        switch (no) {
        case 0:
            widget.setText(String.valueOf(getCurrentObject().getValues().getValue0()));
            break;
        case 1:
            widget.setText(String.valueOf(getCurrentObject().getValues().getValue1()));
            break;
        case 2:
            widget.setText(String.valueOf(getCurrentObject().getValues().getValue2()));
            break;
        case 3:
            widget.setText(getCurrentObject().getValues().getValue3());
            break;
        case 4:
            widget.setText(getCurrentObject().getValues().getValue4());
            break;
        case 5:
            widget.setText(getCurrentObject().getValues().getValue5());
            break;
        }
    }

    private void createItemValueTypeEdit(Value value) {
        QComboBox widget;
        int no = value.getNo();
        if( valueWidgets[no] == null || !valueWidgets[no].getClass().getName().equals(QComboBox.class.getName()) ) {
            widget = new QComboBox();
            widget.currentIndexChanged.connect(this, "itemValueComboBoxChanged(int)");
            itemAddValueWidget(no,widget);
        }
        widget = (QComboBox)valueWidgets[no];
        for (Subvalue subvalue : value.getSubvalues().getSubvalue()) {
            try {
                widget.addItem(subvalue.getName(), Integer.parseInt(subvalue.getValue()));
            } catch (NumberFormatException e) {
                widget.addItem(subvalue.getName(), subvalue.getValue());
            }
        }
        widget.setCurrentIndex(getItemValueIndex(widget,value.getNo()));
    }

    private void createItemValueFlagsWidget(Value value) {
        ValueFlagsWidget widget;
        int no = value.getNo();
        if( valueWidgets[no] == null || !valueWidgets[no].getClass().getName().equals(ValueFlagsWidget.class.getName()) ) {
            widget = new ValueFlagsWidget(no);
            widget.clicked.connect(this, "itemValueFlagsButtonClicked()");
            itemAddValueWidget(no,widget);
        }
        widget = (ValueFlagsWidget)valueWidgets[no];
        switch (no) {
        case 0:
            widget.setFWFlags(Long.valueOf(getCurrentObject().getValues().getValue0()));
            break;
        case 1:
            widget.setFWFlags(Long.valueOf(getCurrentObject().getValues().getValue1()));
            break;
        case 2:
            widget.setFWFlags(Long.valueOf(getCurrentObject().getValues().getValue2()));
            break;
        case 3:
            widget.setFWFlags(Long.valueOf(getCurrentObject().getValues().getValue3()));
            break;
        case 4:
            widget.setFWFlags(Long.valueOf(getCurrentObject().getValues().getValue4()));
            break;
        case 5:
            widget.setFWFlags(Long.valueOf(getCurrentObject().getValues().getValue5()));
            break;
        }

        pl.swmud.ns.swaedit.flags.ObjectFactory of = new pl.swmud.ns.swaedit.flags.ObjectFactory();
        List<Flag> flags = new LinkedList<Flag>();
        Flag flag;
        for (Subvalue subvalue : value.getSubvalues().getSubvalue()) {
            flag = of.createFlag();
            flag.setName(subvalue.getName());
            flag.setValue(new BigInteger(subvalue.getValue()));
            flags.add(flag);
        }
        
        widget.setFlagsTree(flags);
    }

    private void itemAddValueWidget(int no,QWidget widget) {
        if( valueWidgets[no] != null ) {
            valueWidgets[no].close();
        }
        valueWidgets[no] = widget;
        valueWidgets[no].setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        valueLayouts[no].addWidget(widget);
        widget.setObjectName(String.valueOf(no));
    }
    
    private Object getCurrentObject() {
        return (Object)ui.itemNavigatorComboBox.itemData(ui.itemNavigatorComboBox.currentIndex());
    }
    
    @SuppressWarnings("unused")
    private void on_itemNavigatorComboBox_currentIndexChanged(int idx) {
        if( !itemCanChange )
            return;
        itemCanChange = false;
        fillItemData((Object)ui.itemNavigatorComboBox.itemData(idx));
        itemCanChange = true;
    }

    @SuppressWarnings("unused")
    private void on_itemTypeComboBox_currentIndexChanged(int idx) {
        if( !itemCanChange )
            return;
        int type = (Integer)ui.itemTypeComboBox.itemData(idx);
        getCurrentObject().setType(type);
        setModified();
        setItemValues(type);
        if (ui.itemTypeComboBox.itemText(idx).equalsIgnoreCase("key")) {
            fillExitKeys(getCurrentExit());
        }
    }
    
    private String stringValue(java.lang.Object value) {
        return (value instanceof Integer) ? ((Integer)value).toString() : (String)value;
    }
    
    //TODO: exception handling when widget.itemData(idx) does not exist
    @SuppressWarnings("unused")
    private void itemValueComboBoxChanged(int idx) {
        if( !itemCanChange )
            return;
        QComboBox widget = (QComboBox)signalSender();
        switch (Integer.parseInt(widget.objectName())) {
        case 0:
            getCurrentObject().getValues().setValue0((Integer)widget.itemData(idx));
            break;
        case 1:
            getCurrentObject().getValues().setValue1((Integer)widget.itemData(idx));
            break;
        case 2:
            getCurrentObject().getValues().setValue2((Integer)widget.itemData(idx));
            break;
        case 3:
            getCurrentObject().getValues().setValue3(stringValue(widget.itemData(idx)));
            break;
        case 4:
            getCurrentObject().getValues().setValue4(stringValue(widget.itemData(idx)));
            break;
        case 5:
            getCurrentObject().getValues().setValue5(stringValue(widget.itemData(idx)));
            break;
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void itemValueFlagsButtonClicked() {
        if( !itemCanChange )
            return;
        ValueFlagsWidget widget = (ValueFlagsWidget)signalSender();
        int no = Integer.parseInt(widget.objectName());
        widget.setTitle(valueLabels[no].text().replace(":", ""));
        widget.setValues(getCurrentObject().getValues());
        widget.showFlagsWidget();
    }

    @SuppressWarnings("unused")
    private void itemValueEditChanged(String str) {
        if( !itemCanChange )
            return;
        QLineEdit widget = (QLineEdit)signalSender();
        switch (Integer.parseInt(widget.objectName())) {
        case 0:
            try {
                getCurrentObject().getValues().setValue0(Integer.parseInt(widget.text()));
                setModified();
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", valueLabels[0].text().replace(":", "")+" must have number value!");
                    ((QLineEdit)valueWidgets[0]).setText(String.valueOf(getCurrentObject().getValues().getValue0()));
                }
            }
            break;
        case 1:
            try {
                getCurrentObject().getValues().setValue1(Integer.parseInt(widget.text()));
                setModified();
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", valueLabels[1].text().replace(":", "")+" must have number value!");
                    ((QLineEdit)valueWidgets[1]).setText(String.valueOf(getCurrentObject().getValues().getValue1()));
                }
            }
            break;
        case 2:
            try {
                getCurrentObject().getValues().setValue2(Integer.parseInt(widget.text()));
                setModified();
            } catch (NumberFormatException e) {
                if( !str.isEmpty() ) {
                    QMessageBox.critical(null, "Invalid Number Value", valueLabels[2].text().replace(":", "")+" must have number value!");
                    ((QLineEdit)valueWidgets[2]).setText(String.valueOf(getCurrentObject().getValues().getValue2()));
                }
            }
            break;
        case 3:
            getCurrentObject().getValues().setValue3(widget.text());
            setModified();
            break;
        case 4:
            getCurrentObject().getValues().setValue4(widget.text());
            setModified();
            break;
        case 5:
            getCurrentObject().getValues().setValue5(widget.text());
            setModified();
            break;
        }
    }

    @SuppressWarnings("unused")
    private void on_itemDescription_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().setDescription(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemActionDescription_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().setActiondesc(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemExtraFlagsEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        try {
            getCurrentObject().setExtraflags(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Extra Flags must have number value!");
                ui.itemExtraFlagsEdit.setText(String.valueOf(getCurrentObject().getExtraflags()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_itemExtraFlagsButton_clicked() {
        if( !itemCanChange )
            return;
        new FlagsWidget(itemExtraFlags.getFlag(),new FlagsWrapper(getCurrentObject().getExtraflags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentObject().setExtraflags(flagsValue);
                        ui.itemExtraFlagsEdit.setText(flagsValue.toString());
                        setModified();
                    }
        }),"Item Extra Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_itemWearFlagsEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        try {
            getCurrentObject().setWearflags(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Wear Flags must have number value!");
                ui.itemWearFlagsEdit.setText(String.valueOf(getCurrentObject().getWearflags()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_itemWearFlagsButton_clicked() {
        if( !itemCanChange )
            return;
        new FlagsWidget(itemWearFlags.getFlag(),new FlagsWrapper(getCurrentObject().getWearflags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentObject().setWearflags(flagsValue);
                        ui.itemWearFlagsEdit.setText(flagsValue.toString());
                        setModified();
                    }
        }),"Item Wear Flags").show();
    }
    
    @SuppressWarnings("unused")
    private void on_itemInflect0Edit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().getShort().setInflect0(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemInflect1Edit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().getShort().setInflect1(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemInflect2Edit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().getShort().setInflect2(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemInflect3Edit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().getShort().setInflect3(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemInflect4Edit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().getShort().setInflect4(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemInflect5Edit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().getShort().setInflect5(str);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_itemExtraDescriptionButton_clicked() {
        if( !itemCanChange )
            return;
        new ExtraDescWidget(getCurrentObject().getExtradescs()).show();
    }
    
    @SuppressWarnings("unused")
    private void on_itemGenderComboBox_currentIndexChanged(int idx) {
        if( !itemCanChange )
            return;
        getCurrentObject().setGender(java.lang.Short.valueOf(String.valueOf((Integer)ui.itemGenderComboBox.itemData(idx))));
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_itemWeightEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        try {
            getCurrentObject().setWeight(Integer.parseInt(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Weight must have number value!");
                ui.itemWeightEdit.setText(String.valueOf(getCurrentObject().getWeight()));
            }
        }
    }
    
    @SuppressWarnings("unused")
    private void on_itemCostEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        try {
            getCurrentObject().setCost(Integer.parseInt(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Cost must have number value!");
                ui.itemCostEdit.setText(String.valueOf(getCurrentObject().getCost()));
            }
        }
    }
    
    @SuppressWarnings("unused")
    private void on_itemLevelSpinBox_valueChanged(int value) {
        if( !itemCanChange )
            return;
        getCurrentObject().setLevel((short)value);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_itemLayersEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        try {
            getCurrentObject().setLayers(java.lang.Short.parseShort(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Layers must have number value!");
                ui.itemLayersEdit.setText(String.valueOf(getCurrentObject().getLayers()));
            }
        }
    }
    
    /* DANGEROUS! */
    @SuppressWarnings("unused")
    private void on_itemVnumEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        return;
        /* Implement wisely -> remember about renumbering dependencies!
         * it is better to implement renumber() function and do not touch particular
         * vnums */
//        try {
//            getCurrentObject().setVnum(BigInteger.valueOf(Long.parseLong(str)));
//            setModified();
//        } catch (NumberFormatException e) {
//            if( !str.isEmpty() ) {
//                QMessageBox.critical(null, "Invalid Number Value", "Vnum must have number value!");
//                ui.itemVnumEdit.setText(String.valueOf(getCurrentObject().getVnum()));
//            }
//        }
    }
    
    @SuppressWarnings("unused")
    private void on_itemNameEdit_textChanged(String str) {
        if( !itemCanChange )
            return;
        getCurrentObject().setName(str);
        ui.itemNavigatorComboBox.setItemText(ui.itemNavigatorComboBox.currentIndex(), getCurrentObject().getVnum()+" - "+str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Name", "Name cannot be empty!");
            ui.itemNameEdit.setFocus();
        }
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_itemProgramsButton_clicked() {
        if( !itemCanChange )
            return;
        new ProgramsWidget(getCurrentObject().getPrograms()).show();
    }
    
    /* Mobile Data section */
    private Mobile newMobile() {
        if (area == null) {
            QMessageBox.critical(null, "Mobile Creation", "No Area. Create one first.");
            return null;
        }
        ObjectFactory of = new ObjectFactory();
        Mobile mob = of.createMobilesMobile();
        BigInteger vnum;
        boolean found;
        for (vnum = new BigInteger(area.getHead().getVnums().getLvnum().toString());
        vnum.compareTo(area.getHead().getVnums().getUvnum()) < 1; vnum = vnum.add(BigInteger.ONE)) {
            found = false;
            for (Mobile m : area.getMobiles().getMobile()) {
                if (m.getVnum().equals(vnum)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        if (vnum.compareTo(area.getHead().getVnums().getUvnum()) > 0) {
            QMessageBox.critical(null, "Mobile Creation", "Out of mobile vnums. Mobile not created.");
            return null;
        }
        mob.setAct(1); /* NPC */
        mob.setAffected(0);
        mob.setAlignment((short)0);
        mob.setCredits(0);
        mob.setDescription("");
        mob.setDialog("");
        mob.setLevel(0);
        mob.setLong("");
        mob.setName("m"+vnum);
        mob.setPosition((short)0);
        mob.setPrograms(of.createPrograms());
        mob.setRace((String)((JAXBElement<String>)races.getName().get(0)).getValue());
        Sectiona sa = of.createSectiona();
        sa.setStr((short)0);
        sa.setInt((short)0);
        sa.setWis((short)0);
        sa.setDex((short)0);
        sa.setCon((short)0);
        sa.setCha((short)0);
        sa.setLck((short)0);
        mob.setSectiona(sa);
        Sectionr sr = of.createSectionr();
        sr.setHeight((short)0);
        sr.setNumattacks((short)0);
        sr.setSpeaking((String)((JAXBElement<String>)languages.getName().get(0)).getValue());
        sr.setSpeaks(0);
        sr.setWeight((short)0);
        mob.setSectionr(sr);
        Sections ss = of.createSections();
        ss.setSavingBreath(0);
        ss.setSavingParaPetri(0);
        ss.setSavingPoisonDeath(0);
        ss.setSavingSpellStaff(0);
        ss.setSavingWand(0);
        mob.setSections(ss);
        Sectiont st = of.createSectiont();
        st.setAc((short)0);
        st.setDamnodice(0);
        st.setDamplus(0);
        st.setDamsizedice(0);
        st.setHitnodice(0);
        st.setHitplus(0);
        st.setHitsizedice(0);
        st.setThac0((short)0);
        mob.setSectiont(st);
        Sectionv sv = of.createSectionv();
        sv.setVipflags("");
        mob.setSectionv(sv);
        Sectionx sx = of.createSectionx();
        sx.setAttacks(0);
        sx.setDamroll((short)0);
        sx.setDefenses(0);
        sx.setHitroll((short)0);
        sx.setImmune(0);
        sx.setResistant(0);
        sx.setSusceptible(0);
        sx.setXflags(0);
        mob.setSectionx(sx);
        mob.setSex((short)1);
        mob.setShort(newShort(of));
        mob.setVnum(vnum);
        
        return mob;
    }

    private void fillMobileData(Mobile mob) {
        if (mob == null) {
            clearMobileData();
            return;
        }
        mobCanChange = false;
        int idx = -1;
        int i = 0;
        ui.mobNavigatorComboBox.clear();
        for (Mobile mobile : area.getMobiles().getMobile()) {
            ui.mobNavigatorComboBox.addItem(mobile.getVnum()+" - "+mobile.getName(), mobile);
            if (mobile == mob) {
                idx = i;
            }
            i++;
        }
        ui.mobNavigatorComboBox.setCurrentIndex(idx);
        
        Sectionr sr = mob.getSectionr();
        Sectionx sx = mob.getSectionx();
        Sectiont st = mob.getSectiont();
        Sectiona sa = mob.getSectiona();
        ui.mobVnumEdit.setText(mob.getVnum().toString());
        ui.mobNameEdit.setText(mob.getName());
        ui.mobLongDescriptionEdit.setText(mob.getLong());
        ui.mobDescriptionText.setText(mob.getDescription());
        ui.mobSexComboBox.setCurrentIndex(ui.mobSexComboBox.findData((int)mob.getSex()));
        ui.mobRaceComboBox.setCurrentIndex(ui.mobRaceComboBox.findText(mob.getRace()));
        ui.mobPositionComboBox.setCurrentIndex(ui.mobPositionComboBox.findData((int)mob.getPosition()));
        ui.mobVipFlagsComboBox.setCurrentIndex(Math.max(0,ui.mobVipFlagsComboBox.findText(mob.getSectionv().getVipflags())));
        ui.mobSpeakingComboBox.setCurrentIndex(ui.mobSpeakingComboBox.findText(sr.getSpeaking()));
        ui.mobInflect0Edit.setText(mob.getShort().getInflect0());
        ui.mobInflect1Edit.setText(mob.getShort().getInflect1());
        ui.mobInflect2Edit.setText(mob.getShort().getInflect2());
        ui.mobInflect3Edit.setText(mob.getShort().getInflect3());
        ui.mobInflect4Edit.setText(mob.getShort().getInflect4());
        ui.mobInflect5Edit.setText(mob.getShort().getInflect5());
        ui.mobLevelSpinBox.setValue(mob.getLevel());
        ui.mobAlignmentBox.setValue(mob.getAlignment());
        ui.mobAttacksNoBox.setValue(mob.getSectionr().getNumattacks());
        ui.mobHeightBox.setValue(sr.getHeight());
        ui.mobWeightBox.setValue(sr.getWeight());
        ui.mobHitRollBox.setValue(sx.getHitroll());
        ui.mobDamRollBox.setValue(sx.getDamroll());
        ui.mobCreditsEdit.setText(String.valueOf(mob.getCredits()));
        ui.mobArmorClassBox.setValue(st.getAc());
        ui.mobHitNoDiceBox.setValue(st.getHitnodice());
        ui.mobHitPlusBox.setValue(st.getHitplus());
        ui.mobHitSizeDiceBox.setValue(st.getHitsizedice());
        ui.mobDamNoDiceBox.setValue(st.getDamnodice());
        ui.mobDamPlusBox.setValue(st.getDamplus());
        ui.mobDamSizeDiceBox.setValue(st.getDamsizedice());
        ui.mobStrBox.setValue(sa.getStr());
        ui.mobIntBox.setValue(sa.getInt());
        ui.mobWisBox.setValue(sa.getWis());
        ui.mobDexBox.setValue(sa.getDex());
        ui.mobConBox.setValue(sa.getCon());
        ui.mobChaBox.setValue(sa.getCha());
        ui.mobLckBox.setValue(sa.getLck());
        ui.mobDialogNameEdit.setText(mob.getDialog());

        mobCanChange = ui.mobNavigatorComboBox.count() > 0;
    }

    private void clearMobileData() {
        mobCanChange = false;
        ui.mobNavigatorComboBox.clear();
        ui.mobVnumEdit.setText("");
        ui.mobNameEdit.setText("");
        ui.mobLongDescriptionEdit.setText("");
        ui.mobDescriptionText.setText("");
        ui.mobSexComboBox.setCurrentIndex(-1);
        ui.mobRaceComboBox.setCurrentIndex(-1);
        ui.mobPositionComboBox.setCurrentIndex(-1);
        ui.mobVipFlagsComboBox.setCurrentIndex(-1);
        ui.mobSpeakingComboBox.setCurrentIndex(-1);
        ui.mobInflect0Edit.setText("");
        ui.mobInflect1Edit.setText("");
        ui.mobInflect2Edit.setText("");
        ui.mobInflect3Edit.setText("");
        ui.mobInflect4Edit.setText("");
        ui.mobInflect5Edit.setText("");
        ui.mobLevelSpinBox.setValue(0);
        ui.mobAlignmentBox.setValue(0);
        ui.mobAttacksNoBox.setValue(0);
        ui.mobHeightBox.setValue(0);
        ui.mobWeightBox.setValue(0);
        ui.mobHitRollBox.setValue(0);
        ui.mobDamRollBox.setValue(0);
        ui.mobCreditsEdit.setText("0");
        ui.mobArmorClassBox.setValue(0);
        ui.mobHitNoDiceBox.setValue(0);
        ui.mobHitPlusBox.setValue(0);
        ui.mobHitSizeDiceBox.setValue(0);
        ui.mobDamNoDiceBox.setValue(0);
        ui.mobDamPlusBox.setValue(0);
        ui.mobDamSizeDiceBox.setValue(0);
        ui.mobStrBox.setValue(0);
        ui.mobIntBox.setValue(0);
        ui.mobWisBox.setValue(0);
        ui.mobDexBox.setValue(0);
        ui.mobConBox.setValue(0);
        ui.mobChaBox.setValue(0);
        ui.mobLckBox.setValue(0);
        ui.mobDialogNameEdit.setText("");
    }

    /* fills also some other constants */
    private void fillMobileSex() {
        ui.mobSexComboBox.clear();
        ui.mobSexComboBox.addItem("neutral", 0);
        ui.mobSexComboBox.addItem("male", 1);
        ui.mobSexComboBox.addItem("female", 2);
        
        ui.mobRaceComboBox.clear();
        for (JAXBElement<String> jxbe : races.getName()) {
            ui.mobRaceComboBox.addItem(jxbe.getValue());
        }

        ui.mobSpeakingComboBox.clear();
        for (JAXBElement<String> jxbe : languages.getName()) {
            ui.mobSpeakingComboBox.addItem(jxbe.getValue());
        }
        
        ui.mobVipFlagsComboBox.clear();
        ui.mobVipFlagsComboBox.addItem("");
        for (JAXBElement<String> jxbe : planets.getName()) {
            ui.mobVipFlagsComboBox.addItem(jxbe.getValue());
        }
        
        ui.mobPositionComboBox.clear();
        for (Type pos : positions.getType()) {
            ui.mobPositionComboBox.addItem(pos.getName(), pos.getValue());
        }
    }

    private Mobile getCurrentMob() {
        return (Mobile)ui.mobNavigatorComboBox.itemData(ui.mobNavigatorComboBox.currentIndex());
    }
    
    @SuppressWarnings("unused")
    private void on_mobNavigatorComboBox_currentIndexChanged(int idx) {
        if (!mobCanChange) {
            return;
        }
        fillMobileData((Mobile)ui.mobNavigatorComboBox.itemData(idx));
    }

    @SuppressWarnings("unused")
    private void on_mobNameEdit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        Mobile mob = getCurrentMob();
        mob.setName(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Name", "Mobile name cannot be empty!");
            ui.mobNameEdit.setFocus();
        }
        ui.mobNavigatorComboBox.setItemText(ui.mobNavigatorComboBox.currentIndex(), mob.getVnum()+" - "+mob.getName());
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobLongDescriptionEdit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setLong(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Long Description", "Mobile long description cannot be empty!");
            ui.mobLongDescriptionEdit.setFocus();
        }
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobDescriptionText_textChanged() {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setDescription(ui.mobDescriptionText.toPlainText());
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobSexComboBox_currentIndexChanged(int idx) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setSex(((Integer)ui.mobSexComboBox.itemData(idx)).shortValue());
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobRaceComboBox_currentIndexChanged(int idx) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setRace(ui.mobRaceComboBox.itemText(idx));
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobActFlagsButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(mobileActFlags.getFlag(),new FlagsWrapper(getCurrentMob().getAct(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().setAct(flagsValue);
                        setModified();
                    }
        }),"Mobile Act Flags").show();
    }
    
    @SuppressWarnings("unused")
    private void on_mobAffectedFlagsButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(mobileAffectedFlags.getFlag(),new FlagsWrapper(getCurrentMob().getAffected(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().setAffected(flagsValue);
                        setModified();
                    }
        }),"Mobile Affected Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobPositionComboBox_currentIndexChanged(int idx) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setPosition(((Integer)ui.mobPositionComboBox.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobSpeakingComboBox_currentIndexChanged(int idx) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionr().setSpeaking(ui.mobSpeakingComboBox.itemText(idx));
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobCreditsEdit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        try {
            getCurrentMob().setCredits(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if( !str.isEmpty() ) {
                QMessageBox.critical(null, "Invalid Number Value", "Credits must have number value!");
                ui.mobCreditsEdit.setText(String.valueOf(getCurrentMob().getCredits()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_mobProgramsButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new ProgramsWidget(getCurrentMob().getPrograms()).show();
    }
    
    @SuppressWarnings("unused")
    private void on_mobSpecialsButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new MobileSpecialsWidget(getCurrentMob()).show();
    }
    
    @SuppressWarnings("unused")
    private void on_mobLevelSpinBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setLevel(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobAlignmentBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setAlignment((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobAttacksNoBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionr().setNumattacks((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobHeightBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionr().setHeight((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobWeightBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionr().setWeight((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobHitRollBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionx().setHitroll((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobDamRollBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionx().setDamroll((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobInflect0Edit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getShort().setInflect0(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Inflect0", "Mobile inflect0 cannot be empty!");
            ui.mobInflect0Edit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobInflect1Edit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getShort().setInflect1(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Inflect1", "Mobile inflect1 cannot be empty!");
            ui.mobInflect1Edit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobInflect2Edit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getShort().setInflect2(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Inflect2", "Mobile inflect2 cannot be empty!");
            ui.mobInflect2Edit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobInflect3Edit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getShort().setInflect3(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Inflect3", "Mobile inflect3 cannot be empty!");
            ui.mobInflect3Edit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobInflect4Edit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getShort().setInflect4(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Inflect4", "Mobile inflect4 cannot be empty!");
            ui.mobInflect4Edit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobInflect5Edit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getShort().setInflect5(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Inflect5", "Mobile inflect5 cannot be empty!");
            ui.mobInflect5Edit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobVipFlagsComboBox_currentIndexChanged(int idx) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectionv().setVipflags(ui.mobVipFlagsComboBox.itemText(idx));
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobXFlagsButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(xFlags.getFlag(),new FlagsWrapper(getCurrentMob().getSectionx().getXflags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().getSectionx().setXflags(flagsValue);
                        setModified();
                    }
        }),"Mobile XFlags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobResistancesButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(resistFlags.getFlag(),new FlagsWrapper(getCurrentMob().getSectionx().getResistant(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().getSectionx().setResistant(flagsValue);
                        setModified();
                    }
        }),"Mobile Resistance Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobImmunitiesButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(resistFlags.getFlag(),new FlagsWrapper(getCurrentMob().getSectionx().getImmune(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().getSectionx().setImmune(flagsValue);
                        setModified();
                    }
        }),"Mobile Immunities Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobSusceptibilitiesButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(resistFlags.getFlag(),new FlagsWrapper(getCurrentMob().getSectionx().getSusceptible(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().getSectionx().setSusceptible(flagsValue);
                        setModified();
                    }
        }),"Mobile Susceptibilities Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobAttacksButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(attackFlags.getFlag(),new FlagsWrapper(getCurrentMob().getSectionx().getAttacks(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().getSectionx().setAttacks(flagsValue);
                        setModified();
                    }
        }),"Mobile Attacks Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobDefensesButton_clicked() {
        if (!mobCanChange) {
            return;
        }
        new FlagsWidget(defenseFlags.getFlag(),new FlagsWrapper(getCurrentMob().getSectionx().getDefenses(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentMob().getSectionx().setDefenses(flagsValue);
                        setModified();
                    }
        }),"Mobile Defenses Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_mobArmorClassBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setAc((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobHitNoDiceBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setHitnodice(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobHitSizeDiceBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setHitsizedice(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobHitPlusBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setHitplus(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobDamNoDiceBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setDamnodice(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobDamSizeDiceBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setDamsizedice(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobDamPlusBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiont().setDamplus(value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobStrBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setStr((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobIntBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setInt((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobWisBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setWis((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobDexBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setDex((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobConBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setCon((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobChaBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setCha((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_mobLckBox_valueChanged(int value) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().getSectiona().setLck((short)value);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_mobDialogNameEdit_textChanged(String str) {
        if (!mobCanChange) {
            return;
        }
        getCurrentMob().setDialog(str);
        setModified();
    }

    /* Room Data section */
    private Room newRoom() {
        if (area == null) {
            QMessageBox.critical(null, "Room Creation", "No Area. Create one first.");
            return null;
        }
        ObjectFactory of = new ObjectFactory();
        Room room = of.createRoomsRoom();
        BigInteger vnum;
        boolean found;
        for (vnum = new BigInteger(area.getHead().getVnums().getLvnum().toString());
        vnum.compareTo(area.getHead().getVnums().getUvnum()) < 1; vnum = vnum.add(BigInteger.ONE)) {
            found = false;
            for (Room r : area.getRooms().getRoom()) {
                if (r.getVnum().equals(vnum)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        if (vnum.compareTo(area.getHead().getVnums().getUvnum()) > 0) {
            QMessageBox.critical(null, "Room Creation", "Out of room vnums. Room not created.");
            return null;
        }
        room.setDescription("");
        room.setExits(of.createRoomsRoomExits());
        room.setExtradescs(of.createExtradescs());
        room.setFlags(0);
        room.setLight((short)1);
        room.setName("r"+vnum);
        room.setNightdesc("");
        room.setPrograms(of.createPrograms());
        room.setSector(roomSectorTypes.getType().get(0).getValue());
        room.setTeledelay(BigInteger.ZERO);
        room.setTelevnum(BigInteger.ZERO);
        room.setTunnel(BigInteger.ZERO);
        room.setVnum(vnum);
        
        return room;
    }

    private void fillRoomData(Room room) {
        if (room == null) {
            clearRoomData();
            return;
        }
        roomCanChange = false;
        int idx = -1;
        int i = 0;
        ui.roomNavigatorComboBox.clear();
        for (Room r : area.getRooms().getRoom()) {
            ui.roomNavigatorComboBox.addItem(r.getVnum()+" - "+r.getName(), r);
            if (r == room) {
                idx = i;
            }
            i++;
        }
        ui.roomNavigatorComboBox.setCurrentIndex(idx);
        
        ui.roomVnumEdit.setText(room.getVnum().toString());
        ui.roomNameEdit.setText(room.getName());
        ui.roomDescriptionText.setText(room.getDescription());
        ui.roomNightDescriptionText.setText(room.getNightdesc());
        ui.roomLightSpinBox.setValue(room.getLight());
        ui.roomFlagsEdit.setText(String.valueOf(room.getFlags()));
        ui.roomTeleDelaySpinBox.setValue(room.getTeledelay().intValue());
        ui.roomTunnelSpinBox.setValue(room.getTunnel().intValue());
        ui.roomSectorComboBox.setCurrentIndex(ui.roomSectorComboBox.findData(room.getSector()));
        fillRoomTeleVnum(room);
        fillExitData(room);
        roomCanChange = ui.roomNavigatorComboBox.count() > 0;
    }

    private void clearRoomData() {
        roomCanChange = false;
        ui.roomNavigatorComboBox.clear();
        ui.roomVnumEdit.setText("");
        ui.roomNameEdit.setText("");
        ui.roomDescriptionText.setText("");
        ui.roomNightDescriptionText.setText("");
        ui.roomLightSpinBox.setValue(0);
        ui.roomFlagsEdit.setText("0");
        ui.roomTeleDelaySpinBox.setValue(0);
        ui.roomTunnelSpinBox.setValue(0);
        ui.roomSectorComboBox.setCurrentIndex(-1);
        ui.roomTeleVnumComboBox.clear();
        ui.roomExitNavigatorComboBox.clear();
        ui.roomExitDescriptionEdit.setText("");
        ui.roomExitKeywordEdit.setText("");
        ui.roomExitFlagsEdit.setText("0");
        ui.roomExitDistanceSpinBox.setValue(0);
        ui.roomExitKeyComboBox.clear();
    }

    private Room getCurrentRoom() {
        return (Room)ui.roomNavigatorComboBox.itemData(ui.roomNavigatorComboBox.currentIndex());
    }
        
    /* creates also resetsMap */
    private void createRoomConstants() {
        fillRoomSectors();
        for (pl.swmud.ns.swaedit.exits.Exit exit : exits.getExit()) {
            exitsMap.put(exit.getValue(),exit);
        }
        for (Itemtype type : itemtypes.getItemtype()) {
            if (type.getName().compareToIgnoreCase("key") == 0) {
                keyValue = type.getValue();
                break;
            }
        }
    }
    
    private void fillRoomSectors() {
        ui.roomSectorComboBox.clear();
        for (Type roomSector : roomSectorTypes.getType()) {
            ui.roomSectorComboBox.addItem(roomSector.getName(), roomSector.getValue());
        }
    }
    
    private void fillRoomTeleVnum(Room room) {
        boolean tmpRoomCanChange = roomCanChange;
        roomCanChange = false;
        ui.roomTeleVnumComboBox.clear();
        ui.roomTeleVnumComboBox.addItem("0 - <no tele_vnum>",null);
        int idx = 0;
        int i = 1;
        for (Room r : area.getRooms().getRoom()) {
            if (r != room) {
                ui.roomTeleVnumComboBox.addItem(r.getVnum()+" - "+r.getName(), r);
                if (room.getTelevnum().equals(r.getVnum())) {
                    idx = i;
                }
                i++;
            }
        }
        ui.roomTeleVnumComboBox.setCurrentIndex(idx);
        roomCanChange = tmpRoomCanChange;
    }

    @SuppressWarnings("unused")
    private void on_roomNavigatorComboBox_currentIndexChanged(int idx) {
        if (!roomCanChange) {
            return;
        }
        Room room = (Room)ui.roomNavigatorComboBox.itemData(idx);
        fillRoomData(room);
        if (mapWidget != null && !mapSelection) {
	        mapWidget.showRoom(room.getVnum());
        }
    }
    
    private void mapRoomVnumSelected(BigInteger vnum) {
    	for (int i = 0; i < ui.roomNavigatorComboBox.count(); i++) {
	        Room r = (Room)ui.roomNavigatorComboBox.itemData(i);
	        if (r.getVnum().equals(vnum)) {
	        	mapSelection = true;
	            ui.roomNavigatorComboBox.setCurrentIndex(i);
	            mapSelection = false;
	            break;
            }
        }
    }
    
    @SuppressWarnings("unused")
    private void on_roomNameEdit_textChanged(String str) {
        if (!roomCanChange) {
            return;
        }
        Room room = getCurrentRoom();
        room.setName(str);
        if (str.isEmpty()) {
            QMessageBox.warning(null, "Invalid Name", "Room name should not be empty!");
            ui.roomNameEdit.setFocus();
        }
        ui.roomNavigatorComboBox.setItemText(ui.roomNavigatorComboBox.currentIndex(), room.getVnum()+" - "+room.getName());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_roomDescriptionText_textChanged() {
        if (!roomCanChange) {
            return;
        }
        getCurrentRoom().setDescription(ui.roomDescriptionText.toPlainText());
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_roomNightDescriptionText_textChanged() {
        if (!roomCanChange) {
            return;
        }
        getCurrentRoom().setNightdesc(ui.roomNightDescriptionText.toPlainText());
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_roomSectorComboBox_currentIndexChanged(int idx) {
        if (!roomCanChange) {
            return;
        }
        getCurrentRoom().setSector((Integer)ui.roomSectorComboBox.itemData(idx));
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_roomLightSpinBox_valueChanged(int value) {
        if (!roomCanChange) {
            return;
        }
        getCurrentRoom().setLight((short)value);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_roomFlagsEdit_textChanged(String str) {
        if (!roomCanChange) {
            return;
        }
        try {
            getCurrentRoom().setFlags(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if (!str.isEmpty()) {
                QMessageBox.critical(null, "Invalid Number Value", "Room Flags must have number value!");
                ui.roomFlagsEdit.setText(String.valueOf(getCurrentRoom().getFlags()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_roomFlagsButton_clicked() {
        if (!roomCanChange) {
            return;
        }
        new FlagsWidget(roomFlags.getFlag(),new FlagsWrapper(getCurrentRoom().getFlags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentRoom().setFlags(flagsValue);
                        ui.roomFlagsEdit.setText(flagsValue.toString());
                        setModified();
                    }
        }),"Room Flags").show();
    }

    @SuppressWarnings("unused")
    private void on_roomTeleVnumComboBox_currentIndexChanged(int idx) {
        if (!roomCanChange) {
            return;
        }
        Room room = (Room)ui.roomTeleVnumComboBox.itemData(idx);
        getCurrentRoom().setTelevnum((room == null) ? BigInteger.ZERO : room.getVnum());
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_roomTunnelSpinBox_valueChanged(int value) {
        if (!roomCanChange) {
            return;
        }
        getCurrentRoom().setTunnel(BigInteger.valueOf(value));
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_roomTeleDelaySpinBox_valueChanged(int value) {
        if (!roomCanChange) {
            return;
        }
        getCurrentRoom().setTeledelay(BigInteger.valueOf(value));
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_roomExtraDescriptionButton_clicked() {
        if( !roomCanChange )
            return;
        new ExtraDescWidget(getCurrentRoom().getExtradescs()).show();
    }
    
    @SuppressWarnings("unused")
    private void on_roomProgramsButton_clicked() {
        if( !roomCanChange )
            return;
        new ProgramsWidget(getCurrentRoom().getPrograms()).show();
    }
    
    /* Room Exit Data */
    private void fillExitKeys(Exit exit) {
        boolean tmpRoomCanChange = roomCanChange;
        roomCanChange = false;
        ui.roomExitKeyComboBox.clear();
        int idx = -1;
        int i = 0;
        for (Object key : area.getObjects().getObject()) {
            if (key.getType() == keyValue) {
                ui.roomExitKeyComboBox.addItem("("+key.getVnum()+") "+key.getName(), key);
                if (exit != null && idx == -1 && key.getVnum().equals(exit.getKey())) {
                    idx = i; 
                }
            }
            i++;
        }
        ui.roomExitKeyComboBox.setCurrentIndex(idx);
        roomCanChange = tmpRoomCanChange;
    }
    
    private void fillExit(Exit exit) {
        boolean tmpRoomCanChange = roomCanChange;
        roomCanChange = false;
        ui.roomExitDescriptionEdit.setText(exit.getDescription());
        ui.roomExitKeywordEdit.setText(exit.getKeyword());
        ui.roomExitFlagsEdit.setText(String.valueOf(exit.getFlags()));
        ui.roomExitDistanceSpinBox.setValue(exit.getDistance());
        fillExitKeys(exit);
        roomCanChange = tmpRoomCanChange;
    }
    
    void fillExitData(Room room) {
        boolean tmpRoomCanChange = roomCanChange;
        roomCanChange = false;
        String roomName = "<unknown>";
        ui.roomExitNavigatorComboBox.clear();
        for (Exit exit : room.getExits().getExit()) {
            for (Room exitRoom : area.getRooms().getRoom()) {
                if (exitRoom.getVnum().equals(exit.getVnum())) {
                    roomName = exitRoom.getName();
                    break;
                }
            }
            ui.roomExitNavigatorComboBox.addItem(exitsMap.get(exit.getDirection()).getName()+" to: "+exit.getVnum()+" - "+roomName, exit);
        }
        
        if (ui.roomExitNavigatorComboBox.count() > 0) {
            ui.roomExitNavigatorComboBox.setCurrentIndex(0);
            fillExit((Exit)ui.roomExitNavigatorComboBox.itemData(0));
        }
        roomCanChange = tmpRoomCanChange;
    }
    
    void setLastExitIndex() {
        ui.roomExitNavigatorComboBox.setCurrentIndex(ui.roomExitNavigatorComboBox.count()-1);
    }

    private Exit getCurrentExit() {
        return (Exit)ui.roomExitNavigatorComboBox.itemData(ui.roomExitNavigatorComboBox.currentIndex());
    }
    
    @SuppressWarnings("unused")
    private void on_roomExitAddButton_clicked() {
        if (!roomCanChange) {
            return;
        }
        new NewExitWidget(getCurrentRoom(),exits).show();
    }

    @SuppressWarnings("unused")
    private void on_roomExitDeleteButton_clicked() {
        if (!roomCanChange || ui.roomExitNavigatorComboBox.count() < 1) {
            return;
        }
        Exit exit = getCurrentExit();
        Room room = getCurrentRoom();
        outer:       
            for (Room destRoom : area.getRooms().getRoom()) {
                if (exit.getVnum().equals(destRoom.getVnum())) {
                    for (Exit revExit : destRoom.getExits().getExit()) {
                        if (revExit.getVnum().equals(room.getVnum())
                                && (revExit.getDirection() == exitsMap.get(exit.getDirection()).getOpposite()
                                        || (exitsMap.get(exit.getDirection()).getName().equalsIgnoreCase("somewhere")
                                                && revExit.getDirection() == exit.getDirection()
                                                && revExit.getKeyword().equals(exit.getKeyword())))) {
                            QMessageBox.StandardButton button =
                                QMessageBox.question(null, "Deleting two way exit", "Do you want to delete the corresponding reverse exit?",
                                        new QMessageBox.StandardButtons(QMessageBox.StandardButton.Yes, QMessageBox.StandardButton.No,
                                                QMessageBox.StandardButton.Cancel) );
                                switch (button) {
                                case Yes:
                                    destRoom.getExits().getExit().remove(revExit);
                                case No:
                                    break outer;
                                default: /* Cancel */
                                    return;
                                }
                        }
                    }
                }
            }
        room.getExits().getExit().remove(exit);
        fillExitData(room);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_roomExitKeywordEdit_textChanged(String str) {
        if (!roomCanChange) {
            return;
        }
        Exit exit = getCurrentExit(); 
        exit.setKeyword(str);
        if (str.isEmpty() && exitsMap.get(exit.getDirection()).getName().equalsIgnoreCase("somewhere")) {
            QMessageBox.warning(null, "Invalid Keyword", "Keyword for 'somewhere' exit should not be empty!");
            ui.roomExitKeywordEdit.setFocus();
        }
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_roomExitDescriptionEdit_textChanged(String str) {
        if (!roomCanChange) {
            return;
        }
        getCurrentExit().setDescription(str);
        setModified();
    }

    @SuppressWarnings("unused")
    private void on_roomExitFlagsEdit_textChanged(String str) {
        if (!roomCanChange) {
            return;
        }
        try {
            getCurrentExit().setFlags(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if (!str.isEmpty()) {
                QMessageBox.critical(null, "Invalid Number Value", "Exit Flags must have number value!");
                ui.roomExitFlagsEdit.setText(String.valueOf(getCurrentExit().getFlags()));
            }
        }
    }

    @SuppressWarnings("unused")
    private void on_roomExitFlagsButton_clicked() {
        if (!roomCanChange || getCurrentExit() == null) {
            return;
        }
        new FlagsWidget(exitFlags.getFlag(),new FlagsWrapper(getCurrentExit().getFlags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentExit().setFlags(flagsValue);
                        ui.roomExitFlagsEdit.setText(flagsValue.toString());
                        setModified();
                    }
        }),"Exit Flags").show();
    }
    
    @SuppressWarnings("unused")
    private void on_roomExitNavigatorComboBox_currentIndexChanged(int idx) {
        if (!roomCanChange) {
            return;
        }
        Exit exit = (Exit)ui.roomExitNavigatorComboBox.itemData(idx); 
        fillExit(exit);
        if (mapWidget != null && !mapSelection && exit != null) {
	        mapWidget.showExit(getCurrentRoom().getVnum(), idx);
        }
    }
    
    @SuppressWarnings("unused")
    private void mapRoomExitSelected(BigInteger ownerRoomVnum, int exitIdx) {
    	if (!getCurrentRoom().getVnum().equals(ownerRoomVnum)) {
	        mapRoomVnumSelected(ownerRoomVnum);
        }
    	if (exitIdx >= 0 && exitIdx < ui.roomNavigatorComboBox.count()) {
    		mapSelection = true;
        	ui.roomExitNavigatorComboBox.setCurrentIndex(exitIdx);
        	mapSelection = false;
        }
    }

    @SuppressWarnings("unused")
    private void on_roomExitKeyComboBox_currentIndexChanged(int idx) {
        if (!roomCanChange) {
            return;
        }
        Exit exit = getCurrentExit();
        if (exit != null && idx >= 0) {
            exit.setKey(((Object)ui.roomExitKeyComboBox.itemData(idx)).getVnum());
        }
    }
    
    @SuppressWarnings("unused")
    private void on_roomExitDistanceSpinBox_valueChanged(int value) {
        if (!roomCanChange) {
            return;
        }
        Exit exit = getCurrentExit();
        if (exit != null) {
            exit.setDistance(value);
        }
    }

    /* Resets section */
    void fillResetData(Reset reset) {
        if (reset == null) {
            clearResetData();
            return;
        }
        resetCanChange = false;
        int idx = -1;
        int i = 0;
        ui.resetNavigatorComboBox.clear();
        for (Reset r : area.getResets().getReset()) {
            ui.resetNavigatorComboBox.addItem(prepareResetStr(r),r);
            if (r == reset) {
                idx = i;
            }
            i++;
        }
        
        ui.resetNavigatorComboBox.setCurrentIndex(idx);
        resetCreateWidgets(reset);
        resetCanChange = i > 0;
    }

    private void clearResetData() {
        resetCanChange = false;
        ui.resetNavigatorComboBox.clear();
        QWidget w;
        for (String key : resetWidgets.keySet()) {
            w = resetWidgets.get(key);
            if (w instanceof QComboBox) {
                ((QComboBox)w).clear();
            } else if (w instanceof QSpinBox) {
                ((QSpinBox)w).setValue(0);
            } else if (w instanceof QLineEdit) {
                ((QLineEdit)w).setText("");
            } else if (w instanceof ResetFlagsWidget) {
                ((ResetFlagsWidget)w).setFWFlags((long)0);
            }
        }
    }
    
    private String prepareResetStr(Reset reset) {
        pl.swmud.ns.swaedit.resets.Reset res;
        String name;
        res = resetsMap.get(reset.getCommand());
        name = res.getName().replaceFirst("[.*]", "");
        String str = "["+reset.getCommand()+"] "+name;
        if (!res.getArg1().getName().equals("")) {
            str += " {"+res.getArg1().getName()+": "+reset.getArg1()+"}";
        }
        if (!res.getArg2().getName().equals("")) {
            str += " {"+res.getArg2().getName()+": "+reset.getArg2()+"}";
        }
        if (!res.getArg3().getName().equals("")) {
            str += " {"+res.getArg3().getName()+": "+reset.getArg3()+"}";
        }
        if (!res.getArg4().getName().equals("")) {
            str += " {"+res.getArg4().getName()+": "+reset.getArg4()+"}";
        }
        if (!res.getExtra().getName().equals("")) {
            str += " {"+res.getExtra().getName()+": "+reset.getExtra()+"}";
        }
        
        return str;
    }
    
    void updateResetNavigatorText(ResetWrapper wrapper) {
        if (!wrapper.getCurrentArg().getName().equals("")) {
            ui.resetNavigatorComboBox.setItemText(ui.resetNavigatorComboBox.currentIndex(),
                    prepareResetStr(wrapper.getReset()));
        }
    }
    
    private void resetCreateConstants() {
        for (pl.swmud.ns.swaedit.resets.Reset reset : resetsInfo.getReset()) {
            resetsMap.put(reset.getValue(), reset);
        }

        resetLayouts.put("extra", (QHBoxLayout)ui.resetParametersBox.layout().children().get(0));
        resetLayouts.put("arg1", (QHBoxLayout)ui.resetParametersBox.layout().children().get(1));
        resetLayouts.put("arg2", (QHBoxLayout)ui.resetParametersBox.layout().children().get(2));
        resetLayouts.put("arg3", (QHBoxLayout)ui.resetParametersBox.layout().children().get(3));
        resetLayouts.put("arg4", (QHBoxLayout)ui.resetParametersBox.layout().children().get(4));
        
        resetLabels.put("extra", ui.lResetExtra);
        resetLabels.put("arg1", ui.lResetArg1);
        resetLabels.put("arg2", ui.lResetArg2);
        resetLabels.put("arg3", ui.lResetArg3);
        resetLabels.put("arg4", ui.lResetArg4);
    }
    
    private void resetAddWidget(QWidget w, ResetWrapper wrapper) {
        w.setObjectName(String.valueOf(wrapper.getCurrent()));
        w.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        resetLayouts.get(wrapper.getCurrentName()).addWidget(w);
        resetWidgets.put(wrapper.getCurrentName(), w);
    }
    
    private void resetCreateVnumWidget(ResetWrapper wrapper) {
        QWidget w = resetWidgets.get(wrapper.getCurrentName());
        QComboBox box;
        if (w == null || !(w instanceof QComboBox)) {
            if (w != null) {
                w.close();
                resetWidgets.remove(wrapper.getCurrentName());
            }
            box = new QComboBox();
            box.currentIndexChanged.connect(this, "resetArgVnumChanged(int)");
            w = box;
            w.setMinimumWidth(300);
            resetAddWidget(w, wrapper);
        }
        
        box = (QComboBox)w;
        box.installEventFilter(new ResetVnumEventFilter(wrapper));
        box.clear();
    }

    private void resetCreateIntWidget(ResetWrapper wrapper) {
        QWidget w = resetWidgets.get(wrapper.getCurrentName());
        QSpinBox box;
        if (w == null || !(w instanceof QSpinBox)) {
            if (w != null) {
                w.close();
                resetWidgets.remove(wrapper.getCurrentName());
            }
            box = new QSpinBox();
            box.setRange(-1, 1000);
            box.setAlignment(Qt.AlignmentFlag.AlignRight);
            box.setMinimumWidth(80);
            box.valueChanged.connect(this, "resetArgIntChanged(int)");
            w = box;
            resetAddWidget(w, wrapper);
        }
        
        box = (QSpinBox)w;
        box.setValue(wrapper.getCurrentValue().intValue());
    }

    private void resetCreateStringWidget(ResetWrapper wrapper) {
        QWidget w = resetWidgets.get(wrapper.getCurrentName());
        QLineEdit text;
        if (w == null || !(w instanceof QLineEdit)) {
            if (w != null) {
                w.close();
                resetWidgets.remove(wrapper.getCurrentName());
            }
            text = new QLineEdit();
            text.setMinimumWidth(200);
            text.textChanged.connect(this, "resetArgStringChanged(String)");
            w = text;
            resetAddWidget(w, wrapper);
        }
        
        text = (QLineEdit)w;
        text.setText("");
    }

    private void resetCreateTypeWidget(ResetWrapper wrapper) {
        QWidget w = resetWidgets.get(wrapper.getCurrentName());
        QComboBox box;
        if (w == null || !(w instanceof QComboBox)) {
            if (w != null) {
                w.close();
                resetWidgets.remove(wrapper.getCurrentName());
            }
            box = new QComboBox();
            box.currentIndexChanged.connect(this, "resetArgTypesChanged(int)");
            w = box;
            w.setMinimumWidth(300);
            resetAddWidget(w, wrapper);
        }
        
        box = (QComboBox)w;
        box.clear();
        int idx = -1;
        int i = 0;
        for (pl.swmud.ns.swaedit.resets.Value value : wrapper.getCurrentArg().getValues().getValue()) {
            box.addItem(value.getName(), new BigInteger(value.getValue()));
            if (value.getValue().equals(wrapper.getCurrentValue().toString())) {
                idx = i;
            }
            i++;
        }
        box.setCurrentIndex(idx);
    }

    private void resetCreateFlagsWidget(ResetWrapper wrapper) {
        QWidget w = resetWidgets.get(wrapper.getCurrentName());
        if (w == null || !(w instanceof ResetFlagsWidget)) {
            if (w != null) {
                w.close();
                resetWidgets.remove(wrapper.getCurrentName());
            }
            w = new ResetFlagsWidget(wrapper);
            resetAddWidget(w, wrapper);
        }
        
        ResetFlagsWidget rfw = (ResetFlagsWidget)w;
        rfw.clicked.connect(this, "resetArgFlagsClicked()");
        rfw.setTitle(wrapper.getCurrentArg().getName());
        rfw.setFWFlags(wrapper.getCurrentValue());

        pl.swmud.ns.swaedit.flags.ObjectFactory of = new pl.swmud.ns.swaedit.flags.ObjectFactory();
        List<Flag> flags = new LinkedList<Flag>();
        Flag flag;
        for (pl.swmud.ns.swaedit.resets.Value value : wrapper.getCurrentArg().getValues().getValue()) {
            flag = of.createFlag();
            flag.setName(value.getName());
            flag.setValue(new BigInteger(value.getValue()));
            flags.add(flag);
        }
        rfw.setFlagsTree(flags);
    }

    private void resetCreateWidgets(Reset reset) {
        pl.swmud.ns.swaedit.resets.Reset res = resetsMap.get(reset.getCommand());
        String type;
        ResetWrapper wrapper = new ResetWrapper(reset, res, 0);

        for (int i = 0; i < MAX_RESET_ARGS; i++) {
            if (wrapper.getArg(i).getValues().getValue().size() > 0
                    && wrapper.getArg(i).getValues().getType().equals("types")) {
                resetCreateTypeWidget(new ResetWrapper(reset, res, i));
            }
            else if (wrapper.getArg(i).getValues().getValue().size() > 0
                    && wrapper.getArg(i).getValues().getType().equals("flags")) {
                resetCreateFlagsWidget(new ResetWrapper(reset, res, i));
            }
            else if ((type = wrapper.getArg(i).getType()).equals("room")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                for (Room room : area.getRooms().getRoom()) {
                    box.addItem(room.getVnum()+" - "+room.getName(), room.getVnum());
                    if (room.getVnum().equals(wrapper.getValue(i))) {
                        idx = j;
                    }
                    j++;
                }
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if(type.equals("room_other")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                for (Room room : area.getRooms().getRoom()) {
                    if (resetIsOtherVnum(room.getVnum(), wrapper)) {
                        ((QComboBox)resetWidgets.get(argName)).addItem(room.getVnum()+" - "+room.getName(),
                                room.getVnum());
                        if (room.getVnum().equals(wrapper.getValue(i))) {
                            idx = j;
                        }
                        j++;
                    }
                    else if (room.getVnum().equals(wrapper.getValue(i))) {
                        wrapper.setCurrentValue(BigInteger.ZERO);
                    }
                }
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if(type.equals("mob")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                for (Mobile mob : area.getMobiles().getMobile()) {
                    ((QComboBox)resetWidgets.get(argName)).addItem(mob.getVnum()+" - "+mob.getName(),
                            mob.getVnum());
                    if (mob.getVnum().equals(wrapper.getValue(i))) {
                        idx = j;
                    }
                    j++;
                }
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if(type.equals("mob_other")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                for (Mobile mob : area.getMobiles().getMobile()) {
                    if (resetIsOtherVnum(mob.getVnum(), wrapper)) {
                        ((QComboBox)resetWidgets.get(argName)).addItem(mob.getVnum()+" - "+mob.getName(),
                                mob.getVnum());
                        if (mob.getVnum().equals(wrapper.getValue(i))) {
                            idx = j;
                        }
                        j++;
                    }
                    else if (mob.getVnum().equals(wrapper.getValue(i))) {
                        wrapper.setCurrentValue(BigInteger.ZERO);
                    }
                }
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if(type.equals("item")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                for (Object item : area.getObjects().getObject()) {
                    ((QComboBox)resetWidgets.get(argName)).addItem(item.getVnum()+" - "+item.getName(),
                            item.getVnum());
                    if (item.getVnum().equals(wrapper.getValue(i))) {
                        idx = j;
                    }
                    j++;
                }
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if(type.equals("item_other")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                for (Object item : area.getObjects().getObject()) {
                    if (resetIsOtherVnum(item.getVnum(), wrapper)) {
                        ((QComboBox)resetWidgets.get(argName)).addItem(item.getVnum()+" - "+item.getName(),
                                item.getVnum());
                        if (item.getVnum().equals(wrapper.getValue(i))) {
                            idx = j;
                        }
                        j++;
                    }
                    else if (item.getVnum().equals(wrapper.getValue(i))) {
                        wrapper.setCurrentValue(BigInteger.ZERO);
                    }
                }
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if(type.equals("ship")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
                /* not fully supported yet */
            }
            else if(type.equals("ship_other")) {
                resetCreateVnumWidget(wrapper = new ResetWrapper(reset, res, i));
                String argName = wrapper.getName(i);
                QComboBox box = (QComboBox)resetWidgets.get(argName);
                int idx = -1;
                int j = 0;
                /* not fully supported yet */
                if (idx < 0 && j > 0) {
                    /* this is a vnum from another area */
                    addOtherVnum(box,wrapper);
                    idx = j;
                }
                box.setCurrentIndex(idx);
            }
            else if (type.equals("intval")) {
                resetCreateIntWidget(new ResetWrapper(reset, res, i));
            }
            else if (type.equals("strval")) {
                resetCreateStringWidget(new ResetWrapper(reset, res, i));
            }
        }
        
        setupResetLabels(reset);
    }
    
    private void addOtherVnum(QComboBox box, ResetWrapper wrapper) {
        if (wrapper.getValue(wrapper.getCurrent()).equals(BigInteger.ZERO)) {
            box.addItem("0 - [none]", wrapper.getCurrentValue());
        }
        else {
            box.addItem(wrapper.getCurrentValue()+" - [from another area]", wrapper.getCurrentValue());
        }
    }
    
    private boolean resetIsOtherVnum(BigInteger vnum, ResetWrapper wrapper) {
        for (int i = 0; i < 5; i++) {
            if (i != wrapper.getCurrent() && wrapper.getArg(i).getType().equals(wrapper.getCurrentArg().getType().replaceFirst("_other", ""))
                    && wrapper.getValue(i).equals(vnum)) {
                return false;
            }
        }
        
        return true;
    }
    
    private String prepareLabelName(String argName, int idx) {
        String name;
        switch (idx) {
        case 1:
            name = "Arg1:";
            break;

        case 2:
            name = "Arg2:";
            break;
        
        case 3:
            name = "Arg3:";
            break;
        
        case 4:
            name = "Arg4:";
            break;

        default: /* 0 */
            name = "Extra:";
            break;
        }
        
        return argName.equals("") ? name : argName+":";
    }
    
    private void setupResetLabels(Reset reset) {
        ResetWrapper wrapper = new ResetWrapper(reset, resetsMap.get(reset.getCommand()), 0);
        for (int i = 0; i < 5; i++) {
            resetLabels.get(wrapper.getName(i)).setText(prepareLabelName(wrapper.getArg(i).getName(),i));
        }
    }
    
    private Reset getCurrentReset() {
        return (Reset)ui.resetNavigatorComboBox.itemData(ui.resetNavigatorComboBox.currentIndex());
    }
    
    /* must be called from inside a slot invoked by signal */
    private ResetWrapper getCurrentResetWrapper() {
        Reset reset = getCurrentReset();
        return new ResetWrapper(reset, resetsMap.get(reset.getCommand()),
                Integer.parseInt(((QObject)signalSender()).objectName()));
    }
    
    @SuppressWarnings("unused")
    private void resetArgIntChanged(int value) {
        if (!resetCanChange) {
            return;
        }

        ResetWrapper wrapper = getCurrentResetWrapper();
        wrapper.setCurrentValue((long)value);
        updateResetNavigatorText(wrapper);
        setModified();
    }

    @SuppressWarnings("unused")
    private void resetArgStringChanged(String str) {
        if (!resetCanChange) {
            return;
        }
        
        /* not supported by swmud (yet) */
        updateResetNavigatorText(getCurrentResetWrapper());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void resetArgVnumChanged(int idx) {
        if (!resetCanChange) {
            return;
        }

        getCurrentResetWrapper().setCurrentValue((BigInteger)((QComboBox)signalSender()).itemData(idx));
        fillResetData(getCurrentReset());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void resetArgFlagsClicked() {
        if (!resetCanChange) {
            return;
        }
        
        ResetFlagsWidget rfw = (ResetFlagsWidget)signalSender();
        rfw.showFlagsWidget();
    }
    
    @SuppressWarnings("unused")
    private void resetArgTypesChanged(int idx) {
        if (!resetCanChange) {
            return;
        }

        ResetWrapper wrapper = getCurrentResetWrapper();
        wrapper.setCurrentValue((BigInteger)((QComboBox)signalSender()).itemData(idx));
        updateResetNavigatorText(wrapper);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_resetNavigatorComboBox_currentIndexChanged(int idx) {
        if (!resetCanChange) {
            return;
        }
        
        fillResetData((Reset)ui.resetNavigatorComboBox.itemData(idx));
    }
    
    @SuppressWarnings("unused")
    private void on_resetAddButton_clicked() {
        on_actionCreate_New_Reset_triggered();
    }
    
    @SuppressWarnings("unused")
    private void on_resetDeleteButton_clicked() {
        on_actionDelete_Current_Reset_triggered();
    }
    
    @SuppressWarnings("unused")
    private void resetCreated() {
        fillResetData(area.getResets().getReset().get(area.getResets().getReset().size()-1));
        setModified();
        statusBar().showMessage("New reset created.", 5000);
    }

    /* Shop Data section */
    private Shop newShop(BigInteger keeperVnum) {
        ObjectFactory of = new ObjectFactory();
        Shop shop = of.createShopsShop();
        
        shop.setKeeper(keeperVnum);
        shop.setOpen((short)10);
        shop.setClose((short)20);
        shop.setFlags(0);
        shop.setProfitbuy(120);
        shop.setProfitsell(0);
        pl.swmud.ns.swmud._1_0.area.Shops.Shop.Types types = of.createShopsShopTypes();
        types.setType0((short)0);
        types.setType1((short)0);
        types.setType2((short)0);
        types.setType3((short)0);
        types.setType4((short)0);
        shop.setTypes(types);
        
        return shop;
    }
    
    private void fillShopData(Shop shop) {
        if (shop == null) {
            clearShopData();
            return;
        }
        shopCanChange = false;
        int idx = -1;
        int i = 0;
        ui.shopKeeperBox.clear();
        for (Mobile mobile : area.getMobiles().getMobile()) {
            for (Shop s : area.getShops().getShop()) {
                if (s.getKeeper().equals(mobile.getVnum())) {
                    ui.shopKeeperBox.addItem(mobile.getVnum()+" - "+mobile.getName(), mobile);
                    if (s == shop) {
                        idx = i;
                    }
                    i++;
                    break;
                }
            }
        }
        ui.shopKeeperBox.setCurrentIndex(idx);
        
        ui.shopFlagsEdit.setText(String.valueOf(shop.getFlags()));
        ui.shopOpenBox.setValue(shop.getOpen());
        ui.shopCloseBox.setValue(shop.getClose());
        ui.shopProfitBuyBox.setValue(shop.getProfitbuy());
        ui.shopProfitSellBox.setValue(shop.getProfitsell());
        ui.shopType0Box.setCurrentIndex(ui.shopType0Box.findData((int)shop.getTypes().getType0()));
        ui.shopType1Box.setCurrentIndex(ui.shopType1Box.findData((int)shop.getTypes().getType1()));
        ui.shopType2Box.setCurrentIndex(ui.shopType2Box.findData((int)shop.getTypes().getType2()));
        ui.shopType3Box.setCurrentIndex(ui.shopType3Box.findData((int)shop.getTypes().getType3()));
        ui.shopType4Box.setCurrentIndex(ui.shopType4Box.findData((int)shop.getTypes().getType4()));

        shopCanChange = area.getShops().getShop().size() > 0;
    }

    private void clearShopData() {
        shopCanChange = false;
        ui.shopKeeperBox.clear();
        ui.shopFlagsEdit.setText("");
        ui.shopOpenBox.setValue(0);
        ui.shopCloseBox.setValue(0);
        ui.shopProfitBuyBox.setValue(0);
        ui.shopProfitSellBox.setValue(0);
        ui.shopType0Box.setCurrentIndex(-1);
        ui.shopType1Box.setCurrentIndex(-1);
        ui.shopType2Box.setCurrentIndex(-1);
        ui.shopType3Box.setCurrentIndex(-1);
        ui.shopType4Box.setCurrentIndex(-1);
    }
    
    private Shop getCurrentShop() {
        if (ui.shopKeeperBox.count() < 1) {
            return null;
        }
        return getShop((Mobile)ui.shopKeeperBox.itemData(ui.shopKeeperBox.currentIndex()));
    }
    
    private Shop getShop(Mobile mob) {
        for (Shop shop : area.getShops().getShop()) {
            if (shop.getKeeper().equals(mob.getVnum())) {
                return shop;
            }
        }
        return null;
    }
    
    /* Also fills Repair constants */
    private void fillShopConstants() {
        for (Itemtype type : itemtypes.getItemtype()) {
            ui.shopType0Box.addItem(type.getName(), type.getValue());
            ui.shopType1Box.addItem(type.getName(), type.getValue());
            ui.shopType2Box.addItem(type.getName(), type.getValue());
            ui.shopType3Box.addItem(type.getName(), type.getValue());
            ui.shopType4Box.addItem(type.getName(), type.getValue());
            ui.repairType0Box.addItem(type.getName(), type.getValue());
            ui.repairType1Box.addItem(type.getName(), type.getValue());
            ui.repairType2Box.addItem(type.getName(), type.getValue());
        }
        for (Type type : repairTypes.getType()) {
            ui.repairShopTypeBox.addItem(type.getName(), type.getValue());
        }
    }

    @SuppressWarnings("unused")
    private void on_shopAddButton_clicked() {
        on_actionCreate_New_Shop_triggered();
    }
    
    @SuppressWarnings("unused")
    private void on_shopDeleteButton_clicked() {
        on_actionDelete_Current_Shop_triggered();
    }
    
    @SuppressWarnings("unused")
    private void on_shopKeeperBox_currentIndexChanged(int idx) {
        if (!shopCanChange) {
            return;
        }

        fillShopData(getShop((Mobile)ui.shopKeeperBox.itemData(idx)));
    }
    
    @SuppressWarnings("unused")
    private void on_shopFlagsButton_clicked() {
        if (!shopCanChange) {
            return;
        }
        new FlagsWidget(shopFlags.getFlag(),new FlagsWrapper(getCurrentShop().getFlags(),
                new IFlagsSetter() {
                    public void setFlags(Long flagsValue) {
                        getCurrentShop().setFlags(flagsValue);
                        ui.shopFlagsEdit.setText(flagsValue.toString());
                        setModified();
                    }
        }),"Shop Flags").show();
    }
    
    @SuppressWarnings("unused")
    private void on_shopFlagsEdit_textChanged(String str) {
        if (!shopCanChange) {
            return;
        }
        try {
            getCurrentShop().setFlags(Long.parseLong(str));
            setModified();
        } catch (NumberFormatException e) {
            if (!str.isEmpty()) {
                QMessageBox.critical(null, "Invalid Number Value", "Shop Flags must have number value!");
                ui.shopFlagsEdit.setText(String.valueOf(getCurrentShop().getFlags()));
            }
        }
    }
    
    @SuppressWarnings("unused")
    private void on_shopOpenBox_valueChanged(int val) {
        if (!shopCanChange) {
            return;
        }
        
        getCurrentShop().setOpen((short)val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopCloseBox_valueChanged(int val) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().setClose((short)val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopProfitBuyBox_valueChanged(int val) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().setProfitbuy(val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopProfitSellBox_valueChanged(int val) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().setProfitsell(val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopType0Box_currentIndexChanged(int idx) {
        if (!shopCanChange) {
            return;
        }
    
        getCurrentShop().getTypes().setType0(((Integer)ui.shopType0Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopType1Box_currentIndexChanged(int idx) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().getTypes().setType1(((Integer)ui.shopType1Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopType2Box_currentIndexChanged(int idx) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().getTypes().setType2(((Integer)ui.shopType2Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopType3Box_currentIndexChanged(int idx) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().getTypes().setType3(((Integer)ui.shopType3Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_shopType4Box_currentIndexChanged(int idx) {
        if (!shopCanChange) {
            return;
        }

        getCurrentShop().getTypes().setType4(((Integer)ui.shopType4Box.itemData(idx)).shortValue());
        setModified();
    }

    @SuppressWarnings("unused")
    private void shopVnumChosen(BigInteger vnum) {
        Shop shop = newShop(vnum);

        area.getShops().getShop().add(shop);
        fillShopData(shop);
        setModified();
        statusBar().showMessage("New shop created.", 5000);
    }
    
    /* Repair Data section */
    private Repair newRepair(BigInteger keeperVnum) {
        ObjectFactory of = new ObjectFactory();
        Repair repair = of.createRepairsRepair();
        
        repair.setKeeper(keeperVnum);
        repair.setOpen((short)10);
        repair.setClose((short)20);
        repair.setProfitfix(100);
        repair.setShoptype(repairTypes.getType().get(0).getValue());
        pl.swmud.ns.swmud._1_0.area.Repairs.Repair.Types types = of.createRepairsRepairTypes();
        types.setType0((short)0);
        types.setType1((short)0);
        types.setType2((short)0);
        repair.setTypes(types);
        
        return repair;
    }

    private void fillRepairData(Repair repair) {
        if (repair == null) {
            clearRepairData();
            return;
        }
        repairCanChange = false;
        int idx = -1;
        int i = 0;
        ui.repairKeeperBox.clear();
        for (Mobile mobile : area.getMobiles().getMobile()) {
            for (Repair r : area.getRepairs().getRepair()) {
                if (r.getKeeper().equals(mobile.getVnum())) {
                    ui.repairKeeperBox.addItem(mobile.getVnum()+" - "+mobile.getName(), mobile);
                    if (r == repair) {
                        idx = i;
                    }
                    i++;
                    break;
                }
            }
        }
        ui.repairKeeperBox.setCurrentIndex(idx);
        
        ui.repairOpenBox.setValue(repair.getOpen());
        ui.repairCloseBox.setValue(repair.getClose());
        ui.repairProfitFixBox.setValue(repair.getProfitfix());
        ui.repairType0Box.setCurrentIndex(ui.repairType0Box.findData((int)repair.getTypes().getType0()));
        ui.repairType1Box.setCurrentIndex(ui.repairType1Box.findData((int)repair.getTypes().getType1()));
        ui.repairType2Box.setCurrentIndex(ui.repairType2Box.findData((int)repair.getTypes().getType2()));
        ui.repairShopTypeBox.setCurrentIndex(ui.repairShopTypeBox.findData(repair.getShoptype()));
        repairCanChange = area.getRepairs().getRepair().size() > 0;
    }

    private void clearRepairData() {
        repairCanChange = false;
        ui.repairKeeperBox.clear();
        ui.repairOpenBox.setValue(0);
        ui.repairCloseBox.setValue(0);
        ui.repairProfitFixBox.setValue(0);
        ui.repairType0Box.setCurrentIndex(-1);
        ui.repairType1Box.setCurrentIndex(-1);
        ui.repairType2Box.setCurrentIndex(-1);
        ui.repairShopTypeBox.setCurrentIndex(-1);
    }

    private Repair getCurrentRepair() {
        if (ui.repairKeeperBox.count() < 1) {
            return null;
        }
        return getRepair((Mobile)ui.repairKeeperBox.itemData(ui.repairKeeperBox.currentIndex()));
    }
    
    private Repair getRepair(Mobile mob) {
        for (Repair repair : area.getRepairs().getRepair()) {
            if (repair.getKeeper().equals(mob.getVnum())) {
                return repair;
            }
        }
        return null;
    }
    
    @SuppressWarnings("unused")
    private void on_repairAddButton_clicked() {
        on_actionCreate_New_Repair_triggered();
    }
    
    @SuppressWarnings("unused")
    private void on_repairDeleteButton_clicked() {
        on_actionDelete_Current_Repair_triggered();
    }
    
    @SuppressWarnings("unused")
    private void on_repairKeeperBox_currentIndexChanged(int idx) {
        if (!repairCanChange) {
            return;
        }

        fillRepairData(getRepair((Mobile)ui.repairKeeperBox.itemData(idx)));
    }
    
    @SuppressWarnings("unused")
    private void on_repairShopTypeBox_currentIndexChanged(int idx) {
        if (!repairCanChange) {
            return;
        }

        getCurrentRepair().setShoptype((Integer)ui.repairShopTypeBox.itemData(idx));
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_repairOpenBox_valueChanged(int val) {
        if (!repairCanChange) {
            return;
        }
        
        getCurrentRepair().setOpen((short)val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_repairCloseBox_valueChanged(int val) {
        if (!repairCanChange) {
            return;
        }

        getCurrentRepair().setClose((short)val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_repairProfitFixBox_valueChanged(int val) {
        if (!repairCanChange) {
            return;
        }

        getCurrentRepair().setProfitfix(val);
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_repairType0Box_currentIndexChanged(int idx) {
        if (!repairCanChange) {
            return;
        }
    
        getCurrentRepair().getTypes().setType0(((Integer)ui.repairType0Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_repairType1Box_currentIndexChanged(int idx) {
        if (!repairCanChange) {
            return;
        }

        getCurrentRepair().getTypes().setType1(((Integer)ui.repairType1Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void on_repairType2Box_currentIndexChanged(int idx) {
        if (!repairCanChange) {
            return;
        }

        getCurrentRepair().getTypes().setType2(((Integer)ui.repairType2Box.itemData(idx)).shortValue());
        setModified();
    }
    
    @SuppressWarnings("unused")
    private void repairVnumChosen(BigInteger vnum) {
        Repair repair = newRepair(vnum);

        area.getRepairs().getRepair().add(repair);
        fillRepairData(repair);
        setModified();
        statusBar().showMessage("New repair created.", 5000);
    }
}

