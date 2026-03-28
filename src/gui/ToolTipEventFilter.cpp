#include "gui/ToolTipEventFilter.h"
#include "gui/BalloonWidget.h"

#include <QEvent>
#include <QHelpEvent>
#include <QWidget>

ToolTipEventFilter::ToolTipEventFilter(QObject *parent)
    : QObject(parent)
{
}

bool ToolTipEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QWidget *w = qobject_cast<QWidget *>(watched);
        if (w) {
            QHelpEvent *he = static_cast<QHelpEvent *>(event);
            if (!w->toolTip().isEmpty() && BalloonWidget::canShowToolTip(w, he->globalPos())) {
                BalloonWidget *balloon = new BalloonWidget(he->globalPos());
                balloon->showToolTip(w);
            }
            event->accept();
            return true;
        }
    }

    return QObject::eventFilter(watched, event);
}
