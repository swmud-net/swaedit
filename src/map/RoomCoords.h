#ifndef ROOMCOORDS_H
#define ROOMCOORDS_H

#include <QString>
#include <QHash>

class RoomCoords {
public:
    RoomCoords(int x, int y, int z, int islandNo);
    RoomCoords(int x, int y, int z);
    RoomCoords();

    int x() const { return x_; }
    void setX(int x) { x_ = x; }

    int y() const { return y_; }
    void setY(int y) { y_ = y; }

    int z() const { return z_; }
    void setZ(int z) { z_ = z; }

    int islandNo() const { return islandNo_; }
    void setIslandNo(int islandNo) { islandNo_ = islandNo; }

    int layer() const { return layer_; }
    void setLayer(int layer) { layer_ = layer; }

    RoomCoords clone() const;

    int getDistance(const RoomCoords &c) const;

    bool operator==(const RoomCoords &other) const;

    QString toString() const;

private:
    int x_ = 0;
    int y_ = 0;
    int z_ = 0;
    int islandNo_ = 0;
    int layer_ = 0;
};

inline size_t qHash(const RoomCoords &c, size_t seed = 0)
{
    return qHash(c.x() + c.y() + c.z() + (c.islandNo() * 10000), seed);
}

#endif // ROOMCOORDS_H
