#include "gui/ProgramsWidget.h"
#include "gui/MainWindow.h"
#include "gui/ProgramsHighlighter.h"
#include "ui_ProgramsWidget.h"

#include <QMessageBox>

ProgramsWidget::ProgramsWidget(QList<Program> *programs,
                                const QList<QString> &progTypes,
                                const QList<HighlighterWordsDef> &highlighterDefs,
                                MainWindow *mainWindow,
                                QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProgramsWidget)
    , originalPrograms_(programs)
    , progTypes_(progTypes)
    , mainWindow_(mainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    // Deep copy
    workingPrograms_ = *originalPrograms_;

    // Fill type combo box
    ui->typeComboBox->clear();
    for (const QString &pt : progTypes_) {
        ui->typeComboBox->addItem(pt);
    }

    // Set up syntax highlighter
    highlighter_ = new ProgramsHighlighter(ui->programEdit->document(), highlighterDefs);

    // Connect buttons
    connect(ui->firstButton, &QPushButton::clicked, this, &ProgramsWidget::onFirstClicked);
    connect(ui->prevButton, &QPushButton::clicked, this, &ProgramsWidget::onPrevClicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &ProgramsWidget::onNextClicked);
    connect(ui->lastButton, &QPushButton::clicked, this, &ProgramsWidget::onLastClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &ProgramsWidget::onAddClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &ProgramsWidget::onDeleteClicked);
    connect(ui->acceptButton, &QPushButton::clicked, this, &ProgramsWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &ProgramsWidget::onCancelClicked);
    connect(ui->wholePhraseCheckBox, &QCheckBox::toggled, this, &ProgramsWidget::onWholePhraseToggled);
    connect(ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProgramsWidget::onTypeComboChanged);
    connect(ui->triggerEdit, &QLineEdit::textChanged, this, &ProgramsWidget::onTriggerEditTextChanged);
    connect(ui->programEdit, &QTextEdit::textChanged, this, &ProgramsWidget::onProgramEditTextChanged);

    if (!workingPrograms_.isEmpty()) {
        navigateTo(0);
    } else {
        setFieldsEnabled(false);
        ui->progLcdNumber->display(0);
    }
}

ProgramsWidget::~ProgramsWidget()
{
    delete ui;
}

void ProgramsWidget::navigateTo(int idx)
{
    if (idx < 0 || idx >= workingPrograms_.size())
        return;

    // Save current before navigating
    saveCurrentToList();

    currentIndex_ = idx;
    fillFields(idx);
    updateNavigationButtons();
}

void ProgramsWidget::saveCurrentToList()
{
    if (!canChange_) return;
    if (currentIndex_ < 0 || currentIndex_ >= workingPrograms_.size())
        return;

    Program &prog = workingPrograms_[currentIndex_];
    prog.type = ui->typeComboBox->currentText();
    prog.comlist = ui->programEdit->toPlainText();

    // Save trigger text directly — "p " prefix is visible in the field
    prog.args = ui->triggerEdit->text();
}

void ProgramsWidget::fillFields(int idx)
{
    canChange_ = false;

    if (idx < 0 || idx >= workingPrograms_.size()) {
        ui->typeComboBox->setCurrentIndex(0);
        ui->triggerEdit->clear();
        ui->programEdit->clear();
        ui->wholePhraseCheckBox->setChecked(false);
        ui->progLcdNumber->display(0);
        setFieldsEnabled(false);
        return;
    }

    setFieldsEnabled(true);

    const Program &prog = workingPrograms_[idx];

    // Set type
    int typeIdx = ui->typeComboBox->findText(prog.type);
    if (typeIdx >= 0) ui->typeComboBox->setCurrentIndex(typeIdx);

    // Set trigger / whole phrase — show full args including "p " prefix
    QString args = prog.args;
    ui->triggerEdit->setText(args);
    ui->wholePhraseCheckBox->setChecked(args.trimmed().startsWith("p "));

    // Set program text
    ui->programEdit->setPlainText(prog.comlist);

    // LCD shows 1-based index
    ui->progLcdNumber->display(idx + 1);

    canChange_ = true;
}

