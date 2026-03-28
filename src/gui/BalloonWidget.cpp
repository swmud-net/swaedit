#include "gui/BalloonWidget.h"

#include <QBitmap>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPalette>
#include <QPen>
#include <QScreen>
#include <QTimer>

// Static members
QPoint BalloonWidget::lastToolTipPos_;
BalloonWidget *BalloonWidget::lastToolTip_ = nullptr;
QWidget *BalloonWidget::lastToolTipWidget_ = nullptr;

BalloonWidget::BalloonWidget(QPoint pos, ColorTheme colorTheme)
    : QWidget(nullptr)
    , pos_(pos)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::ToolTip);

    QString imgDir = QCoreApplication::applicationDirPath() + "/images/";

    switch (colorTheme) {
    case Sith:
        headerColor_ = QColor(0x982100);
        iconImage_ = QImage(imgDir + "sith.png");
        progressHilt_ = QImage(imgDir + "hilt_red.png");
        progressEnd_ = QImage(imgDir + "end_red.png");
        progress1_ = QImage(imgDir + "1_red.png");
        break;

    case Jedi:
        headerColor_ = QColor(0x219800);
        iconImage_ = QImage(imgDir + "jedi.png");
        progressHilt_ = QImage(imgDir + "hilt_green.png");
        progressEnd_ = QImage(imgDir + "end_green.png");
        progress1_ = QImage(imgDir + "1_green.png");
        break;

    default: // Neutral
        headerColor_ = QColor(0x002198);
        iconImage_ = QImage(imgDir + "neutral.png");
        progressHilt_ = QImage(imgDir + "hilt_blue.png");
        progressEnd_ = QImage(imgDir + "end_blue.png");
        progress1_ = QImage(imgDir + "1_blue.png");
        break;
    }

    if (iconImage_.isNull() || iconImage_.width() != ICON_IMAGE_W || iconImage_.height() != ICON_IMAGE_H) {
        iconImage_ = QImage(48, 48, QImage::Format_ARGB32);
        iconImage_.fill(Qt::transparent);
    }

    if (progressHilt_.width() < 1 || progressEnd_.width() < 1 || progress1_.width() < 1) {
        progressHilt_ = QImage();
        progressEnd_ = QImage();
        progress1_ = QImage();
    }

    progressPixels_ = w_ - ICON_IMAGE_W - progressHilt_.width() - 30;
    pxs_ = progressPixels_ / 100.0;

    arrowTop_ = pos_.y() - h_ - ARROW_H < 0;
    arrowLeft_ = pos_.x() - w_ + ARROW_D + RECT_WH < 0;

    drawBalloon();

    resize(w_ + 10, h_ + 10 + ARROW_H);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xffffff));
    setPalette(pal);

    // Title label
    titleLabel_ = new QLabel(this);
    titleLabel_->move(10, 5 + top_);
    titleLabel_->setTextFormat(Qt::PlainText);
    QFont f = titleLabel_->font();
    f.setBold(true);
    titleLabel_->setFont(f);
    pal = titleLabel_->palette();
    pal.setColor(QPalette::WindowText, Qt::white);
    titleLabel_->setPalette(pal);

    // Message label
    msgLabel_ = new QLabel(this);
    msgLabel_->setTextFormat(Qt::PlainText);
    msgLabel_->setContentsMargins(0, 0, MSG_LABEL_MARGIN_RIGHT, 15);
    msgLabel_->resize(w_ - MSG_LABEL_MARGIN_RIGHT - iconImage_.width() - 10, msgLabel_->height());
    msgLabel_->setWordWrap(true);
    msgLabel_->move(iconImage_.width() + 10, 25 + top_);
    f = msgLabel_->font();
    f.setBold(true);
    titleLabel_->setFont(f);
    pal = msgLabel_->palette();
    pal.setColor(QPalette::WindowText, QColor(0x494949));
    msgLabel_->setPalette(pal);
}

void BalloonWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.drawPixmap(0, 0, borderMap_);
    p.fillRect(0, 0, w_ + 1, 20 + top_, QBrush(headerColor_));
    p.drawImage(0, h_ / 2 - iconImage_.height() / 2 + 10 + top_ / 2, iconImage_);
    if (progress_ > -1 && !progressHilt_.isNull()) {
        double pxr = 0; // unprinted floating point data from previous pixel
        int px = iconImage_.width() + 10 + progressHilt_.width();
        p.drawImage(iconImage_.width() + 10, h_ - 15, progressHilt_);
        for (int i = 0; i < static_cast<int>(progress_); i++) {
            int j = 0;
            for (; j < static_cast<int>(pxs_ + pxr); j++, px++) {
                p.drawImage(px, h_ - 15, progress1_);
            }
            pxr += pxs_ - j;
        }
        p.drawImage(px, h_ - 15, progressEnd_);
    }
    p.end();
}

void BalloonWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit balloonClicked();
    }
    close();
}

void BalloonWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit balloonDoubleClicked();
    }
}

void BalloonWidget::closeEvent(QCloseEvent *event)
{
    lastToolTipWidget_ = nullptr;
    lastMessage_ = nullptr;
    lastProgress_ = nullptr;
    QWidget::closeEvent(event);
}

