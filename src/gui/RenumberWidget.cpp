#include "gui/RenumberWidget.h"
#include "ui_RenumberWidget.h"

#include <QCoreApplication>
#include <QFile>

RenumberWidget::RenumberWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RenumberWidget)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    ui->acceptButton->setEnabled(false);

    // Load warning HTML from data/renumberWarning.html
    QString dataDir = QCoreApplication::applicationDirPath() + "/data/";
    QFile warningFile(dataDir + "renumberWarning.html");
    if (warningFile.open(QIODevice::ReadOnly)) {
        QString html = QString::fromUtf8(warningFile.readAll());
        ui->warningBrowser->setHtml(html);
        warningFile.close();
    }

    connect(ui->vnumEdit, &QLineEdit::textChanged,
            this, &RenumberWidget::onVnumEditTextChanged);
    connect(ui->acceptButton, &QPushButton::clicked,
            this, &RenumberWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &RenumberWidget::onCancelClicked);
}

RenumberWidget::~RenumberWidget()
{
    delete ui;
}

void RenumberWidget::onVnumEditTextChanged(const QString &text)
{
    bool ok;
    qint64 val = text.toLongLong(&ok);
    ui->acceptButton->setEnabled(ok && val > 0);
}

void RenumberWidget::onAcceptClicked()
{
    bool ok;
    qint64 newFirstVnum = ui->vnumEdit->text().toLongLong(&ok);
    if (!ok || newFirstVnum <= 0) return;

    int flags = RENUMBER_RELIABLE;
    if (ui->programsButton->isChecked()) {
        flags = RENUMBER_MUDPROGS;
    }

    emit paramsSpecified(newFirstVnum, flags);
    close();
}

void RenumberWidget::onCancelClicked()
{
    close();
}
