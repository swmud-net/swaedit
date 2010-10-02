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

	private int cx, cy;
	private int selected = -1;
	private boolean reportSelected;
	private boolean selection;
	private BigInteger selectedVnum;

	private int w, h;
	private float fov = 60f;
	private float aspect;
	private float near = 1.0f;
	private float far = 100.0f;

	private boolean multisample = true;
	private static final float ALPHA = .5f;

	private float dz = -31;
	private float rot = 0;
	private float angle;
	private float dx, dy;
	private double oldx, oldy;
	private float rx, ry, rz;

	private SortedMap<Integer, List<MapRoom>> islandRooms;
	private float xMiddle, yMiddle;
	private int currentIsland = 0;
	private final int maxIslands;
	private int currentLayer = 0;
	private int maxLayer;

	private boolean drawDistantExits;

	private GLFont glFont;

	private int mx, my;
	private int hovered = -1;
	private boolean reportHovered;
	private boolean hovering;
	private BigInteger hoveredVnum;

	private boolean drawCross;
	private boolean showHelp;
	private int showIslandsLayers;
	private int fulscreenWidth, fulscreenHeight;

	private int bpp = 4; /* bytes per pixel */

	private boolean readPixels;
	private boolean transparentShot;
	private String screenShotPath;
	private File screenShotDir;
	private String screenShotBareFileName;
	private long screenShotCnt;
	private static final String USER_HOME = System.getProperty("user.home");

	private static final List<String> MENU_KEYS = new ArrayList<String>() {
		private static final long serialVersionUID = 1041444819386431807L;
		{
			add("d     - toggle distant exits");
			add("m     - toggle multisampling");
			add("space - toggle rotation");
			add("h     - toggle this help text");
			add("k     - toggle cross");
			add("c     - reset to center");
			add("f     - toggle fullscreen");
			add("up    - next island");
			add("down  - previous island");
			add("right - next layer");
			add("left  - previous layer");
			add("F12   - take a screenshot");
			add("F11   - take a transparent screenshot");
		}
	};

	// private final Signal0 s0 = new Signal0();

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
				System.exit(0);
				// s0.connect(SWAEdit.ref, "close()");
				// s0.emit();
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
				}
			}

			@Override
			public void mouseMoved(MouseEvent e) {
				int px = e.getX();
				int py = e.getY();
				float[] mPos = translateMouse(px, py, w, h);
				mx = px;// mPos[0]*Math.abs(dz);
				my = py;// mPos[1]*Math.abs(dz);
//				hovering = true;
//				reportHovered = true;
			}
		});

		animator = new FPSAnimator[2];
		animator[0] = new FPSAnimator(window, 30);
		animator[1] = new FPSAnimator(window, 30);

		window.addKeyListener(new MapKeyHandler(this, currentIsland, this.maxIslands));
		window.addGLEventListener(this);
	}

	private void setupIsland(int islandNo) {
		currentIsland = islandNo;
		selectBufLen = islandRooms.get(islandNo).size() * 4 + 5;
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
		dz = -31;
		rot = dx = dy = angle = 0;
		oldx = oldy = 0;
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

	private float[] translateMouse(int x, int y, int w, int h) {
		return new float[] { -(1.f - ((float) x * 2.f / (float) w)), 1.f - ((float) y * 2.f / (float) h) };
	}

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
			}
			if (hovering) {
				hovered = -1;
			}
		}

		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPopMatrix();
		gl.glMatrixMode(GL2.GL_MODELVIEW);
	}

	private void processHits(int hits, int[] sb) {
		float minz = Integer.MAX_VALUE;
		int chosen = -1;
		for (int i = 0; i < hits * 4; i += 4) {
			if (sb[i] == 1) {
				float z = sb[i + 1];
				z /= Integer.MAX_VALUE;
				if (z < minz) {
					minz = z;
					chosen = sb[i + 3];
				}
			}
		}

		if (selection) {
			selected = chosen;
		}
		if (hovering) {
			hovered = chosen;
		}
	}

	private void drawWorld() {
		if (animator[0].isAnimating()) {
			rot += 0.002;
			if (rot > Math.PI / 2) {
				rot = 0;
			}
			angle = 360 * Math.abs((float) Math.sin(rot));
		}

		gl.glClear(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_DEPTH_BUFFER_BIT);
		gl.glMatrixMode(GL2.GL_MODELVIEW);
		gl.glInitNames();

		gl.glLoadIdentity();

		gl.glPushMatrix();
		drawIslands();
		gl.glPopMatrix();

		gl.glPushMatrix();
		drawWindRose();
		gl.glPopMatrix();

		if (drawCross) {
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

		if (selected >= 0 && selectedVnum != null || showHelp || showIslandsLayers > 0) {
			gl.glPushName(-2);
			gl.glPushMatrix();
			gl.glPushAttrib(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_TEXTURE_BIT | GL2.GL_DEPTH_BUFFER_BIT);
			setOrthoOn();

			if (selected >= 0 && selectedVnum != null) {
				drawText("vnum selected: " + selectedVnum.toString(), -1.f, -.98f);
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
			readPixels = false;
			Format format = Format.Format_RGB32;
			if (transparentShot) {
				transparentShot = false;
				format = Format.Format_ARGB32;
			}
			byte[] pixels = new byte[w * h * bpp];
			ByteBuffer buf = ByteBuffer.wrap(pixels);
			gl.glReadPixels(0, 0, w, h, GL2.GL_RGBA, GL2.GL_BYTE, buf);
			pixels = flipPixels(pixels);
			RGBAtoARGB(pixels);
			QImage img = new QImage(pixels, w, h, format);
			img.save(screenShotPath);
		}

		cleanDrawnMark();
	}

	private byte[] flipPixels(byte[] imgPixels) {
		byte[] flippedPixels = new byte[imgPixels.length];
		int w4 = w * bpp;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w4; x += bpp) {
				for (int i = 0; i < bpp; i++) {
					flippedPixels[(h - y - 1) * w4 + x + i] = imgPixels[y * w4 + x + i];
				}
			}
		}

		return flippedPixels;
	}

	private byte[] RGBAtoARGB(byte[] pixels) {
		for (int i = 0; i < pixels.length; i += bpp) {
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
		List<MapRoom> island = islandRooms.get(currentIsland);
		gl.glTranslatef(dx, dy, dz);
		gl.glRotatef(angle, 1, 1, 1);
		RoomCoords coords = island.get(0).getCoords();
		gl.glTranslatef(-.5f + xMiddle - coords.getX(), -.5f + yMiddle - coords.getY(), -.5f);
		gl.glPushName(-1);
		int i = 0;
		for (MapRoom mr : island) {
			RoomCoords rc = mr.getCoords();
			if (rc.getLayer() == currentLayer) {
				gl.glPushMatrix();
				gl.glTranslatef(rc.getX() * 2, rc.getY() * 2, rc.getZ() * 2);
				gl.glLoadName(i);
				if (selected == i && !selection) {
					gl.glColor4ub((byte) 218, (byte) 168, (byte) 16, (byte) (255 * ALPHA));
					if (reportSelected) {
						reportSelected = false;
						selectedVnum = mr.getRoom().getVnum();
						// System.out.println("selected vnum: " + selectedVnum +
						// ", " + mr.getCoords());
						// for (ExitWrapper ex : mr.getMapRooms().keySet()) {
						// System.out.println("rev exit: " + ex.getVnum() +
						// " twoWay: " + ex.isTwoWay());
						// }
					}
				} else if (hovered == i && !hovering) {
					gl.glColor4ub((byte) 218, (byte) 218, (byte) 16, (byte) (255 * ALPHA));
					if (reportHovered) {
						reportHovered = false;
	                    System.out.println("hovered: "+mr.getRoom().getVnum());
                    }
				} else {
					if (i == 0) {
						gl.glColor4f(0, 1, 0, ALPHA);
					} else {
						gl.glColor4f(0, 0.7f, 0, ALPHA);
					}
				}

				drawRoom();

				gl.glColor4f(0, 0.7f, 0, ALPHA);
				drawExits(mr);
				gl.glPopMatrix();

				i++;
			}
		}
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

		for (ExitWrapper exit : mr.getMapRooms().keySet()) {
			ExitWrapper revExit = exit.getRevExit();
			if ((revExit == null || !revExit.isDrawn()) && !exit.isDrawn()) {
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
					if (exit.isTwoWay()) {
						glu.gluCylinder(quadratic, 0.2, 0.2, 1, 32, 32);
					} else {
						glu.gluCylinder(quadratic, 0.1, 0.1, 1, 32, 32);
					}
				}
				exit.setDrawn();

				gl.glPopMatrix();
			}
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
		float zRose = -20;
		float[] lb = getLeftBottom(zRose);
		gl.glTranslatef(lb[0] + 1, lb[1] + 1, zRose);
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
		if (hovering) {
			select(mx, my);
			hovering = false;
		}
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
		} else {
			System.err.println("font is null!");
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
