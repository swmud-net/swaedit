#include "map/RoomCoords.h"

#include <cmath>

RoomCoords::RoomCoords(int x, int y, int z, int islandNo)
    : x_(x), y_(y), z_(z), islandNo_(islandNo), layer_(0)
{
}

RoomCoords::RoomCoords(int x, int y, int z)
    : RoomCoords(x, y, z, 0)
{
}

RoomCoords::RoomCoords()
    : RoomCoords(0, 0, 0, 0)
{
}

RoomCoords RoomCoords::clone() const
{
    RoomCoords c(x_, y_, z_, islandNo_);
    c.layer_ = layer_;
    return c;
}

int RoomCoords::getDistance(const RoomCoords &c) const
{
    int ddx = c.x() - x_;
    ddx *= ddx;
    int ddy = c.y() - y_;
    ddy *= ddy;
    int ddz = c.z() - z_;
    ddz *= ddz;

    return static_cast<int>(std::sqrt(ddx + ddy + ddz));
}

bool RoomCoords::operator==(const RoomCoords &other) const
{
    return x_ == other.x_ && y_ == other.y_ && z_ == other.z_ && islandNo_ == other.islandNo_;
}

QString RoomCoords::toString() const
{
    return QStringLiteral("(%1,%2,%3)").arg(x_).arg(y_).arg(z_);
}
