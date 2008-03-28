package pl.swmud.ns.swaedit.gui;

import com.trolltech.qt.core.QPoint;
import com.trolltech.qt.core.QRectF;
import com.trolltech.qt.core.QTimer;
import com.trolltech.qt.core.Qt;
import com.trolltech.qt.gui.QBitmap;
import com.trolltech.qt.gui.QBrush;
import com.trolltech.qt.gui.QCloseEvent;
import com.trolltech.qt.gui.QColor;
import com.trolltech.qt.gui.QContentsMargins;
import com.trolltech.qt.gui.QFont;
import com.trolltech.qt.gui.QImage;
import com.trolltech.qt.gui.QLabel;
import com.trolltech.qt.gui.QMouseEvent;
import com.trolltech.qt.gui.QPaintEvent;
import com.trolltech.qt.gui.QPainter;
import com.trolltech.qt.gui.QPainterPath;
import com.trolltech.qt.gui.QPalette;
import com.trolltech.qt.gui.QPen;
import com.trolltech.qt.gui.QPixmap;
import com.trolltech.qt.gui.QWidget;

public class BalloonWidget extends QWidget {
    
    public enum ColorTheme {
        Neutral,
        Jedi,
        Sith
    }

    private QImage iconImage;
    private QPixmap borderMap;
    private QLabel titleLabel;
    private QLabel msgLabel;
    private final int rectWH = 20;
    private final int arrowW = 30;
    private final int arrowH = 30;
    private final int arrowD = 10; /* arrow shift */
    private final int arrowWD = arrowW+arrowD;
    private final int msgLabelMarginRight = 10;
    private int iconImageW = 48;
    private int iconImageH = 48;
    private boolean arrowTop;
    private boolean arrowLeft;
    private int top = 0;
    private QColor headerColor;
    private QPoint pos;
    private double progress = -1;
    private double progressMax = 0;
    private QImage progressHilt;
    private QImage progressEnd;
    private QImage progress1;
    private int progressPixels;
    private boolean closeOnComplete = true;
    private double pxs; /* pixels per 1 per cent */
    private static QPoint lastToolTipPos;
    private static BalloonWidget lastToolTip;
    private static QWidget lastToolTipWidget;
    private BalloonWidget lastMessage;
    private BalloonWidget lastProgress;
    protected int w = 250;
    protected int h = 70;
    protected int messageTimeout = 3000;
    protected int toolTipTimeout = 3000;
    public Signal0 progressCompleted = new Signal0();
    public Signal0 balloonClicked = new Signal0();
    public Signal0 balloonDoubleClicked = new Signal0();


    public BalloonWidget(QPoint pos) {
        this(pos,ColorTheme.Neutral);
    }

    public BalloonWidget(QPoint pos, ColorTheme colorTheme) {
        setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose);
        setWindowFlags(Qt.WindowType.ToolTip);
        this.pos = pos;

        switch (colorTheme) {
        case Sith:
            headerColor = new QColor(0x982100);
            iconImage = new QImage("images/sith.png");
            progressHilt = new QImage("images/hilt_red.png");
            progressEnd = new QImage("images/end_red.png");
            progress1 = new QImage("images/1_red.png");
            break;

        case Jedi:
            headerColor = new QColor(0x219800);            
            iconImage = new QImage("images/jedi.png");
            progressHilt = new QImage("images/hilt_green.png");
            progressEnd = new QImage("images/end_green.png");
            progress1 = new QImage("images/1_green.png");
            break;

        default: /* Neutral */
            headerColor = new QColor(0x002198);
            iconImage = new QImage("images/neutral.png");
            progressHilt = new QImage("images/hilt_blue.png");
            progressEnd = new QImage("images/end_blue.png");
            progress1 = new QImage("images/1_blue.png");
            break;
        }
        
        if (iconImage == null || iconImage.width() != iconImageW || iconImage.height() != iconImageH) {
            iconImage = new QImage(48,48,QImage.Format.Format_ARGB32);
            iconImage.fill(QColor.transparent.rgba());
        }
        
        if (progressHilt.width() < 1 || progressEnd.width() < 1 || progress1.width() < 1) {
            progressHilt = null;
            progressEnd = null;
            progress1 = null;
        }
        
        progressPixels = w-iconImageW-progressHilt.width()-30;
        pxs = progressPixels/100.0;

        arrowTop = pos.y() - h - arrowH < 0;
        arrowLeft = pos.x() - w + arrowD + rectWH < 0;
        
