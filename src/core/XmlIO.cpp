#include "core/XmlIO.h"
#include "model/Area.h"
#include "model/ConfigData.h"

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <libxml/xmlschemas.h>
#include <libxml/parser.h>

// ---------------------------------------------------------------------------
// XSD Validation (libxml2)
// ---------------------------------------------------------------------------

static QString s_validationErrors;

static void xsdValidationErrorHandler(void * /*ctx*/, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    char buf[2048];
    vsnprintf(buf, sizeof(buf), msg, args);
    va_end(args);
    s_validationErrors += QString::fromUtf8(buf);
}

QString XmlIO::validateXml(const QString &xmlFilePath, const QString &xsdFilePath)
{
    s_validationErrors.clear();

    QByteArray xsdPath = xsdFilePath.toUtf8();
    QByteArray xmlPath = xmlFilePath.toUtf8();

    xmlDocPtr schemaDoc = xmlReadFile(xsdPath.constData(), nullptr, 0);
    if (!schemaDoc)
        return QStringLiteral("Failed to parse XSD schema: ") + xsdFilePath;

    xmlSchemaParserCtxtPtr parserCtxt = xmlSchemaNewDocParserCtxt(schemaDoc);
    if (!parserCtxt) {
        xmlFreeDoc(schemaDoc);
        return QStringLiteral("Failed to create schema parser context");
    }

    xmlSchemaPtr schema = xmlSchemaParse(parserCtxt);
    xmlSchemaFreeParserCtxt(parserCtxt);
    if (!schema) {
        xmlFreeDoc(schemaDoc);
        return QStringLiteral("Failed to parse XSD schema");
    }

    xmlSchemaValidCtxtPtr validCtxt = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(validCtxt, xsdValidationErrorHandler, xsdValidationErrorHandler, nullptr);

    xmlDocPtr doc = xmlReadFile(xmlPath.constData(), nullptr, 0);
    if (!doc) {
        xmlSchemaFreeValidCtxt(validCtxt);
        xmlSchemaFree(schema);
        xmlFreeDoc(schemaDoc);
        return QStringLiteral("Failed to parse XML file: ") + xmlFilePath;
    }

    int ret = xmlSchemaValidateDoc(validCtxt, doc);

    xmlFreeDoc(doc);
    xmlSchemaFreeValidCtxt(validCtxt);
    xmlSchemaFree(schema);
    xmlFreeDoc(schemaDoc);

    if (ret == 0)
        return QString(); // valid
    return s_validationErrors.isEmpty()
        ? QStringLiteral("XML validation failed")
        : s_validationErrors.trimmed();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void skipToEnd(QXmlStreamReader &xml, const QString &tagName)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name() == tagName)
            return;
    }
}

static QString readText(QXmlStreamReader &xml)
{
    return xml.readElementText();
}

static int readInt(QXmlStreamReader &xml)
{
    return xml.readElementText().toInt();
}

static qint64 readLong(QXmlStreamReader &xml)
{
    return xml.readElementText().toLongLong();
}

// Open a file and position the reader past the XML declaration.
// Returns false if the file can't be opened.
static bool openXml(const QString &filePath, QFile &file, QXmlStreamReader &xml)
{
    file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    xml.setDevice(&file);
    return true;
}

// ---------------------------------------------------------------------------
// Config loaders
// ---------------------------------------------------------------------------

QList<FlagDef> XmlIO::loadFlags(const QString &filePath)
{
    QList<FlagDef> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"flag") {
            FlagDef f;
            while (!(xml.isEndElement() && xml.name() == u"flag")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"name")       f.name = readText(xml);
                else if (xml.name() == u"value")  f.value = readInt(xml);
            }
            result.append(f);
        }
    }
    return result;
}

QList<ExitDef> XmlIO::loadExits(const QString &filePath)
{
    QList<ExitDef> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"exit") {
            ExitDef e;
            auto attrs = xml.attributes();
            if (attrs.hasAttribute(QStringLiteral("empty")))
                e.empty = (attrs.value(QStringLiteral("empty")) == u"true");
            if (attrs.hasAttribute(QStringLiteral("opposite")))
                e.opposite = attrs.value(QStringLiteral("opposite")).toInt();

            while (!(xml.isEndElement() && xml.name() == u"exit")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"abbreviation")  e.abbreviation = readText(xml);
                else if (xml.name() == u"name")      e.name = readText(xml);
                else if (xml.name() == u"value")     e.value = readInt(xml);
            }
            result.append(e);
        }
    }
    return result;
}

