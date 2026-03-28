#include "gui/ExitEventFilter.h"

#include <QEvent>
#include <QMouseEvent>

ExitEventFilter::ExitEventFilter(QObject *parent)
    : QObject(parent)
{
}

bool ExitEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        Q_UNUSED(me);
        emit pressed();
        return true;
    }

    // Swallow all other mouse events
    if (event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::MouseMove) {
        return true;
    }

    return QObject::eventFilter(watched, event);
}
