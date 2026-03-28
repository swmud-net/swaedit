#include "map/ExitWrapper.h"

ExitWrapper::ExitWrapper(const Exit &exit, ExitWrapper *revExit)
    : direction_(exit.direction)
    , description_(exit.description)
    , keyword_(exit.keyword)
    , flags_(exit.flags)
    , key_(exit.key)
    , vnum_(exit.vnum)
    , distance_(exit.distance)
    , revExit_(revExit)
    , drawn_(false)
    , distant_(false)
{
}

int ExitWrapper::getRevDirection() const
{
    switch (direction_) {
    case 0:  /* north */      return 2;
    case 1:  /* east */       return 3;
    case 2:  /* south */      return 0;
    case 3:  /* west */       return 1;
    case 4:  /* up */         return 5;
    case 5:  /* down */       return 4;
    case 6:  /* north-east */ return 9;
    case 7:  /* north-west */ return 8;
    case 8:  /* south-east */ return 7;
    case 9:  /* south-west */ return 6;
    default: /* somewhere */  return 10;
    }
}

QString ExitWrapper::getDirectionName() const
{
    switch (direction_) {
    case 0:  return QStringLiteral("north");
    case 1:  return QStringLiteral("east");
    case 2:  return QStringLiteral("south");
    case 3:  return QStringLiteral("west");
    case 4:  return QStringLiteral("up");
    case 5:  return QStringLiteral("down");
    case 6:  return QStringLiteral("north-east");
    case 7:  return QStringLiteral("north-west");
    case 8:  return QStringLiteral("south-east");
    case 9:  return QStringLiteral("south-west");
    default: return QStringLiteral("somewhere");
    }
}

bool ExitWrapper::operator<(const ExitWrapper &other) const
{
    return direction_ < other.direction_;
}
