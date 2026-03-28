#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QSystemTrayIcon>

#include "model/Area.h"
#include "model/ConfigData.h"
#include "map/Mapper.h"
#include "map/MapWidget.h"

namespace Ui {
class SWAEdit;
}

class QComboBox;
class QLineEdit;
class QSpinBox;
class MapWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static constexpr int MAX_RESET_ARGS = 5;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Public so child dialogs can call these
    void setModified();
    void updateResetNavigatorText();

public slots:
    void splashScreenClosed();

    // Accessors for child dialogs
    Area &area() { return area_; }
    const ConfigData &config() const { return config_; }
    const QMap<int, ExitDef> &exitsMap() const { return exitsMap_; }
    const QMap<QString, ResetInfoDef> &resetsMap() const { return resetsMap_; }

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // --- Initialization helpers ---
    void loadConfigData();
    void installToolTipEventFilter();
    void setupConnections();
    void setUpItemValueLabels();
    void fillItemTypes();
    void fillMobileSex();
    void createRoomConstants();
    void fillRoomSectors();
    void resetCreateConstants();
    void fillShopConstants();
    void setSystemTray();

    // --- Common / state ---
    void setNotModified();
    bool canLeaveCurrent();

    // --- File operations ---
    void openArea();
    void saveArea();
    void saveAreaAs();
    void timerBackup();

    // --- Master fill ---
    void fillAll();

    // --- Area Data (Head) ---
    void fillAreaData();
    void clearAreaData();

    // --- Item Data ---
    AreaObject *getCurrentObject();
    AreaObject newItem();
    void fillItemData(int objectIndex);
    void clearItemData();
    void setItemValues(int type);
    void clearItemLabelValues();
    void createItemValueEdit(int no);
    void createItemValueTypeEdit(const ItemTypeValueDef &value);
    void createItemValueFlagsWidget(const ItemTypeValueDef &value);
    void itemAddValueWidget(int no, QWidget *widget);
    int getItemValueIndex(QComboBox *cbox, int no);

    // --- Mobile Data ---
    Mobile *getCurrentMob();
    Mobile newMobile();
    void fillMobileData(int mobIndex);
    void clearMobileData();

    // --- Room Data ---
    Room *getCurrentRoom();
    Room newRoom();
    void fillRoomData(int roomIndex);
    void clearRoomData();
    void fillRoomTeleVnum(Room *room);

    // --- Room Exit Data ---
    Exit *getCurrentExit();
    void fillExitData(Room *room);
    void fillExit(Exit *exit);
    void fillExitKeys(Exit *exit);
    void setLastExitIndex();

    // --- Reset Data ---
    AreaReset *getCurrentReset();
    void fillResetData(int resetIndex);
    void clearResetData();
    QString prepareResetStr(const AreaReset &reset);
    void resetCreateWidgets(AreaReset *reset);
    void resetAddWidget(QWidget *w, const QString &argName, int argIndex);
    void resetCreateVnumWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex);
    void resetCreateIntWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex);
    void resetCreateStringWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex);
    void resetCreateTypeWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex);
    void resetCreateFlagsWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex);
    void setupResetLabels(const AreaReset &reset);
    QString prepareLabelName(const QString &argName, int idx);
    bool resetIsOtherVnum(int vnum, AreaReset *reset, const ResetInfoDef &resDef, int argIndex);
    void addOtherVnum(QComboBox *box, int vnum);
    int getResetArgValue(const AreaReset &reset, int argIndex);
    void setResetArgValue(AreaReset *reset, int argIndex, int value);
    const ResetArgDef &getResetArgDef(const ResetInfoDef &resDef, int argIndex);
    QString getResetArgName(int argIndex);

    // --- Shop Data ---
    Shop *getCurrentShop();
    Shop *getShopByKeeperVnum(int keeperVnum);
    Shop newShop(int keeperVnum);
    void fillShopData(int shopIndex);
    void clearShopData();

    // --- Repair Data ---
    Repair *getCurrentRepair();
    Repair *getRepairByKeeperVnum(int keeperVnum);
    Repair newRepair(int keeperVnum);
    void fillRepairData(int repairIndex);
    void clearRepairData();

    // --- Entity creation helpers ---
    ShortDesc newShortDesc();
    int findNextFreeVnum(const QString &entityType);

    // --- Map helpers ---
    void mapRoomVnumSelected(int vnum);
    void mapRoomExitSelected(int ownerRoomVnum, int exitDirection, int destRoomVnum);

    // --- Renumber ---
    void renumber(int newFirstVnum, int optionsFlags);

    // --- Utility ---
    static void centerChildOnParent(QWidget *child, QWidget *parent);

