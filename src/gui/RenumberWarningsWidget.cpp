#include "gui/RenumberWarningsWidget.h"
#include "ui_RenumberWarningsWidget.h"

RenumberWarningsWidget::RenumberWarningsWidget(const QStringList &warnings,
                                               QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RenumberWarningsWidget)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);

    for (const QString &w : warnings) {
        ui->warningsListWidget->addItem(w);
    }

    // The close button connection is already wired in the .ui file
}

RenumberWarningsWidget::~RenumberWarningsWidget()
{
    delete ui;
}
