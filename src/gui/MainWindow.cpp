#include "gui/MainWindow.h"
#include "ui_SWAEdit.h"
#include "core/XmlIO.h"
#include "core/Renumberer.h"

#include "map/Mapper.h"
#include "map/MapWidget.h"
#include "map/ExitWrapper.h"

#include "gui/FlagsWidget.h"
#include "gui/ExtraDescWidget.h"
#include "gui/ProgramsWidget.h"
#include "gui/ValueFlagsWidget.h"
#include "gui/ResetFlagsWidget.h"
#include "gui/NewExitWidget.h"
#include "gui/NewResetWidget.h"
#include "gui/NewShopWidget.h"
#include "gui/NewAreaWidget.h"
#include "gui/MobileSpecialsWidget.h"
#include "gui/RenumberWidget.h"
#include "gui/RenumberWarningsWidget.h"
#include "gui/VnumQuestionWidget.h"
#include "gui/ResetVnumEventFilter.h"
#include "gui/ToolTipEventFilter.h"

#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QLabel>
#include <QTextBrowser>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

// ===========================================================================
// Constructor / Destructor
// ===========================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SWAEdit)
{
    ui->setupUi(this);

    loadConfigData();
    setUpItemValueLabels();
    fillItemTypes();
    createRoomConstants();
    fillMobileSex();
    resetCreateConstants();
    fillShopConstants();
    setSystemTray();
    setupConnections();
    installToolTipEventFilter();

    connect(&backupTimer_, &QTimer::timeout, this, &MainWindow::timerBackup);
}

MainWindow::~MainWindow()
{
    delete sysTray_;
    delete ui;
}

// ===========================================================================
// Initialization helpers
// ===========================================================================

void MainWindow::loadConfigData()
{
    QString dataDir = QCoreApplication::applicationDirPath() + "/data/";
    QStringList errors;
    config_ = XmlIO::loadAllConfig(dataDir, &errors);
    if (!errors.isEmpty()) {
        QMessageBox::critical(this, "Config Validation Error",
            QStringLiteral("Configuration files failed XSD validation. "
                           "The application may not work correctly.\n\n%1")
                .arg(errors.join("\n\n")));
    }
}

void MainWindow::installToolTipEventFilter()
{
    ToolTipEventFilter *ttef = new ToolTipEventFilter(this);
    const QWidgetList allWidgets = QApplication::allWidgets();
    for (QWidget *w : allWidgets) {
        if (qobject_cast<QAbstractButton *>(w)
                || qobject_cast<QLabel *>(w)
                || qobject_cast<QLineEdit *>(w)
                || qobject_cast<QTextEdit *>(w)
                || qobject_cast<QTextBrowser *>(w)
                || qobject_cast<QAbstractSpinBox *>(w)
                || qobject_cast<QComboBox *>(w)) {
            w->installEventFilter(ttef);
        }
    }
}

void MainWindow::setUpItemValueLabels()
{
    valueLabels_[0] = ui->itemValue0;
    valueLabels_[1] = ui->itemValue1;
    valueLabels_[2] = ui->itemValue2;
    valueLabels_[3] = ui->itemValue3;
    valueLabels_[4] = ui->itemValue4;
    valueLabels_[5] = ui->itemValue5;

    // Each label lives in a QHBoxLayout row inside the itemValueBox QGroupBox's
    // QVBoxLayout.  Walk the group box layout to find the QHBoxLayout that
    // contains each label and cache it.
    QGroupBox *box = ui->itemValueBox;
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout *>(box->layout());
    for (int i = 0; i < 6; ++i) {
        valueLayouts_[i] = nullptr;
        valueWidgets_[i] = nullptr;
    }
    if (vbox) {
        for (int j = 0; j < vbox->count(); ++j) {
            QHBoxLayout *hbox = qobject_cast<QHBoxLayout *>(vbox->itemAt(j)->layout());
            if (!hbox) continue;
            for (int k = 0; k < hbox->count(); ++k) {
                QWidget *w = hbox->itemAt(k)->widget();
                if (!w) continue;
                for (int i = 0; i < 6; ++i) {
                    if (w == valueLabels_[i]) {
                        valueLayouts_[i] = hbox;
                    }
                }
            }
        }
    }
}

void MainWindow::fillItemTypes()
{
    // Fill item type combo box (all types, matching Java)
    ui->itemTypeComboBox->clear();
    for (const ItemTypeDef &t : config_.itemTypes) {
        ui->itemTypeComboBox->addItem(t.name, t.value);
    }

    // Fill item gender combo box (hardcoded as in original Java)
    ui->itemGenderComboBox->clear();
    ui->itemGenderComboBox->addItem("neutral", 0);
    ui->itemGenderComboBox->addItem("male", 1);
    ui->itemGenderComboBox->addItem("female", 2);
    ui->itemGenderComboBox->addItem("plural", 3);
}

void MainWindow::createRoomConstants()
{
    fillRoomSectors();

    // Build exits map from config
    exitsMap_.clear();
    for (const ExitDef &e : config_.exits) {
        exitsMap_[e.value] = e;
    }

    // Find keyValue: scan item types for "key" type
    keyValue_ = -1;
    for (const ItemTypeDef &t : config_.itemTypes) {
        if (t.name.toLower() == "key") {
            keyValue_ = t.value;
            break;
        }
    }
}

void MainWindow::fillRoomSectors()
{
    ui->roomSectorComboBox->clear();
    for (const TypeDef &s : config_.roomSectorTypes) {
        ui->roomSectorComboBox->addItem(s.name, s.value);
    }
}

void MainWindow::fillMobileSex()
{
    // Sex combo box (hardcoded as in original Java)
    ui->mobSexComboBox->clear();
    ui->mobSexComboBox->addItem("neutral", 0);
    ui->mobSexComboBox->addItem("male", 1);
    ui->mobSexComboBox->addItem("female", 2);

    // Race combo box
    ui->mobRaceComboBox->clear();
    for (const QString &r : config_.races) {
        ui->mobRaceComboBox->addItem(r);
    }

    // Speaking combo box
    ui->mobSpeakingComboBox->clear();
    for (const QString &l : config_.languages) {
        ui->mobSpeakingComboBox->addItem(l);
    }

    // VIP flags combo box
    ui->mobVipFlagsComboBox->clear();
    ui->mobVipFlagsComboBox->addItem("", 0);
    for (const QString &p : config_.planets) {
        ui->mobVipFlagsComboBox->addItem(p);
    }

    // Position combo box
    ui->mobPositionComboBox->clear();
    for (const TypeDef &p : config_.positions) {
        ui->mobPositionComboBox->addItem(p.name, p.value);
    }
}

void MainWindow::resetCreateConstants()
{
    // Build resets map: command string -> ResetInfoDef
    resetsMap_.clear();
    for (const ResetInfoDef &r : config_.resetsInfo) {
        resetsMap_[r.value] = r;
    }

    // Cache label pointers for the reset parameter rows.
    resetLabels_["extra"] = ui->lResetExtra;
    resetLabels_["arg1"]  = ui->lResetArg1;
    resetLabels_["arg2"]  = ui->lResetArg2;
    resetLabels_["arg3"]  = ui->lResetArg3;
    resetLabels_["arg4"]  = ui->lResetArg4;

    // Find the QHBoxLayouts that contain these labels.
    QGroupBox *box = ui->resetParametersBox;
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout *>(box->layout());
    if (vbox) {
        for (int i = 0; i < vbox->count(); ++i) {
            QHBoxLayout *hbox = qobject_cast<QHBoxLayout *>(vbox->itemAt(i)->layout());
            if (!hbox) continue;
            for (int k = 0; k < hbox->count(); ++k) {
                QWidget *w = hbox->itemAt(k)->widget();
                if (!w) continue;
                if (w == ui->lResetExtra)     resetLayouts_["extra"] = hbox;
                else if (w == ui->lResetArg1) resetLayouts_["arg1"]  = hbox;
                else if (w == ui->lResetArg2) resetLayouts_["arg2"]  = hbox;
                else if (w == ui->lResetArg3) resetLayouts_["arg3"]  = hbox;
                else if (w == ui->lResetArg4) resetLayouts_["arg4"]  = hbox;
            }
        }
    }

    // Initialize widget pointers to null
    resetWidgets_["extra"] = nullptr;
    resetWidgets_["arg1"]  = nullptr;
    resetWidgets_["arg2"]  = nullptr;
    resetWidgets_["arg3"]  = nullptr;
    resetWidgets_["arg4"]  = nullptr;
}

void MainWindow::fillShopConstants()
{
    // Shop type combo boxes (5 of them)
    QComboBox *shopBoxes[] = {
        ui->shopType0Box, ui->shopType1Box, ui->shopType2Box,
        ui->shopType3Box, ui->shopType4Box
    };
    for (int i = 0; i < 5; ++i) {
        shopBoxes[i]->clear();
        for (const ItemTypeDef &t : config_.itemTypes) {
            shopBoxes[i]->addItem(t.name, t.value);
        }
    }

    // Repair type combo boxes (3 of them)
    QComboBox *repairBoxes[] = {
        ui->repairType0Box, ui->repairType1Box, ui->repairType2Box
    };
    for (int i = 0; i < 3; ++i) {
        repairBoxes[i]->clear();
        for (const ItemTypeDef &t : config_.itemTypes) {
            repairBoxes[i]->addItem(t.name, t.value);
        }
    }

    // Repair shop type combo box
    ui->repairShopTypeBox->clear();
    for (const TypeDef &t : config_.repairTypes) {
        ui->repairShopTypeBox->addItem(t.name, t.value);
    }
}

void MainWindow::setSystemTray()
{
    sysTray_ = new QSystemTrayIcon(QIcon("images/icon.png"), this);

    QMenu *trayMenu = new QMenu(this);
    QAction *quitAction = trayMenu->addAction("Quit");
    quitAction->setShortcut(ui->actionQuit->shortcut());
    connect(quitAction, &QAction::triggered, this, &MainWindow::onActionQuit);
    sysTray_->setContextMenu(trayMenu);

    connect(sysTray_, &QSystemTrayIcon::activated,
            this, &MainWindow::sysTrayActivated);

    sysTray_->show();
}

// ===========================================================================
// setupConnections
// ===========================================================================

