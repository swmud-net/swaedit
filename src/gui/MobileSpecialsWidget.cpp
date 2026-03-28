#include "gui/MobileSpecialsWidget.h"
#include "gui/MainWindow.h"
#include "ui_MobileSpecialsWidget.h"

MobileSpecialsWidget::MobileSpecialsWidget(Mobile *mob,
                                           Area *area,
                                           const QList<QString> &specFunctions,
                                           MainWindow *mainWindow,
                                           QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MobileSpecialsWidget)
    , mob_(mob)
    , area_(area)
    , specFunctions_(specFunctions)
    , mainWindow_(mainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    // Look up existing Special for this mob
    for (int i = 0; i < area_->specials.size(); ++i) {
        if (area_->specials[i].vnum == mob_->vnum) {
            workingSpecial_ = area_->specials[i];
            hasExistingSpecial_ = true;
            existingSpecialIndex_ = i;
            break;
        }
    }

    if (!hasExistingSpecial_) {
        workingSpecial_.vnum = mob_->vnum;
        workingSpecial_.function = "";
        workingSpecial_.function2 = "";
    }

    // Fill spec1Box: empty entry first, then all spec functions
    ui->spec1Box->clear();
    ui->spec1Box->addItem("");
    for (const QString &sf : specFunctions_) {
        ui->spec1Box->addItem(sf);
    }

    // Set current spec1
    if (!workingSpecial_.function.isEmpty()) {
        int idx = ui->spec1Box->findText(workingSpecial_.function);
        if (idx >= 0) ui->spec1Box->setCurrentIndex(idx);
    }

    // Fill spec2
    fillSpec2Box();

    connect(ui->spec1Box, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MobileSpecialsWidget::onSpec1Changed);
    connect(ui->acceptButton, &QPushButton::clicked,
            this, &MobileSpecialsWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &MobileSpecialsWidget::onCancelClicked);
}

MobileSpecialsWidget::~MobileSpecialsWidget()
{
    delete ui;
}

void MobileSpecialsWidget::fillSpec2Box()
{
    ui->spec2Box->blockSignals(true);
    ui->spec2Box->clear();

    QString spec1Selection = ui->spec1Box->currentText();

    if (spec1Selection.isEmpty()) {
        ui->spec2Box->setEnabled(false);
        ui->spec2Box->blockSignals(false);
        return;
    }

    ui->spec2Box->setEnabled(true);
    ui->spec2Box->addItem("");

    for (const QString &sf : specFunctions_) {
        if (sf != spec1Selection) {
            ui->spec2Box->addItem(sf);
        }
    }

    // Set current spec2
    if (!workingSpecial_.function2.isEmpty()) {
        int idx = ui->spec2Box->findText(workingSpecial_.function2);
        if (idx >= 0) ui->spec2Box->setCurrentIndex(idx);
    }

    ui->spec2Box->blockSignals(false);
}

void MobileSpecialsWidget::onSpec1Changed(int idx)
{
    Q_UNUSED(idx);
    fillSpec2Box();
}

void MobileSpecialsWidget::onAcceptClicked()
{
    workingSpecial_.function = ui->spec1Box->currentText();
    workingSpecial_.function2 = ui->spec2Box->currentText();

    if (hasExistingSpecial_) {
        // Update existing
        if (existingSpecialIndex_ >= 0 && existingSpecialIndex_ < area_->specials.size()) {
            area_->specials[existingSpecialIndex_] = workingSpecial_;
        }
    } else {
        // Create new
        area_->specials.append(workingSpecial_);
    }

    if (mainWindow_) {
        mainWindow_->setModified();
    }

    close();
}

void MobileSpecialsWidget::onCancelClicked()
{
    close();
}
