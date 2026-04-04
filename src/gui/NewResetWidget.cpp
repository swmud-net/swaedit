#include "gui/NewResetWidget.h"
#include "ui_NewResetWidget.h"

NewResetWidget::NewResetWidget(Area *area,
                               const QList<ResetInfoDef> &resetsInfo,
                               QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NewResetWidget)
    , area_(area)
    , resetsInfo_(resetsInfo)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    // Fill type box with reset types whose requirements are met
    ui->typeBox->clear();
    for (int i = 0; i < resetsInfo_.size(); ++i) {
        const ResetInfoDef &ri = resetsInfo_[i];
        if (requirementMet(ri.requires)) {
            ui->typeBox->addItem(ri.name, ri.value);
        }
    }

    connect(ui->acceptButton, &QPushButton::clicked, this, &NewResetWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &NewResetWidget::onCancelClicked);
}

NewResetWidget::~NewResetWidget()
{
    delete ui;
}

bool NewResetWidget::requirementMet(const QString &requires) const
{
    if (requires.isEmpty())
        return true;

    // Check if any existing reset has the required command
    for (const AreaReset &r : area_->resets) {
        if (r.command == requires)
            return true;
    }

    return false;
}

void NewResetWidget::onAcceptClicked()
{
    if (ui->typeBox->currentIndex() < 0) return;

    QString command = ui->typeBox->currentData().toString();

    AreaReset reset;
    reset.command = command;
    reset.extra = 0;
    reset.arg1 = 0;
    reset.arg2 = 0;
    reset.arg3 = 0;
    reset.arg4 = 0;

    area_->resets.append(reset);

    emit resetCreated();
    close();
}

void NewResetWidget::onCancelClicked()
{
    close();
}
