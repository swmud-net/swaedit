package pl.swmud.ns.swaedit.gui;

import java.awt.Font;
import java.io.File;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.SortedMap;

import javax.media.opengl.GL2;
import javax.media.opengl.GLAutoDrawable;
import javax.media.opengl.GLCapabilities;
import javax.media.opengl.GLEventListener;
import javax.media.opengl.GLProfile;
import javax.media.opengl.glu.GLU;
import javax.media.opengl.glu.GLUquadric;

import pl.swmud.ns.swaedit.map.ExitWrapper;
import pl.swmud.ns.swaedit.map.GLFont;
import pl.swmud.ns.swaedit.map.MapRoom;
import pl.swmud.ns.swaedit.map.RoomCoords;
import pl.swmud.ns.swaedit.map.RoomSpread;

import com.jogamp.common.nio.Buffers;
import com.jogamp.newt.event.MouseAdapter;
import com.jogamp.newt.event.MouseEvent;
import com.jogamp.newt.event.WindowAdapter;
import com.jogamp.newt.event.WindowEvent;
import com.jogamp.newt.opengl.GLWindow;
import com.jogamp.opengl.util.Animator;
import com.jogamp.opengl.util.FPSAnimator;
import com.trolltech.qt.QSignalEmitter;
import com.trolltech.qt.gui.QImage;
import com.trolltech.qt.gui.QImage.Format;

public class MapWidget extends QSignalEmitter implements GLEventListener, MapKeyHandler.Minion {
	private int selectBufLen;

	private static final float SENSITIVITY = 0.06f;
	private static final int FPS = 30;
	private final GLWindow window;
	private Animator[] animator;

	private GL2 gl;
	private GLU glu;

	private boolean multisample = true;
	private static final float ALPHA = .5f;

	private static final int MAX_NAMESTACK_SIZE = 2;
	private static final byte[] selectionColor = { (byte) 218, (byte) 168, (byte) 16, (byte) (255 * ALPHA) };
	private static final float[] objectColor = { .0f, .7f, .0f, ALPHA };
	private int cx, cy;
	private int selected = -1;
	private boolean reportSelected;
	private boolean selection;
	private BigInteger selectedVnum;
	private int exitShift;
	private ExitWrapper selectedExit;

	private int w, h;
	private float fov = 60f;
	private float aspect;
	private float near = 1.0f;
	private float far = 500.0f;

	private static final float PI2 = (float) Math.PI * .5f;
	private static final float ROT_SENS_MULT = .03f;
	private static final float INITIAL_Z = -31.f;
	private float dz = INITIAL_Z;
	private float rot, angle, dx, dy, oldx, oldy, xRot, yRot, zRot, xAngle, yAngle, zAngle;
	private boolean xRotAxis, yRotAxis, zRotAxis;

	private SortedMap<Integer, List<MapRoom>> islandRooms;
	private float xMiddle, yMiddle;
	private int currentIsland;
	private int currentIslandSize;
	private final int maxIslands;
	private int currentLayer;
	private int maxLayer;

	private boolean drawDistantExits;

	private GLFont glFont;

	// private int mx, my;
	// private int hovered = -1;
	// private boolean reportHovered;
	// private boolean hovering;
	// private BigInteger hoveredVnum;

	private boolean drawCross;
	private boolean showHelp;
	private int showIslandsLayers;
	private int fulscreenWidth, fulscreenHeight;

	private static final int BPP = 4; /* bytes per pixel */

	private boolean readPixels;
	private boolean transparentShot;
	private String screenShotPath;
	private File screenShotDir;
	private String screenShotBareFileName;
	private long screenShotCnt = 1;
	private static final String USER_HOME = System.getProperty("user.home");

	private static final List<String> MENU_KEYS = new ArrayList<String>() {
		private static final long serialVersionUID = 1041444819386431807L;
		{
			add("d      - toggle distant exits");
			add("m      - toggle multisampling");
			add("space  - toggle rotation");
			add("h      - toggle this help text");
			add("k      - toggle cross");
			add("c      - reset to center");
			add("f      - toggle fullscreen");
			add("right  - next island");
			add("left   - previous island");
			add("up     - next layer");
			add("down   - previous layer");
			add("F12    - take a screenshot");
			add("F11    - take a transparent screenshot");
			add("lMouse - drag to move, click to select room/exit");
			add("rMouse - drag to change rotation angle");
			add("shift  - hold to apply angle to X-axis");
			add("ctrl   - hold to apply angle to Y-axis");
			add("alt    - hold to apply angle to Z-axis");
		}
	};