void MainWindow::setupConnections()
{
    // --- Menu actions ---
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::onActionQuit);
    connect(ui->actionOpen_Area, &QAction::triggered, this, &MainWindow::onActionOpenArea);
    connect(ui->actionSave_Area, &QAction::triggered, this, &MainWindow::onActionSaveArea);
    connect(ui->actionSave_Area_As, &QAction::triggered, this, &MainWindow::onActionSaveAreaAs);
    connect(ui->actionCreate_New_Area, &QAction::triggered, this, &MainWindow::onActionCreateNewArea);
    connect(ui->actionRenumber, &QAction::triggered, this, &MainWindow::onActionRenumber);
    connect(ui->actionCreate_New_Item, &QAction::triggered, this, &MainWindow::onActionCreateNewItem);
    connect(ui->actionDelete_Current_Item, &QAction::triggered, this, &MainWindow::onActionDeleteCurrentItem);
    connect(ui->actionCreate_New_Mobile, &QAction::triggered, this, &MainWindow::onActionCreateNewMobile);
    connect(ui->actionDelete_Current_Mobile, &QAction::triggered, this, &MainWindow::onActionDeleteCurrentMobile);
    connect(ui->actionCreate_New_Room, &QAction::triggered, this, &MainWindow::onActionCreateNewRoom);
    connect(ui->actionDelete_Current_Room, &QAction::triggered, this, &MainWindow::onActionDeleteCurrentRoom);
    connect(ui->actionShow_Map, &QAction::triggered, this, &MainWindow::onActionShowMap);
    connect(ui->actionCreate_New_Reset, &QAction::triggered, this, &MainWindow::onActionCreateNewReset);
    connect(ui->actionDelete_Current_Reset, &QAction::triggered, this, &MainWindow::onActionDeleteCurrentReset);
    connect(ui->actionCreate_New_Shop, &QAction::triggered, this, &MainWindow::onActionCreateNewShop);
    connect(ui->actionDelete_Current_Shop, &QAction::triggered, this, &MainWindow::onActionDeleteCurrentShop);
    connect(ui->actionCreate_New_Repair, &QAction::triggered, this, &MainWindow::onActionCreateNewRepair);
    connect(ui->actionDelete_Current_Repair, &QAction::triggered, this, &MainWindow::onActionDeleteCurrentRepair);
    connect(ui->actionMessages, &QAction::triggered, this, &MainWindow::onActionMessages);

    // --- Area Data (Head) field signals ---
    connect(ui->nameEdit, &QLineEdit::textChanged, this, &MainWindow::onNameEditTextChanged);
    connect(ui->authorsEdit, &QLineEdit::textChanged, this, &MainWindow::onAuthorsEditTextChanged);
    connect(ui->buildersEdit, &QLineEdit::textChanged, this, &MainWindow::onBuildersEditTextChanged);
    connect(ui->securitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSecuritySpinBoxValueChanged);
    connect(ui->lvnumEdit, &QLineEdit::textChanged, this, &MainWindow::onLvnumEditTextChanged);
    connect(ui->uvnumEdit, &QLineEdit::textChanged, this, &MainWindow::onUvnumEditTextChanged);
    connect(ui->flagsEdit, &QLineEdit::textChanged, this, &MainWindow::onFlagsEditTextChanged);
    connect(ui->flagsButton, &QPushButton::clicked, this, &MainWindow::onFlagsButtonClicked);
    connect(ui->leconomyEdit, &QLineEdit::textChanged, this, &MainWindow::onLeconomyEditTextChanged);
    connect(ui->heconomyEdit, &QLineEdit::textChanged, this, &MainWindow::onHeconomyEditTextChanged);
    connect(ui->resetFreqEdit, &QLineEdit::textChanged, this, &MainWindow::onResetFreqEditTextChanged);
    connect(ui->resetMsgEdit, &QLineEdit::textChanged, this, &MainWindow::onResetMsgEditTextChanged);
    connect(ui->lrangeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onLrangeSpinBoxValueChanged);
    connect(ui->hrangeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onHrangeSpinBoxValueChanged);

    // --- Item Data field signals ---
    connect(ui->itemNavigatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onItemNavigatorComboBoxCurrentIndexChanged);
    connect(ui->itemTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onItemTypeComboBoxCurrentIndexChanged);
    connect(ui->itemGenderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onItemGenderComboBoxCurrentIndexChanged);
    connect(ui->itemVnumEdit, &QLineEdit::textChanged, this, &MainWindow::onItemVnumEditTextChanged);
    connect(ui->itemNameEdit, &QLineEdit::textChanged, this, &MainWindow::onItemNameEditTextChanged);
    connect(ui->itemDescription, &QLineEdit::textChanged, this, &MainWindow::onItemDescriptionTextChanged);
    connect(ui->itemActionDescription, &QLineEdit::textChanged, this, &MainWindow::onItemActionDescriptionTextChanged);
    connect(ui->itemExtraFlagsEdit, &QLineEdit::textChanged, this, &MainWindow::onItemExtraFlagsEditTextChanged);
    connect(ui->itemExtraFlagsButton, &QPushButton::clicked, this, &MainWindow::onItemExtraFlagsButtonClicked);
    connect(ui->itemWearFlagsEdit, &QLineEdit::textChanged, this, &MainWindow::onItemWearFlagsEditTextChanged);
    connect(ui->itemWearFlagsButton, &QPushButton::clicked, this, &MainWindow::onItemWearFlagsButtonClicked);
    connect(ui->itemInflect0Edit, &QLineEdit::textChanged, this, &MainWindow::onItemInflect0EditTextChanged);
    connect(ui->itemInflect1Edit, &QLineEdit::textChanged, this, &MainWindow::onItemInflect1EditTextChanged);
    connect(ui->itemInflect2Edit, &QLineEdit::textChanged, this, &MainWindow::onItemInflect2EditTextChanged);
    connect(ui->itemInflect3Edit, &QLineEdit::textChanged, this, &MainWindow::onItemInflect3EditTextChanged);
    connect(ui->itemInflect4Edit, &QLineEdit::textChanged, this, &MainWindow::onItemInflect4EditTextChanged);
    connect(ui->itemInflect5Edit, &QLineEdit::textChanged, this, &MainWindow::onItemInflect5EditTextChanged);
    connect(ui->itemWeightEdit, &QLineEdit::textChanged, this, &MainWindow::onItemWeightEditTextChanged);
    connect(ui->itemCostEdit, &QLineEdit::textChanged, this, &MainWindow::onItemCostEditTextChanged);
    connect(ui->itemLevelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onItemLevelSpinBoxValueChanged);
    connect(ui->itemLayersEdit, &QLineEdit::textChanged, this, &MainWindow::onItemLayersEditTextChanged);
    connect(ui->itemExtraDescriptionButton, &QPushButton::clicked, this, &MainWindow::onItemExtraDescriptionButtonClicked);
    connect(ui->itemProgramsButton, &QPushButton::clicked, this, &MainWindow::onItemProgramsButtonClicked);
    connect(ui->itemAffectsButton, &QPushButton::clicked, this, &MainWindow::onItemAffectsButtonClicked);
    connect(ui->itemRequirementsButton, &QPushButton::clicked, this, &MainWindow::onItemRequirementsButtonClicked);

    // --- Mobile Data field signals ---
    connect(ui->mobNavigatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMobNavigatorComboBoxCurrentIndexChanged);
    connect(ui->mobNameEdit, &QLineEdit::textChanged, this, &MainWindow::onMobNameEditTextChanged);
    connect(ui->mobLongDescriptionEdit, &QLineEdit::textChanged, this, &MainWindow::onMobLongDescriptionEditTextChanged);
    connect(ui->mobDescriptionText, &QTextEdit::textChanged, this, &MainWindow::onMobDescriptionTextChanged);
    connect(ui->mobSexComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMobSexComboBoxCurrentIndexChanged);
    connect(ui->mobRaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMobRaceComboBoxCurrentIndexChanged);
    connect(ui->mobPositionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMobPositionComboBoxCurrentIndexChanged);
    connect(ui->mobSpeakingComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMobSpeakingComboBoxCurrentIndexChanged);
    connect(ui->mobVipFlagsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMobVipFlagsComboBoxCurrentIndexChanged);
    connect(ui->mobActFlagsButton, &QPushButton::clicked, this, &MainWindow::onMobActFlagsButtonClicked);
    connect(ui->mobAffectedFlagsButton, &QPushButton::clicked, this, &MainWindow::onMobAffectedFlagsButtonClicked);
    connect(ui->mobXFlagsButton, &QPushButton::clicked, this, &MainWindow::onMobXFlagsButtonClicked);
    connect(ui->mobResistancesButton, &QPushButton::clicked, this, &MainWindow::onMobResistancesButtonClicked);
    connect(ui->mobImmunitiesButton, &QPushButton::clicked, this, &MainWindow::onMobImmunitiesButtonClicked);
    connect(ui->mobSusceptibilitiesButton, &QPushButton::clicked, this, &MainWindow::onMobSusceptibilitiesButtonClicked);
    connect(ui->mobAttacksButton, &QPushButton::clicked, this, &MainWindow::onMobAttacksButtonClicked);
    connect(ui->mobDefensesButton, &QPushButton::clicked, this, &MainWindow::onMobDefensesButtonClicked);
    connect(ui->mobProgramsButton, &QPushButton::clicked, this, &MainWindow::onMobProgramsButtonClicked);
    connect(ui->mobSpecialsButton, &QPushButton::clicked, this, &MainWindow::onMobSpecialsButtonClicked);
    connect(ui->mobCreditsEdit, &QLineEdit::textChanged, this, &MainWindow::onMobCreditsEditTextChanged);
    connect(ui->mobLevelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobLevelSpinBoxValueChanged);
    connect(ui->mobAlignmentBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobAlignmentBoxValueChanged);
    connect(ui->mobAttacksNoBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobAttacksNoBoxValueChanged);
    connect(ui->mobHeightBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobHeightBoxValueChanged);
    connect(ui->mobWeightBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobWeightBoxValueChanged);
    connect(ui->mobHitRollBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobHitRollBoxValueChanged);
    connect(ui->mobDamRollBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobDamRollBoxValueChanged);
    connect(ui->mobArmorClassBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobArmorClassBoxValueChanged);
    connect(ui->mobHitNoDiceBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobHitNoDiceBoxValueChanged);
    connect(ui->mobHitSizeDiceBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobHitSizeDiceBoxValueChanged);
    connect(ui->mobHitPlusBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobHitPlusBoxValueChanged);
    connect(ui->mobDamNoDiceBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobDamNoDiceBoxValueChanged);
    connect(ui->mobDamSizeDiceBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobDamSizeDiceBoxValueChanged);
    connect(ui->mobDamPlusBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobDamPlusBoxValueChanged);
    connect(ui->mobStrBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobStrBoxValueChanged);
    connect(ui->mobIntBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobIntBoxValueChanged);
    connect(ui->mobWisBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobWisBoxValueChanged);
    connect(ui->mobDexBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobDexBoxValueChanged);
    connect(ui->mobConBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobConBoxValueChanged);
    connect(ui->mobChaBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobChaBoxValueChanged);
    connect(ui->mobLckBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onMobLckBoxValueChanged);
    connect(ui->mobInflect0Edit, &QLineEdit::textChanged, this, &MainWindow::onMobInflect0EditTextChanged);
    connect(ui->mobInflect1Edit, &QLineEdit::textChanged, this, &MainWindow::onMobInflect1EditTextChanged);
    connect(ui->mobInflect2Edit, &QLineEdit::textChanged, this, &MainWindow::onMobInflect2EditTextChanged);
    connect(ui->mobInflect3Edit, &QLineEdit::textChanged, this, &MainWindow::onMobInflect3EditTextChanged);
    connect(ui->mobInflect4Edit, &QLineEdit::textChanged, this, &MainWindow::onMobInflect4EditTextChanged);
    connect(ui->mobInflect5Edit, &QLineEdit::textChanged, this, &MainWindow::onMobInflect5EditTextChanged);
    connect(ui->mobDialogNameEdit, &QLineEdit::textChanged, this, &MainWindow::onMobDialogNameEditTextChanged);

    // --- Room Data field signals ---
    connect(ui->roomNavigatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRoomNavigatorComboBoxCurrentIndexChanged);
    connect(ui->roomNameEdit, &QLineEdit::textChanged, this, &MainWindow::onRoomNameEditTextChanged);
    connect(ui->roomDescriptionText, &QTextEdit::textChanged, this, &MainWindow::onRoomDescriptionTextChanged);
    connect(ui->roomNightDescriptionText, &QTextEdit::textChanged, this, &MainWindow::onRoomNightDescriptionTextChanged);
    connect(ui->roomSectorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRoomSectorComboBoxCurrentIndexChanged);
    connect(ui->roomLightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRoomLightSpinBoxValueChanged);
    connect(ui->roomFlagsEdit, &QLineEdit::textChanged, this, &MainWindow::onRoomFlagsEditTextChanged);
    connect(ui->roomFlagsButton, &QPushButton::clicked, this, &MainWindow::onRoomFlagsButtonClicked);
    connect(ui->roomTeleVnumComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRoomTeleVnumComboBoxCurrentIndexChanged);
    connect(ui->roomTunnelSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRoomTunnelSpinBoxValueChanged);
    connect(ui->roomTeleDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRoomTeleDelaySpinBoxValueChanged);
    connect(ui->roomExtraDescriptionButton, &QPushButton::clicked, this, &MainWindow::onRoomExtraDescriptionButtonClicked);
    connect(ui->roomProgramsButton, &QPushButton::clicked, this, &MainWindow::onRoomProgramsButtonClicked);

    // --- Room Exit field signals ---
    connect(ui->roomExitNavigatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRoomExitNavigatorComboBoxCurrentIndexChanged);
    connect(ui->roomExitAddButton, &QPushButton::clicked, this, &MainWindow::onRoomExitAddButtonClicked);
    connect(ui->roomExitDeleteButton, &QPushButton::clicked, this, &MainWindow::onRoomExitDeleteButtonClicked);
    connect(ui->roomExitKeywordEdit, &QLineEdit::textChanged, this, &MainWindow::onRoomExitKeywordEditTextChanged);
    connect(ui->roomExitDescriptionEdit, &QLineEdit::textChanged, this, &MainWindow::onRoomExitDescriptionEditTextChanged);
    connect(ui->roomExitFlagsEdit, &QLineEdit::textChanged, this, &MainWindow::onRoomExitFlagsEditTextChanged);
    connect(ui->roomExitFlagsButton, &QPushButton::clicked, this, &MainWindow::onRoomExitFlagsButtonClicked);
    connect(ui->roomExitKeyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRoomExitKeyComboBoxCurrentIndexChanged);
    connect(ui->roomExitDistanceSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRoomExitDistanceSpinBoxValueChanged);

    // --- Reset Data field signals ---
    connect(ui->resetNavigatorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onResetNavigatorComboBoxCurrentIndexChanged);
    connect(ui->resetAddButton, &QPushButton::clicked, this, &MainWindow::onResetAddButtonClicked);
    connect(ui->resetDeleteButton, &QPushButton::clicked, this, &MainWindow::onResetDeleteButtonClicked);

    // --- Shop Data field signals ---
    connect(ui->shopKeeperBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShopKeeperBoxCurrentIndexChanged);
    connect(ui->shopFlagsEdit, &QLineEdit::textChanged, this, &MainWindow::onShopFlagsEditTextChanged);
    connect(ui->shopFlagsButton, &QPushButton::clicked, this, &MainWindow::onShopFlagsButtonClicked);
    connect(ui->shopOpenBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onShopOpenBoxValueChanged);
    connect(ui->shopCloseBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onShopCloseBoxValueChanged);
    connect(ui->shopProfitBuyBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onShopProfitBuyBoxValueChanged);
    connect(ui->shopProfitSellBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onShopProfitSellBoxValueChanged);
    connect(ui->shopType0Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShopType0BoxCurrentIndexChanged);
    connect(ui->shopType1Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShopType1BoxCurrentIndexChanged);
    connect(ui->shopType2Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShopType2BoxCurrentIndexChanged);
    connect(ui->shopType3Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShopType3BoxCurrentIndexChanged);
    connect(ui->shopType4Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShopType4BoxCurrentIndexChanged);
    connect(ui->shopAddButton, &QPushButton::clicked, this, &MainWindow::onShopAddButtonClicked);
    connect(ui->shopDeleteButton, &QPushButton::clicked, this, &MainWindow::onShopDeleteButtonClicked);

    // --- Repair Data field signals ---
    connect(ui->repairKeeperBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRepairKeeperBoxCurrentIndexChanged);
    connect(ui->repairShopTypeBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRepairShopTypeBoxCurrentIndexChanged);
    connect(ui->repairOpenBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRepairOpenBoxValueChanged);
    connect(ui->repairCloseBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRepairCloseBoxValueChanged);
    connect(ui->repairProfitFixBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRepairProfitFixBoxValueChanged);
    connect(ui->repairType0Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRepairType0BoxCurrentIndexChanged);
    connect(ui->repairType1Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRepairType1BoxCurrentIndexChanged);
    connect(ui->repairType2Box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRepairType2BoxCurrentIndexChanged);
    connect(ui->repairAddButton, &QPushButton::clicked, this, &MainWindow::onRepairAddButtonClicked);
    connect(ui->repairDeleteButton, &QPushButton::clicked, this, &MainWindow::onRepairDeleteButtonClicked);
}

// ===========================================================================
// Common / state
// ===========================================================================

void MainWindow::setModified()
{
    modified_ = true;
    setWindowModified(true);
}

void MainWindow::setNotModified()
{
    modified_ = false;
    setWindowModified(false);
}

bool MainWindow::canLeaveCurrent()
{
    if (!modified_)
        return true;

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Leaving swaedit");
    msgBox.setText("Area was modified. Are you sure you want to exit?\n"
                   "You will loose your changes if you say Ok now!\n");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Save);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok) {
        return true;
    } else if (ret == QMessageBox::Save) {
        saveArea();
        return true;
    }
    return false;
}

// ===========================================================================
// closeEvent
// ===========================================================================

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (canLeaveCurrent()) {
        sysTray_->hide();
        event->accept();
        QApplication::quit();
    } else {
        event->ignore();
    }
}

// ===========================================================================
// File operations
// ===========================================================================

void MainWindow::openArea()
{
    if (!canLeaveCurrent())
        return;

    QString fileName = QFileDialog::getOpenFileName(
        this, "Open an area", QString(), "swmud 1.0 area files (*.xml)");
    if (fileName.isEmpty())
        return;

    // Validate against XSD — reject invalid files
    QString schemaDir = QCoreApplication::applicationDirPath() + QStringLiteral("/schemas/");
    QString validationError = XmlIO::validateXml(fileName, schemaDir + QStringLiteral("area.xsd"));
    if (!validationError.isEmpty()) {
        QMessageBox::critical(this, "XSD Validation Error",
            QStringLiteral("The area file does not conform to the schema and cannot be loaded.\n\n%1")
                .arg(validationError));
        return;
    }

    area_ = XmlIO::loadArea(fileName);
    currentFileName_ = fileName;
    areaLoaded_ = true;

    fillAll();
    setNotModified();

    backupTimer_.start(300000);
}

void MainWindow::saveArea()
{
    if (!areaLoaded_) return;
    if (currentFileName_.isEmpty()) {
        saveAreaAs();
        return;
    }

    QString fileName = currentFileName_;
    if (!fileName.endsWith(".xml", Qt::CaseInsensitive))
        fileName += ".xml";

    XmlIO::saveArea(area_, fileName);
    QFile::remove(currentFileName_ + "~");
    currentFileName_ = fileName;
    setNotModified();
    statusBar()->showMessage("Area saved to " + fileName, 5000);
}

void MainWindow::saveAreaAs()
{
    if (!areaLoaded_) return;
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Area As", currentFileName_, "Area Files (*.xml);;All Files (*)");
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".xml", Qt::CaseInsensitive))
        fileName += ".xml";

    XmlIO::saveArea(area_, fileName);
    QFile::remove(currentFileName_ + "~");
    currentFileName_ = fileName;
    setNotModified();
    statusBar()->showMessage("Area saved to " + fileName, 5000);
}

void MainWindow::timerBackup()
{
    if (currentFileName_.isEmpty())
        return;

    QString backupName = currentFileName_ + "~";
    XmlIO::saveArea(area_, backupName);
}

// ===========================================================================
// fillAll
// ===========================================================================

void MainWindow::fillAll()
{
    fillAreaData();

    if (!area_.objects.isEmpty())
        fillItemData(0);
    else
        clearItemData();

    if (!area_.mobiles.isEmpty())
        fillMobileData(0);
    else
        clearMobileData();

    if (!area_.rooms.isEmpty())
        fillRoomData(0);
    else
        clearRoomData();

    if (!area_.resets.isEmpty())
        fillResetData(0);
    else
        clearResetData();

    if (!area_.shops.isEmpty())
        fillShopData(0);
    else
        clearShopData();

    if (!area_.repairs.isEmpty())
        fillRepairData(0);
    else
        clearRepairData();
}

// ===========================================================================
// Area Data (Head)
// ===========================================================================

void MainWindow::fillAreaData()
{
    headCanChange_ = false;

    ui->nameEdit->setText(area_.head.name);
    ui->authorsEdit->setText(area_.head.authors);
    ui->buildersEdit->setText(area_.head.builders);
    ui->securitySpinBox->setValue(area_.head.security);
    ui->lvnumEdit->setText(QString::number(area_.head.vnums.lvnum));
    ui->uvnumEdit->setText(QString::number(area_.head.vnums.uvnum));
    ui->flagsEdit->setText(QString::number(area_.head.flags));
    ui->leconomyEdit->setText(QString::number(area_.head.economy.low));
    ui->heconomyEdit->setText(QString::number(area_.head.economy.high));
    ui->resetFreqEdit->setText(QString::number(area_.head.reset.frequency));
    ui->resetMsgEdit->setText(area_.head.reset.message);
    ui->lrangeSpinBox->setValue(area_.head.ranges.low);
    ui->hrangeSpinBox->setValue(area_.head.ranges.high);

    headCanChange_ = true;

    if (!area_.head.name.isEmpty()) {
        statusBar()->showMessage(
            "'" + area_.head.name + "' loaded with: "
            + QString::number(area_.objects.size()) + " objects, "
            + QString::number(area_.mobiles.size()) + " mobiles, "
            + QString::number(area_.rooms.size()) + " rooms.", 5000);
    }
}

