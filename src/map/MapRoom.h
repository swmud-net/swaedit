#ifndef MAPROOM_H
#define MAPROOM_H

#include "map/RoomCoords.h"

#include <QMap>

struct Room;
class ExitWrapper;

class MapRoom {
public:
    MapRoom(Room *room, const RoomCoords &coords, ExitWrapper *parentExit = nullptr);

    Room *room() const { return room_; }
    void setRoom(Room *room) { room_ = room; }

    const RoomCoords &coords() const { return coords_; }
    RoomCoords &coords() { return coords_; }
    void setCoords(const RoomCoords &coords) { coords_ = coords; }

    ExitWrapper *parentExit() const { return parentExit_; }
    void setParentExit(ExitWrapper *parentExit) { parentExit_ = parentExit; }

    const QMap<ExitWrapper *, MapRoom *> &mapRooms() const { return mapRooms_; }
    QMap<ExitWrapper *, MapRoom *> &mapRooms() { return mapRooms_; }

    void addChildRoom(MapRoom *mapRoom, ExitWrapper *exWrapper);

    int getDistance(const MapRoom &other) const;

    bool drawn = false;

private:
    Room *room_ = nullptr;
    RoomCoords coords_;
    ExitWrapper *parentExit_ = nullptr;
    QMap<ExitWrapper *, MapRoom *> mapRooms_;
};

#endif // MAPROOM_H
