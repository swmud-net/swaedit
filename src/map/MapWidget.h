#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMap>
#include <QList>
#include <QPoint>
#include <QTimer>
#include <QString>
#include <QStringList>

class MapRoom;
class ExitWrapper;

class MapWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    MapWidget(const QMap<int, QList<MapRoom *>> &islandRooms,
              int currentIsland, int maxIslands,
              QWidget *parent = nullptr);
    ~MapWidget() override;

    void refreshMap(const QMap<int, QList<MapRoom *>> &islandRooms, int maxIslands);
    void showRoom(int vnum);
    void showExit(int ownerRoomVnum, ExitWrapper *ex);
    void setRoomMarkingFlag(qint64 flag) { roomMarkingFlag_ = flag; }

    // Take ownership of map data (MapRoom/ExitWrapper objects) so they
    // survive after the Mapper goes out of scope.
    void takeOwnership(const QList<MapRoom *> &rooms, const QList<ExitWrapper *> &exits);

signals:
    void vnumSelected(int vnum);
    void exitSelected(int ownerRoomVnum, int exitDirection, int destRoomVnum);
    void windowClosed();
    void mapRefreshed();
    void roomMarkingFlagRequested();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void closeEvent(QCloseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    // --- Constants ---
    static constexpr int FPS = 30;
    static constexpr float ALPHA = 0.5f;
    static constexpr float INITIAL_Z = -31.0f;

    static const unsigned char selectionColor[4];
    static const unsigned char selection1Color[4];
    static const float objectColor[4];
    static const float object1Color[4];
    static const unsigned char flagColor[4];
    static const unsigned char flag1Color[4];

    static constexpr float fov = 60.0f;
    static constexpr float nearPlane = 1.0f;
    static constexpr float farPlane = 500.0f;

    static constexpr int BPP = 4;

    // --- GL helpers ---
    void genLists();
    void drawWorld();
    void drawIslands();
    void drawExits(MapRoom *mr);
    void drawWindRose();
    void drawCylinder(float baseRadius, float topRadius, float height, int slices);
    void drawCone(float baseRadius, float height, int slices);
    void drawDisk(float innerRadius, float outerRadius, int slices);
    void cleanDrawnMark();

    // --- Selection (color-based picking) ---
    void performSelection(int cx, int cy);
    void drawSceneForPicking();
    void setPickingColor(int id);
    int decodePickingColor(unsigned char r, unsigned char g, unsigned char b);

    // --- Text rendering ---
    void drawTextOverlay();

    // --- View helpers ---
    void setupIsland(int islandNo);
    void center();
    int normalizeAngle(int angle);
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void incLayer();
    void decLayer();

    // --- Screenshots ---
    void screenShot();
    void transparentScreenShot();
    void setScreenShotParent();
    QString getScreenShotPath();
    void takeScreenshot();
    int getLayerNum() const { return currentLayer_ + 1; }
    int getMaxLayerNum() const { return maxLayer_ + 1; }
    int getIslandNum() const { return currentIsland_ + 1; }
    int getMaxIslandNum() const { return maxIslands_; }

    // --- Island search ---
    struct IslandRoom {
        int island;
        int roomIndex;
    };
    IslandRoom findIsland(int vnum);

    void showIslandsLayers();

    // --- Member variables ---
    QTimer *animTimer_ = nullptr;

    unsigned int glList_ = 0;
    bool glListsGenerated_ = false;

    QMap<int, QList<MapRoom *>> islandRooms_;
    int currentIsland_ = 0;
    int currentIslandSize_ = 0;
    int maxIslands_ = 0;

    float xMiddle_ = 0.0f;
    float yMiddle_ = 0.0f;
    float zMiddle_ = 0.0f;

    int currentLayer_ = 0;
    int maxLayer_ = 0;

    float dz_ = INITIAL_Z;
    float dx_ = 0.0f;
    float dy_ = 0.0f;
    float xRot_ = 0.0f;
    float yRot_ = 0.0f;
    float zRot_ = 0.0f;
    float rot_ = 0.0f;
    float angle_ = 0.0f;

    bool animate_ = true;
    bool multisample_ = true;
    bool drawDistantExits_ = false;
    bool drawCross_ = false;
    bool showHelp_ = false;
    int showIslandsLayers_ = 0;
    qint64 roomMarkingFlag_ = 0;

    int selected_ = -1;
    int selectedVnum_ = -1;
    ExitWrapper *selectedExit_ = nullptr;
    bool selection_ = false;
    bool reportSelected_ = false;
    int cx_ = 0;
    int cy_ = 0;
    int exitShift_ = 0;

    int w_ = 800;
    int h_ = 600;
    float aspect_ = 1.333f;

    QPoint lastPos_;
    bool zRotAxis_ = false;

    bool readPixels_ = false;
    bool transparentShot_ = false;
    QString screenShotPath_;
    QString screenShotDir_;
    QString screenShotBareFileName_;
    long screenShotCnt_ = 1;

    static const QStringList MENU_KEYS;

    // Owned map data (transferred from Mapper)
    QList<MapRoom *> ownedMapRooms_;
    QList<ExitWrapper *> ownedExitWrappers_;
    void cleanupOwnedData();
};

#endif // MAPWIDGET_H
