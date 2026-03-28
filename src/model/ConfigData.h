#ifndef CONFIGDATA_H
#define CONFIGDATA_H

#include <QString>
#include <QList>

// --- Flags (areaflags.xml, roomflags.xml, exitflags.xml, etc.) ---

struct FlagDef {
    QString name;
    int value = 0;
};

// --- Exits (exits.xml) ---

struct ExitDef {
    QString abbreviation;
    QString name;
    int value = 0;
    bool empty = false;
    int opposite = -1;
};

// --- Item types (itemtypes.xml) ---

struct SubvalueDef {
    QString name;
    QString value;
    QString constraint; // "none" or "spell"
};

struct ItemTypeValueDef {
    int no = 0;
    QString name;
    QString subvaluesType; // "auto", "value", "flags", "types"
    QList<SubvalueDef> subvalues;
};

struct ItemTypeDef {
    QString name;
    int value = 0;
    bool visible = true;
    QList<ItemTypeValueDef> values;
};

// --- Names (races.xml, languages.xml, planets.xml, progtypes.xml, mobilespecfunctions.xml) ---
// Just a list of strings

// --- Types (positions.xml, repairtypes.xml, roomsectortypes.xml) ---

struct TypeDef {
    QString name;
    int value = 0;
};

// --- Resets info (resetsinfo.xml) ---

struct ResetArgValueDef {
    QString name;
    QString value;
};

struct ResetArgDef {
    QString name;
    QString type; // "room", "mob", "item", "ship", "intval", "strval", etc.
    QString valuesType; // "types" or "flags"
    QList<ResetArgValueDef> values;
};

struct ResetInfoDef {
    QString name;
    QString value; // command char: M, O, P, G, E, D, R, S, C
    ResetArgDef extra;
    ResetArgDef arg1;
    ResetArgDef arg2;
    ResetArgDef arg3;
    ResetArgDef arg4;
    QString requires;
};

// --- Highlighter (highlighter.xml) ---

struct HighlighterWordsDef {
    QString type;  // "flowInstruction", "progCommand", "ifCheck", "variable", "ratmVariable"
    QString color; // hex string like "0x8B008B", or "0"
    QList<QString> words;
};

// --- Aggregate config ---

struct ConfigData {
    QList<FlagDef> areaFlags;
    QList<FlagDef> itemWearFlags;
    QList<FlagDef> itemExtraFlags;
    QList<FlagDef> mobileActFlags;
    QList<FlagDef> mobileAffectedFlags;
    QList<FlagDef> roomFlags;
    QList<FlagDef> exitFlags;
    QList<FlagDef> xFlags;
    QList<FlagDef> resistFlags;
    QList<FlagDef> attackFlags;
    QList<FlagDef> defenseFlags;
    QList<FlagDef> shopFlags;

    QList<ExitDef> exits;
    QList<ItemTypeDef> itemTypes;

    QList<QString> races;
    QList<QString> languages;
    QList<QString> planets;
    QList<QString> progTypes;
    QList<QString> mobileSpecFunctions;

    QList<TypeDef> positions;
    QList<TypeDef> repairTypes;
    QList<TypeDef> roomSectorTypes;

    QList<ResetInfoDef> resetsInfo;
    QList<HighlighterWordsDef> highlighter;
};

#endif // CONFIGDATA_H