QList<ItemTypeDef> XmlIO::loadItemTypes(const QString &filePath)
{
    QList<ItemTypeDef> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"itemtype") {
            ItemTypeDef it;
            auto attrs = xml.attributes();
            if (attrs.hasAttribute(QStringLiteral("visible")))
                it.visible = (attrs.value(QStringLiteral("visible")) != u"false");

            while (!(xml.isEndElement() && xml.name() == u"itemtype")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;

                if (xml.name() == u"name") {
                    it.name = readText(xml);
                } else if (xml.name() == u"value" && it.name.isEmpty() == false && it.values.isEmpty()) {
                    // This is the top-level <value> (item type id), not a sub-value
                    it.value = readInt(xml);
                } else if (xml.name() == u"values") {
                    // Parse value definitions
                    while (!(xml.isEndElement() && xml.name() == u"values")) {
                        xml.readNext();
                        if (!xml.isStartElement()) continue;
                        if (xml.name() == u"value") {
                            ItemTypeValueDef v;
                            while (!(xml.isEndElement() && xml.name() == u"value")) {
                                xml.readNext();
                                if (!xml.isStartElement()) continue;
                                if (xml.name() == u"no") {
                                    v.no = readInt(xml);
                                } else if (xml.name() == u"name") {
                                    v.name = readText(xml);
                                } else if (xml.name() == u"subvalues") {
                                    auto svAttrs = xml.attributes();
                                    v.subvaluesType = svAttrs.hasAttribute(QStringLiteral("type"))
                                        ? svAttrs.value(QStringLiteral("type")).toString() : "auto";
                                    while (!(xml.isEndElement() && xml.name() == u"subvalues")) {
                                        xml.readNext();
                                        if (!xml.isStartElement()) continue;
                                        if (xml.name() == u"subvalue") {
                                            SubvalueDef sv;
                                            auto svAttr = xml.attributes();
                                            sv.constraint = svAttr.hasAttribute(QStringLiteral("constraint"))
                                                ? svAttr.value(QStringLiteral("constraint")).toString() : "none";
                                            while (!(xml.isEndElement() && xml.name() == u"subvalue")) {
                                                xml.readNext();
                                                if (!xml.isStartElement()) continue;
                                                if (xml.name() == u"name")       sv.name = readText(xml);
                                                else if (xml.name() == u"value") sv.value = readText(xml);
                                            }
                                            v.subvalues.append(sv);
                                        }
                                    }
                                }
                            }
                            it.values.append(v);
                        }
                    }
                }
            }
            result.append(it);
        }
    }
    return result;
}

QList<QString> XmlIO::loadNames(const QString &filePath)
{
    QList<QString> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"name")
            result.append(readText(xml));
    }
    return result;
}

QList<TypeDef> XmlIO::loadTypes(const QString &filePath)
{
    QList<TypeDef> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"type") {
            TypeDef t;
            while (!(xml.isEndElement() && xml.name() == u"type")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"name")       t.name = readText(xml);
                else if (xml.name() == u"value") t.value = readInt(xml);
            }
            result.append(t);
        }
    }
    return result;
}

static ResetArgDef parseResetArg(QXmlStreamReader &xml, const QString &tagName)
{
    ResetArgDef arg;
    auto attrs = xml.attributes();
    arg.type = attrs.hasAttribute(QStringLiteral("type")) ? attrs.value(QStringLiteral("type")).toString() : "intval";

    while (!(xml.isEndElement() && xml.name() == tagName)) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"name") {
            arg.name = readText(xml);
        } else if (xml.name() == u"values") {
            auto vAttrs = xml.attributes();
            arg.valuesType = vAttrs.hasAttribute(QStringLiteral("type"))
                ? vAttrs.value(QStringLiteral("type")).toString() : "types";
            while (!(xml.isEndElement() && xml.name() == u"values")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"value") {
                    ResetArgValueDef v;
                    while (!(xml.isEndElement() && xml.name() == u"value")) {
                        xml.readNext();
                        if (!xml.isStartElement()) continue;
                        if (xml.name() == u"name")       v.name = readText(xml);
                        else if (xml.name() == u"value") v.value = readText(xml);
                    }
                    arg.values.append(v);
                }
            }
        }
    }
    return arg;
}

QList<ResetInfoDef> XmlIO::loadResetsInfo(const QString &filePath)
{
    QList<ResetInfoDef> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"reset") {
            ResetInfoDef r;
            while (!(xml.isEndElement() && xml.name() == u"reset")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"name")          r.name = readText(xml);
                else if (xml.name() == u"value")    r.value = readText(xml);
                else if (xml.name() == u"extra")    r.extra = parseResetArg(xml, QStringLiteral("extra"));
                else if (xml.name() == u"arg1")     r.arg1 = parseResetArg(xml, QStringLiteral("arg1"));
                else if (xml.name() == u"arg2")     r.arg2 = parseResetArg(xml, QStringLiteral("arg2"));
                else if (xml.name() == u"arg3")     r.arg3 = parseResetArg(xml, QStringLiteral("arg3"));
                else if (xml.name() == u"arg4")     r.arg4 = parseResetArg(xml, QStringLiteral("arg4"));
                else if (xml.name() == u"requires") r.requires = readText(xml);
            }
            result.append(r);
        }
    }
    return result;
}

QList<HighlighterWordsDef> XmlIO::loadHighlighter(const QString &filePath)
{
    QList<HighlighterWordsDef> result;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return result;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"words") {
            HighlighterWordsDef w;
            auto attrs = xml.attributes();
            w.type = attrs.value(QStringLiteral("type")).toString();
            w.color = attrs.hasAttribute(QStringLiteral("color"))
                ? attrs.value(QStringLiteral("color")).toString() : "0";
            while (!(xml.isEndElement() && xml.name() == u"words")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"word")
                    w.words.append(readText(xml));
            }
            result.append(w);
        }
    }
    return result;
}

// Helper: validate a config XML file against its XSD.
// Returns error string (empty on success).
static QString validateConfig(const QString &xmlFile, const QString &xsdFile)
{
    QString err = XmlIO::validateXml(xmlFile, xsdFile);
    if (!err.isEmpty()) {
        return QStringLiteral("%1: %2").arg(xmlFile, err.trimmed());
    }
    return QString();
}

