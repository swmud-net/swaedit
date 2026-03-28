#include "gui/WelcomeScreen.h"

#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>
#include <QTimer>

WelcomeScreen::WelcomeScreen(QWidget *parent)
    : QWidget(parent)
{
    if (parent) {
        setWindowTitle(parent->windowTitle());
    }
    setWindowIcon(QApplication::windowIcon());

    img_ = QImage(QCoreApplication::applicationDirPath() + "/images/header.png");

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Dialog);
}

void WelcomeScreen::showNow()
{
    if (!img_.isNull()) {
        QTimer::singleShot(10000, this, &WelcomeScreen::close);
        showFullScreen();
    } else {
        emit closed();
    }
}

void WelcomeScreen::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (!img_.isNull()) {
        int x = width() / 2 - img_.width() / 2;
        int y = height() / 2 - img_.height() / 2;
        p.drawImage(x, y, img_);
    }
}

void WelcomeScreen::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    close();
}

void WelcomeScreen::closeEvent(QCloseEvent *event)
{
    hide();
    emit closed();
    QWidget::closeEvent(event);
}
