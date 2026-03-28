#include "map/MapWidget.h"
#include "map/MapRoom.h"
#include "map/ExitWrapper.h"
#include "map/RoomCoords.h"
#include "map/RoomSpread.h"
#include "model/Area.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QSurfaceFormat>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QStandardPaths>
#include <QOpenGLFunctions>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Static constants ---

const unsigned char MapWidget::selectionColor[4]  = { 218, 168, 16, 127 };
const unsigned char MapWidget::selection1Color[4]  = { 249, 188, 11, 127 };
const float MapWidget::objectColor[4]  = { 0.0f, 0.7f, 0.0f, ALPHA };
const float MapWidget::object1Color[4]  = { 0.0f, 1.0f, 0.0f, ALPHA };
const unsigned char MapWidget::flagColor[4]  = { 0, 224, 255, 127 };
const unsigned char MapWidget::flag1Color[4]  = { 0, 255, 255, 127 };

const QStringList MapWidget::MENU_KEYS = {
    "d      - toggle distant exits",
    "m      - toggle multisampling",
    "space  - toggle rotation",
    "h      - toggle this help text",
    "k      - toggle cross",
    "c      - reset to center",
    "f      - toggle fullscreen",
    "r      - refresh map",
    "i      - select room marking flag",
    "right  - next island",
    "left   - previous island",
    "up     - next layer",
    "down   - previous layer",
    "F12    - take a screenshot",
    "F11    - take a transparent screenshot",
    "lMouse - drag to move, click to select room/exit",
    "rMouse - drag to change rotation angle",
    "shift  - hold to switch Y to Z axis"
};

// ===================================================================
// Constructor / Destructor
// ===================================================================

MapWidget::MapWidget(const QMap<int, QList<MapRoom *>> &islandRooms,
                     int currentIsland, int maxIslands,
                     QWidget *parent)
    : QOpenGLWidget(parent)
    , currentIsland_(currentIsland)
{
    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setSamples(4);
    fmt.setAlphaBufferSize(8);
    fmt.setDepthBufferSize(24);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    setFormat(fmt);

    resize(800, 600);
    setFocusPolicy(Qt::StrongFocus);

    animTimer_ = new QTimer(this);
    animTimer_->setInterval(1000 / FPS);
    connect(animTimer_, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
    animTimer_->start();

    refreshMap(islandRooms, maxIslands);
}

MapWidget::~MapWidget()
{
    makeCurrent();
    if (glListsGenerated_) {
        glDeleteLists(glList_, 5);
        glListsGenerated_ = false;
    }
    doneCurrent();
    cleanupOwnedData();
}

void MapWidget::takeOwnership(const QList<MapRoom *> &rooms, const QList<ExitWrapper *> &exits)
{
    cleanupOwnedData();
    ownedMapRooms_ = rooms;
    ownedExitWrappers_ = exits;
}

void MapWidget::cleanupOwnedData()
{
    qDeleteAll(ownedExitWrappers_);
    ownedExitWrappers_.clear();
    qDeleteAll(ownedMapRooms_);
    ownedMapRooms_.clear();
}

// ===================================================================
// Public methods
// ===================================================================

void MapWidget::refreshMap(const QMap<int, QList<MapRoom *>> &islandRooms, int maxIslands)
{
    islandRooms_ = islandRooms;
    maxIslands_ = maxIslands;
    if (currentIsland_ >= maxIslands_)
        currentIsland_ = 0;
    setupIsland(currentIsland_);
    setScreenShotParent();

    if (isVisible()) {
        makeCurrent();
        genLists();
        doneCurrent();
        update();
    }
}

void MapWidget::showRoom(int vnum)
{
    IslandRoom ir = findIsland(vnum);
    if (ir.island >= 0) {
        if (currentIsland_ != ir.island) {
            setupIsland(ir.island);
        }
        selected_ = ir.roomIndex;
        selectedVnum_ = vnum;
    }
}

void MapWidget::showExit(int ownerRoomVnum, ExitWrapper *ex)
{
    IslandRoom ir = findIsland(ownerRoomVnum);
    if (ir.island < 0)
        return;

    if (currentIsland_ != ir.island) {
        setupIsland(ir.island);
    }

    int exitNo = 0;
    for (MapRoom *mr : islandRooms_.value(currentIsland_)) {
        QMapIterator<ExitWrapper *, MapRoom *> eit(mr->mapRooms());
        while (eit.hasNext()) {
            eit.next();
            ExitWrapper *exit = eit.key();
            bool exitFound = false;

            if (mr->room()->vnum == ownerRoomVnum
                && exit->vnum() == ex->vnum()
                && exit->direction() == ex->direction()) {
                selectedVnum_ = ownerRoomVnum;
                exitFound = true;
            } else if (exit->isTwoWay()
                       && exit->vnum() == ownerRoomVnum
                       && exit->direction() == ex->getRevDirection()) {
                selectedVnum_ = mr->room()->vnum;
                exitFound = true;
            }

            if (exitFound) {
                selectedExit_ = exit;
                selected_ = currentIslandSize_ + exitNo;
                if (exit->isDistant() && !drawDistantExits_) {
                    drawDistantExits_ = true;
                }
                return;
            }
            exitNo++;
        }
    }
}

// ===================================================================
// OpenGL initialization
// ===================================================================

void MapWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    genLists();
}