ConfigData XmlIO::loadAllConfig(const QString &dataDir, QStringList *validationErrors)
{
    QString schemaDir = QCoreApplication::applicationDirPath() + QStringLiteral("/schemas/");

    // Validate config files against their schemas
    struct { const char *xml; const char *xsd; } configFiles[] = {
        { "areaflags.xml",           "flags.xsd" },
        { "itemwearflags.xml",       "flags.xsd" },
        { "itemextraflags.xml",      "flags.xsd" },
        { "mobileactflags.xml",      "flags.xsd" },
        { "mobileaffectedflags.xml", "flags.xsd" },
        { "roomflags.xml",           "flags.xsd" },
        { "exitflags.xml",           "flags.xsd" },
        { "xflags.xml",              "flags.xsd" },
        { "resistflags.xml",         "flags.xsd" },
        { "attackflags.xml",         "flags.xsd" },
        { "defenseflags.xml",        "flags.xsd" },
        { "shopflags.xml",           "flags.xsd" },
        { "exits.xml",               "exits.xsd" },
        { "itemtypes.xml",           "itemtypes.xsd" },
        { "races.xml",               "names.xsd" },
        { "languages.xml",           "names.xsd" },
        { "planets.xml",             "names.xsd" },
        { "progtypes.xml",           "names.xsd" },
        { "mobilespecfunctions.xml", "names.xsd" },
        { "positions.xml",           "types.xsd" },
        { "repairtypes.xml",         "types.xsd" },
        { "roomsectortypes.xml",     "types.xsd" },
        { "resetsinfo.xml",          "resets.xsd" },
        { "highlighter.xml",         "highlighter.xsd" },
    };
    for (const auto &cf : configFiles) {
        QString err = validateConfig(dataDir + "/" + cf.xml, schemaDir + cf.xsd);
        if (!err.isEmpty() && validationErrors)
            validationErrors->append(err);
    }

    ConfigData c;
    c.areaFlags            = loadFlags(dataDir + "/areaflags.xml");
    c.itemWearFlags        = loadFlags(dataDir + "/itemwearflags.xml");
    c.itemExtraFlags       = loadFlags(dataDir + "/itemextraflags.xml");
    c.mobileActFlags       = loadFlags(dataDir + "/mobileactflags.xml");
    c.mobileAffectedFlags  = loadFlags(dataDir + "/mobileaffectedflags.xml");
    c.roomFlags            = loadFlags(dataDir + "/roomflags.xml");
    c.exitFlags            = loadFlags(dataDir + "/exitflags.xml");
    c.xFlags               = loadFlags(dataDir + "/xflags.xml");
    c.resistFlags          = loadFlags(dataDir + "/resistflags.xml");
    c.attackFlags          = loadFlags(dataDir + "/attackflags.xml");
    c.defenseFlags         = loadFlags(dataDir + "/defenseflags.xml");
    c.shopFlags            = loadFlags(dataDir + "/shopflags.xml");
    c.exits                = loadExits(dataDir + "/exits.xml");
    c.itemTypes            = loadItemTypes(dataDir + "/itemtypes.xml");
    c.races                = loadNames(dataDir + "/races.xml");
    c.languages            = loadNames(dataDir + "/languages.xml");
    c.planets              = loadNames(dataDir + "/planets.xml");
    c.progTypes            = loadNames(dataDir + "/progtypes.xml");
    c.mobileSpecFunctions  = loadNames(dataDir + "/mobilespecfunctions.xml");
    c.positions            = loadTypes(dataDir + "/positions.xml");
    c.repairTypes          = loadTypes(dataDir + "/repairtypes.xml");
    c.roomSectorTypes      = loadTypes(dataDir + "/roomsectortypes.xml");
    c.resetsInfo           = loadResetsInfo(dataDir + "/resetsinfo.xml");
    c.highlighter          = loadHighlighter(dataDir + "/highlighter.xml");
    return c;
}

// ---------------------------------------------------------------------------
// Area loading
// ---------------------------------------------------------------------------

static ShortDesc parseShortDesc(QXmlStreamReader &xml)
{
    ShortDesc s;
    while (!(xml.isEndElement() && xml.name() == u"short")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"inflect0")       s.inflect0 = readText(xml);
        else if (xml.name() == u"inflect1")  s.inflect1 = readText(xml);
        else if (xml.name() == u"inflect2")  s.inflect2 = readText(xml);
        else if (xml.name() == u"inflect3")  s.inflect3 = readText(xml);
        else if (xml.name() == u"inflect4")  s.inflect4 = readText(xml);
        else if (xml.name() == u"inflect5")  s.inflect5 = readText(xml);
    }
    return s;
}

static ExtraDesc parseExtraDesc(QXmlStreamReader &xml)
{
    ExtraDesc ed;
    while (!(xml.isEndElement() && xml.name() == u"extradesc")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"keyword")           ed.keyword = readText(xml);
        else if (xml.name() == u"description")  ed.description = readText(xml);
    }
    return ed;
}

static QList<ExtraDesc> parseExtraDescs(QXmlStreamReader &xml)
{
    QList<ExtraDesc> result;
    while (!(xml.isEndElement() && xml.name() == u"extradescs")) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"extradesc")
            result.append(parseExtraDesc(xml));
    }
    return result;
}

static Program parseProgram(QXmlStreamReader &xml)
{
    Program p;
    while (!(xml.isEndElement() && xml.name() == u"program")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"type")        p.type = readText(xml);
        else if (xml.name() == u"args")   p.args = readText(xml);
        else if (xml.name() == u"comlist") p.comlist = readText(xml);
    }
    return p;
}

static QList<Program> parsePrograms(QXmlStreamReader &xml)
{
    QList<Program> result;
    while (!(xml.isEndElement() && xml.name() == u"programs")) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"program")
            result.append(parseProgram(xml));
    }
    return result;
}

