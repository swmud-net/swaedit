#ifndef RENUMBERER_H
#define RENUMBERER_H

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "model/Area.h"
#include "model/ConfigData.h"

class Renumberer {
public:
    static constexpr int RENUMBER_RELIABLE = 0;
    static constexpr int RENUMBER_MUDPROGS = 1;

    Renumberer(Area *area, int newFirstVnum, int flags,
               const QMap<QString, ResetInfoDef> &resetsMap);

    void renumber();

    QStringList getWarnings() const;
    void saveWarnings(const QString &path) const;

private:
    void renumberMudprogText(QString &comlist, int oldLvnum, int oldUvnum, int diff);

    Area *area_;
    int newFirstVnum_;
    int flags_;
    QMap<QString, ResetInfoDef> resetsMap_;
    QStringList warnings_;
};

#endif // RENUMBERER_H
