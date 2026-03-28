#ifndef ROOMSPREAD_H
#define ROOMSPREAD_H

#include "map/MapRoom.h"
#include "map/RoomCoords.h"

#include <QList>
#include <cmath>
#include <functional>

// Port of RoomSpread.java -- computes the middle of an island along one axis.
// Uses a function to extract the axis value from RoomCoords.

inline int getRoomSpreadMiddle(const QList<MapRoom *> &island,
                               std::function<int(const RoomCoords &)> getValue)
{
    if (island.size() < 1)
        return 0;

    const RoomCoords &coords = island.at(0)->coords();
    int min = getValue(coords);
    int max = min;

    for (MapRoom *mr : island) {
        int value = getValue(mr->coords());
        if (value > max) {
            max = value;
        } else if (value < min) {
            min = value;
        }
    }

    return std::abs(min) - std::abs(max);
}

inline int getXMiddle(const QList<MapRoom *> &island)
{
    return getRoomSpreadMiddle(island, [](const RoomCoords &c) { return c.x(); });
}

inline int getYMiddle(const QList<MapRoom *> &island)
{
    return getRoomSpreadMiddle(island, [](const RoomCoords &c) { return c.y(); });
}

inline int getZMiddle(const QList<MapRoom *> &island)
{
    return getRoomSpreadMiddle(island, [](const RoomCoords &c) { return c.z(); });
}

#endif // ROOMSPREAD_H
