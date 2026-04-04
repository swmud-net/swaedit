#ifndef AREA_H
#define AREA_H

#include <QString>
#include <QList>

// --- Simple / leaf structs ---

struct ShortDesc {
    QString inflect0;
    QString inflect1;
    QString inflect2;
    QString inflect3;
    QString inflect4;
    QString inflect5;
};

struct SectionA {
    int str = 0;
    int intel = 0;
    int wis = 0;
    int dex = 0;
    int con = 0;
    int cha = 0;
    int lck = 0;
};

struct SectionS {
    int saving_poison_death = 0;
    int saving_wand = 0;
    int saving_para_petri = 0;
    int saving_breath = 0;
    int saving_spell_staff = 0;
};

struct SectionR {
    int height = 0;
    int weight = 0;
    qint64 speaks = 0;
    QString speaking;
    int numattacks = 0;
};

struct SectionX {
    int hitroll = 0;
    int damroll = 0;
    qint64 xflags = 0;
    qint64 resistant = 0;
    qint64 immune = 0;
    qint64 susceptible = 0;
    qint64 attacks = 0;
    qint64 defenses = 0;
};

struct SectionT {
    int thac0 = 0;
    int ac = 0;
    int hitnodice = 0;
    int hitsizedice = 0;
    int hitplus = 0;
    int damnodice = 0;
    int damsizedice = 0;
    int damplus = 0;
};

struct SectionV {
    QString vipflags;
};

struct Program {
    QString type;
    QString args;
    QString comlist;
};

struct ExtraDesc {
    QString keyword;
    QString description;
};

// --- Head sub-structs ---

struct Vnums {
    qint64 lvnum = 0;
    qint64 uvnum = 0;
};

struct Economy {
    qint64 low = 0;
    qint64 high = 0;
};

struct Reset {
    int frequency = 0;
    QString message;
};

struct Ranges {
    int low = 0;
    int high = 0;
};

// --- Head ---

struct Head {
    QString name;
    QString authors;
    QString builders;
    int security = 0;
    Vnums vnums;
    qint64 flags = 0;
    Economy economy;
    Reset reset;
    Ranges ranges;
};

// --- Mobiles ---

struct Mobile {
    qint64 vnum = 0;
    QString name;
    ShortDesc shortDesc;
    QString longDesc;
    QString description;
    QString race;
    int level = 0;
    qint64 act = 0;
    qint64 affected = 0;
    int alignment = 0;
    int sex = 0;
    qint64 credits = 0;
    int position = 0;
    SectionA sectiona;
    SectionS sections;
    SectionR sectionr;
    SectionX sectionx;
    SectionT sectiont;
    SectionV sectionv;
    QString dialog;
    QList<Program> programs;
};

// --- Objects ---

struct ObjectValues {
    int value0 = 0;
    int value1 = 0;
    int value2 = 0;
    QString value3;
    QString value4;
    QString value5;
};

struct Requirement {
    int location = 0;
    int modifier = 0;
    QString type;
};

struct Affect {
    int location = 0;
    int modifier = 0;
};

struct AreaObject {
    qint64 vnum = 0;
    QString name;
    ShortDesc shortDesc;
    QString description;
    QString actiondesc;
    int type = 0;
    qint64 extraflags = 0;
    qint64 wearflags = 0;
    int layers = 0;
    ObjectValues values;
    qint64 weight = 0;
    qint64 cost = 0;
    qint64 gender = 0;
    int level = 0;
    QList<ExtraDesc> extradescs;
    QList<Requirement> requirements;
    QList<Affect> affects;
    QList<Program> programs;
};

// --- Rooms ---

struct Exit {
    int direction = 0;
    QString description;
    QString keyword;
    qint64 flags = 0;
    qint64 key = 0;
    qint64 vnum = 0;
    int distance = 0;
};

struct Room {
    qint64 vnum = 0;
    QString name;
    QString description;
    QString nightdesc;
    int light = 0;
    qint64 flags = 0;
    int sector = 0;
    qint64 teledelay = 0;
    qint64 televnum = 0;
    qint64 tunnel = 0;
    QList<Exit> exits;
    QList<ExtraDesc> extradescs;
    QList<Program> programs;
};

// --- Resets ---

struct AreaReset {
    QString command;
    qint64 extra = 0;
    qint64 arg1 = 0;
    qint64 arg2 = 0;
    qint64 arg3 = 0;
    qint64 arg4 = 0;
};

// --- Shops ---

struct ShopTypes {
    int type0 = 0;
    int type1 = 0;
    int type2 = 0;
    int type3 = 0;
    int type4 = 0;
};

struct Shop {
    qint64 keeper = 0;
    ShopTypes types;
    int profitbuy = 0;
    int profitsell = 0;
    int open = 0;
    int close = 0;
    qint64 flags = 0;
};

// --- Repairs ---

struct RepairTypes {
    int type0 = 0;
    int type1 = 0;
    int type2 = 0;
};

struct Repair {
    qint64 keeper = 0;
    RepairTypes types;
    int profitfix = 0;
    int shoptype = 0;
    int open = 0;
    int close = 0;
};

// --- Specials ---

struct Special {
    qint64 vnum = 0;
    QString function;
    QString function2;
};

// --- Top-level Area ---

struct Area {
    Head head;
    QList<Mobile> mobiles;
    QList<AreaObject> objects;
    QList<Room> rooms;
    QList<AreaReset> resets;
    QList<Shop> shops;
    QList<Repair> repairs;
    QList<Special> specials;
};

#endif // AREA_H
