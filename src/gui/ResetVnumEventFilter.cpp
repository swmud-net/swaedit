#include "gui/ResetVnumEventFilter.h"
#include "gui/VnumQuestionWidget.h"

#include <QEvent>
#include <QMouseEvent>

ResetVnumEventFilter::ResetVnumEventFilter(int argIndex, QObject *parent)
    : QObject(parent)
    , argIndex_(argIndex)
{
}

bool ResetVnumEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::RightButton) {
            VnumQuestionWidget *w = new VnumQuestionWidget();
            connect(w, &VnumQuestionWidget::vnumSet, this, [this](qint64 vnum) {
                emit vnumOverridden(argIndex_, vnum);
            });
            w->show();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}