void MapWidget::resizeGL(int w, int h)
{
    w_ = w;
    if (h == 0) h = 1;
    h_ = h;
    aspect_ = static_cast<float>(w) / static_cast<float>(h);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // gluPerspective equivalent
    float top = nearPlane * std::tan(fov * 0.5f * static_cast<float>(M_PI) / 180.0f);
    float bottom = -top;
    float right = top * aspect_;
    float left = -right;
    glFrustum(left, right, bottom, top, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
}

// ===================================================================
// Paint
// ===================================================================

void MapWidget::paintGL()
{
    if (multisample_) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    if (selection_) {
        performSelection(cx_, cy_);
        selection_ = false;
    }

    drawWorld();

    // Text overlay using QPainter (replaces GLFont)
    drawTextOverlay();

    if (readPixels_) {
        takeScreenshot();
    }

    cleanDrawnMark();
}

// ===================================================================
// Display list generation
// ===================================================================

void MapWidget::genLists()
{
    if (glListsGenerated_) {
        glDeleteLists(glList_, 5);
    }

    glList_ = glGenLists(5);
    glListsGenerated_ = true;

    // --- List 0: room cube (unit cube 0-1) ---
    glNewList(glList_, GL_COMPILE);
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glVertex3f(1, 1, 0);
    glVertex3f(1, 0, 0);
    // Back face
    glVertex3f(0, 0, 1);
    glVertex3f(0, 1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 0, 1);
    // Left face
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glVertex3f(0, 1, 1);
    glVertex3f(0, 0, 1);
    // Right face
    glVertex3f(1, 0, 0);
    glVertex3f(1, 1, 0);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 0, 1);
    // Bottom face
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(1, 0, 1);
    glVertex3f(0, 0, 1);
    // Top face
    glVertex3f(0, 1, 0);
    glVertex3f(1, 1, 0);
    glVertex3f(1, 1, 1);
    glVertex3f(0, 1, 1);
    glEnd();
    glEndList();

    // --- List 1: two-way exit cylinder (radius 0.2, length 1, 32 segments) ---
    glNewList(glList_ + 1, GL_COMPILE);
    drawCylinder(0.2f, 0.2f, 1.0f, 32);
    glEndList();

    // --- List 2: one-way exit cylinder (radius 0.1) ---
    glNewList(glList_ + 2, GL_COMPILE);
    drawCylinder(0.1f, 0.1f, 1.0f, 32);
    glEndList();

    // --- List 3: wind rose (3 colored axis lines + arrow cones) ---
    glNewList(glList_ + 3, GL_COMPILE);
    glBegin(GL_LINES);
    // X axis - red
    glColor4f(1, 0, 0, 0.9f);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    // Y axis - yellow
    glColor4f(1, 1, 0, 0.9f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    // Z axis - blue
    glColor4f(0, 0, 1, 0.9f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();

    // X arrow cone
    glPushMatrix();
    glTranslatef(1, 0, 0);
    glRotatef(90, 0, 1, 0);
    glColor4f(1, 0, 0, 1);
    drawCone(0.1f, 0.3f, 32);
    drawDisk(0, 0.1f, 32);
    glPopMatrix();

    // Y arrow cone
    glPushMatrix();
    glTranslatef(0, 1, 0);
    glRotatef(90, -1, 0, 0);
    glColor4f(1, 1, 0, 1);
    drawCone(0.1f, 0.3f, 32);
    drawDisk(0, 0.1f, 32);
    glPopMatrix();

    // Z arrow cone
    glPushMatrix();
    glTranslatef(0, 0, 1);
    glColor4f(0, 0, 1, 1);
    drawCone(0.1f, 0.3f, 32);
    drawDisk(0, 0.1f, 32);
    glPopMatrix();
    glEndList();

    // --- List 4: white cross lines ---
    glNewList(glList_ + 4, GL_COMPILE);
    glPushMatrix();
    glTranslatef(0, 0, -1);
    glBegin(GL_LINES);
    glColor3f(1, 1, 1);
    glVertex3f(-1, 0, 0);
    glVertex3f(1, 0, 0);
    glVertex3f(0, -1, 0);
    glVertex3f(0, 1, 0);
    glEnd();
    glPopMatrix();
    glEndList();
}

// ===================================================================
// GLU replacement: cylinder, cone, disk
// ===================================================================

void MapWidget::drawCylinder(float baseRadius, float topRadius, float height, int slices)
{
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
        float cs = std::cos(theta);
        float sn = std::sin(theta);

        // Bottom vertex
        glVertex3f(baseRadius * cs, baseRadius * sn, 0.0f);
        // Top vertex
        glVertex3f(topRadius * cs, topRadius * sn, height);
    }
    glEnd();
}

void MapWidget::drawCone(float baseRadius, float height, int slices)
{
    // Apex at z = height, base at z = 0
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, height);
    for (int i = 0; i <= slices; ++i) {
        float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
        glVertex3f(baseRadius * std::cos(theta), baseRadius * std::sin(theta), 0.0f);
    }
    glEnd();
}

