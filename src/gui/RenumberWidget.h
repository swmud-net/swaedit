#ifndef RENUMBERWIDGET_H
#define RENUMBERWIDGET_H

#include <QWidget>

namespace Ui {
class RenumberWidget;
}

class RenumberWidget : public QWidget {
    Q_OBJECT

public:
    static constexpr int RENUMBER_RELIABLE = 0;
    static constexpr int RENUMBER_MUDPROGS = 1;

    explicit RenumberWidget(QWidget *parent = nullptr);
    ~RenumberWidget();

signals:
    void paramsSpecified(int newFirstVnum, int flags);

private slots:
    void onVnumEditTextChanged(const QString &text);
    void onAcceptClicked();
    void onCancelClicked();

private:
    Ui::RenumberWidget *ui;
};

#endif // RENUMBERWIDGET_H
