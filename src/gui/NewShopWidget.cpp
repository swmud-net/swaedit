#include "gui/NewShopWidget.h"
#include "ui_NewShopWidget.h"

NewShopWidget::NewShopWidget(Area *area, bool isRepair, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NewShopWidget)
    , area_(area)
    , isRepair_(isRepair)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    if (isRepair_) {
        setWindowTitle("New Repair Shop");
    } else {
        setWindowTitle("New Shop");
    }

    // Fill keeper box with mobiles that don't already have a shop (or repair)
    ui->keeperBox->clear();
    for (const Mobile &mob : area_->mobiles) {
        bool alreadyHasShop = false;

        if (isRepair_) {
            for (const Repair &r : area_->repairs) {
                if (r.keeper == mob.vnum) {
                    alreadyHasShop = true;
                    break;
                }
            }
        } else {
            for (const Shop &s : area_->shops) {
                if (s.keeper == mob.vnum) {
                    alreadyHasShop = true;
                    break;
                }
            }
        }

        if (!alreadyHasShop) {
            ui->keeperBox->addItem(
                QString::number(mob.vnum) + " - " + mob.name,
                QVariant(mob.vnum));
        }
    }

    connect(ui->acceptButton, &QPushButton::clicked, this, &NewShopWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &NewShopWidget::onCancelClicked);
}

NewShopWidget::~NewShopWidget()
{
    delete ui;
}

void NewShopWidget::onAcceptClicked()
{
    if (ui->keeperBox->currentIndex() < 0) return;

    qint64 keeperVnum = ui->keeperBox->currentData().toLongLong();
    emit vnumChosen(keeperVnum);
    close();
}

void NewShopWidget::onCancelClicked()
{
    close();
}