	private final Signal1<BigInteger> vnumSelected = new Signal1<BigInteger>();
	private final Signal2<BigInteger, Integer> exitSelected = new Signal2<BigInteger, Integer>();
	private final Signal0 windowClosed = new Signal0();

	public MapWidget(SortedMap<Integer, List<MapRoom>> islandRooms, int currentIsland, int maxIslands) {
		this.islandRooms = islandRooms;
		this.maxIslands = maxIslands;
		setupIsland(currentIsland);
		setScreenShotParent();

		GLProfile profile = GLProfile.get(GLProfile.GL2);
		GLCapabilities caps = new GLCapabilities(profile);
		caps.setSampleBuffers(true);
		caps.setNumSamples(4);
		caps.setAlphaBits(8);

		window = GLWindow.create(caps);
		// window.enablePerfLog(true);
		window.setSize(800, 600);
		window.addWindowListener(new WindowAdapter() {
			@Override
			public void windowDestroyNotify(WindowEvent e) {
				super.windowDestroyNotify(e);
				// System.exit(0);
				windowClosed.connect(SWAEdit.ref, "mapClosed()");
				windowClosed.emit();
			}
		});
		window.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				if (e.getButton() == MouseEvent.BUTTON4) {
					dz += 1;
				} else if (e.getButton() == MouseEvent.BUTTON5) {
					dz -= 1;
				} else if (e.getButton() == MouseEvent.BUTTON1) {
					cx = e.getX();
					cy = e.getY();
					selection = true;
					reportSelected = true;
				}
			}

			@Override
			public void mouseWheelMoved(MouseEvent e) {
				dz += (e.getWheelRotation() > 0 ? 1 : -1);
			}