void BalloonWidget::drawBalloon()
{
    if (arrowTop_) {
        top_ = ARROW_H;
        h_ += top_;
    }

    QPainterPath p;
    QRectF r(0, top_, RECT_WH, RECT_WH);

    p.moveTo(0, RECT_WH + top_);
    p.arcTo(r, 180, -90);

    // Arrow top left
    if (arrowTop_ && arrowLeft_) {
        move(pos_.x() - RECT_WH - ARROW_D, pos_.y());
        p.lineTo(RECT_WH, top_);
        p.lineTo(RECT_WH + ARROW_D, top_ - ARROW_H);
        p.lineTo(RECT_WH + ARROW_WD, top_);
    }
    // Arrow top right
    else if (arrowTop_ && !arrowLeft_) {
        move(pos_.x() - w_ + RECT_WH + ARROW_D, pos_.y());
        p.lineTo(w_ - RECT_WH - ARROW_WD, top_);
        p.lineTo(w_ - RECT_WH - ARROW_D, top_ - ARROW_H);
    }

    p.lineTo(w_ - RECT_WH, top_);
    r.moveTo(w_ - RECT_WH, top_);

    p.arcTo(r, 90, -90);
    p.lineTo(w_, h_ - RECT_WH);
    r.moveTo(w_ - RECT_WH, h_ - RECT_WH);

    p.arcTo(r, 0, -90);

    // Arrow bottom right
    if (!arrowTop_ && !arrowLeft_) {
        move(pos_.x() - w_ + RECT_WH + ARROW_D, pos_.y() - h_ - ARROW_H);
        p.lineTo(w_ - RECT_WH, h_);
        p.lineTo(w_ - RECT_WH - ARROW_D, h_ + ARROW_H);
        p.lineTo(w_ - RECT_WH - ARROW_WD, h_);
    }
    // Arrow bottom left
    else if (!arrowTop_ && arrowLeft_) {
        move(pos_.x() - RECT_WH - ARROW_D, pos_.y() - h_ - ARROW_H);
        p.lineTo(RECT_WH + ARROW_WD, h_);
        p.lineTo(RECT_WH + ARROW_D, h_ + ARROW_H);
    }

    p.lineTo(RECT_WH, h_);
    r.moveTo(0, h_ - RECT_WH);

    p.arcTo(r, 270, -90);
    p.lineTo(0, RECT_WH + top_);

    // Balloon mask
    QBitmap bitmap(800, 600);
    bitmap.fill(Qt::color0);
    {
        QPainter painter(&bitmap);
        painter.setPen(Qt::color1);
        painter.setBrush(Qt::color1);
        painter.drawPath(p);
    }
    setMask(bitmap);

    // Balloon border
    borderMap_ = QPixmap(800, 600);
    {
        QPainter painter(&borderMap_);
        painter.setPen(QPen(headerColor_, 1));
        painter.setBrush(Qt::color0);
        painter.drawPath(p);
    }
}

bool BalloonWidget::canShowMessage() const
{
    return lastMessage_ == nullptr;
}

bool BalloonWidget::canShowProgress() const
{
    return lastProgress_ != nullptr;
}

void BalloonWidget::showMessage(const QString &title, const QString &msg, int msec)
{
    titleLabel_->setText(title);
    msgLabel_->setText(msg);
    lastMessage_ = this;
    show();
    if (msec > 0) {
        QTimer::singleShot(msec, this, &BalloonWidget::close);
    }
}

void BalloonWidget::showMessage(const QString &title, const QString &msg)
{
    showMessage(title, msg, messageTimeout_);
}

void BalloonWidget::setProgress(const QString &title, const QString &msg, double max, bool closeOnComplete)
{
    titleLabel_->setText(title);
    msgLabel_->setText(msg);
    progressMax_ = max;
    progress_ = 0;
    closeOnComplete_ = closeOnComplete;
    lastProgress_ = this;
    show();
}

void BalloonWidget::showProgress(const QString &title, const QString &msg, double progress)
{
    titleLabel_->setText(title);
    msgLabel_->setText(msg);
    showProgress(progress);
}

void BalloonWidget::showProgress(double progress)
{
    if (progress >= 0) {
        if (progress < progressMax_) {
            progress_ = (100 * progress) / progressMax_;
            repaint();
        } else {
            progress_ = 100;
            repaint();
            progress_ = -1;
            progressMax_ = 0;
            emit progressCompleted();
            if (closeOnComplete_) {
                close();
            }
        }
    }
}

bool BalloonWidget::canShowToolTip(QWidget *w, QPoint pos)
{
    if (lastToolTipPos_.isNull() || lastToolTipPos_ != pos) {
        return true;
    }
    if (lastToolTipWidget_ == nullptr || lastToolTipWidget_ != w) {
        return true;
    }
    return false;
}

void BalloonWidget::showToolTip(QWidget *w, const QString &title, const QString &toolTip, int msec)
{
    if (canShowToolTip(w, pos_)) {
        lastToolTipWidget_ = w;
        if (lastToolTip_ != nullptr) {
            lastToolTip_->close();
            lastToolTip_ = nullptr;
        }
        lastToolTip_ = this;
        lastToolTipPos_ = pos_;
        titleLabel_->setText(title);
        msgLabel_->setText(toolTip);
        show();
        if (msec > 0) {
            QTimer::singleShot(msec, this, &BalloonWidget::close);
        }
    }
}

void BalloonWidget::showToolTip(QWidget *w, const QString &title, const QString &toolTip)
{
    showToolTip(w, title, toolTip, toolTipTimeout_);
}

void BalloonWidget::showToolTip(QWidget *w, const QString &toolTip, int msec)
{
    showToolTip(w, QStringLiteral("ToolTip Message"), toolTip, msec);
}

void BalloonWidget::showToolTip(QWidget *w, const QString &toolTip)
{
    showToolTip(w, QStringLiteral("ToolTip Message"), toolTip, toolTipTimeout_);
}

void BalloonWidget::showToolTip(QWidget *w, int msec)
{
    showToolTip(w, QStringLiteral("ToolTip Message"), w->toolTip(), msec);
}

void BalloonWidget::showToolTip(QWidget *w)
{
    showToolTip(w, QStringLiteral("ToolTip Message"), w->toolTip(), toolTipTimeout_);
}