void MainWindow::clearAreaData()
{
    headCanChange_ = false;

    ui->nameEdit->clear();
    ui->authorsEdit->clear();
    ui->buildersEdit->clear();
    ui->securitySpinBox->setValue(0);
    ui->lvnumEdit->clear();
    ui->uvnumEdit->clear();
    ui->flagsEdit->clear();
    ui->leconomyEdit->clear();
    ui->heconomyEdit->clear();
    ui->resetFreqEdit->clear();
    ui->resetMsgEdit->clear();
    ui->lrangeSpinBox->setValue(1);
    ui->hrangeSpinBox->setValue(20);
}

// --- Area Data slots ---

void MainWindow::onNameEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    area_.head.name = str;
    setModified();
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Area Name", "Area Name cannot be empty!");
        ui->nameEdit->setFocus();
    }
}

void MainWindow::onAuthorsEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    area_.head.authors = str;
    setModified();
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Authors", "Authors cannot be empty!");
        ui->authorsEdit->setFocus();
    }
}

void MainWindow::onBuildersEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    area_.head.builders = str;
    setModified();
}

void MainWindow::onSecuritySpinBoxValueChanged(int value)
{
    if (!headCanChange_) return;
    area_.head.security = value;
    setModified();
}

void MainWindow::onLvnumEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Lower Vnum must have number value!");
        ui->lvnumEdit->setText(QString::number(area_.head.vnums.lvnum));
        return;
    }
    area_.head.vnums.lvnum = val;
    setModified();
}

void MainWindow::onUvnumEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Upper Vnum must have number value!");
        ui->uvnumEdit->setText(QString::number(area_.head.vnums.uvnum));
        return;
    }
    area_.head.vnums.uvnum = val;
    setModified();
}

void MainWindow::onFlagsEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Area Flags must have number value!");
        ui->flagsEdit->setText(QString::number(area_.head.flags));
        return;
    }
    area_.head.flags = val;
    setModified();
}

void MainWindow::onFlagsButtonClicked()
{
    FlagsWidget *fw = new FlagsWidget(config_.areaFlags, area_.head.flags,
                                      "Area Flags", false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        area_.head.flags = value;
        headCanChange_ = false;
        ui->flagsEdit->setText(QString::number(value));
        headCanChange_ = true;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onLeconomyEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Low Economy must have number value!");
        ui->leconomyEdit->setText(QString::number(area_.head.economy.low));
        return;
    }
    area_.head.economy.low = val;
    setModified();
}

void MainWindow::onHeconomyEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "High Economy must have number value!");
        ui->heconomyEdit->setText(QString::number(area_.head.economy.high));
        return;
    }
    area_.head.economy.high = val;
    setModified();
}

void MainWindow::onResetFreqEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    bool ok;
    int val = str.toInt(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Reset Frequency must have number value!");
        ui->resetFreqEdit->setText(QString::number(area_.head.reset.frequency));
        return;
    }
    area_.head.reset.frequency = val;
    setModified();
}

void MainWindow::onResetMsgEditTextChanged(const QString &str)
{
    if (!headCanChange_) return;
    area_.head.reset.message = str;
    setModified();
}

void MainWindow::onLrangeSpinBoxValueChanged(int value)
{
    if (!headCanChange_) return;
    area_.head.ranges.low = value;
    setModified();
}

void MainWindow::onHrangeSpinBoxValueChanged(int value)
{
    if (!headCanChange_) return;
    area_.head.ranges.high = value;
    setModified();
}

// ===========================================================================
// Item Data
// ===========================================================================

AreaObject *MainWindow::getCurrentObject()
{
    int idx = ui->itemNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.objects.size())
        return nullptr;
    qint64 vnum = ui->itemNavigatorComboBox->currentData().toLongLong();
    for (int i = 0; i < area_.objects.size(); ++i) {
        if (area_.objects[i].vnum == vnum)
            return &area_.objects[i];
    }
    return nullptr;
}

void MainWindow::fillItemData(int objectIndex)
{
    itemCanChange_ = false;

    ui->itemNavigatorComboBox->blockSignals(true);
    ui->itemNavigatorComboBox->clear();
    for (int i = 0; i < area_.objects.size(); ++i) {
        const AreaObject &obj = area_.objects[i];
        ui->itemNavigatorComboBox->addItem(
            QString::number(obj.vnum) + " - " + obj.name,
            QVariant(obj.vnum));
    }
    if (objectIndex >= 0 && objectIndex < area_.objects.size())
        ui->itemNavigatorComboBox->setCurrentIndex(objectIndex);
    ui->itemNavigatorComboBox->blockSignals(false);

    if (objectIndex < 0 || objectIndex >= area_.objects.size()) {
        clearItemData();
        return;
    }

    const AreaObject &obj = area_.objects[objectIndex];

    ui->itemVnumEdit->setText(QString::number(obj.vnum));
    ui->itemNameEdit->setText(obj.name);
    ui->itemDescription->setText(obj.description);
    ui->itemActionDescription->setText(obj.actiondesc);

    int typeIdx = ui->itemTypeComboBox->findData(obj.type);
    if (typeIdx >= 0) ui->itemTypeComboBox->setCurrentIndex(typeIdx);

    int genderIdx = ui->itemGenderComboBox->findData(static_cast<int>(obj.gender));
    if (genderIdx >= 0) ui->itemGenderComboBox->setCurrentIndex(genderIdx);

    ui->itemExtraFlagsEdit->setText(QString::number(obj.extraflags));
    ui->itemWearFlagsEdit->setText(QString::number(obj.wearflags));
    ui->itemLayersEdit->setText(QString::number(obj.layers));
    ui->itemLevelSpinBox->setValue(obj.level);
    ui->itemWeightEdit->setText(QString::number(obj.weight));
    ui->itemCostEdit->setText(QString::number(obj.cost));

    ui->itemInflect0Edit->setText(obj.shortDesc.inflect0);
    ui->itemInflect1Edit->setText(obj.shortDesc.inflect1);
    ui->itemInflect2Edit->setText(obj.shortDesc.inflect2);
    ui->itemInflect3Edit->setText(obj.shortDesc.inflect3);
    ui->itemInflect4Edit->setText(obj.shortDesc.inflect4);
    ui->itemInflect5Edit->setText(obj.shortDesc.inflect5);

    setItemValues(obj.type);

    itemCanChange_ = (area_.objects.size() > 0);
}

void MainWindow::clearItemData()
{
    itemCanChange_ = false;

    ui->itemNavigatorComboBox->blockSignals(true);
    ui->itemNavigatorComboBox->clear();
    ui->itemNavigatorComboBox->blockSignals(false);

    ui->itemVnumEdit->clear();
    ui->itemNameEdit->clear();
    ui->itemDescription->clear();
    ui->itemActionDescription->clear();
    ui->itemTypeComboBox->setCurrentIndex(-1);
    ui->itemGenderComboBox->setCurrentIndex(-1);
    ui->itemExtraFlagsEdit->setText("0");
    ui->itemWearFlagsEdit->setText("0");
    ui->itemLayersEdit->setText("0");
    ui->itemLevelSpinBox->setValue(0);
    ui->itemWeightEdit->setText("0");
    ui->itemCostEdit->setText("0");
    ui->itemInflect0Edit->clear();
    ui->itemInflect1Edit->clear();
    ui->itemInflect2Edit->clear();
    ui->itemInflect3Edit->clear();
    ui->itemInflect4Edit->clear();
    ui->itemInflect5Edit->clear();

    clearItemLabelValues();
}

void MainWindow::setItemValues(int type)
{
    clearItemLabelValues();

    const ItemTypeDef *typeDef = nullptr;
    for (const ItemTypeDef &t : config_.itemTypes) {
        if (t.value == type) {
            typeDef = &t;
            break;
        }
    }
    if (!typeDef)
        return;

    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    bool filled[6] = {};
    for (const ItemTypeValueDef &val : typeDef->values) {
        int no = val.no;
        if (no < 0 || no > 5) continue;
        filled[no] = true;

        valueLabels_[no]->setText(val.name + ":");

        if (!val.subvalues.isEmpty()
            && (val.subvaluesType == "auto" || val.subvaluesType == "types")) {
            createItemValueTypeEdit(val);
        } else if (!val.subvalues.isEmpty() && val.subvaluesType == "flags") {
            createItemValueFlagsWidget(val);
        } else {
            createItemValueEdit(no);
        }
    }

    // Gap 6: create plain QLineEdit for unfilled value slots
    for (int i = 0; i < 6; ++i) {
        if (!filled[i]) {
            createItemValueEdit(i);
        }
    }
}

void MainWindow::clearItemLabelValues()
{
    for (int i = 0; i < 6; ++i) {
        valueLabels_[i]->setText("value" + QString::number(i) + ":");
        if (valueWidgets_[i]) {
            if (valueLayouts_[i])
                valueLayouts_[i]->removeWidget(valueWidgets_[i]);
            delete valueWidgets_[i];
            valueWidgets_[i] = nullptr;
        }
    }
}

void MainWindow::createItemValueEdit(int no)
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    QLineEdit *edit = new QLineEdit();
    edit->setObjectName("itemValueEdit" + QString::number(no));

    switch (no) {
    case 0: edit->setText(QString::number(obj->values.value0)); break;
    case 1: edit->setText(QString::number(obj->values.value1)); break;
    case 2: edit->setText(QString::number(obj->values.value2)); break;
    case 3: edit->setText(obj->values.value3); break;
    case 4: edit->setText(obj->values.value4); break;
    case 5: edit->setText(obj->values.value5); break;
    }

    connect(edit, &QLineEdit::textChanged, this, &MainWindow::itemValueEditChanged);
    itemAddValueWidget(no, edit);
}

void MainWindow::createItemValueTypeEdit(const ItemTypeValueDef &value)
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    int no = value.no;
    QComboBox *box = new QComboBox();
    box->setObjectName("itemValueCombo" + QString::number(no));

    for (const SubvalueDef &sv : value.subvalues) {
        box->addItem(sv.name, sv.value.toInt());
    }

    int curVal = 0;
    switch (no) {
    case 0: curVal = obj->values.value0; break;
    case 1: curVal = obj->values.value1; break;
    case 2: curVal = obj->values.value2; break;
    case 3: curVal = obj->values.value3.toInt(); break;
    case 4: curVal = obj->values.value4.toInt(); break;
    case 5: curVal = obj->values.value5.toInt(); break;
    }

    int idx = box->findData(curVal);
    if (idx >= 0) box->setCurrentIndex(idx);

    connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::itemValueComboBoxChanged);
    itemAddValueWidget(no, box);
}

void MainWindow::createItemValueFlagsWidget(const ItemTypeValueDef &value)
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    int no = value.no;

    QWidget *container = new QWidget();
    QHBoxLayout *lay = new QHBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0);

    QLineEdit *edit = new QLineEdit();
    edit->setObjectName("itemValueEdit" + QString::number(no));

    switch (no) {
    case 0: edit->setText(QString::number(obj->values.value0)); break;
    case 1: edit->setText(QString::number(obj->values.value1)); break;
    case 2: edit->setText(QString::number(obj->values.value2)); break;
    case 3: edit->setText(obj->values.value3); break;
    case 4: edit->setText(obj->values.value4); break;
    case 5: edit->setText(obj->values.value5); break;
    }

    QPushButton *btn = new QPushButton("Edit Flags");
    btn->setObjectName("itemValueFlagsBtn" + QString::number(no));

    lay->addWidget(edit);
    lay->addWidget(btn);

    connect(edit, &QLineEdit::textChanged, this, &MainWindow::itemValueEditChanged);
    connect(btn, &QPushButton::clicked, this, &MainWindow::itemValueFlagsButtonClicked);
    itemAddValueWidget(no, container);
}

void MainWindow::itemAddValueWidget(int no, QWidget *widget)
{
    if (no < 0 || no > 5) return;
    if (!valueLayouts_[no]) return;

    valueWidgets_[no] = widget;
    valueLayouts_[no]->addWidget(widget);
}

int MainWindow::getItemValueIndex(QComboBox *cbox, int no)
{
    Q_UNUSED(no);
    if (!cbox) return 0;
    return cbox->currentData().toInt();
}

// --- Item Data slots ---

void MainWindow::onItemNavigatorComboBoxCurrentIndexChanged(int idx)
{
    if (!itemCanChange_) return;
    if (idx < 0) return;
    fillItemData(idx);
}

void MainWindow::onItemTypeComboBoxCurrentIndexChanged(int idx)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->type = ui->itemTypeComboBox->itemData(idx).toInt();
    setItemValues(obj->type);
    // Refresh exit key combo if item type changed to "key" (matching Java)
    if (ui->itemTypeComboBox->itemText(idx).compare("key", Qt::CaseInsensitive) == 0) {
        fillExitKeys(getCurrentExit());
    }
    setModified();
}

void MainWindow::onItemGenderComboBoxCurrentIndexChanged(int idx)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->gender = ui->itemGenderComboBox->itemData(idx).toLongLong();
    setModified();
}

void MainWindow::onItemVnumEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    // Intentionally disabled -- use Renumber instead.
    // Editing vnums directly would break reset/shop/repair references.
    return;
}

void MainWindow::onItemNameEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->name = str;
    int curIdx = ui->itemNavigatorComboBox->currentIndex();
    ui->itemNavigatorComboBox->setItemText(curIdx,
        QString::number(obj->vnum) + " - " + obj->name);
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Name cannot be empty!");
        ui->itemNameEdit->setFocus();
    }
    setModified();
}

void MainWindow::onItemDescriptionTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->description = str;
    setModified();
}

void MainWindow::onItemActionDescriptionTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->actiondesc = str;
    setModified();
}

void MainWindow::onItemExtraFlagsEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Item Extra Flags must have number value!");
        ui->itemExtraFlagsEdit->setText(QString::number(obj->extraflags));
        return;
    }
    obj->extraflags = val;
    setModified();
}

void MainWindow::onItemExtraFlagsButtonClicked()
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    FlagsWidget *fw = new FlagsWidget(config_.itemExtraFlags, obj->extraflags,
                                      "Item Extra Flags", false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        AreaObject *obj = getCurrentObject();
        if (!obj) return;
        obj->extraflags = value;
        itemCanChange_ = false;
        ui->itemExtraFlagsEdit->setText(QString::number(value));
        itemCanChange_ = true;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onItemWearFlagsEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Item Wear Flags must have number value!");
        ui->itemWearFlagsEdit->setText(QString::number(obj->wearflags));
        return;
    }
    obj->wearflags = val;
    setModified();
}

void MainWindow::onItemWearFlagsButtonClicked()
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    FlagsWidget *fw = new FlagsWidget(config_.itemWearFlags, obj->wearflags,
                                      "Item Wear Flags", false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        AreaObject *obj = getCurrentObject();
        if (!obj) return;
        obj->wearflags = value;
        itemCanChange_ = false;
        ui->itemWearFlagsEdit->setText(QString::number(value));
        itemCanChange_ = true;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onItemInflect0EditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->shortDesc.inflect0 = str;
    setModified();
}

void MainWindow::onItemInflect1EditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->shortDesc.inflect1 = str;
    setModified();
}

void MainWindow::onItemInflect2EditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->shortDesc.inflect2 = str;
    setModified();
}

void MainWindow::onItemInflect3EditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->shortDesc.inflect3 = str;
    setModified();
}

void MainWindow::onItemInflect4EditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->shortDesc.inflect4 = str;
    setModified();
}

void MainWindow::onItemInflect5EditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->shortDesc.inflect5 = str;
    setModified();
}

void MainWindow::onItemWeightEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Item Weight must have number value!");
        ui->itemWeightEdit->setText(QString::number(obj->weight));
        return;
    }
    obj->weight = val;
    setModified();
}

void MainWindow::onItemCostEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Item Cost must have number value!");
        ui->itemCostEdit->setText(QString::number(obj->cost));
        return;
    }
    obj->cost = val;
    setModified();
}

void MainWindow::onItemLevelSpinBoxValueChanged(int value)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    obj->level = value;
    setModified();
}

void MainWindow::onItemLayersEditTextChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    bool ok;
    int val = str.toInt(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Item Layers must have number value!");
        ui->itemLayersEdit->setText(QString::number(obj->layers));
        return;
    }
    obj->layers = val;
    setModified();
}

void MainWindow::onItemExtraDescriptionButtonClicked()
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    ExtraDescWidget *w = new ExtraDescWidget(&obj->extradescs, this, this);
        centerChildOnParent(w, this);
}

void MainWindow::onItemProgramsButtonClicked()
{
    AreaObject *obj = getCurrentObject();
    if (!obj) return;
    ProgramsWidget *w = new ProgramsWidget(&obj->programs, config_.progTypes,
                                            config_.highlighter, this, this);
        centerChildOnParent(w, this);
}