private slots:
    // --- Menu action slots ---
    void onActionQuit();
    void onActionOpenArea();
    void onActionSaveArea();
    void onActionSaveAreaAs();
    void onActionCreateNewArea();
    void onActionRenumber();
    void onActionCreateNewItem();
    void onActionDeleteCurrentItem();
    void onActionCreateNewMobile();
    void onActionDeleteCurrentMobile();
    void onActionCreateNewRoom();
    void onActionDeleteCurrentRoom();
    void onActionShowMap();
    void onActionCreateNewReset();
    void onActionDeleteCurrentReset();
    void onActionCreateNewShop();
    void onActionDeleteCurrentShop();
    void onActionCreateNewRepair();
    void onActionDeleteCurrentRepair();
    void onActionMessages();

    // --- Area Data (Head) field slots ---
    void onNameEditTextChanged(const QString &str);
    void onAuthorsEditTextChanged(const QString &str);
    void onBuildersEditTextChanged(const QString &str);
    void onSecuritySpinBoxValueChanged(int value);
    void onLvnumEditTextChanged(const QString &str);
    void onUvnumEditTextChanged(const QString &str);
    void onFlagsEditTextChanged(const QString &str);
    void onFlagsButtonClicked();
    void onLeconomyEditTextChanged(const QString &str);
    void onHeconomyEditTextChanged(const QString &str);
    void onResetFreqEditTextChanged(const QString &str);
    void onResetMsgEditTextChanged(const QString &str);
    void onLrangeSpinBoxValueChanged(int value);
    void onHrangeSpinBoxValueChanged(int value);

    // --- Item Data field slots ---
    void onItemNavigatorComboBoxCurrentIndexChanged(int idx);
    void onItemTypeComboBoxCurrentIndexChanged(int idx);
    void onItemGenderComboBoxCurrentIndexChanged(int idx);
    void onItemVnumEditTextChanged(const QString &str);
    void onItemNameEditTextChanged(const QString &str);
    void onItemDescriptionTextChanged(const QString &str);
    void onItemActionDescriptionTextChanged(const QString &str);
    void onItemExtraFlagsEditTextChanged(const QString &str);
    void onItemExtraFlagsButtonClicked();
    void onItemWearFlagsEditTextChanged(const QString &str);
    void onItemWearFlagsButtonClicked();
    void onItemInflect0EditTextChanged(const QString &str);
    void onItemInflect1EditTextChanged(const QString &str);
    void onItemInflect2EditTextChanged(const QString &str);
    void onItemInflect3EditTextChanged(const QString &str);
    void onItemInflect4EditTextChanged(const QString &str);
    void onItemInflect5EditTextChanged(const QString &str);
    void onItemWeightEditTextChanged(const QString &str);
    void onItemCostEditTextChanged(const QString &str);
    void onItemLevelSpinBoxValueChanged(int value);
    void onItemLayersEditTextChanged(const QString &str);
    void onItemExtraDescriptionButtonClicked();
    void onItemProgramsButtonClicked();
    void onItemAffectsButtonClicked();
    void onItemRequirementsButtonClicked();

    // Dynamic item value widget slots (connected at runtime)
    void itemValueEditChanged(const QString &str);
    void itemValueComboBoxChanged(int idx);
    void itemValueFlagsButtonClicked();

    // --- Mobile Data field slots ---
    void onMobNavigatorComboBoxCurrentIndexChanged(int idx);
    void onMobNameEditTextChanged(const QString &str);
    void onMobLongDescriptionEditTextChanged(const QString &str);
    void onMobDescriptionTextChanged();
    void onMobSexComboBoxCurrentIndexChanged(int idx);
    void onMobRaceComboBoxCurrentIndexChanged(int idx);
    void onMobPositionComboBoxCurrentIndexChanged(int idx);
    void onMobSpeakingComboBoxCurrentIndexChanged(int idx);
    void onMobVipFlagsComboBoxCurrentIndexChanged(int idx);
    void onMobActFlagsButtonClicked();
    void onMobAffectedFlagsButtonClicked();
    void onMobXFlagsButtonClicked();
    void onMobResistancesButtonClicked();
    void onMobImmunitiesButtonClicked();
    void onMobSusceptibilitiesButtonClicked();
    void onMobAttacksButtonClicked();
    void onMobDefensesButtonClicked();
    void onMobProgramsButtonClicked();
    void onMobSpecialsButtonClicked();
    void onMobCreditsEditTextChanged(const QString &str);
    void onMobLevelSpinBoxValueChanged(int value);
    void onMobAlignmentBoxValueChanged(int value);
    void onMobAttacksNoBoxValueChanged(int value);
    void onMobHeightBoxValueChanged(int value);
    void onMobWeightBoxValueChanged(int value);
    void onMobHitRollBoxValueChanged(int value);
    void onMobDamRollBoxValueChanged(int value);
    void onMobArmorClassBoxValueChanged(int value);
    void onMobHitNoDiceBoxValueChanged(int value);
    void onMobHitSizeDiceBoxValueChanged(int value);
    void onMobHitPlusBoxValueChanged(int value);
    void onMobDamNoDiceBoxValueChanged(int value);
    void onMobDamSizeDiceBoxValueChanged(int value);
    void onMobDamPlusBoxValueChanged(int value);
    void onMobStrBoxValueChanged(int value);
    void onMobIntBoxValueChanged(int value);
    void onMobWisBoxValueChanged(int value);
    void onMobDexBoxValueChanged(int value);
    void onMobConBoxValueChanged(int value);
    void onMobChaBoxValueChanged(int value);
    void onMobLckBoxValueChanged(int value);
    void onMobInflect0EditTextChanged(const QString &str);
    void onMobInflect1EditTextChanged(const QString &str);
    void onMobInflect2EditTextChanged(const QString &str);
    void onMobInflect3EditTextChanged(const QString &str);
    void onMobInflect4EditTextChanged(const QString &str);
    void onMobInflect5EditTextChanged(const QString &str);
    void onMobDialogNameEditTextChanged(const QString &str);

    // --- Room Data field slots ---
    void onRoomNavigatorComboBoxCurrentIndexChanged(int idx);
    void onRoomNameEditTextChanged(const QString &str);
    void onRoomDescriptionTextChanged();
    void onRoomNightDescriptionTextChanged();
    void onRoomSectorComboBoxCurrentIndexChanged(int idx);
    void onRoomLightSpinBoxValueChanged(int value);
    void onRoomFlagsEditTextChanged(const QString &str);
    void onRoomFlagsButtonClicked();
    void onRoomTeleVnumComboBoxCurrentIndexChanged(int idx);
    void onRoomTunnelSpinBoxValueChanged(int value);
    void onRoomTeleDelaySpinBoxValueChanged(int value);
    void onRoomExtraDescriptionButtonClicked();
    void onRoomProgramsButtonClicked();

    // --- Room Exit field slots ---
    void onRoomExitNavigatorComboBoxCurrentIndexChanged(int idx);
    void onRoomExitAddButtonClicked();
    void onRoomExitDeleteButtonClicked();
    void onRoomExitKeywordEditTextChanged(const QString &str);
    void onRoomExitDescriptionEditTextChanged(const QString &str);
    void onRoomExitFlagsEditTextChanged(const QString &str);
    void onRoomExitFlagsButtonClicked();
    void onRoomExitKeyComboBoxCurrentIndexChanged(int idx);
    void onRoomExitDistanceSpinBoxValueChanged(int value);

    // --- Reset Data field slots ---
    void onResetNavigatorComboBoxCurrentIndexChanged(int idx);
    void onResetAddButtonClicked();
    void onResetDeleteButtonClicked();

    // Dynamic reset arg widget slots (connected at runtime)
    void resetArgIntChanged(int value);
    void resetArgStringChanged(const QString &str);
    void resetArgVnumChanged(int idx);
    void resetArgTypesChanged(int idx);
    void resetArgFlagsClicked();

    // --- Shop Data field slots ---
    void onShopKeeperBoxCurrentIndexChanged(int idx);
    void onShopFlagsEditTextChanged(const QString &str);
    void onShopFlagsButtonClicked();
    void onShopOpenBoxValueChanged(int val);
    void onShopCloseBoxValueChanged(int val);
    void onShopProfitBuyBoxValueChanged(int val);
    void onShopProfitSellBoxValueChanged(int val);
    void onShopType0BoxCurrentIndexChanged(int idx);
    void onShopType1BoxCurrentIndexChanged(int idx);
    void onShopType2BoxCurrentIndexChanged(int idx);
    void onShopType3BoxCurrentIndexChanged(int idx);
    void onShopType4BoxCurrentIndexChanged(int idx);
    void onShopAddButtonClicked();
    void onShopDeleteButtonClicked();

    // --- Repair Data field slots ---
    void onRepairKeeperBoxCurrentIndexChanged(int idx);
    void onRepairShopTypeBoxCurrentIndexChanged(int idx);
    void onRepairOpenBoxValueChanged(int val);
    void onRepairCloseBoxValueChanged(int val);
    void onRepairProfitFixBoxValueChanged(int val);
    void onRepairType0BoxCurrentIndexChanged(int idx);
    void onRepairType1BoxCurrentIndexChanged(int idx);
    void onRepairType2BoxCurrentIndexChanged(int idx);
    void onRepairAddButtonClicked();
    void onRepairDeleteButtonClicked();

    // --- Callback slots (from child dialogs/widgets) ---
    void areaCreated();
    void resetCreated();
    void shopVnumChosen(int vnum);
    void repairVnumChosen(int vnum);
    void mapClosed();
    void mapRefreshed();
    void mapRoomMarkingFlagRequested();
    void mapRoomMarkingFlagAccepted(qint64 value);
    void sysTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    // --- UI ---
    Ui::SWAEdit *ui;

    // --- Data ---
    ConfigData config_;
    Area area_;
    bool areaLoaded_ = false;

    // --- File state ---
    QString currentFileName_;
    bool modified_ = false;

    // --- Change guard bools (prevent recursive signal handling during fills) ---
    bool headCanChange_ = false;
    bool itemCanChange_ = false;
    bool mobCanChange_ = false;
    bool roomCanChange_ = false;
    bool resetCanChange_ = false;
    bool shopCanChange_ = false;
    bool repairCanChange_ = false;

    // --- Item value dynamic widgets ---
    // Labels (itemValue0..itemValue5) are in the UI; we cache pointers here.
    QLabel *valueLabels_[6] = {};
    QWidget *valueWidgets_[6] = {};
    QHBoxLayout *valueLayouts_[6] = {};

    // --- Reset dynamic widgets ---
    // Keys: "extra", "arg1", "arg2", "arg3", "arg4"
    QMap<QString, QLabel *> resetLabels_;
    QMap<QString, QWidget *> resetWidgets_;
    QMap<QString, QHBoxLayout *> resetLayouts_;

    // --- Exits lookup ---
    // Maps exit direction value -> ExitDef from config
    QMap<int, ExitDef> exitsMap_;

    // --- Resets lookup ---
    // Maps reset command string (e.g. "M", "O") -> ResetInfoDef from config
    QMap<QString, ResetInfoDef> resetsMap_;

    // --- Key item type value (for exit key filtering) ---
    int keyValue_ = -1;

    // --- System tray ---
    QSystemTrayIcon *sysTray_ = nullptr;

    // --- Backup timer ---
    QTimer backupTimer_;

    // --- Map ---
    MapWidget *mapWidget_ = nullptr;
    bool mapSelection_ = false;
};

#endif // MAINWINDOW_H