static Head parseHead(QXmlStreamReader &xml)
{
    Head h;
    while (!(xml.isEndElement() && xml.name() == u"head")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"name")          h.name = readText(xml);
        else if (n == u"authors")  h.authors = readText(xml);
        else if (n == u"builders") h.builders = readText(xml);
        else if (n == u"security") h.security = readInt(xml);
        else if (n == u"vnums") {
            while (!(xml.isEndElement() && xml.name() == u"vnums")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"lvnum")      h.vnums.lvnum = readInt(xml);
                else if (xml.name() == u"uvnum")  h.vnums.uvnum = readInt(xml);
            }
        }
        else if (n == u"flags")   h.flags = readLong(xml);
        else if (n == u"economy") {
            while (!(xml.isEndElement() && xml.name() == u"economy")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"low")       h.economy.low = readInt(xml);
                else if (xml.name() == u"high")  h.economy.high = readInt(xml);
            }
        }
        else if (n == u"reset") {
            while (!(xml.isEndElement() && xml.name() == u"reset")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"frequency")     h.reset.frequency = readInt(xml);
                else if (xml.name() == u"message")  h.reset.message = readText(xml);
            }
        }
        else if (n == u"ranges") {
            while (!(xml.isEndElement() && xml.name() == u"ranges")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"low")       h.ranges.low = readInt(xml);
                else if (xml.name() == u"high")  h.ranges.high = readInt(xml);
            }
        }
    }
    return h;
}

static SectionA parseSectionA(QXmlStreamReader &xml)
{
    SectionA s;
    while (!(xml.isEndElement() && xml.name() == u"sectiona")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"str")       s.str = readInt(xml);
        else if (xml.name() == u"int")  s.intel = readInt(xml);
        else if (xml.name() == u"wis")  s.wis = readInt(xml);
        else if (xml.name() == u"dex")  s.dex = readInt(xml);
        else if (xml.name() == u"con")  s.con = readInt(xml);
        else if (xml.name() == u"cha")  s.cha = readInt(xml);
        else if (xml.name() == u"lck")  s.lck = readInt(xml);
    }
    return s;
}

static SectionS parseSectionS(QXmlStreamReader &xml)
{
    SectionS s;
    while (!(xml.isEndElement() && xml.name() == u"sections")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"saving_poison_death")    s.saving_poison_death = readInt(xml);
        else if (xml.name() == u"saving_wand")        s.saving_wand = readInt(xml);
        else if (xml.name() == u"saving_para_petri")  s.saving_para_petri = readInt(xml);
        else if (xml.name() == u"saving_breath")      s.saving_breath = readInt(xml);
        else if (xml.name() == u"saving_spell_staff") s.saving_spell_staff = readInt(xml);
    }
    return s;
}

static SectionR parseSectionR(QXmlStreamReader &xml)
{
    SectionR s;
    while (!(xml.isEndElement() && xml.name() == u"sectionr")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"height")          s.height = readInt(xml);
        else if (xml.name() == u"weight")     s.weight = readInt(xml);
        else if (xml.name() == u"speaks")     s.speaks = readLong(xml);
        else if (xml.name() == u"speaking")   s.speaking = readText(xml);
        else if (xml.name() == u"numattacks") s.numattacks = readInt(xml);
    }
    return s;
}

static SectionX parseSectionX(QXmlStreamReader &xml)
{
    SectionX s;
    while (!(xml.isEndElement() && xml.name() == u"sectionx")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"hitroll")           s.hitroll = readInt(xml);
        else if (xml.name() == u"damroll")      s.damroll = readInt(xml);
        else if (xml.name() == u"xflags")       s.xflags = readLong(xml);
        else if (xml.name() == u"resistant")    s.resistant = readLong(xml);
        else if (xml.name() == u"immune")       s.immune = readLong(xml);
        else if (xml.name() == u"susceptible")  s.susceptible = readLong(xml);
        else if (xml.name() == u"attacks")      s.attacks = readLong(xml);
        else if (xml.name() == u"defenses")     s.defenses = readLong(xml);
    }
    return s;
}

static SectionT parseSectionT(QXmlStreamReader &xml)
{
    SectionT s;
    while (!(xml.isEndElement() && xml.name() == u"sectiont")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"thac0")           s.thac0 = readInt(xml);
        else if (xml.name() == u"ac")         s.ac = readInt(xml);
        else if (xml.name() == u"hitnodice")  s.hitnodice = readInt(xml);
        else if (xml.name() == u"hitsizedice") s.hitsizedice = readInt(xml);
        else if (xml.name() == u"hitplus")    s.hitplus = readInt(xml);
        else if (xml.name() == u"damnodice")  s.damnodice = readInt(xml);
        else if (xml.name() == u"damsizedice") s.damsizedice = readInt(xml);
        else if (xml.name() == u"damplus")    s.damplus = readInt(xml);
    }
    return s;
}

static SectionV parseSectionV(QXmlStreamReader &xml)
{
    SectionV s;
    while (!(xml.isEndElement() && xml.name() == u"sectionv")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"vipflags") s.vipflags = readText(xml);
    }
    return s;
}

static Mobile parseMobile(QXmlStreamReader &xml)
{
    Mobile m;
    while (!(xml.isEndElement() && xml.name() == u"mobile")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"vnum")             m.vnum = readInt(xml);
        else if (n == u"name")        m.name = readText(xml);
        else if (n == u"short")       m.shortDesc = parseShortDesc(xml);
        else if (n == u"long")        m.longDesc = readText(xml);
        else if (n == u"description") m.description = readText(xml);
        else if (n == u"race")        m.race = readText(xml);
        else if (n == u"level")       m.level = readInt(xml);
        else if (n == u"act")         m.act = readLong(xml);
        else if (n == u"affected")    m.affected = readLong(xml);
        else if (n == u"alignment")   m.alignment = readInt(xml);
        else if (n == u"sex")         m.sex = readInt(xml);
        else if (n == u"credits")     m.credits = readInt(xml);
        else if (n == u"position")    m.position = readInt(xml);
        else if (n == u"sectiona")    m.sectiona = parseSectionA(xml);
        else if (n == u"sections")    m.sections = parseSectionS(xml);
        else if (n == u"sectionr")    m.sectionr = parseSectionR(xml);
        else if (n == u"sectionx")    m.sectionx = parseSectionX(xml);
        else if (n == u"sectiont")    m.sectiont = parseSectionT(xml);
        else if (n == u"sectionv")    m.sectionv = parseSectionV(xml);
        else if (n == u"dialog")      m.dialog = readText(xml);
        else if (n == u"programs")    m.programs = parsePrograms(xml);
    }
    return m;
}

