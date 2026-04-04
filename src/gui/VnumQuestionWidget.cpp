#include "gui/VnumQuestionWidget.h"
#include "ui_VnumQuestionWidget.h"

VnumQuestionWidget::VnumQuestionWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VnumQuestionWidget)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    ui->acceptButton->setEnabled(false);

    connect(ui->vnumEdit, &QLineEdit::textChanged,
            this, &VnumQuestionWidget::onVnumEditTextChanged);
    connect(ui->acceptButton, &QPushButton::clicked,
            this, &VnumQuestionWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &VnumQuestionWidget::onCancelClicked);
}

VnumQuestionWidget::~VnumQuestionWidget()
{
    delete ui;
}

void VnumQuestionWidget::onVnumEditTextChanged(const QString &text)
{
    bool ok;
    qint64 val = text.toLongLong(&ok);
    ui->acceptButton->setEnabled(ok && val > 0);
}

void VnumQuestionWidget::onAcceptClicked()
{
    bool ok;
    qint64 vnum = ui->vnumEdit->text().toLongLong(&ok);
    if (!ok || vnum <= 0) return;

    emit vnumSet(vnum);
    close();
}

void VnumQuestionWidget::onCancelClicked()
{
    close();
}
