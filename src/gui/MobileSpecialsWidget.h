#ifndef MOBILESPECIALSWIDGET_H
#define MOBILESPECIALSWIDGET_H

#include <QWidget>
#include <QList>

#include "model/Area.h"

namespace Ui {
class MobileSpecialsWidget;
}

class MainWindow;

class MobileSpecialsWidget : public QWidget {
    Q_OBJECT

public:
    explicit MobileSpecialsWidget(Mobile *mob,
                                  Area *area,
                                  const QList<QString> &specFunctions,
                                  MainWindow *mainWindow,
                                  QWidget *parent = nullptr);
    ~MobileSpecialsWidget();

private slots:
    void onSpec1Changed(int idx);
    void onAcceptClicked();
    void onCancelClicked();

private:
    void fillSpec2Box();

    Ui::MobileSpecialsWidget *ui;
    Mobile *mob_;
    Area *area_;
    QList<QString> specFunctions_;
    MainWindow *mainWindow_;
    Special workingSpecial_;
    bool hasExistingSpecial_ = false;
    int existingSpecialIndex_ = -1;
};

#endif // MOBILESPECIALSWIDGET_H
