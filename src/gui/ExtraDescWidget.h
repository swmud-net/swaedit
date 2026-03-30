#ifndef EXTRADESCWIDGET_H
#define EXTRADESCWIDGET_H

#include <QWidget>
#include <QList>

#include "model/Area.h"

namespace Ui {
class ExtraDescWidget;
}

class MainWindow;

class ExtraDescWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExtraDescWidget(QList<ExtraDesc> *extraDescs,
                             MainWindow *mainWindow,
                             QWidget *parent = nullptr);
    ~ExtraDescWidget();

private slots:
    void onNavigationChanged(int idx);
    void onKeywordsChanged(const QString &text);
    void onDescriptionChanged();
    void onAddClicked();
    void onDeleteClicked();
    void onAcceptClicked();
    void onCancelClicked();

private:
    void fillNavigation();
    void fillFields(int idx);
    void setFieldsEnabled(bool enabled);
    void saveCurrentToList();

    Ui::ExtraDescWidget *ui;
    QList<ExtraDesc> *originalDescs_;
    QList<ExtraDesc> workingDescs_;
    MainWindow *mainWindow_;
    bool canChange_ = false;
    bool modified_ = false;
    int currentIndex_ = -1;
};

#endif // EXTRADESCWIDGET_H
