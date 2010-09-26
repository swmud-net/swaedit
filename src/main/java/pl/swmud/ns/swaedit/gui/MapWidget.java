package pl.swmud.ns.swaedit.gui;


import java.awt.Font;
import java.awt.FontFormatException;
import java.io.File;
import java.io.IOException;
import java.math.BigInteger;
import java.nio.IntBuffer;
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

public class MapWidget extends QSignalEmitter implements GLEventListener, MapKeyHandler.Minion {
	private int selectBufLen;

	private static final float SENSITIVITY = 0.06f;
	private final GLWindow window;
	private Animator[] animator;

	private GL2 gl;
	private GLU glu;

	private int cx, cy;
	private int selected = -1;
	private boolean reportSelected = false;
	private boolean selection = false;
	private BigInteger selectedVnum;

	private int w, h;
	private float fov = 60f;
	private float aspect;
	private float near = 1.0f;
	private float far = 100.0f;

	private boolean multisample = true;
	private float alpha = 0.5f;

	private float dz = -31;
	private float rot = 0;
	private float angle;
	private float dx, dy;
	private double oldx, oldy;

	private SortedMap<Integer, List<MapRoom>> islandRooms;
	private float xMiddle, yMiddle;
	private int currentIsland = 0;
	private final int maxIslands;
	private int currentLayer = 0;
	private int maxLayers;

	private boolean drawDistantExits = false;

	private GLFont glFont;
	private float mx, my;
	boolean drawCross = false;
	boolean showHelp = false;


//	private final Signal0 s0 = new Signal0();

	public MapWidget(SortedMap<Integer, List<MapRoom>> islandRooms, int currentIsland, int maxIslands) {
		this.islandRooms = islandRooms;
		this.maxIslands = maxIslands;
		setupIsland(currentIsland);

		GLProfile profile = GLProfile.get(GLProfile.GL2);
		GLCapabilities caps = new GLCapabilities(profile);
		caps.setSampleBuffers(true);
		caps.setNumSamples(4);
		caps.setAlphaBits(8);

		window = GLWindow.create(caps);
//		window.enablePerfLog(true);
		window.setSize(800, 600);
		window.addWindowListener(new WindowAdapter() {
			@Override
			public void windowDestroyNotify(WindowEvent e) {
				super.windowDestroyNotify(e);
//				System.exit(0);
//				s0.connect(SWAEdit.ref, "close()");
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
				float[] ret = translateMouse(e.getX(), e.getY(), w, h);
				mx = ret[0];
				my = ret[1];
			}
		});

		animator = new FPSAnimator[2];
		animator[0] = new FPSAnimator(window, 30);
		animator[1] = new FPSAnimator(window, 30);

		window.addKeyListener(new MapKeyHandler(this, currentIsland, this.maxIslands));
		window.addGLEventListener(this);
	}

	public void setCurrentIsland(int islandNo) {
		setupIsland(islandNo);
	}