void MainWindow::onItemAffectsButtonClicked()
{
    QMessageBox::information(this, "Information", "Affects editor not yet implemented.");
}

void MainWindow::onItemRequirementsButtonClicked()
{
    QMessageBox::information(this, "Information", "Requirements editor not yet implemented.");
}

// --- Dynamic item value widget slots ---

void MainWindow::itemValueEditChanged(const QString &str)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
    if (!edit) return;

    QString name = edit->objectName();
    int no = -1;
    if (name.startsWith("itemValueEdit"))
        no = name.mid(13).toInt();

    if (no < 0 || no > 5) return;

    switch (no) {
    case 0: {
        bool ok;
        int v = str.toInt(&ok);
        if (ok) {
            obj->values.value0 = v;
            setModified();
        } else if (!str.isEmpty()) {
            QString label = valueLabels_[0]->text().replace(":", "");
            QMessageBox::critical(nullptr, "Invalid Number Value",
                label + " must have number value!");
            edit->setText(QString::number(obj->values.value0));
        }
        break;
    }
    case 1: {
        bool ok;
        int v = str.toInt(&ok);
        if (ok) {
            obj->values.value1 = v;
            setModified();
        } else if (!str.isEmpty()) {
            QString label = valueLabels_[1]->text().replace(":", "");
            QMessageBox::critical(nullptr, "Invalid Number Value",
                label + " must have number value!");
            edit->setText(QString::number(obj->values.value1));
        }
        break;
    }
    case 2: {
        bool ok;
        int v = str.toInt(&ok);
        if (ok) {
            obj->values.value2 = v;
            setModified();
        } else if (!str.isEmpty()) {
            QString label = valueLabels_[2]->text().replace(":", "");
            QMessageBox::critical(nullptr, "Invalid Number Value",
                label + " must have number value!");
            edit->setText(QString::number(obj->values.value2));
        }
        break;
    }
    case 3: obj->values.value3 = str; setModified(); break;
    case 4: obj->values.value4 = str; setModified(); break;
    case 5: obj->values.value5 = str; setModified(); break;
    }
}

void MainWindow::itemValueComboBoxChanged(int idx)
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    QComboBox *box = qobject_cast<QComboBox *>(sender());
    if (!box) return;

    QString name = box->objectName();
    int no = -1;
    if (name.startsWith("itemValueCombo"))
        no = name.mid(14).toInt();

    if (no < 0 || no > 5) return;
    int val = box->itemData(idx).toInt();

    switch (no) {
    case 0: obj->values.value0 = val; break;
    case 1: obj->values.value1 = val; break;
    case 2: obj->values.value2 = val; break;
    case 3: obj->values.value3 = QString::number(val); break;
    case 4: obj->values.value4 = QString::number(val); break;
    case 5: obj->values.value5 = QString::number(val); break;
    }

    setModified();
}

void MainWindow::itemValueFlagsButtonClicked()
{
    if (!itemCanChange_) return;
    AreaObject *obj = getCurrentObject();
    if (!obj) return;

    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;

    QString name = btn->objectName();
    int no = -1;
    if (name.startsWith("itemValueFlagsBtn"))
        no = name.mid(17).toInt();

    if (no < 0 || no > 5) return;

    // Find the flag defs for this value slot from the item type definition
    const ItemTypeDef *typeDef = nullptr;
    for (const ItemTypeDef &t : config_.itemTypes) {
        if (t.value == obj->type) {
            typeDef = &t;
            break;
        }
    }
    if (!typeDef) return;

    QList<FlagDef> flagDefs;
    for (const ItemTypeValueDef &val : typeDef->values) {
        if (val.no == no && val.subvaluesType == "flags") {
            for (const SubvalueDef &sv : val.subvalues) {
                FlagDef fd;
                fd.name = sv.name;
                fd.value = sv.value.toInt();
                flagDefs.append(fd);
            }
            break;
        }
    }

    qint64 currentValue = 0;
    switch (no) {
    case 0: currentValue = obj->values.value0; break;
    case 1: currentValue = obj->values.value1; break;
    case 2: currentValue = obj->values.value2; break;
    case 3: currentValue = obj->values.value3.toLongLong(); break;
    case 4: currentValue = obj->values.value4.toLongLong(); break;
    case 5: currentValue = obj->values.value5.toLongLong(); break;
    }

    FlagsWidget *fw = new FlagsWidget(flagDefs, currentValue,
                                      "Value " + QString::number(no) + " Flags",
                                      false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this, no](qint64 value) {
        AreaObject *obj = getCurrentObject();
        if (!obj) return;

        switch (no) {
        case 0: obj->values.value0 = static_cast<int>(value); break;
        case 1: obj->values.value1 = static_cast<int>(value); break;
        case 2: obj->values.value2 = static_cast<int>(value); break;
        case 3: obj->values.value3 = QString::number(value); break;
        case 4: obj->values.value4 = QString::number(value); break;
        case 5: obj->values.value5 = QString::number(value); break;
        }

        // Update the line edit in the flags widget container
        QLineEdit *edit = findChild<QLineEdit *>("itemValueEdit" + QString::number(no));
        if (edit) {
            edit->setText(QString::number(value));
        }

        setModified();
    });
        centerChildOnParent(fw, this);
}

// ===========================================================================
// Mobile Data
// ===========================================================================

Mobile *MainWindow::getCurrentMob()
{
    int idx = ui->mobNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.mobiles.size())
        return nullptr;
    qint64 vnum = ui->mobNavigatorComboBox->currentData().toLongLong();
    for (int i = 0; i < area_.mobiles.size(); ++i) {
        if (area_.mobiles[i].vnum == vnum)
            return &area_.mobiles[i];
    }
    return nullptr;
}

void MainWindow::fillMobileData(int mobIndex)
{
    mobCanChange_ = false;

    ui->mobNavigatorComboBox->blockSignals(true);
    ui->mobNavigatorComboBox->clear();
    for (int i = 0; i < area_.mobiles.size(); ++i) {
        const Mobile &mob = area_.mobiles[i];
        ui->mobNavigatorComboBox->addItem(
            QString::number(mob.vnum) + " - " + mob.name,
            QVariant(mob.vnum));
    }
    if (mobIndex >= 0 && mobIndex < area_.mobiles.size())
        ui->mobNavigatorComboBox->setCurrentIndex(mobIndex);
    ui->mobNavigatorComboBox->blockSignals(false);

    if (mobIndex < 0 || mobIndex >= area_.mobiles.size()) {
        clearMobileData();
        return;
    }

    const Mobile &mob = area_.mobiles[mobIndex];

    ui->mobVnumEdit->setText(QString::number(mob.vnum));
    ui->mobNameEdit->setText(mob.name);
    ui->mobLongDescriptionEdit->setText(mob.longDesc);
    ui->mobDescriptionText->setPlainText(mob.description);

    int sexIdx = ui->mobSexComboBox->findData(mob.sex);
    if (sexIdx >= 0) ui->mobSexComboBox->setCurrentIndex(sexIdx);

    int raceIdx = ui->mobRaceComboBox->findText(mob.race);
    if (raceIdx >= 0) ui->mobRaceComboBox->setCurrentIndex(raceIdx);

    int posIdx = ui->mobPositionComboBox->findData(mob.position);
    if (posIdx >= 0) ui->mobPositionComboBox->setCurrentIndex(posIdx);

    int speakIdx = ui->mobSpeakingComboBox->findText(mob.sectionr.speaking);
    if (speakIdx >= 0) ui->mobSpeakingComboBox->setCurrentIndex(speakIdx);

    int vipIdx = ui->mobVipFlagsComboBox->findText(mob.sectionv.vipflags);
    if (vipIdx >= 0) ui->mobVipFlagsComboBox->setCurrentIndex(vipIdx);
    else ui->mobVipFlagsComboBox->setCurrentIndex(0);

    ui->mobLevelSpinBox->setValue(mob.level);
    ui->mobAlignmentBox->setValue(mob.alignment);
    ui->mobAttacksNoBox->setValue(mob.sectionr.numattacks);
    ui->mobHeightBox->setValue(mob.sectionr.height);
    ui->mobWeightBox->setValue(mob.sectionr.weight);
    ui->mobHitRollBox->setValue(mob.sectionx.hitroll);
    ui->mobDamRollBox->setValue(mob.sectionx.damroll);
    ui->mobCreditsEdit->setText(QString::number(mob.credits));
    ui->mobArmorClassBox->setValue(mob.sectiont.ac);

    ui->mobHitNoDiceBox->setValue(mob.sectiont.hitnodice);
    ui->mobHitSizeDiceBox->setValue(mob.sectiont.hitsizedice);
    ui->mobHitPlusBox->setValue(mob.sectiont.hitplus);

    ui->mobDamNoDiceBox->setValue(mob.sectiont.damnodice);
    ui->mobDamSizeDiceBox->setValue(mob.sectiont.damsizedice);
    ui->mobDamPlusBox->setValue(mob.sectiont.damplus);

    ui->mobStrBox->setValue(mob.sectiona.str);
    ui->mobIntBox->setValue(mob.sectiona.intel);
    ui->mobWisBox->setValue(mob.sectiona.wis);
    ui->mobDexBox->setValue(mob.sectiona.dex);
    ui->mobConBox->setValue(mob.sectiona.con);
    ui->mobChaBox->setValue(mob.sectiona.cha);
    ui->mobLckBox->setValue(mob.sectiona.lck);

    ui->mobInflect0Edit->setText(mob.shortDesc.inflect0);
    ui->mobInflect1Edit->setText(mob.shortDesc.inflect1);
    ui->mobInflect2Edit->setText(mob.shortDesc.inflect2);
    ui->mobInflect3Edit->setText(mob.shortDesc.inflect3);
    ui->mobInflect4Edit->setText(mob.shortDesc.inflect4);
    ui->mobInflect5Edit->setText(mob.shortDesc.inflect5);

    ui->mobDialogNameEdit->setText(mob.dialog);

    mobCanChange_ = (area_.mobiles.size() > 0);
}

void MainWindow::clearMobileData()
{
    mobCanChange_ = false;

    ui->mobNavigatorComboBox->blockSignals(true);
    ui->mobNavigatorComboBox->clear();
    ui->mobNavigatorComboBox->blockSignals(false);

    ui->mobVnumEdit->clear();
    ui->mobNameEdit->clear();
    ui->mobLongDescriptionEdit->clear();
    ui->mobDescriptionText->clear();
    ui->mobSexComboBox->setCurrentIndex(-1);
    ui->mobRaceComboBox->setCurrentIndex(-1);
    ui->mobPositionComboBox->setCurrentIndex(-1);
    ui->mobSpeakingComboBox->setCurrentIndex(-1);
    ui->mobVipFlagsComboBox->setCurrentIndex(-1);
    ui->mobLevelSpinBox->setValue(0);
    ui->mobAlignmentBox->setValue(0);
    ui->mobAttacksNoBox->setValue(0);
    ui->mobHeightBox->setValue(0);
    ui->mobWeightBox->setValue(0);
    ui->mobHitRollBox->setValue(0);
    ui->mobDamRollBox->setValue(0);
    ui->mobCreditsEdit->setText("0");
    ui->mobArmorClassBox->setValue(0);
    ui->mobHitNoDiceBox->setValue(0);
    ui->mobHitSizeDiceBox->setValue(0);
    ui->mobHitPlusBox->setValue(0);
    ui->mobDamNoDiceBox->setValue(0);
    ui->mobDamSizeDiceBox->setValue(0);
    ui->mobDamPlusBox->setValue(0);
    ui->mobStrBox->setValue(0);
    ui->mobIntBox->setValue(0);
    ui->mobWisBox->setValue(0);
    ui->mobDexBox->setValue(0);
    ui->mobConBox->setValue(0);
    ui->mobChaBox->setValue(0);
    ui->mobLckBox->setValue(0);
    ui->mobInflect0Edit->clear();
    ui->mobInflect1Edit->clear();
    ui->mobInflect2Edit->clear();
    ui->mobInflect3Edit->clear();
    ui->mobInflect4Edit->clear();
    ui->mobInflect5Edit->clear();
    ui->mobDialogNameEdit->clear();
}

// --- Mobile Data slots ---

void MainWindow::onMobNavigatorComboBoxCurrentIndexChanged(int idx)
{
    if (!mobCanChange_) return;
    if (idx < 0) return;
    fillMobileData(idx);
}

void MainWindow::onMobNameEditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->name = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Mobile name cannot be empty!");
        ui->mobNameEdit->setFocus();
    }
    int curIdx = ui->mobNavigatorComboBox->currentIndex();
    ui->mobNavigatorComboBox->setItemText(curIdx,
        QString::number(mob->vnum) + " - " + mob->name);
    setModified();
}

void MainWindow::onMobLongDescriptionEditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->longDesc = str;
    setModified();
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Long Description", "Mobile long description cannot be empty!");
        ui->mobLongDescriptionEdit->setFocus();
    }
}

void MainWindow::onMobDescriptionTextChanged()
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->description = ui->mobDescriptionText->toPlainText();
    setModified();
}

void MainWindow::onMobSexComboBoxCurrentIndexChanged(int idx)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sex = ui->mobSexComboBox->itemData(idx).toInt();
    setModified();
}

void MainWindow::onMobRaceComboBoxCurrentIndexChanged(int idx)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->race = ui->mobRaceComboBox->itemText(idx);
    setModified();
}

void MainWindow::onMobPositionComboBoxCurrentIndexChanged(int idx)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->position = ui->mobPositionComboBox->itemData(idx).toInt();
    setModified();
}

void MainWindow::onMobSpeakingComboBoxCurrentIndexChanged(int idx)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionr.speaking = ui->mobSpeakingComboBox->itemText(idx);
    setModified();
}

void MainWindow::onMobVipFlagsComboBoxCurrentIndexChanged(int idx)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionv.vipflags = ui->mobVipFlagsComboBox->itemText(idx);
    setModified();
}

void MainWindow::onMobActFlagsButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.mobileActFlags, mob->act,
                                      "Mobile Act Flags", false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->act = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobAffectedFlagsButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.mobileAffectedFlags, mob->affected,
                                      "Mobile Affected Flags", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->affected = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobXFlagsButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.xFlags, mob->sectionx.xflags,
                                      "X Flags", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->sectionx.xflags = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobResistancesButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.resistFlags, mob->sectionx.resistant,
                                      "Resistances", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->sectionx.resistant = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobImmunitiesButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.resistFlags, mob->sectionx.immune,
                                      "Immunities", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->sectionx.immune = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobSusceptibilitiesButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.resistFlags, mob->sectionx.susceptible,
                                      "Susceptibilities", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->sectionx.susceptible = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobAttacksButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.attackFlags, mob->sectionx.attacks,
                                      "Attacks", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->sectionx.attacks = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobDefensesButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    FlagsWidget *fw = new FlagsWidget(config_.defenseFlags, mob->sectionx.defenses,
                                      "Defenses", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Mobile *mob = getCurrentMob();
        if (!mob) return;
        mob->sectionx.defenses = value;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onMobProgramsButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    ProgramsWidget *w = new ProgramsWidget(&mob->programs, config_.progTypes,
                                            config_.highlighter, this, this);
        centerChildOnParent(w, this);
}

void MainWindow::onMobSpecialsButtonClicked()
{
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    MobileSpecialsWidget *w = new MobileSpecialsWidget(
        mob, &area_, config_.mobileSpecFunctions, this, this);
        centerChildOnParent(w, this);
}

void MainWindow::onMobCreditsEditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Credits must have number value!");
        ui->mobCreditsEdit->setText(QString::number(mob->credits));
        return;
    }
    mob->credits = val;
    setModified();
}

void MainWindow::onMobLevelSpinBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->level = value;
    setModified();
}

void MainWindow::onMobAlignmentBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->alignment = value;
    setModified();
}

void MainWindow::onMobAttacksNoBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionr.numattacks = value;
    setModified();
}

void MainWindow::onMobHeightBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionr.height = value;
    setModified();
}

void MainWindow::onMobWeightBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionr.weight = value;
    setModified();
}

void MainWindow::onMobHitRollBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionx.hitroll = value;
    setModified();
}

void MainWindow::onMobDamRollBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectionx.damroll = value;
    setModified();
}

void MainWindow::onMobArmorClassBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.ac = value;
    setModified();
}

