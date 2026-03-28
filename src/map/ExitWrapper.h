#ifndef EXITWRAPPER_H
#define EXITWRAPPER_H

#include "model/Area.h"

#include <QString>

class ExitWrapper {
public:
    ExitWrapper(const Exit &exit, ExitWrapper *revExit = nullptr);

    int direction() const { return direction_; }
    QString description() const { return description_; }
    QString keyword() const { return keyword_; }
    qint64 flags() const { return flags_; }
    int key() const { return key_; }
    int vnum() const { return vnum_; }
    int distance() const { return distance_; }

    ExitWrapper *revExit() const { return revExit_; }
    void setRevExit(ExitWrapper *revExit) { revExit_ = revExit; }

    bool isTwoWay() const { return revExit_ != nullptr; }

    bool isDrawn() const { return drawn_; }
    void setDrawn(bool drawn) { drawn_ = drawn; }
    void setDrawn() { drawn_ = true; }

    bool isDistant() const { return distant_; }
    void setDistant() { distant_ = true; }

    int getRevDirection() const;
    QString getDirectionName() const;

    bool operator<(const ExitWrapper &other) const;

private:
    int direction_ = 0;
    QString description_;
    QString keyword_;
    qint64 flags_ = 0;
    int key_ = 0;
    int vnum_ = 0;
    int distance_ = 0;

    ExitWrapper *revExit_ = nullptr;
    bool drawn_ = false;
    bool distant_ = false;
};

#endif // EXITWRAPPER_H
