#include "map/Mapper.h"
#include "map/MapWidget.h"
#include "map/RoomSpread.h"
#include "model/Area.h"

#include <QDebug>

Mapper::Mapper(Area *area)
    : area_(area)
{
}

Mapper::~Mapper()
{
    cleanup();
}

void Mapper::cleanup()
{
    qDeleteAll(allExitWrappers_);
    allExitWrappers_.clear();
    qDeleteAll(allMapRooms_);
    allMapRooms_.clear();
    mapRooms_.clear();
    islandRooms_.clear();
    layers_.clear();
    islandNo_ = 0;
}

void Mapper::createMap()
{
    cleanup();

    for (int i = 0; i < area_->rooms.size(); ++i) {
        Room *room = &area_->rooms[i];
        if (!isAlreadyMade(room)) {
            RoomCoords coords(0, 0, 0, islandNo_++);
            MapRoom *mr = new MapRoom(room, coords);
            allMapRooms_.append(mr);
            mapRooms_.insert(room->vnum, mr);
            makeMapRoom(mr, room);
        }
    }

    assignRevExits();
    createIslands();
    checkCoords();
}

MapWidget *Mapper::makeMap()
{
    createMap();

    MapWidget *mw = new MapWidget(islandRooms_, 0, islandNo_);
    // Transfer ownership of map data to the widget
    QList<MapRoom *> rooms;
    QList<ExitWrapper *> exits;
    releaseOwnership(rooms, exits);
    mw->takeOwnership(rooms, exits);
    return mw;
}

void Mapper::makeMap(MapWidget *mw)
{
    createMap();
    mw->refreshMap(islandRooms_, islandNo_);
    // Transfer ownership of new map data to the widget
    QList<MapRoom *> rooms;
    QList<ExitWrapper *> exits;
    releaseOwnership(rooms, exits);
    mw->takeOwnership(rooms, exits);
}

void Mapper::releaseOwnership(QList<MapRoom *> &outMapRooms, QList<ExitWrapper *> &outExitWrappers)
{
    outMapRooms = allMapRooms_;
    outExitWrappers = allExitWrappers_;
    allMapRooms_.clear();
    allExitWrappers_.clear();
}

void Mapper::makeMapRoom(MapRoom *parent, Room *room)
{
    for (int i = 0; i < room->exits.size(); ++i) {
        const Exit &exit = room->exits[i];
        Room *childRoom = findRoom(exit);
        /* omit loop exits */
        if (childRoom != nullptr && childRoom->vnum != room->vnum) {
            ExitWrapper *exWrapper = new ExitWrapper(exit);
            allExitWrappers_.append(exWrapper);
            MapRoom *mr = new MapRoom(childRoom, getCoords(exit, parent->coords()), exWrapper);
            allMapRooms_.append(mr);
            parent->addChildRoom(mr, exWrapper);
            if (!isAlreadyMade(childRoom)) {
                mapRooms_.insert(childRoom->vnum, mr);
                makeMapRoom(mr, childRoom);
            }
        }
    }
}

void Mapper::checkCoords()
{
    QHash<RoomCoords, MapRoom *> coordsRooms;
    layers_.clear();
    layers_.resize(islandNo_);
    for (int i = 0; i < islandNo_; ++i)
        layers_[i] = 0;

    QMapIterator<int, MapRoom *> it(mapRooms_);
    while (it.hasNext()) {
        it.next();
        MapRoom *attempted = it.value();
        MapRoom *current = coordsRooms.value(attempted->coords(), nullptr);
        if (current != nullptr) {
            RoomCoords &rc = attempted->coords();
            rc.setLayer(++layers_[rc.islandNo()]);
        }
        coordsRooms.insert(attempted->coords(), attempted);
    }
}