void MainWindow::onMobHitNoDiceBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.hitnodice = value;
    setModified();
}

void MainWindow::onMobHitSizeDiceBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.hitsizedice = value;
    setModified();
}

void MainWindow::onMobHitPlusBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.hitplus = value;
    setModified();
}

void MainWindow::onMobDamNoDiceBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.damnodice = value;
    setModified();
}

void MainWindow::onMobDamSizeDiceBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.damsizedice = value;
    setModified();
}

void MainWindow::onMobDamPlusBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiont.damplus = value;
    setModified();
}

void MainWindow::onMobStrBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.str = value;
    setModified();
}

void MainWindow::onMobIntBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.intel = value;
    setModified();
}

void MainWindow::onMobWisBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.wis = value;
    setModified();
}

void MainWindow::onMobDexBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.dex = value;
    setModified();
}

void MainWindow::onMobConBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.con = value;
    setModified();
}

void MainWindow::onMobChaBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.cha = value;
    setModified();
}

void MainWindow::onMobLckBoxValueChanged(int value)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->sectiona.lck = value;
    setModified();
}

void MainWindow::onMobInflect0EditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->shortDesc.inflect0 = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Inflect0", "Mobile inflect0 cannot be empty!");
        ui->mobInflect0Edit->setFocus();
    }
    setModified();
}

void MainWindow::onMobInflect1EditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->shortDesc.inflect1 = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Inflect1", "Mobile inflect1 cannot be empty!");
        ui->mobInflect1Edit->setFocus();
    }
    setModified();
}

void MainWindow::onMobInflect2EditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->shortDesc.inflect2 = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Inflect2", "Mobile inflect2 cannot be empty!");
        ui->mobInflect2Edit->setFocus();
    }
    setModified();
}

void MainWindow::onMobInflect3EditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->shortDesc.inflect3 = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Inflect3", "Mobile inflect3 cannot be empty!");
        ui->mobInflect3Edit->setFocus();
    }
    setModified();
}

void MainWindow::onMobInflect4EditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->shortDesc.inflect4 = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Inflect4", "Mobile inflect4 cannot be empty!");
        ui->mobInflect4Edit->setFocus();
    }
    setModified();
}

void MainWindow::onMobInflect5EditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->shortDesc.inflect5 = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Inflect5", "Mobile inflect5 cannot be empty!");
        ui->mobInflect5Edit->setFocus();
    }
    setModified();
}

void MainWindow::onMobDialogNameEditTextChanged(const QString &str)
{
    if (!mobCanChange_) return;
    Mobile *mob = getCurrentMob();
    if (!mob) return;
    mob->dialog = str;
    setModified();
}

// ===========================================================================
// Room Data
// ===========================================================================

Room *MainWindow::getCurrentRoom()
{
    int idx = ui->roomNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.rooms.size())
        return nullptr;
    qint64 vnum = ui->roomNavigatorComboBox->currentData().toLongLong();
    for (int i = 0; i < area_.rooms.size(); ++i) {
        if (area_.rooms[i].vnum == vnum)
            return &area_.rooms[i];
    }
    return nullptr;
}

void MainWindow::fillRoomData(int roomIndex)
{
    roomCanChange_ = false;

    ui->roomNavigatorComboBox->blockSignals(true);
    ui->roomNavigatorComboBox->clear();
    for (int i = 0; i < area_.rooms.size(); ++i) {
        const Room &room = area_.rooms[i];
        ui->roomNavigatorComboBox->addItem(
            QString::number(room.vnum) + " - " + room.name,
            QVariant(room.vnum));
    }
    if (roomIndex >= 0 && roomIndex < area_.rooms.size())
        ui->roomNavigatorComboBox->setCurrentIndex(roomIndex);
    ui->roomNavigatorComboBox->blockSignals(false);

    if (roomIndex < 0 || roomIndex >= area_.rooms.size()) {
        clearRoomData();
        return;
    }

    const Room &room = area_.rooms[roomIndex];

    ui->roomVnumEdit->setText(QString::number(room.vnum));
    ui->roomNameEdit->setText(room.name);
    ui->roomDescriptionText->setPlainText(room.description);
    ui->roomNightDescriptionText->setPlainText(room.nightdesc);

    int sectorIdx = ui->roomSectorComboBox->findData(room.sector);
    if (sectorIdx >= 0) ui->roomSectorComboBox->setCurrentIndex(sectorIdx);

    ui->roomLightSpinBox->setValue(room.light);
    ui->roomFlagsEdit->setText(QString::number(room.flags));
    ui->roomTunnelSpinBox->setValue(room.tunnel);
    ui->roomTeleDelaySpinBox->setValue(room.teledelay);

    fillRoomTeleVnum(const_cast<Room *>(&room));
    fillExitData(const_cast<Room *>(&room));

    roomCanChange_ = (area_.rooms.size() > 0);
}

void MainWindow::clearRoomData()
{
    roomCanChange_ = false;

    ui->roomNavigatorComboBox->blockSignals(true);
    ui->roomNavigatorComboBox->clear();
    ui->roomNavigatorComboBox->blockSignals(false);

    ui->roomVnumEdit->clear();
    ui->roomNameEdit->clear();
    ui->roomDescriptionText->clear();
    ui->roomNightDescriptionText->clear();
    ui->roomSectorComboBox->setCurrentIndex(-1);
    ui->roomLightSpinBox->setValue(0);
    ui->roomFlagsEdit->setText("0");
    ui->roomTunnelSpinBox->setValue(0);
    ui->roomTeleDelaySpinBox->setValue(0);

    ui->roomTeleVnumComboBox->blockSignals(true);
    ui->roomTeleVnumComboBox->clear();
    ui->roomTeleVnumComboBox->blockSignals(false);

    ui->roomExitNavigatorComboBox->blockSignals(true);
    ui->roomExitNavigatorComboBox->clear();
    ui->roomExitNavigatorComboBox->blockSignals(false);

    ui->roomExitDescriptionEdit->setText("");
    ui->roomExitKeywordEdit->setText("");
    ui->roomExitFlagsEdit->setText("0");
    ui->roomExitKeyComboBox->clear();
    ui->roomExitDistanceSpinBox->setValue(0);
}

void MainWindow::fillRoomTeleVnum(Room *room)
{
    ui->roomTeleVnumComboBox->blockSignals(true);
    ui->roomTeleVnumComboBox->clear();

    ui->roomTeleVnumComboBox->addItem("0 - <no tele_vnum>", QVariant(0));

    for (int i = 0; i < area_.rooms.size(); ++i) {
        const Room &r = area_.rooms[i];
        if (&r == room) continue; // exclude current room (matching Java)
        ui->roomTeleVnumComboBox->addItem(
            QString::number(r.vnum) + " - " + r.name,
            QVariant(r.vnum));
    }

    if (room->televnum != 0) {
        int idx = ui->roomTeleVnumComboBox->findData(room->televnum);
        if (idx >= 0) {
            ui->roomTeleVnumComboBox->setCurrentIndex(idx);
        } else {
            ui->roomTeleVnumComboBox->addItem(
                QString::number(room->televnum) + " - (other area)",
                QVariant(room->televnum));
            ui->roomTeleVnumComboBox->setCurrentIndex(ui->roomTeleVnumComboBox->count() - 1);
        }
    } else {
        ui->roomTeleVnumComboBox->setCurrentIndex(0);
    }

    ui->roomTeleVnumComboBox->blockSignals(false);
}

// ===========================================================================
// Room Exit Data
// ===========================================================================

Exit *MainWindow::getCurrentExit()
{
    Room *room = getCurrentRoom();
    if (!room) return nullptr;
    int idx = ui->roomExitNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= room->exits.size())
        return nullptr;
    return &room->exits[idx];
}

void MainWindow::fillExitData(Room *room)
{
    bool tmpRoomCanChange = roomCanChange_;
    roomCanChange_ = false;

    ui->roomExitNavigatorComboBox->blockSignals(true);
    ui->roomExitNavigatorComboBox->clear();

    if (!room || room->exits.isEmpty()) {
        ui->roomExitNavigatorComboBox->blockSignals(false);
        ui->roomExitKeywordEdit->clear();
        ui->roomExitDescriptionEdit->clear();
        ui->roomExitFlagsEdit->clear();
        ui->roomExitKeyComboBox->clear();
        ui->roomExitDistanceSpinBox->setValue(0);
        roomCanChange_ = tmpRoomCanChange;
        return;
    }

    for (int i = 0; i < room->exits.size(); ++i) {
        const Exit &exit = room->exits[i];
        QString dirName;
        if (exitsMap_.contains(exit.direction))
            dirName = exitsMap_[exit.direction].name;
        else
            dirName = "dir" + QString::number(exit.direction);
        QString roomName = "<unknown>";
        for (const Room &exitRoom : area_.rooms) {
            if (exitRoom.vnum == exit.vnum) {
                roomName = exitRoom.name;
                break;
            }
        }
        ui->roomExitNavigatorComboBox->addItem(
            dirName + " to: " + QString::number(exit.vnum) + " - " + roomName,
            QVariant(i));
    }
    ui->roomExitNavigatorComboBox->setCurrentIndex(0);
    ui->roomExitNavigatorComboBox->blockSignals(false);

    fillExit(&room->exits[0]);
    roomCanChange_ = tmpRoomCanChange;
}

void MainWindow::fillExit(Exit *exit)
{
    if (!exit) return;
    bool tmpRoomCanChange = roomCanChange_;
    roomCanChange_ = false;
    ui->roomExitKeywordEdit->setText(exit->keyword);
    ui->roomExitDescriptionEdit->setText(exit->description);
    ui->roomExitFlagsEdit->setText(QString::number(exit->flags));
    ui->roomExitDistanceSpinBox->setValue(exit->distance);
    fillExitKeys(exit);
    roomCanChange_ = tmpRoomCanChange;
}

void MainWindow::fillExitKeys(Exit *exit)
{
    ui->roomExitKeyComboBox->blockSignals(true);
    ui->roomExitKeyComboBox->clear();

    for (int i = 0; i < area_.objects.size(); ++i) {
        const AreaObject &obj = area_.objects[i];
        if (obj.type == keyValue_) {
            ui->roomExitKeyComboBox->addItem(
                "(" + QString::number(obj.vnum) + ") " + obj.name,
                QVariant(obj.vnum));
        }
    }

    if (exit && exit->key != 0) {
        int idx = ui->roomExitKeyComboBox->findData(exit->key);
        if (idx >= 0) {
            ui->roomExitKeyComboBox->setCurrentIndex(idx);
        } else {
            ui->roomExitKeyComboBox->addItem(
                "(" + QString::number(exit->key) + ") (other area)",
                QVariant(exit->key));
            ui->roomExitKeyComboBox->setCurrentIndex(ui->roomExitKeyComboBox->count() - 1);
        }
    } else {
        ui->roomExitKeyComboBox->setCurrentIndex(-1);
    }

    ui->roomExitKeyComboBox->blockSignals(false);
}

void MainWindow::setLastExitIndex()
{
    Room *room = getCurrentRoom();
    if (!room || room->exits.isEmpty()) return;
    ui->roomExitNavigatorComboBox->setCurrentIndex(room->exits.size() - 1);
}

// --- Room Data slots ---

void MainWindow::onRoomNavigatorComboBoxCurrentIndexChanged(int idx)
{
    if (!roomCanChange_) return;
    if (idx < 0) return;
    fillRoomData(idx);

    if (mapWidget_ != nullptr && !mapSelection_) {
        Room *room = getCurrentRoom();
        if (room) mapWidget_->showRoom(room->vnum);
    }
}

void MainWindow::onRoomNameEditTextChanged(const QString &str)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->name = str;
    if (str.isEmpty()) {
        QMessageBox::warning(this, "Invalid Name", "Room name should not be empty!");
        ui->roomNameEdit->setFocus();
    }
    int curIdx = ui->roomNavigatorComboBox->currentIndex();
    ui->roomNavigatorComboBox->setItemText(curIdx,
        QString::number(room->vnum) + " - " + room->name);
    setModified();
}

void MainWindow::onRoomDescriptionTextChanged()
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->description = ui->roomDescriptionText->toPlainText();
    setModified();
}

void MainWindow::onRoomNightDescriptionTextChanged()
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->nightdesc = ui->roomNightDescriptionText->toPlainText();
    setModified();
}

void MainWindow::onRoomSectorComboBoxCurrentIndexChanged(int idx)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->sector = ui->roomSectorComboBox->itemData(idx).toInt();
    setModified();
}

void MainWindow::onRoomLightSpinBoxValueChanged(int value)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->light = value;
    setModified();
}

void MainWindow::onRoomFlagsEditTextChanged(const QString &str)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Room Flags must have number value!");
        ui->roomFlagsEdit->setText(QString::number(room->flags));
        return;
    }
    room->flags = val;
    setModified();
}

void MainWindow::onRoomFlagsButtonClicked()
{
    Room *room = getCurrentRoom();
    if (!room) return;
    FlagsWidget *fw = new FlagsWidget(config_.roomFlags, room->flags,
                                      "Room Flags", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Room *room = getCurrentRoom();
        if (!room) return;
        room->flags = value;
        roomCanChange_ = false;
        ui->roomFlagsEdit->setText(QString::number(value));
        roomCanChange_ = true;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onRoomTeleVnumComboBoxCurrentIndexChanged(int idx)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->televnum = ui->roomTeleVnumComboBox->itemData(idx).toLongLong();
    setModified();
}

void MainWindow::onRoomTunnelSpinBoxValueChanged(int value)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->tunnel = value;
    setModified();
}

void MainWindow::onRoomTeleDelaySpinBoxValueChanged(int value)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    room->teledelay = value;
    setModified();
}

void MainWindow::onRoomExtraDescriptionButtonClicked()
{
    Room *room = getCurrentRoom();
    if (!room) return;
    ExtraDescWidget *w = new ExtraDescWidget(&room->extradescs, this, this);
        centerChildOnParent(w, this);
}

void MainWindow::onRoomProgramsButtonClicked()
{
    Room *room = getCurrentRoom();
    if (!room) return;
    ProgramsWidget *w = new ProgramsWidget(&room->programs, config_.progTypes,
                                            config_.highlighter, this, this);
        centerChildOnParent(w, this);
}

// --- Room Exit Data slots ---

void MainWindow::onRoomExitNavigatorComboBoxCurrentIndexChanged(int idx)
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room || idx < 0 || idx >= room->exits.size()) return;
    fillExit(&room->exits[idx]);

    if (mapWidget_ != nullptr && !mapSelection_) {
        Exit *exit = getCurrentExit();
        if (exit && room) {
            ExitWrapper ew(*exit);
            mapWidget_->showExit(room->vnum, &ew);
        }
    }
}

void MainWindow::onRoomExitAddButtonClicked()
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;

    NewExitWidget *w = new NewExitWidget(room, config_.exits, config_.exitGridColumns, &area_, exitsMap_, this);
    connect(w, &NewExitWidget::exitCreated, this, [this]() {
        Room *room = getCurrentRoom();
        if (room) {
            fillExitData(room);
            setLastExitIndex();
        }
        setModified();
        statusBar()->showMessage("New exit created.", 5000);
    });
        centerChildOnParent(w, this);
}

void MainWindow::onRoomExitDeleteButtonClicked()
{
    if (!roomCanChange_) return;
    Room *room = getCurrentRoom();
    if (!room) return;
    int idx = ui->roomExitNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= room->exits.size()) return;

    Exit exitToDelete = room->exits[idx];

    // Check for reverse exit in destination room
    bool isSomewhere = exitsMap_.contains(exitToDelete.direction)
                       && exitsMap_[exitToDelete.direction].name.compare("somewhere", Qt::CaseInsensitive) == 0;

    for (int i = 0; i < area_.rooms.size(); ++i) {
        Room &destRoom = area_.rooms[i];
        if (destRoom.vnum != exitToDelete.vnum)
            continue;

        for (int j = 0; j < destRoom.exits.size(); ++j) {
            const Exit &revExit = destRoom.exits[j];
            if (revExit.vnum != room->vnum)
                continue;

            int oppositeDir = -1;
            if (exitsMap_.contains(exitToDelete.direction))
                oppositeDir = exitsMap_[exitToDelete.direction].opposite;

            bool isReverseMatch =
                (revExit.direction == oppositeDir)
                || (isSomewhere
                    && revExit.direction == exitToDelete.direction
                    && revExit.keyword == exitToDelete.keyword);

            if (isReverseMatch) {
                QMessageBox::StandardButton button =
                    QMessageBox::question(nullptr, "Deleting two way exit",
                        "Do you want to delete the corresponding reverse exit?",
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                switch (button) {
                case QMessageBox::Yes:
                    destRoom.exits.removeAt(j);
                    // fall through to break outer
                    Q_FALLTHROUGH();
                case QMessageBox::No:
                    goto outerBreak;
                default: // Cancel
                    return;
                }
            }
        }
    }
outerBreak:

    room->exits.removeAt(idx);
    fillExitData(room);
    setModified();
}