static ObjectValues parseObjectValues(QXmlStreamReader &xml)
{
    ObjectValues v;
    while (!(xml.isEndElement() && xml.name() == u"values")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"value0")       v.value0 = readInt(xml);
        else if (xml.name() == u"value1")  v.value1 = readInt(xml);
        else if (xml.name() == u"value2")  v.value2 = readInt(xml);
        else if (xml.name() == u"value3")  v.value3 = readText(xml);
        else if (xml.name() == u"value4")  v.value4 = readText(xml);
        else if (xml.name() == u"value5")  v.value5 = readText(xml);
    }
    return v;
}

static QList<Requirement> parseRequirements(QXmlStreamReader &xml)
{
    QList<Requirement> result;
    while (!(xml.isEndElement() && xml.name() == u"requirements")) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"requirement") {
            Requirement r;
            while (!(xml.isEndElement() && xml.name() == u"requirement")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"location")       r.location = readInt(xml);
                else if (xml.name() == u"modifier")  r.modifier = readInt(xml);
                else if (xml.name() == u"type")      r.type = readText(xml);
            }
            result.append(r);
        }
    }
    return result;
}

static QList<Affect> parseAffects(QXmlStreamReader &xml)
{
    QList<Affect> result;
    while (!(xml.isEndElement() && xml.name() == u"affects")) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == u"affect") {
            Affect a;
            while (!(xml.isEndElement() && xml.name() == u"affect")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"location")       a.location = readInt(xml);
                else if (xml.name() == u"modifier")  a.modifier = readInt(xml);
            }
            result.append(a);
        }
    }
    return result;
}

static AreaObject parseObject(QXmlStreamReader &xml)
{
    AreaObject o;
    while (!(xml.isEndElement() && xml.name() == u"object")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"vnum")              o.vnum = readInt(xml);
        else if (n == u"name")         o.name = readText(xml);
        else if (n == u"short")        o.shortDesc = parseShortDesc(xml);
        else if (n == u"description")  o.description = readText(xml);
        else if (n == u"actiondesc")   o.actiondesc = readText(xml);
        else if (n == u"type")         o.type = readInt(xml);
        else if (n == u"extraflags")   o.extraflags = readLong(xml);
        else if (n == u"wearflags")    o.wearflags = readLong(xml);
        else if (n == u"layers")       o.layers = readInt(xml);
        else if (n == u"values")       o.values = parseObjectValues(xml);
        else if (n == u"weight")       o.weight = readInt(xml);
        else if (n == u"cost")         o.cost = readInt(xml);
        else if (n == u"gender")       o.gender = readInt(xml);
        else if (n == u"level")        o.level = readInt(xml);
        else if (n == u"extradescs")   o.extradescs = parseExtraDescs(xml);
        else if (n == u"requirements") o.requirements = parseRequirements(xml);
        else if (n == u"affects")      o.affects = parseAffects(xml);
        else if (n == u"programs")     o.programs = parsePrograms(xml);
    }
    return o;
}

static Exit parseExit(QXmlStreamReader &xml)
{
    Exit e;
    while (!(xml.isEndElement() && xml.name() == u"exit")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"direction")       e.direction = readInt(xml);
        else if (xml.name() == u"description") e.description = readText(xml);
        else if (xml.name() == u"keyword")    e.keyword = readText(xml);
        else if (xml.name() == u"flags")      e.flags = readLong(xml);
        else if (xml.name() == u"key")        e.key = readInt(xml);
        else if (xml.name() == u"vnum")       e.vnum = readInt(xml);
        else if (xml.name() == u"distance")   e.distance = readInt(xml);
    }
    return e;
}

static Room parseRoom(QXmlStreamReader &xml)
{
    Room r;
    while (!(xml.isEndElement() && xml.name() == u"room")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"vnum")              r.vnum = readInt(xml);
        else if (n == u"name")         r.name = readText(xml);
        else if (n == u"description")  r.description = readText(xml);
        else if (n == u"nightdesc")    r.nightdesc = readText(xml);
        else if (n == u"light")        r.light = readInt(xml);
        else if (n == u"flags")        r.flags = readLong(xml);
        else if (n == u"sector")       r.sector = readInt(xml);
        else if (n == u"teledelay")    r.teledelay = readInt(xml);
        else if (n == u"televnum")     r.televnum = readInt(xml);
        else if (n == u"tunnel")       r.tunnel = readInt(xml);
        else if (n == u"exits") {
            while (!(xml.isEndElement() && xml.name() == u"exits")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"exit")
                    r.exits.append(parseExit(xml));
            }
        }
        else if (n == u"extradescs")   r.extradescs = parseExtraDescs(xml);
        else if (n == u"programs")     r.programs = parsePrograms(xml);
    }
    return r;
}

static AreaReset parseAreaReset(QXmlStreamReader &xml)
{
    AreaReset r;
    while (!(xml.isEndElement() && xml.name() == u"reset")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"command")      r.command = readText(xml);
        else if (xml.name() == u"extra")   r.extra = readInt(xml);
        else if (xml.name() == u"arg1")    r.arg1 = readInt(xml);
        else if (xml.name() == u"arg2")    r.arg2 = readInt(xml);
        else if (xml.name() == u"arg3")    r.arg3 = readInt(xml);
        else if (xml.name() == u"arg4")    r.arg4 = readInt(xml);
    }
    return r;
}