	private void setupIsland(int islandNo) {
		currentIsland = islandNo;
		selectBufLen = islandRooms.get(islandNo).size() * 4;
		xMiddle = getXMiddle(islandNo);
		yMiddle = getYMiddle(islandNo);
		selected = -1;
		for (MapRoom mr : islandRooms.get(islandNo)) {
			int layer = mr.getCoords().getLayer();
			if (layer > maxLayers) {
				maxLayers = layer;
			}
		}
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
		}
	}

	public void incLayer() {
		if (currentLayer < maxLayers - 1) {
			++currentLayer;
			selected = -1;
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

	public void show(int x, int y) {
		if (!window.isVisible()) {
			window.setPosition(x, y);
			window.setVisible(true);
			animator[0].start();
		}
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
		float[] p = new float[2];

//		p[0] = (2.0f * x - w) / h;
//		p[1] = 1.0f - (2.0f * y) / h;

		p[0] = x*2.0f/w;
		p[1] = -y*2.0f/h;

		return p;
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

		int[] sb = new int[selectBufLen];
		selectBuf.get(sb, 0, hits * 4);
		if (hits > 0) {
			processHits(hits, sb);
		} else {
			selected = -1;
		}

		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPopMatrix();
		gl.glMatrixMode(GL2.GL_MODELVIEW);
	}

	private void processHits(int hits, int[] sb) {
		float minz = Integer.MAX_VALUE;
		selected = 0;
		for (int i = 0; i < hits * 4; i += 4) {
			float z = sb[i + 1];
			z /= Integer.MAX_VALUE;
			if (z < minz) {
				minz = z;
				selected = sb[i + 3];
			}
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

		if (!selection && (selected >= 0 && selectedVnum != null || showHelp)) {
			gl.glPushMatrix();
			gl.glPushAttrib(GL2.GL_COLOR_BUFFER_BIT | GL2.GL_TEXTURE_BIT | GL2.GL_DEPTH_BUFFER_BIT);
			setOrthoOn();
			
			if (!selection && selected >= 0 && selectedVnum != null) {
				drawText("vnum selected: " + selectedVnum.toString(), -1.f, -.98f);
            }
			
			if (showHelp) {
				drawHelp();
            }

			setOrthoOff();
			gl.glPopAttrib();
			gl.glPopMatrix();
		}
		
		gl.glFlush();

		cleanDrawnMark();
	}
	
	private void drawHelp() {
		gl.glPushMatrix();
		
		drawText("d     - toggle distant exits", 1.f, .98f);
		drawText("m     - toggle multisampling", 1.f, .94f);
		drawText("space - toggle rotation", 1.f, .90f);
		drawText("h     - toggle this help text",1.f,  .86f);
		drawText("k     - toggle cross", 1.f, .82f);
		drawText("c     - reset to center", 1.f, .78f);

		gl.glPopMatrix();
	}
	
	private void drawText(String txt, float xShift, float yShift) {
		gl.glPushMatrix();
		float len = txt.length();
		float fScale = .03f;
		float tLen = (len - .5f * len) * fScale;
		tLen = 1.f - tLen - .02f;
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
				if (selected == i++) {
					gl.glColor4ub((byte) 218, (byte) 168, (byte) 16, (byte) (255 * alpha));
					if (reportSelected && !selection) {
						reportSelected = false;
						selectedVnum = mr.getRoom().getVnum();
//						System.out.println("selected vnum: " + selectedVnum + ", " + mr.getCoords());
//						for (ExitWrapper ex : mr.getMapRooms().keySet()) {
//							System.out.println("rev exit: " + ex.getVnum() + " twoWay: " + ex.isTwoWay());
//						}
					}
				} else {
					if (i == 1) {
						gl.glColor4f(0, 1, 0, alpha);
					} else {
						gl.glColor4f(0, 0.7f, 0, alpha);
					}
				}

				drawRoom();

				gl.glColor4f(0, 0.7f, 0, alpha);
				drawExits(mr);
				gl.glPopMatrix();
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
		
		// int[] buf = new int[1];
		// gl.glGetIntegerv(GL2.GL_SAMPLE_BUFFERS, buf, 0);
		// System.out.println("sampleBuffers: " + buf[0]);
		// gl.glGetIntegerv(GL2.GL_SAMPLES, buf, 0);
		// System.out.println("samples:" + buf[0]);
		
		Font f = null;
		try {
	        f = Font.createFont(Font.TRUETYPE_FONT, new File("font.ttf"));
	        f = f.deriveFont(0, 12);
        } catch (Exception e) {
	        e.printStackTrace();
	        f = new Font(Font.SERIF, 0, 12);
        }
        f = new Font(Font.SERIF, 0, 16);
        if (f != null) {
			glFont = new GLFont(f, gl, 0, 0);
        } else {
        	System.err.println("font is null!");
        }
	}

	public void reshape(GLAutoDrawable d, int x, int y, int w, int h) {
		this.w = w;
		this.h = h;
		if (h == 0) {
			h = 1;
		}
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
