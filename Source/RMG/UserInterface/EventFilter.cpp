#include "EventFilter.hpp"

using namespace UserInterface;

EventFilter::EventFilter(QObject *parent) : QObject(parent)
{
}

EventFilter::~EventFilter(void)
{
}

bool EventFilter::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Type::KeyPress:
        emit this->on_EventFilter_KeyPressed((QKeyEvent *)event);
        return true;
    case QEvent::Type::Drop:
        emit this->on_EventFilter_FileDropped((QDropEvent *)event);
        return true;
    case QEvent::Type::KeyRelease:
        emit this->on_EventFilter_KeyReleased((QKeyEvent *)event);
        return true;
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}
