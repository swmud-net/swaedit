#ifndef WELCOMESCREEN_H
#define WELCOMESCREEN_H

#include <QWidget>
#include <QImage>

class WelcomeScreen : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeScreen(QWidget *parent = nullptr);

    void showNow();

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QImage img_;
};

#endif // WELCOMESCREEN_H
