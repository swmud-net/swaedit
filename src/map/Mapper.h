#ifndef MAPPER_H
#define MAPPER_H

#include "map/RoomCoords.h"
#include "map/MapRoom.h"
#include "map/ExitWrapper.h"

#include <QMap>
#include <QList>

struct Area;
struct Room;
struct Exit;
class MapWidget;

class Mapper {
public:
    explicit Mapper(Area *area);
    ~Mapper();

    MapWidget *makeMap();
    void makeMap(MapWidget *mw);

    int getWidth(int islandNo) const;
    int getHeight(int islandNo) const;
    int getDepth(int islandNo) const;

    const QMap<int, QList<MapRoom *>> &getIslandRooms() const { return islandRooms_; }
    int getIslandCount() const { return islandNo_; }

    // Transfer ownership of all MapRoom/ExitWrapper objects to caller.
    // After this, Mapper's destructor will NOT delete them.
    void releaseOwnership(QList<MapRoom *> &outMapRooms, QList<ExitWrapper *> &outExitWrappers);

private:
    void createMap();
    void makeMapRoom(MapRoom *parent, Room *room);
    void checkCoords();
    void assignRevExits();
    ExitWrapper *getRevExit(const Exit &exit, MapRoom *fromRoom, MapRoom *toRoom);
    bool isAlreadyMade(Room *room) const;
    Room *findRoom(const Exit &exit) const;
    RoomCoords getCoords(const Exit &exit, const RoomCoords &crds) const;
    void createIslands();
    void cleanup();

    Area *area_;
    int islandNo_ = 0;
    QMap<int, MapRoom *> mapRooms_;             // vnum -> MapRoom
    QMap<int, QList<MapRoom *>> islandRooms_;   // islandNo -> list of rooms
    QList<int> layers_;

    // We own all ExitWrapper and MapRoom objects; track for cleanup
    QList<ExitWrapper *> allExitWrappers_;
    QList<MapRoom *> allMapRooms_;
};

#endif // MAPPER_H