void MapWidget::drawDisk(float innerRadius, float outerRadius, int slices)
{
    Q_UNUSED(innerRadius);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= slices; ++i) {
        float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(slices);
        glVertex3f(outerRadius * std::cos(theta), outerRadius * std::sin(theta), 0.0f);
    }
    glEnd();
}

// ===================================================================
// Scene drawing
// ===================================================================

void MapWidget::drawWorld()
{
    if (animate_) {
        rot_ += 0.002f;
        if (rot_ > static_cast<float>(M_PI) / 2.0f) {
            rot_ = 0.0f;
        }
        angle_ = 360.0f * std::abs(std::sin(rot_));
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawIslands();
    drawWindRose();

    if (drawCross_) {
        glCallList(glList_ + 4);
    }
}

void MapWidget::drawIslands()
{
    if (!islandRooms_.contains(currentIsland_))
        return;

    glPushMatrix();
    const QList<MapRoom *> &island = islandRooms_.value(currentIsland_);
    if (island.isEmpty()) {
        glPopMatrix();
        return;
    }

    glTranslatef(dx_, dy_, dz_);
    glRotatef(xRot_ / 16.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot_ / 16.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot_ / 16.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(angle_, 1.0f, 1.0f, 1.0f);

    const RoomCoords &coords = island.at(0)->coords();
    glTranslatef(-0.5f + xMiddle_ - coords.x(),
                 -0.5f + yMiddle_ - coords.y(),
                 -0.5f + zMiddle_);

    int i = 0;
    exitShift_ = 0;

    for (MapRoom *mr : island) {
        const RoomCoords &rc = mr->coords();
        if (rc.layer() == currentLayer_) {
            glPushMatrix();
            glTranslatef(rc.x() * 2.0f, rc.y() * 2.0f, rc.z() * 2.0f);

            if (selected_ == i && !selection_) {
                if (i == 0) {
                    glColor4ubv(selection1Color);
                } else {
                    glColor4ubv(selectionColor);
                }
                if (reportSelected_) {
                    reportSelected_ = false;
                    selectedVnum_ = mr->room()->vnum;
                    emit vnumSelected(selectedVnum_);
                }
            } else {
                if (i == 0) {
                    if (roomMarkingFlag_ > 0
                        && (mr->room()->flags & roomMarkingFlag_) == roomMarkingFlag_) {
                        glColor4ubv(flag1Color);
                    } else {
                        glColor4fv(object1Color);
                    }
                } else {
                    if (roomMarkingFlag_ > 0
                        && (mr->room()->flags & roomMarkingFlag_) == roomMarkingFlag_) {
                        glColor4ubv(flagColor);
                    } else {
                        glColor4fv(objectColor);
                    }
                }
            }

            glCallList(glList_);
            drawExits(mr);
            glPopMatrix();

            i++;
        }
    }
    glPopMatrix();
}

void MapWidget::drawExits(MapRoom *mr)
{
    QMapIterator<ExitWrapper *, MapRoom *> it(mr->mapRooms());
    while (it.hasNext()) {
        it.next();
        ExitWrapper *exit = it.key();
        ExitWrapper *revExit = exit->revExit();

        if ((revExit == nullptr || !revExit->isDrawn()) && !exit->isDrawn()) {
            int shift = currentIslandSize_ + exitShift_;
            glPushMatrix();

            switch (exit->direction()) {
            case 0: /* north */
                glTranslatef(0.5f, 0.5f, -1.0f);
                break;
            case 1: /* east */
                glRotatef(90, 0, 1, 0);
                glTranslatef(-0.5f, 0.5f, 1.0f);
                break;
            case 2: /* south */
                glTranslatef(0.5f, 0.5f, 1.0f);
                break;
            case 3: /* west */
                glRotatef(90, 0, 1, 0);
                glTranslatef(-0.5f, 0.5f, -1.0f);
                break;
            case 4: /* up */
                glRotatef(90, 1, 0, 0);
                glTranslatef(0.5f, 0.5f, -2.0f);
                break;
            case 5: /* down */
                glRotatef(90, 1, 0, 0);
                glTranslatef(0.5f, 0.5f, 0.0f);
                break;
            case 6: /* north-east */
                glRotatef(45, 0, -1, 0);
                glTranslatef(0.7f, 0.5f, -2.3f);
                glScalef(1, 1, 1.8f);
                break;
            case 7: /* north-west */
                glRotatef(45, 0, 1, 0);
                glTranslatef(0.0f, 0.5f, -1.6f);
                glScalef(1, 1, 1.8f);
                break;
            case 8: /* south-east */
                glRotatef(45, 0, 1, 0);
                glTranslatef(0.0f, 0.5f, 1.2f);
                glScalef(1, 1, 1.8f);
                break;
            case 9: /* south-west */
                glRotatef(45, 0, -1, 0);
                glTranslatef(0.7f, 0.5f, 0.5f);
                glScalef(1, 1, 1.8f);
                break;
            default: /* somewhere: 10 */
                break;
            }

            if ((!drawDistantExits_ && !exit->isDistant()) || drawDistantExits_) {
                if (!selection_ && selected_ == shift) {
                    glColor4ubv(selectionColor);
                    if (reportSelected_) {
                        reportSelected_ = false;
                        selectedExit_ = exit;
                        selectedVnum_ = mr->room()->vnum;
                        emit exitSelected(selectedVnum_, exit->direction(), exit->vnum());
                    }
                } else {
                    glColor4fv(objectColor);
                }

                if (exit->isTwoWay()) {
                    glCallList(glList_ + 1);
                } else {
                    glCallList(glList_ + 2);
                }
            }
            exit->setDrawn();
            glPopMatrix();
        }
        exitShift_++;
    }
}

void MapWidget::cleanDrawnMark()
{
    if (!islandRooms_.contains(currentIsland_))
        return;
    const QList<MapRoom *> &island = islandRooms_.value(currentIsland_);
    for (MapRoom *mr : island) {
        QMapIterator<ExitWrapper *, MapRoom *> it(mr->mapRooms());
        while (it.hasNext()) {
            it.next();
            it.key()->setDrawn(false);
        }
    }
}

void MapWidget::drawWindRose()
{
    glPushMatrix();
    const float fScale = 0.1f;
    const float zRose = -2.0f;

    // getLeftBottom equivalent
    float topVal = std::tan(fov * static_cast<float>(M_PI) / 360.0f) * (-zRose);
    float leftVal = aspect_ * topVal;

    glTranslatef(leftVal + fScale, topVal + fScale, zRose);
    glScalef(fScale, fScale, fScale);
    glRotatef(xRot_ / 16.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot_ / 16.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot_ / 16.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(angle_, 1, 1, 1);

    glCallList(glList_ + 3);
    glPopMatrix();
}

// ===================================================================
// Color-based picking (replaces GL_SELECT)
// ===================================================================

void MapWidget::setPickingColor(int id)
{
    // Encode id into RGB. id 0 = (1,0,0), id 1 = (2,0,0), etc.
    // Reserve (0,0,0) as background (no selection).
    int encodedId = id + 1;
    unsigned char r = static_cast<unsigned char>(encodedId & 0xFF);
    unsigned char g = static_cast<unsigned char>((encodedId >> 8) & 0xFF);
    unsigned char b = static_cast<unsigned char>((encodedId >> 16) & 0xFF);
    glColor3ub(r, g, b);
}

int MapWidget::decodePickingColor(unsigned char r, unsigned char g, unsigned char b)
{
    if (r == 0 && g == 0 && b == 0)
        return -1;
    int id = (r | (g << 8) | (b << 16)) - 1;
    return id;
}

void MapWidget::performSelection(int cx, int cy)
{
    // Save current GL state
    glDisable(GL_BLEND);
    glDisable(GL_MULTISAMPLE);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSceneForPicking();

    // Read the pixel at click position
    unsigned char pixel[4];
    glReadPixels(cx, h_ - cy, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    int chosen = decodePickingColor(pixel[0], pixel[1], pixel[2]);

    if (chosen >= 0) {
        selected_ = chosen;
    } else {
        selected_ = -1;
        selectedExit_ = nullptr;
    }

    // Restore
    glEnable(GL_BLEND);
    if (multisample_) {
        glEnable(GL_MULTISAMPLE);
    }
    glClearColor(0, 0, 0, 0);
}

void MapWidget::drawSceneForPicking()
{
    if (!islandRooms_.contains(currentIsland_))
        return;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const QList<MapRoom *> &island = islandRooms_.value(currentIsland_);
    if (island.isEmpty())
        return;

    glPushMatrix();
    glTranslatef(dx_, dy_, dz_);
    glRotatef(xRot_ / 16.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot_ / 16.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot_ / 16.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(angle_, 1.0f, 1.0f, 1.0f);

    const RoomCoords &coords = island.at(0)->coords();
    glTranslatef(-0.5f + xMiddle_ - coords.x(),
                 -0.5f + yMiddle_ - coords.y(),
                 -0.5f + zMiddle_);

    int i = 0;
    int pickExitShift = 0;

    for (MapRoom *mr : island) {
        const RoomCoords &rc = mr->coords();
        if (rc.layer() == currentLayer_) {
            glPushMatrix();
            glTranslatef(rc.x() * 2.0f, rc.y() * 2.0f, rc.z() * 2.0f);

            // Draw room cube with picking color
            setPickingColor(i);
            glCallList(glList_);

            // Draw exits with picking colors
            QMapIterator<ExitWrapper *, MapRoom *> eit(mr->mapRooms());
            while (eit.hasNext()) {
                eit.next();
                ExitWrapper *exit = eit.key();
                ExitWrapper *revExit = exit->revExit();

                if ((revExit == nullptr || !revExit->isDrawn()) && !exit->isDrawn()) {
                    int shift = currentIslandSize_ + pickExitShift;

                    if ((!drawDistantExits_ && !exit->isDistant()) || drawDistantExits_) {
                        glPushMatrix();

                        switch (exit->direction()) {
                        case 0: glTranslatef(0.5f, 0.5f, -1.0f); break;
                        case 1: glRotatef(90, 0, 1, 0); glTranslatef(-0.5f, 0.5f, 1.0f); break;
                        case 2: glTranslatef(0.5f, 0.5f, 1.0f); break;
                        case 3: glRotatef(90, 0, 1, 0); glTranslatef(-0.5f, 0.5f, -1.0f); break;
                        case 4: glRotatef(90, 1, 0, 0); glTranslatef(0.5f, 0.5f, -2.0f); break;
                        case 5: glRotatef(90, 1, 0, 0); glTranslatef(0.5f, 0.5f, 0.0f); break;
                        case 6:
                            glRotatef(45, 0, -1, 0);
                            glTranslatef(0.7f, 0.5f, -2.3f);
                            glScalef(1, 1, 1.8f);
                            break;
                        case 7:
                            glRotatef(45, 0, 1, 0);
                            glTranslatef(0.0f, 0.5f, -1.6f);
                            glScalef(1, 1, 1.8f);
                            break;
                        case 8:
                            glRotatef(45, 0, 1, 0);
                            glTranslatef(0.0f, 0.5f, 1.2f);
                            glScalef(1, 1, 1.8f);
                            break;
                        case 9:
                            glRotatef(45, 0, -1, 0);
                            glTranslatef(0.7f, 0.5f, 0.5f);
                            glScalef(1, 1, 1.8f);
                            break;
                        default:
                            break;
                        }

                        setPickingColor(shift);
                        if (exit->isTwoWay()) {
                            glCallList(glList_ + 1);
                        } else {
                            glCallList(glList_ + 2);
                        }
                        glPopMatrix();
                    }
                    exit->setDrawn();
                }
                pickExitShift++;
            }

            glPopMatrix();
            i++;
        }
    }
    glPopMatrix();

    // Clean drawn marks after picking pass
    for (MapRoom *mr : island) {
        QMapIterator<ExitWrapper *, MapRoom *> eit(mr->mapRooms());
        while (eit.hasNext()) {
            eit.next();
            eit.key()->setDrawn(false);
        }
    }
}

// ===================================================================
// Text overlay (QPainter replacement for GLFont)
// ===================================================================

void MapWidget::drawTextOverlay()
{
    bool needOverlay = (selected_ >= 0 && selectedVnum_ >= 0)
                       || showHelp_
                       || showIslandsLayers_ > 0;

    if (!needOverlay)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    QFont font(QStringLiteral("Monospace"), 11);
    font.setStyleHint(QFont::Monospace);
    painter.setFont(font);
    painter.setPen(Qt::white);

    QFontMetrics fm(font);
    int lineHeight = fm.height() + 2;

    // Selected room/exit info at bottom left
    if (selected_ >= 0 && selectedVnum_ >= 0) {
        if (selected_ < currentIslandSize_) {
            QString text = QStringLiteral("vnum selected: %1").arg(selectedVnum_);
            painter.drawText(10, h_ - 10, text);
        } else if (selectedExit_ != nullptr) {
            QString text = QStringLiteral("exit selected: %1->%2->%3")
                               .arg(selectedVnum_)
                               .arg(selectedExit_->getDirectionName())
                               .arg(selectedExit_->vnum());
            painter.drawText(10, h_ - 10, text);
        }
    }

    // Island/layer info at bottom left (above selection text)
    if (showIslandsLayers_ > 0) {
        QString text = QStringLiteral("island: %1/%2 layer: %3/%4")
                           .arg(getIslandNum())
                           .arg(getMaxIslandNum())
                           .arg(getLayerNum())
                           .arg(getMaxLayerNum());
        painter.drawText(10, h_ - 10 - lineHeight, text);
        --showIslandsLayers_;
    }

    // Help text at top right, left-justified block
    if (showHelp_) {
        int y = lineHeight + 5;
        int maxWidth = 0;
        for (const QString &key : MENU_KEYS) {
            int tw = fm.horizontalAdvance(key);
            if (tw > maxWidth) maxWidth = tw;
        }
        int xStart = w_ - maxWidth - 10;
        for (const QString &key : MENU_KEYS) {
            painter.drawText(xStart, y, key);
            y += lineHeight;
        }
    }

    painter.end();
}

// ===================================================================
// View setup
// ===================================================================

void MapWidget::setupIsland(int islandNo)
{
    currentIsland_ = islandNo;
    if (!islandRooms_.contains(islandNo))
        return;

    currentIslandSize_ = islandRooms_.value(islandNo).size();
    xMiddle_ = static_cast<float>(getXMiddle(islandRooms_.value(islandNo)));
    yMiddle_ = static_cast<float>(getYMiddle(islandRooms_.value(islandNo)));
    zMiddle_ = static_cast<float>(getZMiddle(islandRooms_.value(islandNo)));
    selected_ = -1;
    currentLayer_ = 0;
    maxLayer_ = 0;

    for (MapRoom *mr : islandRooms_.value(islandNo)) {
        int layer = mr->coords().layer();
        if (layer > maxLayer_) {
            maxLayer_ = layer;
        }
    }
    showIslandsLayers();
}

void MapWidget::center()
{
    dz_ = INITIAL_Z;
    rot_ = dx_ = dy_ = angle_ = xRot_ = yRot_ = zRot_ = 0.0f;
}

int MapWidget::normalizeAngle(int angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
    return angle;
}

void MapWidget::setXRotation(int angle)
{
    angle = normalizeAngle(angle);
    if (angle != static_cast<int>(xRot_)) {
        xRot_ = static_cast<float>(angle);
    }
}

void MapWidget::setYRotation(int angle)
{
    angle = normalizeAngle(angle);
    if (angle != static_cast<int>(yRot_)) {
        yRot_ = static_cast<float>(angle);
    }
}

void MapWidget::setZRotation(int angle)
{
    angle = normalizeAngle(angle);
    if (angle != static_cast<int>(zRot_)) {
        zRot_ = static_cast<float>(angle);
    }
}

void MapWidget::incLayer()
{
    if (currentLayer_ < maxLayer_) {
        ++currentLayer_;
        selected_ = -1;
        showIslandsLayers();
    }
}

void MapWidget::decLayer()
{
    if (currentLayer_ > 0) {
        --currentLayer_;
        selected_ = -1;
        showIslandsLayers();
    }
}

void MapWidget::showIslandsLayers()
{
    showIslandsLayers_ = FPS * 2;
}

MapWidget::IslandRoom MapWidget::findIsland(int vnum)
{
    QMapIterator<int, QList<MapRoom *>> it(islandRooms_);
    while (it.hasNext()) {
        it.next();
        int islandNo = it.key();
        int roomNo = 0;
        for (MapRoom *mr : it.value()) {
            if (mr->room()->vnum == vnum) {
                return { islandNo, roomNo };
            }
            ++roomNo;
        }
    }
    return { -1, -1 };
}

// ===================================================================
// Screenshots
// ===================================================================

void MapWidget::setScreenShotParent()
{
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    screenShotDir_ = QDir(homeDir).filePath(QStringLiteral(".swaedit/mapshots"));
    screenShotBareFileName_ = QStringLiteral("unnamed");
}

QString MapWidget::getScreenShotPath()
{
    QDir dir(screenShotDir_);
    dir.mkpath(QStringLiteral("."));

    QString path;
    for (; ; screenShotCnt_++) {
        path = dir.filePath(QStringLiteral("%1_%2_%3_%4.png")
                                .arg(screenShotBareFileName_)
                                .arg(getIslandNum())
                                .arg(getLayerNum())
                                .arg(screenShotCnt_));
        if (!QFileInfo::exists(path))
            break;
    }
    return path;
}

void MapWidget::screenShot()
{
    screenShotPath_ = getScreenShotPath();
    readPixels_ = true;
}

void MapWidget::transparentScreenShot()
{
    screenShotPath_ = getScreenShotPath();
    readPixels_ = true;
    transparentShot_ = true;
}

void MapWidget::takeScreenshot()
{
    readPixels_ = false;
    QImage::Format format = QImage::Format_RGB32;
    if (transparentShot_) {
        transparentShot_ = false;
        format = QImage::Format_ARGB32;
    }

    QVector<unsigned char> pixels(w_ * h_ * BPP);
    glReadPixels(0, 0, w_, h_, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Flip vertically and convert RGBA to ARGB
    QImage img(w_, h_, format);
    for (int y = 0; y < h_; ++y) {
        for (int x = 0; x < w_; ++x) {
            int srcIdx = ((h_ - 1 - y) * w_ + x) * BPP;
            unsigned char r = pixels[srcIdx];
            unsigned char g = pixels[srcIdx + 1];
            unsigned char b = pixels[srcIdx + 2];
            unsigned char a = pixels[srcIdx + 3];
            img.setPixelColor(x, y, QColor(r, g, b, a));
        }
    }
    img.save(screenShotPath_);
}

// ===================================================================
// Events
// ===================================================================

void MapWidget::closeEvent(QCloseEvent *e)
{
    animTimer_->stop();
    emit windowClosed();
    e->accept();
}

void MapWidget::keyPressEvent(QKeyEvent *e)
{
    int kc = e->key();

    if (kc == Qt::Key_Space) {
        animate_ = !animate_;
        e->accept();
    } else if (kc == Qt::Key_D) {
        drawDistantExits_ = !drawDistantExits_;
        e->accept();
    } else if (kc == Qt::Key_M) {
        multisample_ = !multisample_;
        e->accept();
    } else if (kc == Qt::Key_K) {
        drawCross_ = !drawCross_;
        e->accept();
    } else if (kc == Qt::Key_C) {
        center();
        e->accept();
    } else if (kc == Qt::Key_H) {
        showHelp_ = !showHelp_;
        e->accept();
    } else if (kc == Qt::Key_F) {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
        e->accept();
    } else if (kc == Qt::Key_R) {
        emit mapRefreshed();
        e->accept();
    } else if (kc == Qt::Key_I) {
        emit roomMarkingFlagRequested();
        e->accept();
    } else if (kc == Qt::Key_Left) {
        if (currentIsland_ > 0) {
            setupIsland(--currentIsland_);
        }
        e->accept();
    } else if (kc == Qt::Key_Right) {
        if (currentIsland_ < maxIslands_ - 1) {
            setupIsland(++currentIsland_);
        }
        e->accept();
    } else if (kc == Qt::Key_Up) {
        incLayer();
        e->accept();
    } else if (kc == Qt::Key_Down) {
        decLayer();
        e->accept();
    } else if (kc == Qt::Key_F12) {
        screenShot();
        e->accept();
    } else if (kc == Qt::Key_F11) {
        transparentScreenShot();
        e->accept();
    } else if (kc == Qt::Key_Shift) {
        zRotAxis_ = true;
        e->accept();
    } else {
        e->ignore();
    }
}

void MapWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Shift) {
        zRotAxis_ = false;
        e->accept();
    } else {
        e->ignore();
    }
}

void MapWidget::wheelEvent(QWheelEvent *e)
{
    dz_ -= static_cast<float>(e->angleDelta().y()) / 120.0f;
    e->accept();
}

void MapWidget::mousePressEvent(QMouseEvent *e)
{
    lastPos_ = e->pos();
    if (e->button() == Qt::LeftButton) {
        cx_ = e->pos().x();
        cy_ = e->pos().y();
        e->accept();
    } else {
        e->ignore();
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        int ex = e->pos().x();
        int ey = e->pos().y();
        if (std::abs(cx_ - ex) < 3 && std::abs(cy_ - ey) < 3) {
            cx_ = ex;
            cy_ = ey;
            selection_ = true;
            reportSelected_ = true;
        }
        e->accept();
    } else {
        e->ignore();
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent *e)
{
    int ddx = e->pos().x() - lastPos_.x();
    int ddy = e->pos().y() - lastPos_.y();
    lastPos_ = e->pos();

    if (e->buttons() & Qt::LeftButton) {
        if (dz_ < 0.0f) {
            dx_ -= (static_cast<float>(ddx) / static_cast<float>(w_)) * dz_;
            dy_ += (static_cast<float>(ddy) / static_cast<float>(h_)) * dz_;
        } else if (dz_ > 0.0f) {
            dx_ += (static_cast<float>(ddx) / static_cast<float>(w_)) * dz_;
            dy_ -= (static_cast<float>(ddy) / static_cast<float>(h_)) * dz_;
        } else {
            dx_ += static_cast<float>(ddx) / static_cast<float>(w_);
            dy_ -= static_cast<float>(ddy) / static_cast<float>(h_);
        }
        e->accept();
    } else if (e->buttons() & Qt::RightButton) {
        setXRotation(static_cast<int>(xRot_) + 8 * ddy);
        if (zRotAxis_) {
            setZRotation(static_cast<int>(zRot_) + 8 * ddx);
        } else {
            setYRotation(static_cast<int>(yRot_) + 8 * ddx);
        }
        e->accept();
    } else {
        e->ignore();
    }
}
