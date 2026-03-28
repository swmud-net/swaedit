#ifndef RENUMBERWARNINGSWIDGET_H
#define RENUMBERWARNINGSWIDGET_H

#include <QMainWindow>
#include <QStringList>

namespace Ui {
class RenumberWarningsWidget;
}

class RenumberWarningsWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit RenumberWarningsWidget(const QStringList &warnings,
                                    QWidget *parent = nullptr);
    ~RenumberWarningsWidget();

private:
    Ui::RenumberWarningsWidget *ui;
};

#endif // RENUMBERWARNINGSWIDGET_H