void Mapper::assignRevExits()
{
    QMapIterator<int, MapRoom *> it(mapRooms_);
    while (it.hasNext()) {
        it.next();
        MapRoom *mr = it.value();
        QMapIterator<ExitWrapper *, MapRoom *> exitIt(mr->mapRooms());
        while (exitIt.hasNext()) {
            exitIt.next();
            ExitWrapper *exit = exitIt.key();
            MapRoom *toRoom = mapRooms_.value(exit->vnum(), nullptr);
            if (toRoom != nullptr && exit->revExit() == nullptr) {
                exit->setRevExit(getRevExit(
                    Exit{exit->direction(), exit->description(), exit->keyword(),
                         exit->flags(), exit->key(), exit->vnum(), exit->distance()},
                    mr, toRoom));
            }

            /* set distance */
            if (toRoom != nullptr && mr->getDistance(*toRoom) > 1) {
                exit->setDistant();
            }
        }
    }
}

ExitWrapper *Mapper::getRevExit(const Exit &exit, MapRoom *fromRoom, MapRoom *toRoom)
{
    QMapIterator<ExitWrapper *, MapRoom *> it(toRoom->mapRooms());
    while (it.hasNext()) {
        it.next();
        ExitWrapper *revExit = it.key();
        if (fromRoom->room()->vnum == revExit->vnum()
            && exit.direction == revExit->getRevDirection()) {
            return revExit;
        }
    }
    return nullptr;
}

bool Mapper::isAlreadyMade(Room *room) const
{
    return mapRooms_.contains(room->vnum);
}

Room *Mapper::findRoom(const Exit &exit) const
{
    for (int i = 0; i < area_->rooms.size(); ++i) {
        if (area_->rooms[i].vnum == exit.vnum) {
            return const_cast<Room *>(&area_->rooms[i]);
        }
    }
    return nullptr;
}

RoomCoords Mapper::getCoords(const Exit &exit, const RoomCoords &crds) const
{
    RoomCoords coords = crds.clone();
    switch (exit.direction) {
    case 0: /* north */
        coords.setZ(coords.z() - 1);
        break;
    case 1: /* east */
        coords.setX(coords.x() + 1);
        break;
    case 2: /* south */
        coords.setZ(coords.z() + 1);
        break;
    case 3: /* west */
        coords.setX(coords.x() - 1);
        break;
    case 4: /* up */
        coords.setY(coords.y() + 1);
        break;
    case 5: /* down */
        coords.setY(coords.y() - 1);
        break;
    case 6: /* north-east */
        coords.setZ(coords.z() - 1);
        coords.setX(coords.x() + 1);
        break;
    case 7: /* north-west */
        coords.setZ(coords.z() - 1);
        coords.setX(coords.x() - 1);
        break;
    case 8: /* south-east */
        coords.setZ(coords.z() + 1);
        coords.setX(coords.x() + 1);
        break;
    case 9: /* south-west */
        coords.setZ(coords.z() + 1);
        coords.setX(coords.x() - 1);
        break;
    default: /* somewhere: 10 */
        qWarning() << "virtual exits not yet supported!";
        break;
    }
    return coords;
}

void Mapper::createIslands()
{
    for (int i = 0; i < islandNo_; ++i) {
        QList<MapRoom *> iRooms;
        QMapIterator<int, MapRoom *> it(mapRooms_);
        while (it.hasNext()) {
            it.next();
            if (it.value()->coords().islandNo() == i) {
                iRooms.append(it.value());
            }
        }
        islandRooms_.insert(i, iRooms);
    }
}

int Mapper::getWidth(int islandNo) const
{
    if (!islandRooms_.contains(islandNo))
        return 0;
    return getXMiddle(islandRooms_.value(islandNo));
}

int Mapper::getHeight(int islandNo) const
{
    if (!islandRooms_.contains(islandNo))
        return 0;
    return getYMiddle(islandRooms_.value(islandNo));
}

int Mapper::getDepth(int islandNo) const
{
    if (!islandRooms_.contains(islandNo))
        return 0;
    return getZMiddle(islandRooms_.value(islandNo));
}
