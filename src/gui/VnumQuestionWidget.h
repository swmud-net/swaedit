#ifndef VNUMQUESTIONWIDGET_H
#define VNUMQUESTIONWIDGET_H

#include <QWidget>

namespace Ui {
class VnumQuestionWidget;
}

class VnumQuestionWidget : public QWidget {
    Q_OBJECT

public:
    explicit VnumQuestionWidget(QWidget *parent = nullptr);
    ~VnumQuestionWidget();

signals:
    void vnumSet(int vnum);

private slots:
    void onVnumEditTextChanged(const QString &text);
    void onAcceptClicked();
    void onCancelClicked();

private:
    Ui::VnumQuestionWidget *ui;
};

#endif // VNUMQUESTIONWIDGET_H