static Shop parseShop(QXmlStreamReader &xml)
{
    Shop s;
    while (!(xml.isEndElement() && xml.name() == u"shop")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"keeper")         s.keeper = readInt(xml);
        else if (n == u"types") {
            while (!(xml.isEndElement() && xml.name() == u"types")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"type0")       s.types.type0 = readInt(xml);
                else if (xml.name() == u"type1")  s.types.type1 = readInt(xml);
                else if (xml.name() == u"type2")  s.types.type2 = readInt(xml);
                else if (xml.name() == u"type3")  s.types.type3 = readInt(xml);
                else if (xml.name() == u"type4")  s.types.type4 = readInt(xml);
            }
        }
        else if (n == u"profitbuy")  s.profitbuy = readInt(xml);
        else if (n == u"profitsell") s.profitsell = readInt(xml);
        else if (n == u"open")       s.open = readInt(xml);
        else if (n == u"close")      s.close = readInt(xml);
        else if (n == u"flags")      s.flags = readLong(xml);
    }
    return s;
}

static Repair parseRepair(QXmlStreamReader &xml)
{
    Repair r;
    while (!(xml.isEndElement() && xml.name() == u"repair")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"keeper")          r.keeper = readInt(xml);
        else if (n == u"types") {
            while (!(xml.isEndElement() && xml.name() == u"types")) {
                xml.readNext();
                if (!xml.isStartElement()) continue;
                if (xml.name() == u"type0")       r.types.type0 = readInt(xml);
                else if (xml.name() == u"type1")  r.types.type1 = readInt(xml);
                else if (xml.name() == u"type2")  r.types.type2 = readInt(xml);
            }
        }
        else if (n == u"profitfix")  r.profitfix = readInt(xml);
        else if (n == u"shoptype")   r.shoptype = readInt(xml);
        else if (n == u"open")       r.open = readInt(xml);
        else if (n == u"close")      r.close = readInt(xml);
    }
    return r;
}

static Special parseSpecial(QXmlStreamReader &xml)
{
    Special s;
    while (!(xml.isEndElement() && xml.name() == u"special")) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        if (xml.name() == u"vnum")           s.vnum = readInt(xml);
        else if (xml.name() == u"function")  s.function = readText(xml);
        else if (xml.name() == u"function2") s.function2 = readText(xml);
    }
    return s;
}

Area XmlIO::loadArea(const QString &filePath)
{
    Area area;
    QFile file;
    QXmlStreamReader xml;
    if (!openXml(filePath, file, xml))
        return area;

    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) continue;
        auto n = xml.name();
        if (n == u"head")          area.head = parseHead(xml);
        else if (n == u"mobiles") {
            while (!(xml.isEndElement() && xml.name() == u"mobiles")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"mobile")
                    area.mobiles.append(parseMobile(xml));
            }
        }
        else if (n == u"objects") {
            while (!(xml.isEndElement() && xml.name() == u"objects")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"object")
                    area.objects.append(parseObject(xml));
            }
        }
        else if (n == u"rooms") {
            while (!(xml.isEndElement() && xml.name() == u"rooms")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"room")
                    area.rooms.append(parseRoom(xml));
            }
        }
        else if (n == u"resets") {
            while (!(xml.isEndElement() && xml.name() == u"resets")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"reset")
                    area.resets.append(parseAreaReset(xml));
            }
        }
        else if (n == u"shops") {
            while (!(xml.isEndElement() && xml.name() == u"shops")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"shop")
                    area.shops.append(parseShop(xml));
            }
        }
        else if (n == u"repairs") {
            while (!(xml.isEndElement() && xml.name() == u"repairs")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"repair")
                    area.repairs.append(parseRepair(xml));
            }
        }
        else if (n == u"specials") {
            while (!(xml.isEndElement() && xml.name() == u"specials")) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == u"special")
                    area.specials.append(parseSpecial(xml));
            }
        }
    }
    return area;
}

// ---------------------------------------------------------------------------
// Area saving
// ---------------------------------------------------------------------------

static void writeShortDesc(QXmlStreamWriter &xml, const ShortDesc &s)
{
    xml.writeStartElement("short");
    xml.writeTextElement("inflect0", s.inflect0);
    xml.writeTextElement("inflect1", s.inflect1);
    xml.writeTextElement("inflect2", s.inflect2);
    xml.writeTextElement("inflect3", s.inflect3);
    xml.writeTextElement("inflect4", s.inflect4);
    xml.writeTextElement("inflect5", s.inflect5);
    xml.writeEndElement();
}

static void writeExtraDescs(QXmlStreamWriter &xml, const QList<ExtraDesc> &eds)
{
    xml.writeStartElement("extradescs");
    for (const auto &ed : eds) {
        xml.writeStartElement("extradesc");
        xml.writeTextElement("keyword", ed.keyword);
        xml.writeTextElement("description", ed.description);
        xml.writeEndElement();
    }
    xml.writeEndElement();
}

static void writePrograms(QXmlStreamWriter &xml, const QList<Program> &progs)
{
    xml.writeStartElement("programs");
    for (const auto &p : progs) {
        xml.writeStartElement("program");
        xml.writeTextElement("type", p.type);
        xml.writeTextElement("args", p.args);
        xml.writeTextElement("comlist", p.comlist);
        xml.writeEndElement();
    }
    xml.writeEndElement();
}

static void writeNum(QXmlStreamWriter &xml, const QString &tag, int val)
{
    xml.writeTextElement(tag, QString::number(val));
}

