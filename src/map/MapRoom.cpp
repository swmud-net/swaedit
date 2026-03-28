#include "map/MapRoom.h"
#include "map/ExitWrapper.h"
#include "model/Area.h"

MapRoom::MapRoom(Room *room, const RoomCoords &coords, ExitWrapper *parentExit)
    : room_(room)
    , coords_(coords)
    , parentExit_(parentExit)
{
}

void MapRoom::addChildRoom(MapRoom *mapRoom, ExitWrapper *exWrapper)
{
    mapRooms_.insert(exWrapper, mapRoom);
}

int MapRoom::getDistance(const MapRoom &other) const
{
    return other.coords().getDistance(coords_);
}
