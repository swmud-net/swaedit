#include "gui/FlagsWidget.h"
#include "ui_FlagsWidget.h"

#include <QGridLayout>
#include <QMessageBox>

FlagsWidget::FlagsWidget(const QList<FlagDef> &flagList,
                         qint64 initialValue,
                         const QString &flagsName,
                         bool allowNone,
                         QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FlagsWidget)
    , flagsValue_(initialValue)
    , initialValue_(initialValue)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle(flagsName);

    // Build the flag list, optionally prepending NONE
    if (allowNone) {
        FlagDef none;
        none.name = "NONE";
        none.value = 0;
        flagList_.append(none);
    }
    flagList_.append(flagList);

    // Set group box title
    ui->flagGroupBox->setTitle(flagsName);

    // Create grid layout inside the flagGroupBox
    QGridLayout *grid = new QGridLayout();
    ui->flagGroupBox->setLayout(grid);

    static const int ROWS_PER_COLUMN = 16;

    for (int i = 0; i < flagList_.size(); ++i) {
        const FlagDef &flag = flagList_[i];
        QCheckBox *cb = new QCheckBox(flag.name);
        cb->setObjectName(QString::number(flag.value));

        // Java: (flagsValue & flagValue) == flagValue — for NONE (0) this is always true
        bool checked = ((flagsValue_ & flag.value) == flag.value);
        cb->setChecked(checked);

        int col = i / ROWS_PER_COLUMN;
        int row = i % ROWS_PER_COLUMN;
        grid->addWidget(cb, row, col);

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        connect(cb, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState s) {
            onCheckBoxStateChanged(static_cast<int>(s));
        });
#else
        connect(cb, &QCheckBox::stateChanged, this, &FlagsWidget::onCheckBoxStateChanged);
#endif
        checkboxes_.append(cb);
    }

    // Set initial value in the edit field
    ui->flagsValueEdit->setText(QString::number(flagsValue_));
    ui->acceptButton->setEnabled(false);

    connect(ui->flagsValueEdit, &QLineEdit::textChanged,
            this, &FlagsWidget::onFlagsValueEditTextChanged);
    connect(ui->acceptButton, &QPushButton::clicked,
            this, &FlagsWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &FlagsWidget::onCancelClicked);

    adjustSize();
}

FlagsWidget::~FlagsWidget()
{
    delete ui;
}

void FlagsWidget::onCheckBoxStateChanged(int state)
{
    if (updatingFromCode_)
        return;

    QCheckBox *cb = qobject_cast<QCheckBox *>(sender());
    if (!cb) return;

    bool ok;
    qint64 flagValue = cb->objectName().toLongLong(&ok);
    if (!ok) return;

    // Java: checked → flagsValue |= flagValue; unchecked → flagsValue &= ~flagValue
    // For NONE (0): |= 0 is no-op, &= ~0 is no-op — matches Java behavior
    if (state == Qt::Checked) {
        flagsValue_ |= flagValue;
    } else {
        flagsValue_ &= ~flagValue;
    }

    updatingFromCode_ = true;
    ui->flagsValueEdit->setText(QString::number(flagsValue_));
    updateCheckboxesFromValue();
    updatingFromCode_ = false;

    ui->acceptButton->setEnabled(flagsValue_ != initialValue_);
}

void FlagsWidget::onFlagsValueEditTextChanged(const QString &text)
{
    if (updatingFromCode_)
        return;

    bool ok;
    qint64 val = text.toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Invalid Flags Value", "All flags must have number value!");
        updatingFromCode_ = true;
        ui->flagsValueEdit->setText(QString::number(flagsValue_));
        updatingFromCode_ = false;
        return;
    }

    flagsValue_ = val;

    updatingFromCode_ = true;
    updateCheckboxesFromValue();
    updatingFromCode_ = false;

    ui->acceptButton->setEnabled(flagsValue_ != initialValue_);
}

void FlagsWidget::updateCheckboxesFromValue()
{
    for (QCheckBox *cb : checkboxes_) {
        bool ok;
        qint64 flagValue = cb->objectName().toLongLong(&ok);
        if (!ok) continue;

        // Java: (flagsValue & flagValue) == flagValue — for NONE (0) always true
        bool checked = ((flagsValue_ & flagValue) == flagValue);
        cb->setChecked(checked);
    }
}

void FlagsWidget::onAcceptClicked()
{
    emit flagsAccepted(flagsValue_);
    close();
}

void FlagsWidget::onCancelClicked()
{
    close();
}
