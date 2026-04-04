#include "core/Renumberer.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

Renumberer::Renumberer(Area *area, qint64 newFirstVnum, int flags,
                       const QMap<QString, ResetInfoDef> &resetsMap)
    : area_(area)
    , newFirstVnum_(newFirstVnum)
    , flags_(flags)
    , resetsMap_(resetsMap)
{
}

void Renumberer::renumber()
{
    qint64 oldLvnum = area_->head.vnums.lvnum;
    qint64 oldUvnum = area_->head.vnums.uvnum;
    qint64 diff = newFirstVnum_ - oldLvnum;

    if (diff == 0)
        return;

    // Renumber head vnums
    area_->head.vnums.lvnum += diff;
    area_->head.vnums.uvnum += diff;

    // Renumber items (only vnums within the area range)
    for (AreaObject &obj : area_->objects) {
        if (obj.vnum >= oldLvnum && obj.vnum <= oldUvnum) {
            obj.vnum += diff;
        }

        if (flags_ & RENUMBER_MUDPROGS) {
            int progNo = 0;
            for (Program &prog : obj.programs) {
                ++progNo;
                renumberMudprogText(prog.comlist, oldLvnum, oldUvnum, diff,
                                    QStringLiteral("item"), obj.vnum, progNo);
            }
        }
    }

    // Renumber mobiles (only vnums within the area range)
    for (Mobile &mob : area_->mobiles) {
        if (mob.vnum >= oldLvnum && mob.vnum <= oldUvnum) {
            mob.vnum += diff;
        }

        if (flags_ & RENUMBER_MUDPROGS) {
            int progNo = 0;
            for (Program &prog : mob.programs) {
                ++progNo;
                renumberMudprogText(prog.comlist, oldLvnum, oldUvnum, diff,
                                    QStringLiteral("mobile"), mob.vnum, progNo);
            }
        }
    }

    // Renumber rooms (only vnums within the area range)
    for (Room &room : area_->rooms) {
        if (room.vnum >= oldLvnum && room.vnum <= oldUvnum) {
            room.vnum += diff;
        }

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
            int progNo = 0;
            for (Program &prog : room.programs) {
                ++progNo;
                renumberMudprogText(prog.comlist, oldLvnum, oldUvnum, diff,
                                    QStringLiteral("room"), room.vnum, progNo);
            }
        }
    }

    // Renumber resets
    for (AreaReset &reset : area_->resets) {
        if (!resetsMap_.contains(reset.command))
            continue;

        const ResetInfoDef &resDef = resetsMap_[reset.command];

        // Helper lambda to renumber a single arg based on its type
        auto renumberArg = [&](qint64 &argVal, const ResetArgDef &argDef) {
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

void Renumberer::renumberMudprogText(QString &comlist, qint64 oldLvnum, qint64 oldUvnum, qint64 diff,
                                     const QString &ownerType, qint64 ownerVnum, int progNo)
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
        int matchStart;  // start of full match (for offset calculation)
        qint64 num;
        qint64 newNum;
    };
    QList<Replacement> replacements;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        bool ok;
        qint64 num = match.captured(2).toLongLong(&ok);
        if (ok && num >= oldLvnum && num <= oldUvnum) {
            qint64 newNum = num + diff;
            Replacement r;
            // Replace only the numeric part, preserving the prefix
            r.start = match.capturedStart(2);
            r.length = match.capturedLength(2);
            r.newText = QString::number(newNum);
            r.matchStart = match.capturedStart(0);
            r.num = num;
            r.newNum = newNum;
            replacements.append(r);
        }
    }

    // Generate warnings before applying replacements (offsets are still valid)
    for (const Replacement &r : replacements) {
        // Compute line number and offset within line
        int lineNo = comlist.left(r.matchStart).count('\n') + 1;
        int lastNewline = comlist.lastIndexOf('\n', r.matchStart - 1);
        int offset = (lastNewline >= 0) ? (r.matchStart - lastNewline - 1) : r.matchStart;

        warnings_.append(QStringLiteral("changed ") + ownerType + "'s: " +
                         QString::number(ownerVnum) + " program's: " +
                         QString::number(progNo) + " vnum: " +
                         QString::number(r.num) + " to: " +
                         QString::number(r.newNum) + " in line: " +
                         QString::number(lineNo) + " at offset: " +
                         QString::number(offset));
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
