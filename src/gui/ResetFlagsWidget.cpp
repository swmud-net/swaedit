#include "gui/ResetFlagsWidget.h"
#include "gui/FlagsWidget.h"
#include "gui/MainWindow.h"
#include "ui_ValueFlagsWidget.h"

ResetFlagsWidget::ResetFlagsWidget(int argIndex,
                                   qint64 flags,
                                   const QList<FlagDef> &flagsTree,
                                   AreaReset *reset,
                                   const QString &title,
                                   MainWindow *mainWindow,
                                   QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ValueFlagsWidget)
    , argIndex_(argIndex)
    , flags_(flags)
    , flagsTree_(flagsTree)
    , reset_(reset)
    , title_(title)
    , mainWindow_(mainWindow)
{
    ui->setupUi(this);

    ui->edit->setText(QString::number(flags_));

    connect(ui->edit, &QLineEdit::textChanged, this, &ResetFlagsWidget::onEditTextChanged);
    connect(ui->button, &QPushButton::clicked, this, &ResetFlagsWidget::onButtonClicked);
}

ResetFlagsWidget::~ResetFlagsWidget()
{
    delete ui;
}

void ResetFlagsWidget::setFWFlags(qint64 flags)
{
    canEdit_ = false;
    flags_ = flags;
    ui->edit->setText(QString::number(flags_));
    canEdit_ = true;
}

void ResetFlagsWidget::onEditTextChanged(const QString &text)
{
    if (!canEdit_) return;
    emit textChanged(text);
}

void ResetFlagsWidget::onButtonClicked()
{
    emit clicked();
    showFlagsWidget();
}

void ResetFlagsWidget::showFlagsWidget()
{
    bool ok;
    qint64 currentValue = ui->edit->text().toLongLong(&ok);
    if (!ok) currentValue = flags_;

    FlagsWidget *fw = new FlagsWidget(flagsTree_, currentValue, title_, false, this);
    connect(fw, &FlagsWidget::flagsAccepted, this, [this](qint64 value) {
        flags_ = value;

        // Update the reset arg value
        if (reset_) {
            switch (argIndex_) {
            case 0: reset_->extra = static_cast<int>(value); break;
            case 1: reset_->arg1 = static_cast<int>(value); break;
            case 2: reset_->arg2 = static_cast<int>(value); break;
            case 3: reset_->arg3 = static_cast<int>(value); break;
            case 4: reset_->arg4 = static_cast<int>(value); break;
            }
        }

        setFWFlags(value);

        if (mainWindow_) {
            mainWindow_->setModified();
            mainWindow_->updateResetNavigatorText();
        }
    });

    fw->show();
}
