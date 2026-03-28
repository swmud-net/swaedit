#include "gui/ValueFlagsWidget.h"
#include "gui/FlagsWidget.h"
#include "gui/MainWindow.h"
#include "ui_ValueFlagsWidget.h"

ValueFlagsWidget::ValueFlagsWidget(int valueNo,
                                   qint64 flags,
                                   const QList<FlagDef> &flagsTree,
                                   ObjectValues *values,
                                   const QString &title,
                                   MainWindow *mainWindow,
                                   QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ValueFlagsWidget)
    , valueNo_(valueNo)
    , flags_(flags)
    , flagsTree_(flagsTree)
    , values_(values)
    , title_(title)
    , mainWindow_(mainWindow)
{
    ui->setupUi(this);

    ui->edit->setText(QString::number(flags_));

    connect(ui->edit, &QLineEdit::textChanged, this, &ValueFlagsWidget::onEditTextChanged);
    connect(ui->button, &QPushButton::clicked, this, &ValueFlagsWidget::onButtonClicked);
}

ValueFlagsWidget::~ValueFlagsWidget()
{
    delete ui;
}

void ValueFlagsWidget::setFWFlags(qint64 flags)
{
    canEdit_ = false;
    flags_ = flags;
    ui->edit->setText(QString::number(flags_));
    canEdit_ = true;
}

void ValueFlagsWidget::onEditTextChanged(const QString &text)
{
    if (!canEdit_) return;
    emit textChanged(text);
}

void ValueFlagsWidget::onButtonClicked()
{
    emit clicked();
    showFlagsWidget();
}

void ValueFlagsWidget::showFlagsWidget()
{
    bool ok;
    qint64 currentValue = ui->edit->text().toLongLong(&ok);
    if (!ok) currentValue = flags_;

    FlagsWidget *fw = new FlagsWidget(flagsTree_, currentValue, title_, false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        flags_ = value;

        // Update the ObjectValues
        if (values_) {
            switch (valueNo_) {
            case 0: values_->value0 = static_cast<int>(value); break;
            case 1: values_->value1 = static_cast<int>(value); break;
            case 2: values_->value2 = static_cast<int>(value); break;
            case 3: values_->value3 = QString::number(value); break;
            case 4: values_->value4 = QString::number(value); break;
            case 5: values_->value5 = QString::number(value); break;
            }
        }

        setFWFlags(value);

        if (mainWindow_) {
            mainWindow_->setModified();
        }
    });

    fw->show();
}
