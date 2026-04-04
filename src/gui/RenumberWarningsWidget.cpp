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

    statusBar()->showMessage(QStringLiteral("Number of warnings: %1.").arg(warnings.size()));
}

RenumberWarningsWidget::~RenumberWarningsWidget()
{
    delete ui;
}
