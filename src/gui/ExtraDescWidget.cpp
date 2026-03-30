#include "gui/ExtraDescWidget.h"
#include "gui/MainWindow.h"
#include "ui_ExtraDescWidget.h"

#include <QMessageBox>

ExtraDescWidget::ExtraDescWidget(QList<ExtraDesc> *extraDescs,
                                 MainWindow *mainWindow,
                                 QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExtraDescWidget)
    , originalDescs_(extraDescs)
    , mainWindow_(mainWindow)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);

    // Deep copy the list
    workingDescs_ = *originalDescs_;

    connect(ui->navigationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExtraDescWidget::onNavigationChanged);
    connect(ui->keywordsEdit, &QLineEdit::textChanged,
            this, &ExtraDescWidget::onKeywordsChanged);
    connect(ui->descriptionEdit, &QTextEdit::textChanged,
            this, &ExtraDescWidget::onDescriptionChanged);
    connect(ui->addButton, &QPushButton::clicked,
            this, &ExtraDescWidget::onAddClicked);
    connect(ui->deleteButton, &QPushButton::clicked,
            this, &ExtraDescWidget::onDeleteClicked);
    connect(ui->acceptButton, &QPushButton::clicked,
            this, &ExtraDescWidget::onAcceptClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &ExtraDescWidget::onCancelClicked);

    fillNavigation();

    if (!workingDescs_.isEmpty()) {
        ui->navigationComboBox->setCurrentIndex(0);
        fillFields(0);
    }
}

ExtraDescWidget::~ExtraDescWidget()
{
    delete ui;
}

void ExtraDescWidget::fillNavigation()
{
    canChange_ = false;
    ui->navigationComboBox->blockSignals(true);
    ui->navigationComboBox->clear();

    for (int i = 0; i < workingDescs_.size(); ++i) {
        ui->navigationComboBox->addItem(workingDescs_[i].keyword, i);
    }

    ui->navigationComboBox->blockSignals(false);

    bool hasItems = !workingDescs_.isEmpty();
    setFieldsEnabled(hasItems);
    ui->acceptButton->setEnabled(modified_);

    if (hasItems) {
        canChange_ = true;
    }
}

void ExtraDescWidget::fillFields(int idx)
{
    canChange_ = false;
    currentIndex_ = idx;

    if (idx < 0 || idx >= workingDescs_.size()) {
        ui->keywordsEdit->clear();
        ui->descriptionEdit->clear();
        canChange_ = !workingDescs_.isEmpty();
        return;
    }

    const ExtraDesc &ed = workingDescs_[idx];
    ui->keywordsEdit->setText(ed.keyword);
    ui->descriptionEdit->setPlainText(ed.description);

    canChange_ = true;
}

void ExtraDescWidget::setFieldsEnabled(bool enabled)
{
    ui->navigationComboBox->setEnabled(enabled);
    ui->keywordsEdit->setEnabled(enabled);
    ui->descriptionEdit->setEnabled(enabled);
    ui->deleteButton->setEnabled(enabled);
}

void ExtraDescWidget::saveCurrentToList()
{
    if (currentIndex_ < 0 || currentIndex_ >= workingDescs_.size())
        return;

    workingDescs_[currentIndex_].keyword = ui->keywordsEdit->text();
    workingDescs_[currentIndex_].description = ui->descriptionEdit->toPlainText();
}

void ExtraDescWidget::onNavigationChanged(int idx)
{
    if (!canChange_) return;

    // Save current before switching
    saveCurrentToList();

    fillFields(idx);
}

void ExtraDescWidget::onKeywordsChanged(const QString &text)
{
    if (!canChange_) return;
    if (currentIndex_ < 0 || currentIndex_ >= workingDescs_.size())
        return;

    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Keyword", "Keyword cannot be empty!");
    }

    workingDescs_[currentIndex_].keyword = text;
    modified_ = true;
    ui->acceptButton->setEnabled(true);

    // Update navigation combo text
    ui->navigationComboBox->blockSignals(true);
    ui->navigationComboBox->setItemText(currentIndex_, text);
    ui->navigationComboBox->blockSignals(false);
}

void ExtraDescWidget::onDescriptionChanged()
{
    if (!canChange_) return;
    if (currentIndex_ < 0 || currentIndex_ >= workingDescs_.size())
        return;

    workingDescs_[currentIndex_].description = ui->descriptionEdit->toPlainText();
    modified_ = true;
    ui->acceptButton->setEnabled(true);
}

void ExtraDescWidget::onAddClicked()
{
    // Save current first
    saveCurrentToList();

    // Find a unique name
    QString baseName = "newExtraDescription";
    int counter = 0;
    QString newName = baseName + QString::number(counter);
    bool exists = true;
    while (exists) {
        exists = false;
        for (const ExtraDesc &ed : workingDescs_) {
            if (ed.keyword == newName) {
                exists = true;
                counter++;
                newName = baseName + QString::number(counter);
                break;
            }
        }
    }

    ExtraDesc ed;
    ed.keyword = newName;
    ed.description = "";
    workingDescs_.append(ed);
    modified_ = true;

    fillNavigation();
    int newIdx = workingDescs_.size() - 1;
    ui->navigationComboBox->setCurrentIndex(newIdx);
    fillFields(newIdx);

    ui->keywordsEdit->setFocus();
    ui->keywordsEdit->selectAll();
}

void ExtraDescWidget::onDeleteClicked()
{
    if (currentIndex_ < 0 || currentIndex_ >= workingDescs_.size())
        return;

    workingDescs_.removeAt(currentIndex_);
    currentIndex_ = -1;
    modified_ = (workingDescs_.size() != originalDescs_->size());

    fillNavigation();

    if (!workingDescs_.isEmpty()) {
        ui->navigationComboBox->setCurrentIndex(0);
        fillFields(0);
    } else {
        fillFields(-1);
    }
}

void ExtraDescWidget::onAcceptClicked()
{
    // Save current edits
    saveCurrentToList();

    // Copy back to original
    *originalDescs_ = workingDescs_;

    if (mainWindow_) {
        mainWindow_->setModified();
    }

    close();
}

void ExtraDescWidget::onCancelClicked()
{
    close();
}