void MainWindow::onRoomExitKeywordEditTextChanged(const QString &str)
{
    if (!roomCanChange_) return;
    Exit *exit = getCurrentExit();
    if (!exit) return;
    exit->keyword = str;
    if (str.isEmpty()
        && exitsMap_.contains(exit->direction)
        && exitsMap_[exit->direction].name.compare("somewhere", Qt::CaseInsensitive) == 0) {
        QMessageBox::warning(nullptr, "Invalid Keyword",
            "Keyword for 'somewhere' exit should not be empty!");
        ui->roomExitKeywordEdit->setFocus();
    }
    setModified();
}

void MainWindow::onRoomExitDescriptionEditTextChanged(const QString &str)
{
    if (!roomCanChange_) return;
    Exit *exit = getCurrentExit();
    if (!exit) return;
    exit->description = str;
    setModified();
}

void MainWindow::onRoomExitFlagsEditTextChanged(const QString &str)
{
    if (!roomCanChange_) return;
    Exit *exit = getCurrentExit();
    if (!exit) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Exit Flags must have number value!");
        ui->roomExitFlagsEdit->setText(QString::number(exit->flags));
        return;
    }
    exit->flags = val;
    setModified();
}

void MainWindow::onRoomExitFlagsButtonClicked()
{
    Exit *exit = getCurrentExit();
    if (!exit) return;
    FlagsWidget *fw = new FlagsWidget(config_.exitFlags, exit->flags,
                                      "Exit Flags", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Exit *exit = getCurrentExit();
        if (!exit) return;
        exit->flags = value;
        roomCanChange_ = false;
        ui->roomExitFlagsEdit->setText(QString::number(value));
        roomCanChange_ = true;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onRoomExitKeyComboBoxCurrentIndexChanged(int idx)
{
    if (!roomCanChange_) return;
    Exit *exit = getCurrentExit();
    if (!exit) return;
    exit->key = ui->roomExitKeyComboBox->itemData(idx).toLongLong();
    setModified();
}

void MainWindow::onRoomExitDistanceSpinBoxValueChanged(int value)
{
    if (!roomCanChange_) return;
    Exit *exit = getCurrentExit();
    if (!exit) return;
    exit->distance = value;
    setModified();
}

// ===========================================================================
// Reset Data
// ===========================================================================

AreaReset *MainWindow::getCurrentReset()
{
    int idx = ui->resetNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.resets.size())
        return nullptr;
    return &area_.resets[idx];
}

QString MainWindow::prepareResetStr(const AreaReset &reset)
{
    if (!resetsMap_.contains(reset.command))
        return reset.command;

    const ResetInfoDef &resDef = resetsMap_[reset.command];
    QString name = resDef.name;
    name.replace(QRegularExpression("[.*]"), "");
    QString str = "[" + reset.command + "] " + name;
    if (!resDef.arg1.name.isEmpty())
        str += " {" + resDef.arg1.name + ": " + QString::number(reset.arg1) + "}";
    if (!resDef.arg2.name.isEmpty())
        str += " {" + resDef.arg2.name + ": " + QString::number(reset.arg2) + "}";
    if (!resDef.arg3.name.isEmpty())
        str += " {" + resDef.arg3.name + ": " + QString::number(reset.arg3) + "}";
    if (!resDef.arg4.name.isEmpty())
        str += " {" + resDef.arg4.name + ": " + QString::number(reset.arg4) + "}";
    if (!resDef.extra.name.isEmpty())
        str += " {" + resDef.extra.name + ": " + QString::number(reset.extra) + "}";
    return str;
}

void MainWindow::fillResetData(int resetIndex)
{
    resetCanChange_ = false;

    ui->resetNavigatorComboBox->blockSignals(true);
    ui->resetNavigatorComboBox->clear();
    for (int i = 0; i < area_.resets.size(); ++i) {
        ui->resetNavigatorComboBox->addItem(prepareResetStr(area_.resets[i]));
    }
    if (resetIndex >= 0 && resetIndex < area_.resets.size())
        ui->resetNavigatorComboBox->setCurrentIndex(resetIndex);
    ui->resetNavigatorComboBox->blockSignals(false);

    if (resetIndex < 0 || resetIndex >= area_.resets.size()) {
        clearResetData();
        return;
    }

    AreaReset *reset = &area_.resets[resetIndex];
    resetCreateWidgets(reset);

    resetCanChange_ = (area_.resets.size() > 0);
}

void MainWindow::clearResetData()
{
    resetCanChange_ = false;

    ui->resetNavigatorComboBox->blockSignals(true);
    ui->resetNavigatorComboBox->clear();
    ui->resetNavigatorComboBox->blockSignals(false);

    // Clear all dynamic widgets
    for (auto it = resetWidgets_.begin(); it != resetWidgets_.end(); ++it) {
        if (it.value()) {
            if (resetLayouts_.contains(it.key()) && resetLayouts_[it.key()])
                resetLayouts_[it.key()]->removeWidget(it.value());
            delete it.value();
            it.value() = nullptr;
        }
    }

    ui->lResetExtra->setText("Extra:");
    ui->lResetArg1->setText("Arg1:");
    ui->lResetArg2->setText("Arg2:");
    ui->lResetArg3->setText("Arg3:");
    ui->lResetArg4->setText("Arg4:");
}

void MainWindow::resetCreateWidgets(AreaReset *reset)
{
    if (!reset) return;

    // Clear previous dynamic widgets
    for (auto it = resetWidgets_.begin(); it != resetWidgets_.end(); ++it) {
        if (it.value()) {
            if (resetLayouts_.contains(it.key()) && resetLayouts_[it.key()])
                resetLayouts_[it.key()]->removeWidget(it.value());
            delete it.value();
            it.value() = nullptr;
        }
    }

    if (!resetsMap_.contains(reset->command))
        return;

    const ResetInfoDef &resDef = resetsMap_[reset->command];

    setupResetLabels(*reset);

    // Create widgets for each arg (extra=0, arg1=1, arg2=2, arg3=3, arg4=4)
    // Order matches Java: types/flags first, then type-specific, then intval/strval
    for (int argIndex = 0; argIndex <= 4; ++argIndex) {
        const ResetArgDef &argDef = getResetArgDef(resDef, argIndex);
        if (argDef.name.isEmpty())
            continue;

        QString argType = argDef.type;

        if (!argDef.values.isEmpty() && argDef.valuesType == "types") {
            resetCreateTypeWidget(reset, resDef, argIndex);
        } else if (!argDef.values.isEmpty() && argDef.valuesType == "flags") {
            resetCreateFlagsWidget(reset, resDef, argIndex);
        } else if (argType == "room" || argType == "mob" || argType == "item" || argType == "ship") {
            resetCreateVnumWidget(reset, resDef, argIndex);
        } else if (argType == "room_other" || argType == "mob_other"
                   || argType == "item_other" || argType == "ship_other") {
            resetCreateVnumWidget(reset, resDef, argIndex);
        } else if (argType == "intval") {
            resetCreateIntWidget(reset, resDef, argIndex);
        } else if (argType == "strval") {
            resetCreateStringWidget(reset, resDef, argIndex);
        } else {
            resetCreateIntWidget(reset, resDef, argIndex);
        }
    }
}

void MainWindow::resetCreateVnumWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex)
{
    const ResetArgDef &argDef = getResetArgDef(resDef, argIndex);
    qint64 curVal = getResetArgValue(*reset, argIndex);
    bool isOther = argDef.type.endsWith("_other");
    QString baseType = isOther ? argDef.type.left(argDef.type.length() - 6) : argDef.type;

    QComboBox *box = new QComboBox();
    box->setObjectName("resetArg" + QString::number(argIndex));

    int selectedIdx = -1;
    int j = 0;

    if (baseType == "room") {
        for (const Room &r : area_.rooms) {
            if (isOther) {
                if (resetIsOtherVnum(r.vnum, reset, resDef, argIndex)) {
                    box->addItem(QString::number(r.vnum) + " - " + r.name, QVariant(r.vnum));
                    if (r.vnum == curVal) selectedIdx = j;
                    j++;
                } else if (r.vnum == curVal) {
                    setResetArgValue(reset, argIndex, 0);
                    curVal = 0;
                }
            } else {
                box->addItem(QString::number(r.vnum) + " - " + r.name, QVariant(r.vnum));
                if (r.vnum == curVal) selectedIdx = j;
                j++;
            }
        }
    } else if (baseType == "mob") {
        for (const Mobile &m : area_.mobiles) {
            if (isOther) {
                if (resetIsOtherVnum(m.vnum, reset, resDef, argIndex)) {
                    box->addItem(QString::number(m.vnum) + " - " + m.name, QVariant(m.vnum));
                    if (m.vnum == curVal) selectedIdx = j;
                    j++;
                } else if (m.vnum == curVal) {
                    setResetArgValue(reset, argIndex, 0);
                    curVal = 0;
                }
            } else {
                box->addItem(QString::number(m.vnum) + " - " + m.name, QVariant(m.vnum));
                if (m.vnum == curVal) selectedIdx = j;
                j++;
            }
        }
    } else if (baseType == "item") {
        for (const AreaObject &o : area_.objects) {
            if (isOther) {
                if (resetIsOtherVnum(o.vnum, reset, resDef, argIndex)) {
                    box->addItem(QString::number(o.vnum) + " - " + o.name, QVariant(o.vnum));
                    if (o.vnum == curVal) selectedIdx = j;
                    j++;
                } else if (o.vnum == curVal) {
                    setResetArgValue(reset, argIndex, 0);
                    curVal = 0;
                }
            } else {
                box->addItem(QString::number(o.vnum) + " - " + o.name, QVariant(o.vnum));
                if (o.vnum == curVal) selectedIdx = j;
                j++;
            }
        }
    }
    // ship and ship_other: not fully supported yet (no items to iterate)

    if (selectedIdx < 0 && j > 0) {
        // vnum from another area
        addOtherVnum(box, curVal);
        selectedIdx = j;
    }
    if (selectedIdx >= 0)
        box->setCurrentIndex(selectedIdx);

    ResetVnumEventFilter *filter = new ResetVnumEventFilter(argIndex, box);
    box->installEventFilter(filter);
    connect(filter, &ResetVnumEventFilter::vnumOverridden,
            this, [this](int argIdx, qint64 vnum) {
        AreaReset *r = getCurrentReset();
        if (!r) return;
        setResetArgValue(r, argIdx, vnum);
        int resetIdx = ui->resetNavigatorComboBox->currentIndex();
        fillResetData(resetIdx);
        setModified();
    });

    connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::resetArgVnumChanged);

    QString argName = getResetArgName(argIndex);
    resetAddWidget(box, argName, argIndex);
}

void MainWindow::resetCreateIntWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex)
{
    Q_UNUSED(resDef);
    qint64 curVal = getResetArgValue(*reset, argIndex);

    QSpinBox *spin = new QSpinBox();
    spin->setObjectName("resetArg" + QString::number(argIndex));
    spin->setMinimum(-999999);
    spin->setMaximum(999999);
    spin->setValue(static_cast<int>(curVal));

    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::resetArgIntChanged);

    QString argName = getResetArgName(argIndex);
    resetAddWidget(spin, argName, argIndex);
}

void MainWindow::resetCreateStringWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex)
{
    Q_UNUSED(resDef);
    qint64 curVal = getResetArgValue(*reset, argIndex);

    QLineEdit *edit = new QLineEdit();
    edit->setObjectName("resetArg" + QString::number(argIndex));
    edit->setText(QString::number(curVal));

    connect(edit, &QLineEdit::textChanged, this, &MainWindow::resetArgStringChanged);

    QString argName = getResetArgName(argIndex);
    resetAddWidget(edit, argName, argIndex);
}

void MainWindow::resetCreateTypeWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex)
{
    const ResetArgDef &argDef = getResetArgDef(resDef, argIndex);
    qint64 curVal = getResetArgValue(*reset, argIndex);

    QComboBox *box = new QComboBox();
    box->setObjectName("resetArg" + QString::number(argIndex));

    for (const ResetArgValueDef &v : argDef.values)
        box->addItem(v.name, v.value.toInt());

    int idx = box->findData(static_cast<int>(curVal));
    if (idx >= 0) box->setCurrentIndex(idx);

    connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::resetArgTypesChanged);

    QString argName = getResetArgName(argIndex);
    resetAddWidget(box, argName, argIndex);
}

void MainWindow::resetCreateFlagsWidget(AreaReset *reset, const ResetInfoDef &resDef, int argIndex)
{
    const ResetArgDef &argDef = getResetArgDef(resDef, argIndex);
    qint64 curVal = getResetArgValue(*reset, argIndex);

    // Build flag defs from the arg values
    QList<FlagDef> flagDefs;
    for (const ResetArgValueDef &v : argDef.values) {
        FlagDef fd;
        fd.name = v.name;
        fd.value = v.value.toInt();
        flagDefs.append(fd);
    }

    ResetFlagsWidget *rfw = new ResetFlagsWidget(
        argIndex, curVal, flagDefs, reset, argDef.name, this);

    QString argName = getResetArgName(argIndex);
    resetAddWidget(rfw, argName, argIndex);
}

void MainWindow::resetAddWidget(QWidget *w, const QString &argName, int argIndex)
{
    Q_UNUSED(argIndex);
    if (!resetLayouts_.contains(argName) || !resetLayouts_[argName])
        return;
    resetWidgets_[argName] = w;
    resetLayouts_[argName]->addWidget(w);
}

void MainWindow::setupResetLabels(const AreaReset &reset)
{
    if (!resetsMap_.contains(reset.command))
        return;

    const ResetInfoDef &resDef = resetsMap_[reset.command];

    ui->lResetExtra->setText(prepareLabelName(resDef.extra.name, 0));
    ui->lResetArg1->setText(prepareLabelName(resDef.arg1.name, 1));
    ui->lResetArg2->setText(prepareLabelName(resDef.arg2.name, 2));
    ui->lResetArg3->setText(prepareLabelName(resDef.arg3.name, 3));
    ui->lResetArg4->setText(prepareLabelName(resDef.arg4.name, 4));
}

QString MainWindow::prepareLabelName(const QString &argName, int idx)
{
    if (argName.isEmpty()) {
        switch (idx) {
        case 1:  return QStringLiteral("Arg1:");
        case 2:  return QStringLiteral("Arg2:");
        case 3:  return QStringLiteral("Arg3:");
        case 4:  return QStringLiteral("Arg4:");
        default: return QStringLiteral("Extra:");
        }
    }
    return argName + ":";
}

void MainWindow::updateResetNavigatorText()
{
    AreaReset *reset = getCurrentReset();
    if (!reset) return;
    int idx = ui->resetNavigatorComboBox->currentIndex();
    if (idx >= 0)
        ui->resetNavigatorComboBox->setItemText(idx, prepareResetStr(*reset));
}

bool MainWindow::resetIsOtherVnum(qint64 vnum, AreaReset *reset, const ResetInfoDef &resDef, int argIndex)
{
    const ResetArgDef &currentArgDef = getResetArgDef(resDef, argIndex);
    // For _other types: the base type is the type without _other suffix
    QString baseType = currentArgDef.type;
    baseType.replace("_other", "");

    // Check whether the vnum appears as the value of any OTHER arg in the same
    // reset whose type is the non-other variant (the base type).
    for (int i = 0; i < MAX_RESET_ARGS; ++i) {
        if (i == argIndex) continue;
        const ResetArgDef &otherArgDef = getResetArgDef(resDef, i);
        if (otherArgDef.type == baseType
            && getResetArgValue(*reset, i) == vnum) {
            return false;
        }
    }

    return true;
}

void MainWindow::addOtherVnum(QComboBox *box, qint64 vnum)
{
    if (vnum == 0)
        box->addItem("0 - [none]", QVariant(0));
    else
        box->addItem(QString::number(vnum) + " - [from another area]", QVariant(vnum));
}

