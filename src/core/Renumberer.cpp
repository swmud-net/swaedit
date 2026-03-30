#include "core/Renumberer.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

Renumberer::Renumberer(Area *area, int newFirstVnum, int flags,
                       const QMap<QString, ResetInfoDef> &resetsMap)
    : area_(area)
    , newFirstVnum_(newFirstVnum)
    , flags_(flags)
    , resetsMap_(resetsMap)
{
}

void Renumberer::renumber()
{
    int oldLvnum = area_->head.vnums.lvnum;
    int oldUvnum = area_->head.vnums.uvnum;
    int diff = newFirstVnum_ - oldLvnum;

    if (diff == 0)
        return;

    // Renumber head vnums
    area_->head.vnums.lvnum += diff;
    area_->head.vnums.uvnum += diff;

    // Renumber items
    for (AreaObject &obj : area_->objects) {
        obj.vnum += diff;

        if (flags_ & RENUMBER_MUDPROGS) {
            for (Program &prog : obj.programs) {
                renumberMudprogText(prog.comlist, oldLvnum, oldUvnum, diff);
            }
        }
    }

    // Renumber mobiles
    for (Mobile &mob : area_->mobiles) {
        mob.vnum += diff;

        if (flags_ & RENUMBER_MUDPROGS) {
            for (Program &prog : mob.programs) {
                renumberMudprogText(prog.comlist, oldLvnum, oldUvnum, diff);
            }
        }
    }

    // Renumber rooms
    for (Room &room : area_->rooms) {
        room.vnum += diff;

        // Renumber exit destination vnums (only if they fall within old range)
        for (Exit &exit : room.exits) {
            if (exit.vnum >= oldLvnum && exit.vnum <= oldUvnum) {
                exit.vnum += diff;
            }
        }

        // Renumber televnum if in range
        if (room.televnum >= oldLvnum && room.televnum <= oldUvnum) {
            room.televnum += diff;
        }

        if (flags_ & RENUMBER_MUDPROGS) {
            for (Program &prog : room.programs) {
                renumberMudprogText(prog.comlist, oldLvnum, oldUvnum, diff);
            }
        }
    }

    // Renumber resets
    for (AreaReset &reset : area_->resets) {
        if (!resetsMap_.contains(reset.command))
            continue;

        const ResetInfoDef &resDef = resetsMap_[reset.command];

        // Helper lambda to renumber a single arg based on its type
        auto renumberArg = [&](int &argVal, const ResetArgDef &argDef) {
            if (argDef.type == "room" || argDef.type == "room_other" ||
                argDef.type == "mob"  || argDef.type == "mob_other"  ||
                argDef.type == "item" || argDef.type == "item_other" ||
                argDef.type == "ship" || argDef.type == "ship_other") {
                if (argVal >= oldLvnum && argVal <= oldUvnum) {
                    argVal += diff;
                }
            }
        };

        renumberArg(reset.extra, resDef.extra);
        renumberArg(reset.arg1, resDef.arg1);
        renumberArg(reset.arg2, resDef.arg2);
        renumberArg(reset.arg3, resDef.arg3);
        renumberArg(reset.arg4, resDef.arg4);
    }

    // Renumber shops
    for (Shop &shop : area_->shops) {
        if (shop.keeper >= oldLvnum && shop.keeper <= oldUvnum) {
            shop.keeper += diff;
        }
    }

    // Renumber repairs
    for (Repair &repair : area_->repairs) {
        if (repair.keeper >= oldLvnum && repair.keeper <= oldUvnum) {
            repair.keeper += diff;
        }
    }

    // Renumber specials
    for (Special &spec : area_->specials) {
        if (spec.vnum >= oldLvnum && spec.vnum <= oldUvnum) {
            spec.vnum += diff;
        }
    }
}

void Renumberer::renumberMudprogText(QString &comlist, int oldLvnum, int oldUvnum, int diff)
{
    // Scan for numbers optionally prefixed with m/i/o (matching Java regex)
    // Pattern matches: optional m/i/o prefix followed by digits starting with 1-9
    QRegularExpression re("\\b([mio]?)([1-9][0-9]*)\\b", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = re.globalMatch(comlist);

    // Collect replacements in reverse order to avoid offset issues
    struct Replacement {
        int start;
        int length;
        QString newText;
    };
    QList<Replacement> replacements;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString prefix = match.captured(1);
        bool ok;
        int num = match.captured(2).toInt(&ok);
        if (ok && num >= oldLvnum && num <= oldUvnum) {
            int newNum = num + diff;
            Replacement r;
            // Replace only the numeric part, preserving the prefix
            r.start = match.capturedStart(2);
            r.length = match.capturedLength(2);
            r.newText = QString::number(newNum);
            replacements.append(r);

            warnings_.append("Mudprog: replaced " + QString::number(num) +
                             " with " + QString::number(newNum) +
                             " in program text");
        }
    }

    // Apply replacements in reverse order
    for (int i = replacements.size() - 1; i >= 0; --i) {
        const Replacement &r = replacements[i];
        comlist.replace(r.start, r.length, r.newText);
    }
}

QStringList Renumberer::getWarnings() const
{
    return warnings_;
}

void Renumberer::saveWarnings(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const QString &w : warnings_) {
        out << w << "\n";
    }
    file.close();
}