			@Override
			public void mouseDragged(MouseEvent e) {
				if (e.getButton() == MouseEvent.BUTTON1) {
					float px = (float) e.getX();
					if (px > oldx) {
						dx += SENSITIVITY;
					} else if (px < oldx) {
						dx -= SENSITIVITY;
					}
					oldx = px;

					float py = (float) e.getY();
					if (py < oldy) {
						dy += SENSITIVITY;
					} else if (py > oldy) {
						dy -= SENSITIVITY;
					}
					oldy = py;
				} else if (e.getButton() == MouseEvent.BUTTON3) {
					float px = (float) e.getX();
					float py = (float) e.getY();
					if (xRotAxis) {
						if (py < oldy) {
							xRot -= SENSITIVITY * ROT_SENS_MULT;
						} else if (py > oldy) {
							xRot += SENSITIVITY * ROT_SENS_MULT;
						}

						if (xRot > PI2) {
							xRot = .0f;
						} else if (xRot < .0f) {
							xRot = PI2;
						}

						xAngle = 360.f * Math.abs((float) Math.sin(xRot));
					}

					if (yRotAxis) {
						if (px > oldx) {
							yRot += SENSITIVITY * ROT_SENS_MULT;
						} else if (px < oldx) {
							yRot -= SENSITIVITY * ROT_SENS_MULT;
						}

						if (yRot > PI2) {
							yRot = .0f;
						} else if (yRot < .0f) {
							yRot = PI2;
						}

						yAngle = 360.f * Math.abs((float) Math.sin(yRot));
					}

					if (zRotAxis) {
						if (py < oldy) {
							zRot += SENSITIVITY * ROT_SENS_MULT;
						} else if (py > oldy) {
							zRot -= SENSITIVITY * ROT_SENS_MULT;
						}

						if (zRot > PI2) {
							zRot = .0f;
						} else if (zRot < .0f) {
							zRot = PI2;
						}

						zAngle = 360.f * Math.abs((float) Math.sin(zRot));
					}

					oldx = px;
					oldy = py;
				}
			}
			/*
			 * @Override public void mouseMoved(MouseEvent e) { int px =
			 * e.getX(); int py = e.getY(); float[] mPos = translateMouse(px,
			 * py, w, h); mx = px;// mPos[0]*Math.abs(dz); my = py;//
			 * mPos[1]*Math.abs(dz); // hovering = true; // reportHovered =
			 * true; }
			 */
		});

		vnumSelected.connect(SWAEdit.ref, "mapRoomVnumSelected(BigInteger)");
		exitSelected.connect(SWAEdit.ref, "mapRoomExitSelected(BigInteger,int)");

		animator = new FPSAnimator[2];
		animator[0] = new FPSAnimator(window, 30);
		animator[1] = new FPSAnimator(window, 30);

		window.addKeyListener(new MapKeyHandler(this, currentIsland, this.maxIslands));
		window.addGLEventListener(this);
	}

	private void setupIsland(int islandNo) {
		currentIsland = islandNo;
		currentIslandSize = islandRooms.get(islandNo).size();
		selectBufLen = currentIslandSize * 4 + 5;
		xMiddle = getXMiddle(islandNo);
		yMiddle = getYMiddle(islandNo);
		selected = -1;
		currentLayer = 0;
		maxLayer = 0;
		for (MapRoom mr : islandRooms.get(islandNo)) {
			int layer = mr.getCoords().getLayer();
			if (layer > maxLayer) {
				maxLayer = layer;
			}
		}
		showIslandsLayers();
	}

	public void setCurrentIsland(int islandNo) {
		setupIsland(islandNo);
	}

	public void rotationStopped() {
		toggleAnimator();
	}

	public void drawDistantExits() {
		drawDistantExits = !drawDistantExits;
	}

	public void decLayer() {
		if (currentLayer > 0) {
			--currentLayer;
			selected = -1;
			showIslandsLayers();
		}
	}

	public void incLayer() {
		if (currentLayer < maxLayer) {
			++currentLayer;
			selected = -1;
			showIslandsLayers();
		}
	}

	public void multisampling() {
		multisample = !multisample;
	}

	public void drawCross() {
		drawCross = !drawCross;
	}

	public void center() {
		dz = INITIAL_Z;
		rot = dx = dy = angle = oldx = oldy = xRot = yRot = zRot = xAngle = yAngle = zAngle = .0f;
	}

	public void showHelp() {
		showHelp = !showHelp;
	}

	public void showFullscreen() {
		if (!window.isFullscreen()) {
			fulscreenWidth = w;
			fulscreenHeight = h;
			window.setFullscreen(true);
		} else {
			window.setFullscreen(false);
			window.setSize(fulscreenWidth, fulscreenHeight);

		}
	}

	public void screenShot() {
		screenShotPath = getScreenShotPath();
		readPixels = true;
	}

	public void transparentScreenShot() {
		screenShotPath = getScreenShotPath();
		readPixels = true;
		transparentShot = true;
	}

	public void xRotOff() {
		xRotAxis = false;
	}

	public void xRotOn() {
		xRotAxis = true;
	}

	public void yRotOff() {
		yRotAxis = false;
	}

	public void yRotOn() {
		yRotAxis = true;
	}

	public void zRotOff() {
		zRotAxis = false;
	}

	public void zRotOn() {
		zRotAxis = true;
	}

	private void setScreenShotParent() {
		File parent;
		if (SWAEdit.ref.currentFileName != null) {
			File currentFile = new File(SWAEdit.ref.currentFileName);
			parent = currentFile.getParentFile();
			screenShotBareFileName = currentFile.getName().replace(".xml", "");
		} else {
			parent = new File(USER_HOME, ".swaedit");
			screenShotBareFileName = "unnamed";
		}
		screenShotDir = new File(new File(parent, "mapshots"), screenShotBareFileName).getAbsoluteFile();
	}

	private String getScreenShotPath() {
		screenShotDir.mkdirs();

		File shot = null;
		for (; shot == null || shot.exists(); screenShotCnt++) {
			shot = new File(screenShotDir, screenShotBareFileName + "_" + getIslandNum() + "_" + getLayerNum() + "_"
			        + screenShotCnt + ".png");
		}

		return shot.getAbsolutePath();
	}

	private int getLayerNum() {
		return currentLayer + 1;
	}

	private int getMaxLayerNum() {
		return maxLayer + 1;
	}

	private int getIslandNum() {
		return currentIsland + 1;
	}

	private int getMaxIslandNum() {
		return maxIslands;
	}

	public void show(int x, int y) {
		if (!window.isVisible()) {
			window.setPosition(x, y);
			window.setVisible(true);
			animator[0].start();
		}
	}

	private void showIslandsLayers() {
		showIslandsLayers = FPS * 2;
	}

	private void toggleAnimator() {
		if (animator[0].isAnimating()) {
			animator[0].stop();
			animator[1].start();
		} else {
			animator[1].stop();
			animator[0].start();
		}
	}

	private int getXMiddle(int islandNo) {
		return new RoomSpread(islandRooms.get(islandNo)) {
			@Override
			public int getValue(RoomCoords coords) {
				return coords.getX();
			}
		}.getMiddle();
	}

	private int getYMiddle(int islandNo) {
		return new RoomSpread(islandRooms.get(islandNo)) {
			@Override
			public int getValue(RoomCoords coords) {
				return coords.getY();
			}
		}.getMiddle();
	}

	public void showRoom(BigInteger vnum) {
		int[] islandRoom = findIsland(vnum);
		if (islandRoom != null) {
			if (currentIsland != islandRoom[0]) {
				setupIsland(islandRoom[0]);
			}
			selected = islandRoom[1];
			selectedVnum = vnum;
		}
	}

	private int[] findIsland(BigInteger vnum) {
		for (int islandNo : islandRooms.keySet()) {
			int roomNo = 0;
			for (MapRoom mr : islandRooms.get(islandNo)) {
				if (vnum.equals(mr.getRoom().getVnum())) {
					return new int[] { islandNo, roomNo };
				}
				++roomNo;
			}
		}
		return null;
	}

	public void showExit(BigInteger ownerRoomVnum, int exitIdx) {
		int[] islandRoom = findIsland(ownerRoomVnum);
		if (islandRoom != null) {
			if (currentIsland != islandRoom[0]) {
				setupIsland(islandRoom[0]);
			}
			selectedVnum = ownerRoomVnum;

			int exitNo = 0;
			boolean foundRoom = false;
			for (MapRoom mr : islandRooms.get(currentIsland)) {
				if (mr.getRoom().getVnum().equals(ownerRoomVnum)) {
					foundRoom = true;
				}

				int eI = 0;
				for (ExitWrapper exit : mr.getMapRooms().keySet()) {
					int shift = currentIslandSize + exitNo;
					if (foundRoom && eI == exitIdx) {
						selectedExit = exit;
						selected = shift;
						if (exit.isDistant() && !drawDistantExits) {
	                        drawDistantExits = true;
                        }
						return;
					}
					exitNo++;
					eI++;
				}
			}
		}
	}

	// private float[] translateMouse(int x, int y, int w, int h) {
	// return new float[] { -(1.f - ((float) x * 2.f / (float) w)), 1.f -
	// ((float) y * 2.f / (float) h) };
	// }

	private void select(int cx, int cy) {
		int[] viewport = new int[4];
		IntBuffer selectBuf = Buffers.newDirectIntBuffer(selectBufLen);

		gl.glSelectBuffer(selectBufLen, selectBuf);

		gl.glGetIntegerv(GL2.GL_VIEWPORT, viewport, 0);

		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPushMatrix();

		gl.glRenderMode(GL2.GL_SELECT);

		gl.glLoadIdentity();
		glu.gluPickMatrix(cx, viewport[3] - cy + viewport[1], 2, 2, viewport, 0);
		glu.gluPerspective(fov, aspect, near, far);

		drawWorld();

		int hits = gl.glRenderMode(GL2.GL_RENDER);

		if (hits > 0) {
			int[] sb = new int[selectBufLen];
			selectBuf.get(sb, 0, hits * 4);
			processHits(hits, sb);
		} else {
			if (selection) {
				selected = -1;
				selectedExit = null;
			}
			// if (hovering) {
			// hovered = -1;
			// }
		}

		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPopMatrix();
		gl.glMatrixMode(GL2.GL_MODELVIEW);
	}

	private void processHits(int hits, int[] sb) {
		float minz = Float.MAX_VALUE;
		int chosen = -1;
		for (int i = 0; i < hits * 4; i += 4) {
			if (sb[i] < MAX_NAMESTACK_SIZE) {
				float z = sb[i + 1];
				z /= Float.MAX_VALUE;
				if (z < minz) {
					minz = z;
					chosen = sb[i + 3];
				}
			}
		}

		if (selection) {
			selected = chosen;
		}
		// if (hovering) {
		// hovered = chosen;
		// }
	}

	private void drawWorld() {
		if (animator[0].isAnimating()) {
			rot += 0.002;
			if (rot > Math.PI / 2.f) {
				rot = .0f;
			}
			angle = 360.f * Math.abs((float) Math.sin(rot));
		}

		gl.glClear(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_DEPTH_BUFFER_BIT);
		gl.glMatrixMode(GL2.GL_MODELVIEW);
		gl.glInitNames();

		gl.glLoadIdentity();

		drawIslands();

		drawWindRose();

		if (drawCross) {
			drawCrossGL();
		}

		if (selected >= 0 && selectedVnum != null || showHelp || showIslandsLayers > 0) {
			gl.glPushName(-2);
			gl.glPushMatrix();
			gl.glPushAttrib(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_TEXTURE_BIT | GL2.GL_DEPTH_BUFFER_BIT);
			setOrthoOn();

			if (selected >= 0) {
				if (selectedVnum != null) {
					if (selected < currentIslandSize) {
						drawText("vnum selected: " + selectedVnum.toString(), -1.f, -.98f);
					} else if (selectedExit != null) {
						drawText("exit selected: " + selectedVnum.toString() + "->" + selectedExit.getDirectionName()
						        + "->" + selectedExit.getVnum().toString(), -1.f, -.98f);
					}
				}
			}

			if (showHelp) {
				float yShift = .98f;
				for (String mKey : MENU_KEYS) {
					drawText(mKey, 1.f, yShift);
					yShift -= .04f;
				}
			}

			if (showIslandsLayers > 0) {
				drawText("island: " + getIslandNum() + "/" + getMaxIslandNum() + " layer: " + getLayerNum() + "/"
				        + getMaxLayerNum(), -1.f, -.94f);
				--showIslandsLayers;
			}

			setOrthoOff();
			gl.glPopAttrib();
			gl.glPopMatrix();
			gl.glPopName();
		}

		gl.glFlush();

		if (readPixels) {
			takeScreenshot();
		}

		cleanDrawnMark();
	}

	private void drawCrossGL() {
		gl.glPushMatrix();
		gl.glTranslatef(0, 0, -1);
		gl.glColor3f(1, 1, 1);
		gl.glBegin(GL2.GL_LINES);
		gl.glVertex3f(-1, 0, 0);
		gl.glVertex3f(1, 0, 0);
		gl.glVertex3f(0, -1, 0);
		gl.glVertex3f(0, 1, 0);
		gl.glEnd();
		gl.glPopMatrix();
	}

	private void takeScreenshot() {
		readPixels = false;
		Format format = Format.Format_RGB32;
		if (transparentShot) {
			transparentShot = false;
			format = Format.Format_ARGB32;
		}
		byte[] pixels = new byte[w * h * BPP];
		ByteBuffer buf = ByteBuffer.wrap(pixels);
		gl.glReadPixels(0, 0, w, h, GL2.GL_RGBA, GL2.GL_BYTE, buf);
		pixels = flipPixels(pixels);
		RGBAtoARGB(pixels);
		QImage img = new QImage(pixels, w, h, format);
		img.save(screenShotPath);
	}

	private byte[] flipPixels(byte[] imgPixels) {
		byte[] flippedPixels = new byte[imgPixels.length];
		int w4 = w * BPP;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w4; x += BPP) {
				for (int i = 0; i < BPP; i++) {
					flippedPixels[(h - y - 1) * w4 + x + i] = imgPixels[y * w4 + x + i];
				}
			}
		}

		return flippedPixels;
	}

	private byte[] RGBAtoARGB(byte[] pixels) {
		for (int i = 0; i < pixels.length; i += BPP) {
			byte r = pixels[i];
			pixels[i] = pixels[i + 2];// b;
			pixels[i + 2] = r;
		}

		return pixels;
	}

	private void drawText(String txt, float xShift, float yShift) {
		gl.glPushMatrix();
		float len = txt.length();
		float fScale = .03f;
		float tLen = (len - .5f * len) * fScale;
		tLen = 1.f - tLen - .01f;
		gl.glTranslatef(xShift * -tLen, yShift, 0);
		gl.glScalef(fScale, fScale, 1.f);
		gl.glColor3f(1, 1, 1);
		glFont.print(0, 0, txt);
		gl.glPopMatrix();
	}

	private float[] getLeftBottom(float near) {
		float[] lb = new float[2];
		lb[1] = (float) Math.tan(fov * Math.PI / 360.0) * near;
		lb[0] = aspect * lb[1];

		return lb;
	}

	private void drawIslands() {
		gl.glPushMatrix();
		List<MapRoom> island = islandRooms.get(currentIsland);
		gl.glTranslatef(dx, dy, dz);
		gl.glRotatef(xAngle, 1.f, .0f, .0f);
		gl.glRotatef(yAngle, .0f, 1.f, .0f);
		gl.glRotatef(zAngle, .0f, .0f, 1.f);
		gl.glRotatef(angle, 1.f, 1.f, 1.f);
		RoomCoords coords = island.get(0).getCoords();
		gl.glTranslatef(-.5f + xMiddle - coords.getX(), -.5f + yMiddle - coords.getY(), -.5f);
		gl.glPushName(-1);
		int i = 0;
		exitShift = 0;
		for (MapRoom mr : island) {
			RoomCoords rc = mr.getCoords();
			if (rc.getLayer() == currentLayer) {
				gl.glPushMatrix();
				gl.glTranslatef(rc.getX() * 2, rc.getY() * 2, rc.getZ() * 2);
				gl.glLoadName(i);
				gl.glPassThrough((float) i);
				if (selected == i && !selection) {
					gl.glColor4ubv(selectionColor, 0);
					if (reportSelected) {
						reportSelected = false;
						selectedVnum = mr.getRoom().getVnum();
						vnumSelected.emit(selectedVnum);
						// System.out.println("selected vnum: " + selectedVnum +
						// ", " + mr.getCoords());
						// for (ExitWrapper ex : mr.getMapRooms().keySet()) {
						// System.out.println("rev exit: " + ex.getVnum() +
						// " twoWay: " + ex.isTwoWay());
						// }
					}
					// } else if (hovered == i && !hovering) {
					// gl.glColor4ub((byte) 218, (byte) 218, (byte) 16, (byte)
					// (255 * ALPHA));
					// if (reportHovered) {
					// reportHovered = false;
					// System.out.println("hovered: " + mr.getRoom().getVnum());
					// }
				} else {
					if (i == 0) {
						gl.glColor4f(.0f, 1.f, .0f, ALPHA);
					} else {
						gl.glColor4fv(objectColor, 0);
					}
				}

				drawRoom();

				drawExits(mr);
				gl.glPopMatrix();

				i++;
			}
		}
		gl.glPopMatrix();
	}

	private void cleanDrawnMark() {
		List<MapRoom> island = islandRooms.get(currentIsland);
		for (MapRoom mr : island) {
			for (ExitWrapper exit : mr.getMapRooms().keySet()) {
				exit.setDrawn(false);
			}
		}
	}

	private void drawExits(MapRoom mr) {
		GLUquadric quadratic = glu.gluNewQuadric();
		glu.gluQuadricNormals(quadratic, GLU.GLU_SMOOTH);
		glu.gluQuadricTexture(quadratic, true);

		int shift;
		int i = 0;
		for (ExitWrapper exit : mr.getMapRooms().keySet()) {
			ExitWrapper revExit = exit.getRevExit();
			if ((revExit == null || !revExit.isDrawn()) && !exit.isDrawn()) {
				shift = currentIslandSize + exitShift;
				gl.glLoadName(shift);
				gl.glPushMatrix();

				switch (exit.getDirection()) {
				case 0: /* north */
					gl.glTranslatef(.5f, .5f, -1.f);
					break;

				case 1: /* east */
					gl.glRotatef(90, 0, 1, 0);
					gl.glTranslatef(-.5f, .5f, 1.f);
					break;

				case 2: /* south */
					gl.glTranslatef(.5f, .5f, 1.f);
					break;

				case 3: /* west */
					gl.glRotatef(90, 0, 1, 0);
					gl.glTranslatef(-.5f, .5f, -1.f);
					break;

				case 4: /* up */
					gl.glRotatef(90, 1, 0, 0);
					gl.glTranslatef(.5f, .5f, -2.f);
					break;

				case 5: /* down */
					gl.glRotatef(90, 1, 0, 0);
					gl.glTranslatef(.5f, .5f, .0f);
					break;

				case 6: /* north-east */
					gl.glRotatef(45, 0, -1, 0);
					gl.glTranslatef(.7f, .5f, -2.3f);
					gl.glScalef(1, 1, 1.8f);
					break;

				case 7: /* north-west */
					gl.glRotatef(45, 0, 1, 0);
					gl.glTranslatef(.0f, .5f, -1.6f);
					gl.glScalef(1, 1, 1.8f);
					break;

				case 8: /* south-east */
					gl.glRotatef(45, 0, 1, 0);
					gl.glTranslatef(.0f, .5f, 1.2f);
					gl.glScalef(1, 1, 1.8f);
					break;

				case 9: /* south-west */
					gl.glRotatef(45, 0, -1, 0);
					gl.glTranslatef(.7f, .5f, -2.3f);
					gl.glScalef(1, 1, 1.8f);
					break;

				default: /* somewhere: 10 */
					System.err.println("virtual exits not yet supported!");
					/* TODO: find a way to draw virtual exit */
				}

				if ((!drawDistantExits && !exit.isDistant()) || drawDistantExits) {
					if (!selection && selected == shift) {
						gl.glColor4ubv(selectionColor, 0);
						if (reportSelected) {
							reportSelected = false;
							selectedExit = exit;
							selectedVnum = mr.getRoom().getVnum();
							exitSelected.emit(selectedVnum, i);
						}
					} else {
						gl.glColor4fv(objectColor, 0);
					}

					if (exit.isTwoWay()) {
						glu.gluCylinder(quadratic, 0.2, 0.2, 1, 32, 32);
					} else {
						glu.gluCylinder(quadratic, 0.1, 0.1, 1, 32, 32);
					}
				}
				exit.setDrawn();

				gl.glPopMatrix();
			}
			exitShift++;
			i++;
		}

		glu.gluDeleteQuadric(quadratic);
	}

	private void drawRoom() {
		gl.glBegin(GL2.GL_QUADS);

		gl.glVertex3f(0, 0, 0);
		gl.glVertex3f(0, 1, 0);
		gl.glVertex3f(1, 1, 0);
		gl.glVertex3f(1, 0, 0);

		gl.glVertex3f(0, 0, 1);
		gl.glVertex3f(0, 1, 1);
		gl.glVertex3f(1, 1, 1);
		gl.glVertex3f(1, 0, 1);

		gl.glVertex3f(0, 0, 0);
		gl.glVertex3f(0, 1, 0);
		gl.glVertex3f(0, 1, 1);
		gl.glVertex3f(0, 0, 1);

		gl.glVertex3f(1, 0, 0);
		gl.glVertex3f(1, 1, 0);
		gl.glVertex3f(1, 1, 1);
		gl.glVertex3f(1, 0, 1);

		gl.glVertex3f(0, 0, 0);
		gl.glVertex3f(1, 0, 0);
		gl.glVertex3f(1, 0, 1);
		gl.glVertex3f(0, 0, 1);

		gl.glVertex3f(0, 1, 0);
		gl.glVertex3f(1, 1, 0);
		gl.glVertex3f(1, 1, 1);
		gl.glVertex3f(0, 1, 1);

		gl.glEnd();
	}

	private void drawWindRose() {
		gl.glPushMatrix();
		final float fScale = .1f;
		final float zRose = -2.f;
		float[] lb = getLeftBottom(zRose);
		gl.glTranslatef(lb[0] + fScale, lb[1] + fScale, zRose);
		gl.glScalef(fScale, fScale, fScale);
		gl.glRotatef(xAngle, 1.f, .0f, .0f);
		gl.glRotatef(yAngle, .0f, 1.f, .0f);
		gl.glRotatef(zAngle, .0f, .0f, 1.f);
		gl.glRotatef(angle, 1, 1, 1);

		gl.glBegin(GL2.GL_LINES);
		gl.glColor4f(1, 0, 0, 0.9f);
		gl.glVertex3f(0, 0, 0);
		gl.glVertex3f(1, 0, 0);

		gl.glColor4f(1, 1, 0, 0.9f);
		gl.glVertex3f(0, 0, 0);
		gl.glVertex3f(0, 1, 0);

		gl.glColor4f(0, 0, 1, 0.9f);
		gl.glVertex3f(0, 0, 0);
		gl.glVertex3f(0, 0, 1);
		gl.glEnd();

		gl.glPushMatrix();
		gl.glTranslatef(1, 0, 0);
		gl.glRotatef(90, 0, 1, 0);
		gl.glColor4f(1, 0, 0, 1);
		drawArrow();
		gl.glPopMatrix();

		gl.glPushMatrix();
		gl.glTranslatef(0, 1, 0);
		gl.glRotatef(90, -1, 0, 0);
		gl.glColor4f(1, 1, 0, 1);
		drawArrow();
		gl.glPopMatrix();

		gl.glPushMatrix();
		gl.glTranslatef(0, 0, 1);
		gl.glColor4f(0, 0, 1, 1);
		drawArrow();
		gl.glPopMatrix();
		gl.glPopMatrix();
	}

	private void drawArrow() {
		GLUquadric quadratic = glu.gluNewQuadric();
		glu.gluQuadricNormals(quadratic, GLU.GLU_SMOOTH);
		glu.gluQuadricTexture(quadratic, true);

		glu.gluCylinder(quadratic, 0.1, 0, 0.3, 32, 32);
		glu.gluDisk(quadratic, 0, 0.1, 32, 32);

		glu.gluDeleteQuadric(quadratic);
	}

	public void display(GLAutoDrawable d) {
		if (multisample) {
			gl.glEnable(GL2.GL_MULTISAMPLE);
		} else {
			gl.glDisable(GL2.GL_MULTISAMPLE);
		}
		if (selection) {
			select(cx, cy);
			selection = false;
		}
		// if (hovering) {
		// select(mx, my);
		// hovering = false;
		// }
		drawWorld();
	}

	public void dispose(GLAutoDrawable d) {
	}

	public void init(GLAutoDrawable d) {
		gl = d.getGL().getGL2();
		glu = GLU.createGLU(gl);

		gl.glEnable(GL2.GL_DEPTH_TEST);
		gl.glDepthFunc(GL2.GL_LEQUAL);
		gl.glEnable(GL2.GL_BLEND);
		gl.glBlendFunc(GL2.GL_SRC_ALPHA, GL2.GL_ONE_MINUS_SRC_ALPHA);
		gl.glEnable(GL2.GL_MULTISAMPLE);
		gl.glEnable(GL2.GL_TEXTURE_2D);
		gl.glHint(GL2.GL_PERSPECTIVE_CORRECTION_HINT, GL2.GL_NICEST);
		gl.glShadeModel(GL2.GL_SMOOTH);

		// if (1==1) {
		// gl.glEnable(GL2.GL_LIGHTING);
		// float[] ambient = { 1.f, 1.f, 1.f, 1.f };
		// gl.glLightModelfv(GL2.GL_LIGHT_MODEL_AMBIENT, ambient, 0);
		// gl.glEnable(GL2.GL_COLOR_MATERIAL);
		// gl.glColorMaterial(GL2.GL_FRONT, GL2.GL_AMBIENT_AND_DIFFUSE);
		// }

		// int[] buf = new int[1];
		// gl.glGetIntegerv(GL2.GL_SAMPLE_BUFFERS, buf, 0);
		// System.out.println("sampleBuffers: " + buf[0]);
		// gl.glGetIntegerv(GL2.GL_SAMPLES, buf, 0);
		// System.out.println("samples:" + buf[0]);

		Font f = new Font(Font.SERIF, 0, 16);
		if (f != null) {
			glFont = new GLFont(f, gl, 0, 0);
		}
	}

	public void reshape(GLAutoDrawable d, int x, int y, int w, int h) {
		this.w = w;
		if (h == 0) {
			h = 1;
		}
		this.h = h;

		aspect = w / h;

		update();
	}

	private void update() {
		gl.glViewport(0, 0, w, h);

		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glLoadIdentity();

		glu.gluPerspective(fov, aspect, near, far);
	}

	public void setOrthoOn() {
		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPushMatrix();
		gl.glLoadIdentity();
		gl.glOrtho(0, 0, 0, 0, -1, 1);
		gl.glMatrixMode(GL2.GL_MODELVIEW);
		gl.glPushMatrix();
		gl.glLoadIdentity();
		gl.glDisable(GL2.GL_DEPTH_TEST);
	}

	public void setOrthoOff() {
		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPopMatrix();
		gl.glMatrixMode(GL2.GL_MODELVIEW);
		gl.glPopMatrix();
		gl.glEnable(GL2.GL_DEPTH_TEST);
	}
}