qint64 MainWindow::getResetArgValue(const AreaReset &reset, int argIndex)
{
    switch (argIndex) {
    case 0: return reset.extra;
    case 1: return reset.arg1;
    case 2: return reset.arg2;
    case 3: return reset.arg3;
    case 4: return reset.arg4;
    default: return 0;
    }
}

void MainWindow::setResetArgValue(AreaReset *reset, int argIndex, qint64 value)
{
    switch (argIndex) {
    case 0: reset->extra = value; break;
    case 1: reset->arg1 = value; break;
    case 2: reset->arg2 = value; break;
    case 3: reset->arg3 = value; break;
    case 4: reset->arg4 = value; break;
    }
}

const ResetArgDef &MainWindow::getResetArgDef(const ResetInfoDef &resDef, int argIndex)
{
    switch (argIndex) {
    case 0: return resDef.extra;
    case 1: return resDef.arg1;
    case 2: return resDef.arg2;
    case 3: return resDef.arg3;
    case 4: return resDef.arg4;
    default: return resDef.extra;
    }
}

QString MainWindow::getResetArgName(int argIndex)
{
    switch (argIndex) {
    case 0: return "extra";
    case 1: return "arg1";
    case 2: return "arg2";
    case 3: return "arg3";
    case 4: return "arg4";
    default: return "extra";
    }
}

// --- Reset Data slots ---

void MainWindow::onResetNavigatorComboBoxCurrentIndexChanged(int idx)
{
    if (!resetCanChange_) return;
    if (idx < 0) return;
    resetCanChange_ = false;
    AreaReset *reset = &area_.resets[idx];
    resetCreateWidgets(reset);
    resetCanChange_ = true;
}

void MainWindow::onResetAddButtonClicked()
{
    if (!areaLoaded_) return;

    NewResetWidget *w = new NewResetWidget(&area_, config_.resetsInfo, this);
    connect(w, &NewResetWidget::resetCreated, this, &MainWindow::resetCreated);
        centerChildOnParent(w, this);
}

void MainWindow::onResetDeleteButtonClicked()
{
    if (!resetCanChange_) return;
    int idx = ui->resetNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.resets.size()) return;

    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Deleting Reset", "Are you sure you want to delete the current reset?",
        QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    area_.resets.removeAt(idx);

    if (!area_.resets.isEmpty()) {
        int newIdx = (idx > 0) ? idx - 1 : 0;
        fillResetData(newIdx);
    } else {
        clearResetData();
    }
    setModified();
    statusBar()->showMessage("Reset deleted.", 5000);
}

// --- Dynamic reset arg slots ---

void MainWindow::resetArgIntChanged(int value)
{
    if (!resetCanChange_) return;
    AreaReset *reset = getCurrentReset();
    if (!reset) return;

    QSpinBox *spin = qobject_cast<QSpinBox *>(sender());
    if (!spin) return;

    QString name = spin->objectName();
    int argIdx = -1;
    if (name.startsWith("resetArg"))
        argIdx = name.mid(8).toInt();

    if (argIdx >= 0 && argIdx <= 4) {
        setResetArgValue(reset, argIdx, value);
        updateResetNavigatorText();
        setModified();
    }
}

void MainWindow::resetArgStringChanged(const QString &str)
{
    if (!resetCanChange_) return;
    AreaReset *reset = getCurrentReset();
    if (!reset) return;

    QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
    if (!edit) return;

    QString name = edit->objectName();
    int argIdx = -1;
    if (name.startsWith("resetArg"))
        argIdx = name.mid(8).toInt();

    if (argIdx >= 0 && argIdx <= 4) {
        bool ok;
        qint64 val = str.toLongLong(&ok);
        if (ok) {
            setResetArgValue(reset, argIdx, val);
            updateResetNavigatorText();
            setModified();
        }
    }
}

void MainWindow::resetArgVnumChanged(int idx)
{
    if (!resetCanChange_) return;
    AreaReset *reset = getCurrentReset();
    if (!reset) return;

    QComboBox *box = qobject_cast<QComboBox *>(sender());
    if (!box) return;

    QString name = box->objectName();
    int argIdx = -1;
    if (name.startsWith("resetArg"))
        argIdx = name.mid(8).toInt();

    if (argIdx >= 0 && argIdx <= 4) {
        qint64 val = box->itemData(idx).toLongLong();
        setResetArgValue(reset, argIdx, val);
        // Re-fill all reset data so _other combos re-filter, matching Java behavior
        int resetIdx = ui->resetNavigatorComboBox->currentIndex();
        fillResetData(resetIdx);
        setModified();
    }
}

void MainWindow::resetArgTypesChanged(int idx)
{
    if (!resetCanChange_) return;
    AreaReset *reset = getCurrentReset();
    if (!reset) return;

    QComboBox *box = qobject_cast<QComboBox *>(sender());
    if (!box) return;

    QString name = box->objectName();
    int argIdx = -1;
    if (name.startsWith("resetArg"))
        argIdx = name.mid(8).toInt();

    if (argIdx >= 0 && argIdx <= 4) {
        qint64 val = box->itemData(idx).toLongLong();
        setResetArgValue(reset, argIdx, val);
        updateResetNavigatorText();
        setModified();
    }
}

void MainWindow::resetArgFlagsClicked()
{
    if (!resetCanChange_) return;
    AreaReset *reset = getCurrentReset();
    if (!reset) return;

    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;

    QString name = btn->objectName();
    int argIdx = -1;
    if (name.startsWith("resetArgFlagsBtn"))
        argIdx = name.mid(16).toInt();

    if (argIdx < 0 || argIdx > 4) return;
    if (!resetsMap_.contains(reset->command)) return;

    const ResetInfoDef &resDef = resetsMap_[reset->command];
    const ResetArgDef &argDef = getResetArgDef(resDef, argIdx);

    // Build flag defs from the arg values
    QList<FlagDef> flagDefs;
    for (const ResetArgValueDef &v : argDef.values) {
        FlagDef fd;
        fd.name = v.name;
        fd.value = v.value.toInt();
        flagDefs.append(fd);
    }

    qint64 currentValue = getResetArgValue(*reset, argIdx);

    FlagsWidget *fw = new FlagsWidget(flagDefs, currentValue, argDef.name, true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this, argIdx](qint64 value) {
        AreaReset *reset = getCurrentReset();
        if (!reset) return;
        setResetArgValue(reset, argIdx, value);

        // Update the line edit
        QLineEdit *edit = findChild<QLineEdit *>("resetArg" + QString::number(argIdx));
        if (edit) {
            edit->setText(QString::number(value));
        }

        updateResetNavigatorText();
        setModified();
    });
        centerChildOnParent(fw, this);
}

// ===========================================================================
// Shop Data
// ===========================================================================

Shop *MainWindow::getCurrentShop()
{
    int idx = ui->shopKeeperBox->currentIndex();
    if (idx < 0 || idx >= ui->shopKeeperBox->count())
        return nullptr;
    qint64 keeperVnum = ui->shopKeeperBox->currentData().toLongLong();
    return getShopByKeeperVnum(keeperVnum);
}

Shop *MainWindow::getShopByKeeperVnum(qint64 keeperVnum)
{
    for (int i = 0; i < area_.shops.size(); ++i) {
        if (area_.shops[i].keeper == keeperVnum)
            return &area_.shops[i];
    }
    return nullptr;
}

void MainWindow::fillShopData(int shopIndex)
{
    shopCanChange_ = false;

    ui->shopKeeperBox->blockSignals(true);
    ui->shopKeeperBox->clear();
    // Iterate mobiles first, adding those that are shopkeepers (matching Java order)
    for (const Mobile &m : area_.mobiles) {
        for (const Shop &shop : area_.shops) {
            if (shop.keeper == m.vnum) {
                ui->shopKeeperBox->addItem(
                    QString::number(m.vnum) + " - " + m.name,
                    QVariant(m.vnum));
                break;
            }
        }
    }
    if (shopIndex >= 0 && shopIndex < ui->shopKeeperBox->count())
        ui->shopKeeperBox->setCurrentIndex(shopIndex);
    ui->shopKeeperBox->blockSignals(false);

    if (shopIndex < 0 || shopIndex >= area_.shops.size()) {
        clearShopData();
        return;
    }

    const Shop &shop = area_.shops[shopIndex];

    QComboBox *shopBoxes[] = {
        ui->shopType0Box, ui->shopType1Box, ui->shopType2Box,
        ui->shopType3Box, ui->shopType4Box
    };
    int typeValues[] = { shop.types.type0, shop.types.type1, shop.types.type2,
                         shop.types.type3, shop.types.type4 };
    for (int i = 0; i < 5; ++i) {
        int idx = shopBoxes[i]->findData(typeValues[i]);
        if (idx >= 0) shopBoxes[i]->setCurrentIndex(idx);
    }

    ui->shopProfitBuyBox->setValue(shop.profitbuy);
    ui->shopProfitSellBox->setValue(shop.profitsell);
    ui->shopOpenBox->setValue(shop.open);
    ui->shopCloseBox->setValue(shop.close);
    ui->shopFlagsEdit->setText(QString::number(shop.flags));

    shopCanChange_ = (area_.shops.size() > 0);
}

void MainWindow::clearShopData()
{
    shopCanChange_ = false;

    ui->shopKeeperBox->blockSignals(true);
    ui->shopKeeperBox->clear();
    ui->shopKeeperBox->blockSignals(false);

    ui->shopType0Box->setCurrentIndex(-1);
    ui->shopType1Box->setCurrentIndex(-1);
    ui->shopType2Box->setCurrentIndex(-1);
    ui->shopType3Box->setCurrentIndex(-1);
    ui->shopType4Box->setCurrentIndex(-1);
    ui->shopProfitBuyBox->setValue(0);
    ui->shopProfitSellBox->setValue(0);
    ui->shopOpenBox->setValue(0);
    ui->shopCloseBox->setValue(0);
    ui->shopFlagsEdit->clear();
}

// --- Shop Data slots ---

void MainWindow::onShopKeeperBoxCurrentIndexChanged(int idx)
{
    if (!shopCanChange_) return;
    if (idx < 0) return;
    fillShopData(idx);
}

void MainWindow::onShopFlagsEditTextChanged(const QString &str)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    bool ok;
    qint64 val = str.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Number Value", "Shop Flags must have number value!");
        ui->shopFlagsEdit->setText(QString::number(shop->flags));
        return;
    }
    shop->flags = val;
    setModified();
}

void MainWindow::onShopFlagsButtonClicked()
{
    Shop *shop = getCurrentShop();
    if (!shop) return;
    FlagsWidget *fw = new FlagsWidget(config_.shopFlags, shop->flags,
                                      "Shop Flags", true, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        Shop *shop = getCurrentShop();
        if (!shop) return;
        shop->flags = value;
        shopCanChange_ = false;
        ui->shopFlagsEdit->setText(QString::number(value));
        shopCanChange_ = true;
        setModified();
    });
        centerChildOnParent(fw, this);
}

void MainWindow::onShopOpenBoxValueChanged(int val)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->open = val;
    setModified();
}

void MainWindow::onShopCloseBoxValueChanged(int val)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->close = val;
    setModified();
}

void MainWindow::onShopProfitBuyBoxValueChanged(int val)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->profitbuy = val;
    setModified();
}

void MainWindow::onShopProfitSellBoxValueChanged(int val)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->profitsell = val;
    setModified();
}

void MainWindow::onShopType0BoxCurrentIndexChanged(int idx)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->types.type0 = ui->shopType0Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onShopType1BoxCurrentIndexChanged(int idx)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->types.type1 = ui->shopType1Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onShopType2BoxCurrentIndexChanged(int idx)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->types.type2 = ui->shopType2Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onShopType3BoxCurrentIndexChanged(int idx)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->types.type3 = ui->shopType3Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onShopType4BoxCurrentIndexChanged(int idx)
{
    if (!shopCanChange_) return;
    Shop *shop = getCurrentShop();
    if (!shop) return;
    shop->types.type4 = ui->shopType4Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onShopAddButtonClicked()
{
    if (!areaLoaded_) return;
    NewShopWidget *w = new NewShopWidget(&area_, false, this);
    connect(w, &NewShopWidget::vnumChosen, this, &MainWindow::shopVnumChosen);
        centerChildOnParent(w, this);
}

void MainWindow::onShopDeleteButtonClicked()
{
    if (!shopCanChange_) return;
    int idx = ui->shopKeeperBox->currentIndex();
    if (idx < 0 || idx >= area_.shops.size()) return;

    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Deleting Shop", "Are you sure you want to delete the current shop?",
        QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    area_.shops.removeAt(idx);

    if (!area_.shops.isEmpty()) {
        int newIdx = (idx > 0) ? idx - 1 : 0;
        fillShopData(newIdx);
    } else {
        clearShopData();
    }

    setModified();
    statusBar()->showMessage("Shop deleted.", 5000);
}

// ===========================================================================
// Repair Data
// ===========================================================================

Repair *MainWindow::getCurrentRepair()
{
    int idx = ui->repairKeeperBox->currentIndex();
    if (idx < 0 || idx >= ui->repairKeeperBox->count())
        return nullptr;
    qint64 keeperVnum = ui->repairKeeperBox->currentData().toLongLong();
    return getRepairByKeeperVnum(keeperVnum);
}

Repair *MainWindow::getRepairByKeeperVnum(qint64 keeperVnum)
{
    for (int i = 0; i < area_.repairs.size(); ++i) {
        if (area_.repairs[i].keeper == keeperVnum)
            return &area_.repairs[i];
    }
    return nullptr;
}

void MainWindow::fillRepairData(int repairIndex)
{
    repairCanChange_ = false;

    ui->repairKeeperBox->blockSignals(true);
    ui->repairKeeperBox->clear();
    // Iterate mobiles first, adding those that are repair keepers (matching Java order)
    for (const Mobile &m : area_.mobiles) {
        for (const Repair &repair : area_.repairs) {
            if (repair.keeper == m.vnum) {
                ui->repairKeeperBox->addItem(
                    QString::number(m.vnum) + " - " + m.name,
                    QVariant(m.vnum));
                break;
            }
        }
    }
    if (repairIndex >= 0 && repairIndex < ui->repairKeeperBox->count())
        ui->repairKeeperBox->setCurrentIndex(repairIndex);
    ui->repairKeeperBox->blockSignals(false);

    if (repairIndex < 0 || repairIndex >= area_.repairs.size()) {
        clearRepairData();
        return;
    }

    const Repair &repair = area_.repairs[repairIndex];

    QComboBox *repairBoxes[] = {
        ui->repairType0Box, ui->repairType1Box, ui->repairType2Box
    };
    int typeValues[] = { repair.types.type0, repair.types.type1, repair.types.type2 };
    for (int i = 0; i < 3; ++i) {
        int idx = repairBoxes[i]->findData(typeValues[i]);
        if (idx >= 0) repairBoxes[i]->setCurrentIndex(idx);
    }

    int stIdx = ui->repairShopTypeBox->findData(repair.shoptype);
    if (stIdx >= 0) ui->repairShopTypeBox->setCurrentIndex(stIdx);

    ui->repairProfitFixBox->setValue(repair.profitfix);
    ui->repairOpenBox->setValue(repair.open);
    ui->repairCloseBox->setValue(repair.close);

    repairCanChange_ = (area_.repairs.size() > 0);
}

void MainWindow::clearRepairData()
{
    repairCanChange_ = false;

    ui->repairKeeperBox->blockSignals(true);
    ui->repairKeeperBox->clear();
    ui->repairKeeperBox->blockSignals(false);

    ui->repairType0Box->setCurrentIndex(-1);
    ui->repairType1Box->setCurrentIndex(-1);
    ui->repairType2Box->setCurrentIndex(-1);
    ui->repairShopTypeBox->setCurrentIndex(-1);
    ui->repairProfitFixBox->setValue(0);
    ui->repairOpenBox->setValue(0);
    ui->repairCloseBox->setValue(0);
}

// --- Repair Data slots ---

void MainWindow::onRepairKeeperBoxCurrentIndexChanged(int idx)
{
    if (!repairCanChange_) return;
    if (idx < 0) return;
    fillRepairData(idx);
}

void MainWindow::onRepairShopTypeBoxCurrentIndexChanged(int idx)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->shoptype = ui->repairShopTypeBox->itemData(idx).toInt();
    setModified();
}

void MainWindow::onRepairOpenBoxValueChanged(int val)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->open = val;
    setModified();
}

void MainWindow::onRepairCloseBoxValueChanged(int val)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->close = val;
    setModified();
}