void ProgramsWidget::updateNavigationButtons()
{
    bool hasItems = !workingPrograms_.isEmpty();
    bool notFirst = hasItems && currentIndex_ > 0;
    bool notLast = hasItems && currentIndex_ < workingPrograms_.size() - 1;

    ui->firstButton->setEnabled(notFirst);
    ui->prevButton->setEnabled(notFirst);
    ui->nextButton->setEnabled(notLast);
    ui->lastButton->setEnabled(notLast);
    ui->deleteButton->setEnabled(hasItems);
    ui->acceptButton->setEnabled(modified_);
}

void ProgramsWidget::setFieldsEnabled(bool enabled)
{
    ui->typeComboBox->setEnabled(enabled);
    ui->triggerEdit->setEnabled(enabled);
    ui->programEdit->setEnabled(enabled);
    ui->wholePhraseCheckBox->setEnabled(enabled);
    ui->deleteButton->setEnabled(enabled);
}

void ProgramsWidget::onFirstClicked()
{
    navigateTo(0);
}

void ProgramsWidget::onPrevClicked()
{
    if (currentIndex_ > 0)
        navigateTo(currentIndex_ - 1);
}

void ProgramsWidget::onNextClicked()
{
    if (currentIndex_ < workingPrograms_.size() - 1)
        navigateTo(currentIndex_ + 1);
}

void ProgramsWidget::onLastClicked()
{
    navigateTo(workingPrograms_.size() - 1);
}

void ProgramsWidget::onAddClicked()
{
    saveCurrentToList();

    Program prog;
    prog.type = "speech_prog";
    prog.args = "witaj";
    prog.comlist = "say Witaj $n";
    workingPrograms_.append(prog);

    modified_ = true;
    navigateTo(workingPrograms_.size() - 1);
}

void ProgramsWidget::onDeleteClicked()
{
    if (currentIndex_ < 0 || currentIndex_ >= workingPrograms_.size())
        return;

    canChange_ = false;
    workingPrograms_.removeAt(currentIndex_);
    modified_ = true;

    if (workingPrograms_.isEmpty()) {
        modified_ = !originalPrograms_->isEmpty();
        currentIndex_ = -1;
        fillFields(-1);
        updateNavigationButtons();
    } else {
        int newIdx = (currentIndex_ > 0) ? currentIndex_ - 1 : 0;
        currentIndex_ = -1; // Reset so navigateTo doesn't save
        navigateTo(newIdx);
    }
}

void ProgramsWidget::onAcceptClicked()
{
    saveCurrentToList();

    // Copy back to original
    *originalPrograms_ = workingPrograms_;

    if (mainWindow_) {
        mainWindow_->setModified();
    }

    close();
}

void ProgramsWidget::onCancelClicked()
{
    close();
}

void ProgramsWidget::onTypeComboChanged(int idx)
{
    Q_UNUSED(idx);
    if (!canChange_) return;
    modified_ = true;
    updateNavigationButtons();
}

void ProgramsWidget::onTriggerEditTextChanged(const QString &text)
{
    if (!canChange_) return;
    modified_ = true;
    updateNavigationButtons();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Trigger", "Trigger cannot be empty!");
    }
}

void ProgramsWidget::onProgramEditTextChanged()
{
    if (!canChange_) return;
    modified_ = true;
    updateNavigationButtons();
    if (ui->programEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Invalid Program", "Program cannot be empty!");
    }
}

void ProgramsWidget::onWholePhraseToggled(bool checked)
{
    if (!canChange_) return;
    modified_ = true;
    updateNavigationButtons();
    if (currentIndex_ < 0 || currentIndex_ >= workingPrograms_.size())
        return;

    Program &prog = workingPrograms_[currentIndex_];
    QString currentArgs = prog.args.trimmed();

    if (checked) {
        prog.args = "p " + currentArgs;
    } else {
        if (currentArgs.startsWith("p "))
            prog.args = currentArgs.mid(2);
        else
            prog.args = currentArgs;
    }

    // Update the trigger field to show the modified args
    canChange_ = false;
    ui->triggerEdit->setText(prog.args);
    canChange_ = true;
}
