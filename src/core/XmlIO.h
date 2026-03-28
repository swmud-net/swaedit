#ifndef XMLIO_H
#define XMLIO_H

#include <QString>
#include <QStringList>
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

// XSD validation — returns empty string on success, error message on failure
QString validateXml(const QString &xmlFilePath, const QString &xsdFilePath);

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

// Load all config data from the data/ directory.
// If validationErrors is non-null, XSD validation errors are appended to it.
ConfigData loadAllConfig(const QString &dataDir, QStringList *validationErrors = nullptr);

} // namespace XmlIO

#endif // XMLIO_H