void MainWindow::onRepairProfitFixBoxValueChanged(int val)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->profitfix = val;
    setModified();
}

void MainWindow::onRepairType0BoxCurrentIndexChanged(int idx)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->types.type0 = ui->repairType0Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onRepairType1BoxCurrentIndexChanged(int idx)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->types.type1 = ui->repairType1Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onRepairType2BoxCurrentIndexChanged(int idx)
{
    if (!repairCanChange_) return;
    Repair *repair = getCurrentRepair();
    if (!repair) return;
    repair->types.type2 = ui->repairType2Box->itemData(idx).toInt();
    setModified();
}

void MainWindow::onRepairAddButtonClicked()
{
    if (!areaLoaded_) return;
    NewShopWidget *w = new NewShopWidget(&area_, true, this);
    connect(w, &NewShopWidget::vnumChosen, this, &MainWindow::repairVnumChosen);
        centerChildOnParent(w, this);
}

void MainWindow::onRepairDeleteButtonClicked()
{
    if (!repairCanChange_) return;
    int idx = ui->repairKeeperBox->currentIndex();
    if (idx < 0 || idx >= area_.repairs.size()) return;

    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Deleting Repair", "Are you sure you want to delete the current repair?",
        QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    area_.repairs.removeAt(idx);

    if (!area_.repairs.isEmpty()) {
        int newIdx = (idx > 0) ? idx - 1 : 0;
        fillRepairData(newIdx);
    } else {
        clearRepairData();
    }

    setModified();
    statusBar()->showMessage("Repair deleted.", 5000);
}

// ===========================================================================
// Entity creation helpers
// ===========================================================================

ShortDesc MainWindow::newShortDesc()
{
    ShortDesc sd;
    sd.inflect0 = "";
    sd.inflect1 = "";
    sd.inflect2 = "";
    sd.inflect3 = "";
    sd.inflect4 = "";
    sd.inflect5 = "";
    return sd;
}

qint64 MainWindow::findNextFreeVnum(const QString &entityType)
{
    qint64 lvnum = area_.head.vnums.lvnum;
    qint64 uvnum = area_.head.vnums.uvnum;

    for (qint64 v = lvnum; v <= uvnum; ++v) {
        bool used = false;

        if (entityType == "item") {
            for (const AreaObject &o : area_.objects) {
                if (o.vnum == v) { used = true; break; }
            }
        } else if (entityType == "mobile") {
            for (const Mobile &m : area_.mobiles) {
                if (m.vnum == v) { used = true; break; }
            }
        } else if (entityType == "room") {
            for (const Room &r : area_.rooms) {
                if (r.vnum == v) { used = true; break; }
            }
        }

        if (!used)
            return v;
    }

    return -1;
}

AreaObject MainWindow::newItem()
{
    qint64 vnum = findNextFreeVnum("item");
    if (vnum < 0) {
        QMessageBox::warning(this, "Warning", "No free vnums available for items!");
        return AreaObject();
    }

    AreaObject obj;
    obj.vnum = vnum;
    obj.name = "i" + QString::number(vnum);
    obj.shortDesc = newShortDesc();
    obj.description = "";
    obj.actiondesc = "";
    obj.type = 0;
    obj.extraflags = 0;
    obj.wearflags = 0;
    obj.layers = 0;
    obj.values.value0 = 0;
    obj.values.value1 = 0;
    obj.values.value2 = 0;
    obj.values.value3 = "0";
    obj.values.value4 = "0";
    obj.values.value5 = "0";
    obj.weight = 0;
    obj.cost = 0;
    obj.gender = 0;
    obj.level = 0;
    return obj;
}

Mobile MainWindow::newMobile()
{
    qint64 vnum = findNextFreeVnum("mobile");
    if (vnum < 0) {
        QMessageBox::warning(this, "Warning", "No free vnums available for mobiles!");
        return Mobile();
    }

    Mobile mob;
    mob.vnum = vnum;
    mob.name = "m" + QString::number(vnum);
    mob.shortDesc = newShortDesc();
    mob.longDesc = "";
    mob.description = "";
    mob.act = 1; // NPC
    mob.affected = 0;
    mob.alignment = 0;
    mob.sex = 1; // male
    mob.credits = 0;
    mob.position = 8; // standing
    mob.level = 0;

    if (!config_.races.isEmpty())
        mob.race = config_.races[0];

    if (!config_.languages.isEmpty())
        mob.sectionr.speaking = config_.languages[0];

    mob.dialog = "";
    return mob;
}

Room MainWindow::newRoom()
{
    qint64 vnum = findNextFreeVnum("room");
    if (vnum < 0) {
        QMessageBox::warning(this, "Warning", "No free vnums available for rooms!");
        return Room();
    }

    Room room;
    room.vnum = vnum;
    room.name = "r" + QString::number(vnum);
    room.description = "";
    room.nightdesc = "";
    room.light = 1;
    room.flags = 0;
    room.teledelay = 0;
    room.televnum = 0;
    room.tunnel = 0;

    if (!config_.roomSectorTypes.isEmpty())
        room.sector = config_.roomSectorTypes[0].value;

    return room;
}

Shop MainWindow::newShop(qint64 keeperVnum)
{
    Shop shop;
    shop.keeper = keeperVnum;
    shop.types.type0 = 0;
    shop.types.type1 = 0;
    shop.types.type2 = 0;
    shop.types.type3 = 0;
    shop.types.type4 = 0;
    shop.profitbuy = 120;
    shop.profitsell = 0;
    shop.open = 10;
    shop.close = 20;
    shop.flags = 0;
    return shop;
}

Repair MainWindow::newRepair(qint64 keeperVnum)
{
    Repair repair;
    repair.keeper = keeperVnum;
    repair.types.type0 = 0;
    repair.types.type1 = 0;
    repair.types.type2 = 0;
    repair.profitfix = 100;
    repair.shoptype = config_.repairTypes.isEmpty() ? 0 : config_.repairTypes.first().value;
    repair.open = 10;
    repair.close = 20;
    return repair;
}

// ===========================================================================
// Menu action slots
// ===========================================================================

void MainWindow::onActionQuit()
{
    close();
}

void MainWindow::onActionOpenArea()
{
    openArea();
}

void MainWindow::onActionSaveArea()
{
    saveArea();
}

void MainWindow::onActionSaveAreaAs()
{
    saveAreaAs();
}

void MainWindow::onActionCreateNewArea()
{
    if (!canLeaveCurrent()) return;

    NewAreaWidget *w = new NewAreaWidget(this);
    connect(w, &NewAreaWidget::areaCreated, this, [this](Area area) {
        area_ = area;
        areaLoaded_ = true;
        currentFileName_.clear();
        fillAll();
        setModified();
        statusBar()->showMessage("New area created.", 5000);
        ui->tabWidget->setCurrentIndex(0);
        ui->nameEdit->setFocus();
    });
        centerChildOnParent(w, this);
}

void MainWindow::onActionRenumber()
{
    if (!areaLoaded_) return;

    RenumberWidget *w = new RenumberWidget(this);
    connect(w, &RenumberWidget::paramsSpecified, this, &MainWindow::renumber);
        centerChildOnParent(w, this);
}

void MainWindow::onActionCreateNewItem()
{
    if (!areaLoaded_) return;

    AreaObject obj = newItem();
    if (obj.vnum <= 0) return;

    area_.objects.append(obj);
    fillItemData(area_.objects.size() - 1);
    setModified();
    statusBar()->showMessage("New item created.", 5000);
}

void MainWindow::onActionDeleteCurrentItem()
{
    if (!itemCanChange_) return;
    int idx = ui->itemNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.objects.size()) return;

    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Deleting Item", "Are you sure you want to delete the current item?",
        QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    area_.objects.removeAt(idx);

    if (!area_.objects.isEmpty()) {
        int newIdx = (idx > 0) ? idx - 1 : 0;
        fillItemData(newIdx);
    } else {
        clearItemData();
    }
    setModified();
    statusBar()->showMessage("Item deleted.", 5000);
}

void MainWindow::onActionCreateNewMobile()
{
    if (!areaLoaded_) return;

    Mobile mob = newMobile();
    if (mob.vnum <= 0) return;

    area_.mobiles.append(mob);
    fillMobileData(area_.mobiles.size() - 1);
    setModified();
    statusBar()->showMessage("New mobile created.", 5000);
}

void MainWindow::onActionDeleteCurrentMobile()
{
    if (!mobCanChange_) return;
    int idx = ui->mobNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.mobiles.size()) return;

    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Deleting Mobile", "Are you sure you want to delete the current mobile?",
        QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    area_.mobiles.removeAt(idx);

    if (!area_.mobiles.isEmpty()) {
        int newIdx = (idx > 0) ? idx - 1 : 0;
        fillMobileData(newIdx);
    } else {
        clearMobileData();
    }
    setModified();
    statusBar()->showMessage("Mobile deleted.", 5000);
}

void MainWindow::onActionCreateNewRoom()
{
    if (!areaLoaded_) return;

    Room room = newRoom();
    if (room.vnum <= 0) return;

    area_.rooms.append(room);
    fillRoomData(area_.rooms.size() - 1);
    setModified();
    statusBar()->showMessage("New room created.", 5000);
}

void MainWindow::onActionDeleteCurrentRoom()
{
    if (!roomCanChange_) return;
    int idx = ui->roomNavigatorComboBox->currentIndex();
    if (idx < 0 || idx >= area_.rooms.size()) return;

    QMessageBox::StandardButton button = QMessageBox::question(
        this, "Deleting Room", "Are you sure you want to delete the current room?",
        QMessageBox::Yes | QMessageBox::No);
    if (button != QMessageBox::Yes) return;

    area_.rooms.removeAt(idx);

    if (!area_.rooms.isEmpty()) {
        int newIdx = (idx > 0) ? idx - 1 : 0;
        fillRoomData(newIdx);
    } else {
        clearRoomData();
    }
    setModified();
    statusBar()->showMessage("Room deleted.", 5000);
}

void MainWindow::onActionShowMap()
{
    if (!areaLoaded_)
        return;

    if (area_.rooms.isEmpty()) {
        QMessageBox::information(this, "Information", "No rooms in area to map.");
        return;
    }

    if (mapWidget_ != nullptr) {
        // Refresh existing map
        Mapper mapper(&area_);
        mapper.makeMap(mapWidget_);
        mapWidget_->setWindowTitle(area_.head.name);
        mapWidget_->raise();
        mapWidget_->activateWindow();
        return;
    }

    Mapper mapper(&area_);
    mapWidget_ = mapper.makeMap();
    if (mapWidget_ == nullptr) {
        QMessageBox::warning(this, "Error", "Failed to create map widget.");
        return;
    }

    mapWidget_->setWindowTitle(area_.head.name);
    mapWidget_->setWindowIcon(QApplication::windowIcon());
    mapWidget_->setAttribute(Qt::WA_DeleteOnClose);

    connect(mapWidget_, &MapWidget::vnumSelected,
            this, &MainWindow::mapRoomVnumSelected);
    connect(mapWidget_, &MapWidget::exitSelected,
            this, &MainWindow::mapRoomExitSelected);
    connect(mapWidget_, &MapWidget::windowClosed,
            this, &MainWindow::mapClosed);
    connect(mapWidget_, &MapWidget::mapRefreshed,
            this, &MainWindow::mapRefreshed);
    connect(mapWidget_, &MapWidget::roomMarkingFlagRequested,
            this, &MainWindow::mapRoomMarkingFlagRequested);

    mapWidget_->show();
}

void MainWindow::onActionCreateNewReset()
{
    onResetAddButtonClicked();
}

void MainWindow::onActionDeleteCurrentReset()
{
    onResetDeleteButtonClicked();
}

void MainWindow::onActionCreateNewShop()
{
    onShopAddButtonClicked();
}

void MainWindow::onActionDeleteCurrentShop()
{
    onShopDeleteButtonClicked();
}

void MainWindow::onActionCreateNewRepair()
{
    onRepairAddButtonClicked();
}

void MainWindow::onActionDeleteCurrentRepair()
{
    onRepairDeleteButtonClicked();
}

void MainWindow::onActionMessages()
{
    QMessageBox::information(this, "Information", "Messages not yet implemented.");
}

// ===========================================================================
// Callback slots (from child dialogs/widgets)
// ===========================================================================

void MainWindow::resetCreated()
{
    fillResetData(area_.resets.size() - 1);
    setModified();
    statusBar()->showMessage("New reset created.", 5000);
}

void MainWindow::shopVnumChosen(qint64 vnum)
{
    Shop shop = newShop(vnum);
    area_.shops.append(shop);
    fillShopData(area_.shops.size() - 1);
    setModified();
    statusBar()->showMessage("New shop created.", 5000);
}

void MainWindow::repairVnumChosen(qint64 vnum)
{
    Repair repair = newRepair(vnum);
    area_.repairs.append(repair);
    fillRepairData(area_.repairs.size() - 1);
    setModified();
    statusBar()->showMessage("New repair created.", 5000);
}

void MainWindow::mapClosed()
{
    mapWidget_ = nullptr;
}

void MainWindow::mapRefreshed()
{
    if (mapWidget_ == nullptr || !areaLoaded_)
        return;

    Mapper mapper(&area_);
    mapper.makeMap(mapWidget_);
}

void MainWindow::mapRoomMarkingFlagRequested()
{
    if (mapWidget_ == nullptr)
        return;

    FlagsWidget *fw = new FlagsWidget(config_.roomFlags, 0,
                                      QStringLiteral("Map Room Marking Flag"),
                                      false, mapWidget_);
    fw->setAttribute(Qt::WA_DeleteOnClose);
    connect(fw, &FlagsWidget::flagsAccepted,
            this, &MainWindow::mapRoomMarkingFlagAccepted);
    fw->show();
}

void MainWindow::mapRoomMarkingFlagAccepted(qint64 value)
{
    if (mapWidget_ != nullptr) {
        mapWidget_->setRoomMarkingFlag(value);
    }
}

void MainWindow::splashScreenClosed()
{
    show();
    openArea();
}

void MainWindow::sysTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        setVisible(!isVisible());
    }
}

// ===========================================================================
// Map helpers
// ===========================================================================

void MainWindow::mapRoomVnumSelected(qint64 vnum)
{
    mapSelection_ = true;
    for (int i = 0; i < area_.rooms.size(); ++i) {
        if (area_.rooms[i].vnum == vnum) {
            fillRoomData(i);
            break;
        }
    }
    mapSelection_ = false;
}

void MainWindow::mapRoomExitSelected(qint64 ownerRoomVnum, int exitDirection, qint64 destRoomVnum)
{
    mapSelection_ = true;

    for (int i = 0; i < area_.rooms.size(); ++i) {
        if (area_.rooms[i].vnum == ownerRoomVnum) {
            fillRoomData(i);

            Room *room = &area_.rooms[i];
            for (int j = 0; j < room->exits.size(); ++j) {
                if (room->exits[j].direction == exitDirection &&
                    room->exits[j].vnum == destRoomVnum) {
                    ui->roomExitNavigatorComboBox->setCurrentIndex(j);
                    break;
                }
            }
            break;
        }
    }
    mapSelection_ = false;
}

// ===========================================================================
// Renumber (stub)
// ===========================================================================

void MainWindow::renumber(qint64 newFirstVnum, int optionsFlags)
{
    Renumberer renumberer(&area_, newFirstVnum, optionsFlags, resetsMap_);
    renumberer.renumber();

    fillAll();
    setModified();
    statusBar()->showMessage("Area renumbered to start at vnum " +
                             QString::number(newFirstVnum) + ".", 5000);

    QStringList warnings = renumberer.getWarnings();
    if (!warnings.isEmpty()) {
        // Save warnings to file
        QString warningsPath = currentFileName_.isEmpty()
            ? "renumber_warnings.txt"
            : currentFileName_ + ".renumber_warnings.txt";
        renumberer.saveWarnings(warningsPath);

        // Show warnings dialog
        RenumberWarningsWidget *ww = new RenumberWarningsWidget(warnings, this);
                centerChildOnParent(ww, this);
    }
}

// ===========================================================================
// Utility
// ===========================================================================

void MainWindow::centerChildOnParent(QWidget *child, QWidget *parent)
{
    if (!child || !parent) return;
    child->adjustSize();
    child->show();
    QPoint parentCenter = parent->mapToGlobal(parent->rect().center());
    child->move(parentCenter - QPoint(child->frameGeometry().width() / 2,
                                       child->frameGeometry().height() / 2));
}
