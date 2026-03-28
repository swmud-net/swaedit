#include "gui/NewAreaWidget.h"
#include "ui_NewAreaWidget.h"

#include <QMessageBox>

NewAreaWidget::NewAreaWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NewAreaWidget)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    // 100 is checked by default (from .ui)
    ui->vnumsOtherBox->setEnabled(false);

    connect(ui->vnumsOtherRadio, &QRadioButton::toggled,
            this, &NewAreaWidget::onOtherRadioToggled);
    connect(ui->acceptButton, &QPushButton::clicked,
            this, &NewAreaWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &NewAreaWidget::onCancelClicked);
}

NewAreaWidget::~NewAreaWidget()
{
    delete ui;
}

void NewAreaWidget::onOtherRadioToggled(bool checked)
{
    ui->vnumsOtherBox->setEnabled(checked);
}

void NewAreaWidget::onAcceptClicked()
{
    int amount = 0;

    if (ui->vnums50Radio->isChecked()) {
        amount = 50;
    } else if (ui->vnums100Radio->isChecked()) {
        amount = 100;
    } else if (ui->vnums150Radio->isChecked()) {
        amount = 150;
    } else if (ui->vnumsOtherRadio->isChecked()) {
        amount = ui->vnumsOtherBox->value();
    }

    // Validate
    if (amount < 50 || (amount % 50) != 0) {
        QMessageBox::critical(this, "Invalid Vnum Ammount",
            "Vnum ammount must be greater than 49 and divisible by 50!");
        return;
    }

    // Create new area with defaults
    Area area;
    area.head.name = "";
    area.head.authors = "";
    area.head.builders = "";
    area.head.security = 0;
    area.head.vnums.lvnum = 1;
    area.head.vnums.uvnum = amount;
    area.head.flags = 0;
    area.head.economy.low = 0;
    area.head.economy.high = 0;
    area.head.reset.frequency = 120;
    area.head.reset.message = "";
    area.head.ranges.low = 1;
    area.head.ranges.high = 100;

    emit areaCreated(area);
    close();
}

void NewAreaWidget::onCancelClicked()
{
    close();
}
