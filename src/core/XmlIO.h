#ifndef XMLIO_H
#define XMLIO_H

#include <QString>
#include <QList>

struct Area;
struct FlagDef;
struct ExitDef;
struct ItemTypeDef;
struct TypeDef;
struct ResetInfoDef;
struct HighlighterWordsDef;
struct ConfigData;

class QXmlStreamReader;
class QXmlStreamWriter;

namespace XmlIO {

// Area file I/O
Area loadArea(const QString &filePath);
bool saveArea(const Area &area, const QString &filePath);

// Config file loaders
QList<FlagDef> loadFlags(const QString &filePath);
QList<ExitDef> loadExits(const QString &filePath);
QList<ItemTypeDef> loadItemTypes(const QString &filePath);
QList<QString> loadNames(const QString &filePath);
QList<TypeDef> loadTypes(const QString &filePath);
QList<ResetInfoDef> loadResetsInfo(const QString &filePath);
QList<HighlighterWordsDef> loadHighlighter(const QString &filePath);

// Load all config data from the data/ directory
ConfigData loadAllConfig(const QString &dataDir);

} // namespace XmlIO

#endif // XMLIO_H