        drawBalloon();
        
        resize(w+10,h+10+arrowH);
        QPalette pal = palette();
        pal.setColor(QPalette.ColorRole.Window, new QColor(0xffffff));
        setPalette(pal);
        
        titleLabel = new QLabel(this);
        titleLabel.move(10, 5+top);
        titleLabel.setTextFormat(Qt.TextFormat.PlainText);
        QFont f = titleLabel.font();
        f.setBold(true);
        titleLabel.setFont(f);
        pal = titleLabel.palette();
        pal.setColor(QPalette.ColorRole.WindowText, QColor.white);
        titleLabel.setPalette(pal);
        
        msgLabel = new QLabel(this);
        msgLabel.setTextFormat(Qt.TextFormat.PlainText);
        QContentsMargins cm = msgLabel.getContentsMargins();
        cm.right = msgLabelMarginRight;
        cm.bottom = 15;
        cm.top = 0;
        msgLabel.setContentsMargins(cm);
        msgLabel.resize(w-cm.right-iconImage.width()-10, msgLabel.height());
        msgLabel.setWordWrap(true);
        msgLabel.move(iconImage.width()+10, 25+top);
        f = msgLabel.font();
        f.setBold(true);
        titleLabel.setFont(f);
        pal = msgLabel.palette();
        pal.setColor(QPalette.ColorRole.WindowText, new QColor(0x494949));
        msgLabel.setPalette(pal);
    }
    
    protected void paintEvent(QPaintEvent e) {
        QPainter p = new QPainter(this);
        p.drawPixmap(0, 0, borderMap);
        p.fillRect(0, 0, w+1, 20+top, new QBrush(headerColor));
        p.drawImage(0,h/2-iconImage.height()/2+10+top/2, iconImage);
        if (progress > -1 && progressHilt != null) {
            double pxr = 0; /* unprinted floating point data from previous pixel */
            int px = iconImage.width()+10+progressHilt.width();
            p.drawImage(iconImage.width()+10, h-15, progressHilt);
            for (int i = 0; i < (int)progress; i++) {
                int j = 0;
                for (; j < (int)(pxs+pxr); j++, px++) {
                    p.drawImage(px, h-15, progress1);
                }
                pxr += pxs-j;
            }
            p.drawImage(px, h-15, progressEnd);
        }
        p.end();
    }
    
    protected void mouseReleaseEvent(QMouseEvent e) {
        if (e.button() == Qt.MouseButton.LeftButton) {
            balloonClicked.emit();
        }
        close();
    }
    
    protected void mouseDoubleClickEvent(QMouseEvent e) {
        if (e.button() == Qt.MouseButton.LeftButton) {
            balloonDoubleClicked.emit();
        }
    }
    
    protected void closeEvent(QCloseEvent e) {
        lastToolTipWidget = null;
        lastMessage = null;
        lastProgress = null;
        super.closeEvent(e);
    }
    
    private void drawBalloon() {
        if (arrowTop) {
            top = arrowH;
            h += top;
        }
        
        QPainterPath p = new QPainterPath();
        QRectF r = new QRectF(0,top,rectWH,rectWH);

        p.moveTo(0, rectWH+top);
        p.arcTo(r, 180, -90);
        /* arrow top left */
        if (arrowTop && arrowLeft) {
            move(pos.x()-rectWH-arrowD,pos.y());
            p.lineTo(rectWH, top);
            p.lineTo(rectWH+arrowD, top-arrowH);
            p.lineTo(rectWH+arrowWD, top);
        }
        /* arrow top right */
        else
        if (arrowTop && !arrowLeft) {
            move(pos.x()-w+rectWH+arrowD,pos.y());
            p.lineTo(w-rectWH-arrowWD, top);
            p.lineTo(w-rectWH-arrowD, top-arrowH);
        }
        p.lineTo(w-rectWH, top);
        r.moveTo(w-rectWH, top);

        p.arcTo(r, 90, -90);
        p.lineTo(w, h-rectWH);
        r.moveTo(w-rectWH, h-rectWH);

        p.arcTo(r, 0, -90);
        /* arrow bottom right */
        if (!arrowTop && !arrowLeft) {
            move(pos.x()-w+rectWH+arrowD,pos.y()-h-arrowH);
            p.lineTo(w-rectWH, h);
            p.lineTo(w-rectWH-arrowD, h+arrowH);
            p.lineTo(w-rectWH-arrowWD, h);
        }
        /* arrow bottom left */
        else
        if (!arrowTop && arrowLeft) {
            move(pos.x()-rectWH-arrowD,pos.y()-h-arrowH);
            p.lineTo(rectWH+arrowWD, h);
            p.lineTo(rectWH+arrowD, h+arrowH);
        }
        p.lineTo(rectWH, h);
        r.moveTo(0, h-rectWH);
        
        p.arcTo(r, 270, -90);
        p.lineTo(0,rectWH+top);
        
        /* balloon mask */
        QBitmap bitmap = new QBitmap(800,600);
        bitmap.fill(QColor.color0);
        QPainter painter = new QPainter(bitmap);
        painter.setPen(QColor.color1);
        painter.setBrush(QColor.color1);
        painter.drawPath(p);
        painter.end();
        setMask(bitmap);

        /* balloon border */
        borderMap = new QPixmap(800,600);
        painter = new QPainter(borderMap);
        painter.setPen(new QPen(headerColor,1));
        painter.setBrush(QColor.color0);
        painter.drawPath(p);
        painter.end();
    }
    
    public boolean canShowMessage() {
        return lastMessage == null;
    }
    
    public boolean canShowProgress() {
        return lastProgress != null;
    }
    
    private void updateLabels() {
//        titleLabel.updateGeometry();
//        titleLabel.update(titleLabel.contentsRect());
//        msgLabel.updateGeometry();
//        msgLabel.update(msgLabel.contentsRect());
    }
    
    public void showMessage(final String title, final String msg, int msec) {
        titleLabel.setText(title);
        msgLabel.setText(msg);
        updateLabels();
        lastMessage = this;
        show();
        if (msec > 0) {
            QTimer.singleShot(msec, this, "timerTimeout()");
        }
    }
    
    public void showMessage(final String title, final String msg) {
        showMessage(title, msg, messageTimeout);
    }
    
    public void setProgress(final String title, final String msg, double max) {
        setProgress(title, msg, max, true);
    }

    public void setProgress(final String title, final String msg, double max, boolean closeOnComplete) {
        titleLabel.setText(title);
        msgLabel.setText(msg);
        updateLabels();
        this.progressMax = max;
        this.progress = 0;
        this.closeOnComplete = closeOnComplete;
        lastProgress = this;
        show();
    }

    public void showProgress(final String title, final String msg, double progress) {
        titleLabel.setText(title);
        msgLabel.setText(msg);
        updateLabels();
        showProgress(progress);
    }

    public void showProgress(double progress) {
        if (progress >= 0) {
            if (progress < progressMax) {
                this.progress = (100*progress)/progressMax;
                repaint();
            }
            else {
                this.progress = 100;
                repaint();
                this.progress = -1;
                this.progressMax = 0;
                progressCompleted.emit();
                if (closeOnComplete) {
                    close();
                }
            }
        }
    }

    @SuppressWarnings("unused")
    private void timerTimeout() {
        close();
    }

    static public boolean canShowToolTip(final QWidget w, QPoint pos) {
        if (lastToolTipPos == null || !lastToolTipPos.equals(pos)
                && (lastToolTipWidget == null || lastToolTipWidget != w)) {
            return true;
        }
        return false;
    }
    
    public void showToolTip(final QWidget w, final String title, final String toolTip, int msec) {
        if (canShowToolTip(w,pos)) {
            lastToolTipWidget = w;
            if (lastToolTip != null) {
                lastToolTip.dispose();
            }
            lastToolTip = this;
            lastToolTipPos = pos;
            titleLabel.setText(title);
            msgLabel.setText(toolTip);
            updateLabels();
            show();
            if (msec > 0) {
                QTimer.singleShot(msec, this, "timerTimeout()");
            }
        }
    }
    
    public void showToolTip(final QWidget w, final String title, final String toolTip) {
        showToolTip(w, title, toolTip, toolTipTimeout);
    }

    public void showToolTip(final QWidget w, final String toolTip, int msec) {
        showToolTip(w, "ToolTip Message", toolTip, msec);
    }
    
    public void showToolTip(final QWidget w, final String toolTip) {
        showToolTip(w, "ToolTip Message", toolTip, toolTipTimeout);
    }

    public void showToolTip(final QWidget w, int msec) {
        showToolTip(w, "ToolTip Message", w.toolTip(), msec);
    }

    public void showToolTip(final QWidget w) {
        showToolTip(w, "ToolTip Message", w.toolTip(), toolTipTimeout);
    }
}