static void writeLong(QXmlStreamWriter &xml, const QString &tag, qint64 val)
{
    xml.writeTextElement(tag, QString::number(val));
}

bool XmlIO::saveArea(const Area &area, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    // Write the XML declaration manually with ISO-8859-2 encoding to match the
    // original Java output
    file.write("<?xml version=\"1.0\" encoding=\"ISO-8859-2\"?>\n");

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(1);
    // Do not call writeStartDocument() — we wrote the declaration above
    xml.writeStartElement("area");
    xml.writeDefaultNamespace("http://swmud.pl/ns/swmud/1.0/area");

    // Head
    {
        const Head &h = area.head;
        xml.writeStartElement("head");
        xml.writeTextElement("name", h.name);
        xml.writeTextElement("authors", h.authors);
        xml.writeTextElement("builders", h.builders);
        writeNum(xml, "security", h.security);
        xml.writeStartElement("vnums");
        writeNum(xml, "lvnum", h.vnums.lvnum);
        writeNum(xml, "uvnum", h.vnums.uvnum);
        xml.writeEndElement();
        writeLong(xml, "flags", h.flags);
        xml.writeStartElement("economy");
        writeNum(xml, "low", h.economy.low);
        writeNum(xml, "high", h.economy.high);
        xml.writeEndElement();
        xml.writeStartElement("reset");
        writeNum(xml, "frequency", h.reset.frequency);
        xml.writeTextElement("message", h.reset.message);
        xml.writeEndElement();
        xml.writeStartElement("ranges");
        writeNum(xml, "low", h.ranges.low);
        writeNum(xml, "high", h.ranges.high);
        xml.writeEndElement();
        xml.writeEndElement(); // head
    }

    // Mobiles
    xml.writeStartElement("mobiles");
    for (const auto &m : area.mobiles) {
        xml.writeStartElement("mobile");
        writeNum(xml, "vnum", m.vnum);
        xml.writeTextElement("name", m.name);
        writeShortDesc(xml, m.shortDesc);
        xml.writeTextElement("long", m.longDesc);
        xml.writeTextElement("description", m.description);
        xml.writeTextElement("race", m.race);
        writeNum(xml, "level", m.level);
        writeLong(xml, "act", m.act);
        writeLong(xml, "affected", m.affected);
        writeNum(xml, "alignment", m.alignment);
        writeNum(xml, "sex", m.sex);
        writeNum(xml, "credits", m.credits);
        writeNum(xml, "position", m.position);
        // sectiona
        xml.writeStartElement("sectiona");
        writeNum(xml, "str", m.sectiona.str);
        writeNum(xml, "int", m.sectiona.intel);
        writeNum(xml, "wis", m.sectiona.wis);
        writeNum(xml, "dex", m.sectiona.dex);
        writeNum(xml, "con", m.sectiona.con);
        writeNum(xml, "cha", m.sectiona.cha);
        writeNum(xml, "lck", m.sectiona.lck);
        xml.writeEndElement();
        // sections
        xml.writeStartElement("sections");
        writeNum(xml, "saving_poison_death", m.sections.saving_poison_death);
        writeNum(xml, "saving_wand", m.sections.saving_wand);
        writeNum(xml, "saving_para_petri", m.sections.saving_para_petri);
        writeNum(xml, "saving_breath", m.sections.saving_breath);
        writeNum(xml, "saving_spell_staff", m.sections.saving_spell_staff);
        xml.writeEndElement();
        // sectionr
        xml.writeStartElement("sectionr");
        writeNum(xml, "height", m.sectionr.height);
        writeNum(xml, "weight", m.sectionr.weight);
        writeLong(xml, "speaks", m.sectionr.speaks);
        xml.writeTextElement("speaking", m.sectionr.speaking);
        writeNum(xml, "numattacks", m.sectionr.numattacks);
        xml.writeEndElement();
        // sectionx
        xml.writeStartElement("sectionx");
        writeNum(xml, "hitroll", m.sectionx.hitroll);
        writeNum(xml, "damroll", m.sectionx.damroll);
        writeLong(xml, "xflags", m.sectionx.xflags);
        writeLong(xml, "resistant", m.sectionx.resistant);
        writeLong(xml, "immune", m.sectionx.immune);
        writeLong(xml, "susceptible", m.sectionx.susceptible);
        writeLong(xml, "attacks", m.sectionx.attacks);
        writeLong(xml, "defenses", m.sectionx.defenses);
        xml.writeEndElement();
        // sectiont
        xml.writeStartElement("sectiont");
        writeNum(xml, "thac0", m.sectiont.thac0);
        writeNum(xml, "ac", m.sectiont.ac);
        writeNum(xml, "hitnodice", m.sectiont.hitnodice);
        writeNum(xml, "hitsizedice", m.sectiont.hitsizedice);
        writeNum(xml, "hitplus", m.sectiont.hitplus);
        writeNum(xml, "damnodice", m.sectiont.damnodice);
        writeNum(xml, "damsizedice", m.sectiont.damsizedice);
        writeNum(xml, "damplus", m.sectiont.damplus);
        xml.writeEndElement();
        // sectionv
        xml.writeStartElement("sectionv");
        xml.writeTextElement("vipflags", m.sectionv.vipflags);
        xml.writeEndElement();
        xml.writeTextElement("dialog", m.dialog);
        writePrograms(xml, m.programs);
        xml.writeEndElement(); // mobile
    }
    xml.writeEndElement(); // mobiles

    // Objects
    xml.writeStartElement("objects");
    for (const auto &o : area.objects) {
        xml.writeStartElement("object");
        writeNum(xml, "vnum", o.vnum);
        xml.writeTextElement("name", o.name);
        writeShortDesc(xml, o.shortDesc);
        xml.writeTextElement("description", o.description);
        xml.writeTextElement("actiondesc", o.actiondesc);
        writeNum(xml, "type", o.type);
        writeLong(xml, "extraflags", o.extraflags);
        writeLong(xml, "wearflags", o.wearflags);
        writeNum(xml, "layers", o.layers);
        xml.writeStartElement("values");
        writeNum(xml, "value0", o.values.value0);
        writeNum(xml, "value1", o.values.value1);
        writeNum(xml, "value2", o.values.value2);
        xml.writeTextElement("value3", o.values.value3);
        xml.writeTextElement("value4", o.values.value4);
        xml.writeTextElement("value5", o.values.value5);
        xml.writeEndElement();
        writeNum(xml, "weight", o.weight);
        writeNum(xml, "cost", o.cost);
        writeNum(xml, "gender", o.gender);
        writeNum(xml, "level", o.level);
        writeExtraDescs(xml, o.extradescs);
        xml.writeStartElement("requirements");
        for (const auto &r : o.requirements) {
            xml.writeStartElement("requirement");
            writeNum(xml, "location", r.location);
            writeNum(xml, "modifier", r.modifier);
            xml.writeTextElement("type", r.type);
            xml.writeEndElement();
        }
        xml.writeEndElement();
        xml.writeStartElement("affects");
        for (const auto &a : o.affects) {
            xml.writeStartElement("affect");
            writeNum(xml, "location", a.location);
            writeNum(xml, "modifier", a.modifier);
            xml.writeEndElement();
        }
        xml.writeEndElement();
        writePrograms(xml, o.programs);
        xml.writeEndElement(); // object
    }
    xml.writeEndElement(); // objects

    // Rooms
    xml.writeStartElement("rooms");
    for (const auto &r : area.rooms) {
        xml.writeStartElement("room");
        writeNum(xml, "vnum", r.vnum);
        xml.writeTextElement("name", r.name);
        xml.writeTextElement("description", r.description);
        if (!r.nightdesc.isEmpty())
            xml.writeTextElement("nightdesc", r.nightdesc);
        writeNum(xml, "light", r.light);
        writeLong(xml, "flags", r.flags);
        writeNum(xml, "sector", r.sector);
        writeNum(xml, "teledelay", r.teledelay);
        writeNum(xml, "televnum", r.televnum);
        writeNum(xml, "tunnel", r.tunnel);
        xml.writeStartElement("exits");
        for (const auto &e : r.exits) {
            xml.writeStartElement("exit");
            writeNum(xml, "direction", e.direction);
            xml.writeTextElement("description", e.description);
            xml.writeTextElement("keyword", e.keyword);
            writeLong(xml, "flags", e.flags);
            writeNum(xml, "key", e.key);
            writeNum(xml, "vnum", e.vnum);
            writeNum(xml, "distance", e.distance);
            xml.writeEndElement();
        }
        xml.writeEndElement(); // exits
        writeExtraDescs(xml, r.extradescs);
        writePrograms(xml, r.programs);
        xml.writeEndElement(); // room
    }
    xml.writeEndElement(); // rooms

    // Resets
    xml.writeStartElement("resets");
    for (const auto &r : area.resets) {
        xml.writeStartElement("reset");
        xml.writeTextElement("command", r.command);
        writeNum(xml, "extra", r.extra);
        writeNum(xml, "arg1", r.arg1);
        writeNum(xml, "arg2", r.arg2);
        writeNum(xml, "arg3", r.arg3);
        writeNum(xml, "arg4", r.arg4);
        xml.writeEndElement();
    }
    xml.writeEndElement(); // resets

    // Shops
    xml.writeStartElement("shops");
    for (const auto &s : area.shops) {
        xml.writeStartElement("shop");
        writeNum(xml, "keeper", s.keeper);
        xml.writeStartElement("types");
        writeNum(xml, "type0", s.types.type0);
        writeNum(xml, "type1", s.types.type1);
        writeNum(xml, "type2", s.types.type2);
        writeNum(xml, "type3", s.types.type3);
        writeNum(xml, "type4", s.types.type4);
        xml.writeEndElement();
        writeNum(xml, "profitbuy", s.profitbuy);
        writeNum(xml, "profitsell", s.profitsell);
        writeNum(xml, "open", s.open);
        writeNum(xml, "close", s.close);
        writeLong(xml, "flags", s.flags);
        xml.writeEndElement();
    }
    xml.writeEndElement(); // shops

    // Repairs
    xml.writeStartElement("repairs");
    for (const auto &r : area.repairs) {
        xml.writeStartElement("repair");
        writeNum(xml, "keeper", r.keeper);
        xml.writeStartElement("types");
        writeNum(xml, "type0", r.types.type0);
        writeNum(xml, "type1", r.types.type1);
        writeNum(xml, "type2", r.types.type2);
        xml.writeEndElement();
        writeNum(xml, "profitfix", r.profitfix);
        writeNum(xml, "shoptype", r.shoptype);
        writeNum(xml, "open", r.open);
        writeNum(xml, "close", r.close);
        xml.writeEndElement();
    }
    xml.writeEndElement(); // repairs

    // Specials
    xml.writeStartElement("specials");
    for (const auto &s : area.specials) {
        xml.writeStartElement("special");
        writeNum(xml, "vnum", s.vnum);
        xml.writeTextElement("function", s.function);
        xml.writeTextElement("function2", s.function2);
        xml.writeEndElement();
    }
    xml.writeEndElement(); // specials

    xml.writeEndElement(); // area
    xml.writeEndDocument();
    return true;
}
