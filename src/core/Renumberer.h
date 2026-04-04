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

    Renumberer(Area *area, qint64 newFirstVnum, int flags,
               const QMap<QString, ResetInfoDef> &resetsMap);

    void renumber();

    QStringList getWarnings() const;
    void saveWarnings(const QString &path) const;

private:
    void renumberMudprogText(QString &comlist, qint64 oldLvnum, qint64 oldUvnum, qint64 diff,
                            const QString &ownerType, qint64 ownerVnum, int progNo);

    Area *area_;
    qint64 newFirstVnum_;
    int flags_;
    QMap<QString, ResetInfoDef> resetsMap_;
    QStringList warnings_;
};

#endif // RENUMBERER_H
