#ifndef PROGRAMSWIDGET_H
#define PROGRAMSWIDGET_H

#include <QWidget>
#include <QList>

#include "model/Area.h"
#include "model/ConfigData.h"

namespace Ui {
class ProgramsWidget;
}

class MainWindow;
class ProgramsHighlighter;

class ProgramsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProgramsWidget(QList<Program> *programs,
                            const QList<QString> &progTypes,
                            const QList<HighlighterWordsDef> &highlighterDefs,
                            MainWindow *mainWindow,
                            QWidget *parent = nullptr);
    ~ProgramsWidget();

private slots:
    void onFirstClicked();
    void onPrevClicked();
    void onNextClicked();
    void onLastClicked();
    void onAddClicked();
    void onDeleteClicked();
    void onAcceptClicked();
    void onCancelClicked();
    void onWholePhraseToggled(bool checked);
    void onTriggerEditTextChanged(const QString &text);
    void onProgramEditTextChanged();

private:
    void navigateTo(int idx);
    void saveCurrentToList();
    void fillFields(int idx);
    void updateNavigationButtons();
    void setFieldsEnabled(bool enabled);

    Ui::ProgramsWidget *ui;
    QList<Program> *originalPrograms_;
    QList<Program> workingPrograms_;
    QList<QString> progTypes_;
    MainWindow *mainWindow_;
    ProgramsHighlighter *highlighter_ = nullptr;
    int currentIndex_ = -1;
    bool canChange_ = false;
};

#endif // PROGRAMSWIDGET_H
