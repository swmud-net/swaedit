#ifndef BALLOONWIDGET_H
#define BALLOONWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QLabel>
#include <QPoint>

class BalloonWidget : public QWidget {
    Q_OBJECT

public:
    enum ColorTheme {
        Neutral,
        Jedi,
        Sith
    };

    explicit BalloonWidget(QPoint pos, ColorTheme colorTheme = Neutral);

    void showMessage(const QString &title, const QString &msg, int msec);
    void showMessage(const QString &title, const QString &msg);
    void showToolTip(QWidget *w, const QString &title, const QString &toolTip, int msec);
    void showToolTip(QWidget *w, const QString &title, const QString &toolTip);
    void showToolTip(QWidget *w, const QString &toolTip, int msec);
    void showToolTip(QWidget *w, const QString &toolTip);
    void showToolTip(QWidget *w, int msec);
    void showToolTip(QWidget *w);
    void setProgress(const QString &title, const QString &msg, double max, bool closeOnComplete = true);
    void showProgress(const QString &title, const QString &msg, double progress);
    void showProgress(double progress);

    bool canShowMessage() const;
    bool canShowProgress() const;

    static bool canShowToolTip(QWidget *w, QPoint pos);

signals:
    void progressCompleted();
    void balloonClicked();
    void balloonDoubleClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void drawBalloon();

    static constexpr int RECT_WH = 20;
    static constexpr int ARROW_W = 30;
    static constexpr int ARROW_H = 30;
    static constexpr int ARROW_D = 10;
    static constexpr int ARROW_WD = ARROW_W + ARROW_D;
    static constexpr int MSG_LABEL_MARGIN_RIGHT = 10;
    static constexpr int ICON_IMAGE_W = 48;
    static constexpr int ICON_IMAGE_H = 48;

    QImage iconImage_;
    QPixmap borderMap_;
    QRegion balloonRegion_;
    QLabel *titleLabel_;
    QLabel *msgLabel_;
    bool arrowTop_;
    bool arrowLeft_;
    int top_ = 0;
    QColor headerColor_;
    QPoint pos_;
    double progress_ = -1;
    double progressMax_ = 0;
    QImage progressHilt_;
    QImage progressEnd_;
    QImage progress1_;
    int progressPixels_;
    bool closeOnComplete_ = true;
    double pxs_; // pixels per 1 percent
    int w_ = 250;
    int h_ = 70;
    int messageTimeout_ = 3000;
    int toolTipTimeout_ = 3000;

    BalloonWidget *lastMessage_ = nullptr;
    BalloonWidget *lastProgress_ = nullptr;

    static QPoint lastToolTipPos_;
    static BalloonWidget *lastToolTip_;
    static QWidget *lastToolTipWidget_;
};

#endif // BALLOONWIDGET_H
