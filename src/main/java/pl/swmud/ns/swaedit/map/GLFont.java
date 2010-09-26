package pl.swmud.ns.swaedit.map;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.awt.image.PixelGrabber;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.HashMap;
import java.util.Map;

import javax.media.opengl.GL2;

import com.jogamp.common.nio.Buffers;

/**
 * GLFont uses the Java Font class to create character sets and render text.
 * GLFont can generate text in nearly any size, and in any font that Java
 * supports.
 * <P>
 */
public class GLFont {
	private static final int MAX_CHARS = 100;
	private GL2 gl;
	private Font font;
	private int fontListBase = -1;
	private int fontTextureHandle = 0;
	private int fontSize = 0;
	private int textureSize = 0;
	private int viewPortW;
	private int viewPortH;
	private int[] charwidths = new int[MAX_CHARS];
	private FontMetrics fontMetrics;
	int[] pxs;

	/**
	 * Dynamically create a texture mapped character set with the given Font.
	 * Text color will be white on a transparent background.
	 * 
	 * @param f
	 *            Java Font object
	 */
	public GLFont(Font f, GL2 gl, int viewPortW, int viewPortH) {
		this.gl = gl;
		this.viewPortW = viewPortW;
		this.viewPortH = viewPortH;
		this.font = f;
		this.fontSize = getFontSize();

		makeFont(new float[] { 1, 1, 1, 1 }, new float[] { 0, 0, 0, 0 });
	}

	/**
	 * Create a texture mapped character set with the given Font, Text color and
	 * background color.
	 * 
	 * @param f
	 *            Java Font object
	 * @param fgColor
	 *            foreground (text) color as rgb or rgba values in range 0-1
	 * @param bgColor
	 *            background color as rgb or rgba values in range 0-1 (set alpha
	 *            to 0 to make transparent)
	 */
	// public GLFont(Font f, float[] fgColor, float[] bgColor) {
	// makeFont(f, fgColor, bgColor);
	// }

	/**
	 * Return the handle to the texture holding the character set.
	 */
	public int getTexture() {
		return fontTextureHandle;
	}

	/**
	 * Prepare a texture mapped character set with the given Font, text color
	 * and background color. Characters will be textured onto quads and stored
	 * in display lists. The base display list id is stored in the fontListBase
	 * variable. After makeFont() is run the print() function can be used to
	 * render text in this font.
	 * 
	 * @param f
	 *            the font to draw characters
	 * @param fgColor
	 *            foreground (text) color as rgb or rgba values in range 0-1
	 * @param bgColor
	 *            background color as rgb or rgba values in range 0-1 (set alpha
	 *            to 0 to make transparent)
	 * 
	 * @see createFontImage()
	 * @see print()
	 */
	public void makeFont(float[] fgColor, float[] bgColor) {
		int charsetTexture = 0;
		if ((charsetTexture = makeFontTexture(fgColor, bgColor)) > 0) {
			// create MAX_CHARS display lists, one for each character
			// textureSize and fontSize are calculated by createFontImage()
			buildFont(charsetTexture, textureSize, fontSize);
			fontTextureHandle = charsetTexture;
		}
	}

	/**
	 * Find a power of two equal to or greater than the given value. Ie.
	 * getPowerOfTwoBiggerThan(800) will return 1024.
	 * <P>
	 * 
	 * @see makeTextureForScreen()
	 * @param dimension
	 * @return a power of two equal to or bigger than the given dimension
	 */
	public int getPowerOfTwoBiggerThan(int n) {
		if (n < 0)
			return 0;
		--n;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		return n + 1;
	}

	/**
	 * Returns true if n is a power of 2. If n is 0 return false.
	 */
	public boolean isPowerOf2(int n) {
		if (n == 0) {
			return false;
		}
		return (n & (n - 1)) == 0;
	}

	/**
	 * Return the Image pixels in default Java int ARGB format.
	 * 
	 * @return
	 */
	public int[] getImagePixels(Image image) {
		int[] pixels = null;
		if (image != null) {
			int imgw = image.getWidth(null);
			int imgh = image.getHeight(null);
			pixels = new int[imgw * imgh];
			PixelGrabber pg = new PixelGrabber(image, 0, 0, imgw, imgh, pixels, 0, imgw);
			try {
				pg.grabPixels();
			} catch (InterruptedException e) {
				e.printStackTrace();
				return null;
			}
		}
		return pixels;
	}

	/**
	 * Convert ARGB pixels to a ByteBuffer containing RGBA pixels. The GL_RGBA
	 * format is a default format used in OpenGL 1.0, but requires that we move
	 * the Alpha byte for each pixel in the image (slow). Would be better to use
	 * OpenGL 1.2 GL_BGRA format and leave pixels in the ARGB format (faster)
	 * but this pixel format caused problems when creating mipmaps (see note
	 * above). .
	 * <P>
	 * If flipVertically is true, pixels will be flipped vertically (for OpenGL
	 * coord system).
	 * 
	 * @return ByteBuffer
	 */
	public ByteBuffer convertImagePixelsRGBA(int[] pixels) {
		byte[] bytes = convertARGBtoRGBA(pixels);
		return Buffers.newDirectByteBuffer(bytes);
	}

	/**
	 * Flip an array of pixels vertically
	 * 
	 * @param imgPixels
	 * @param imgw
	 * @param imgh
	 * @return int[]
	 */
	public static int[] flipPixels(int[] imgPixels, int imgw, int imgh) {
		int[] flippedPixels = null;
		if (imgPixels != null) {
			flippedPixels = new int[imgw * imgh];
			for (int y = 0; y < imgh; y++) {
				for (int x = 0; x < imgw; x++) {
					flippedPixels[((imgh - y - 1) * imgw) + x] = imgPixels[(y * imgw) + x];
				}
			}
		}
		return flippedPixels;
	}

	/**
	 * Convert pixels from java default ARGB int format to byte array in RGBA
	 * format.
	 * 
	 * @param pixels
	 * @return
	 */
	public byte[] convertARGBtoRGBA(int[] pixels) {
		byte[] bytes = new byte[pixels.length * 4]; // will hold pixels as RGBA
		// bytes
		int p, r, g, b, a;
		int j = 0;
		for (int i = 0; i < pixels.length; i++) {
			p = pixels[i];
			a = (p >> 24) & 0xFF;
			r = (p >> 16) & 0xFF;
			g = (p >> 8) & 0xFF;
			b = (p >> 0) & 0xFF;
			bytes[j + 0] = (byte) r;
			bytes[j + 1] = (byte) g;
			bytes[j + 2] = (byte) b;
			bytes[j + 3] = (byte) a;
			j += 4;
		}
		return bytes;
	}

	/**
	 * Create a texture from the given pixels in the default Java ARGB int
	 * format.<BR>
	 * Configure the texture to repeat in both directions and use LINEAR for
	 * magnification.
	 * <P>
	 * 
	 * @return the texture handle
	 */
	public int makeTexture(int[] pixels, int w, int h, boolean anisotropic) {
		if (pixels != null) {
			pixels = flipPixels(pixels, w, h);
			ByteBuffer pixelsRGBA = convertImagePixelsRGBA(pixels);
			return makeTexture(pixelsRGBA, w, h, anisotropic);
		}
		return 0;
	}

	/**
	 * Return true if the OpenGL context supports the given OpenGL extension.
	 */
	public boolean extensionExists(String extensionName) {
		Map<String, String> glExtensions = new HashMap<String, String>();
		String[] GLExtensions = gl.glGetString(GL2.GL_EXTENSIONS).split(" ");
		for (int i = 0; i < GLExtensions.length; i++) {
			glExtensions.put(GLExtensions[i].toUpperCase(), "");
		}
		return (glExtensions.get(extensionName.toUpperCase()) != null);
	}

	/**
	 * Create a texture from the given pixels in the default OpenGL RGBA pixel
	 * format. Configure the texture to repeat in both directions and use LINEAR
	 * for magnification.
	 * <P>
	 * 
	 * @return the texture handle
	 */
	public int makeTexture(ByteBuffer pixels, int w, int h, boolean anisotropic) {
		// get a new empty texture
		IntBuffer textureHandleA = Buffers.newDirectIntBuffer(1);
		gl.glGenTextures(1, textureHandleA);
		int textureHandle = textureHandleA.get(0);
		// preserve currently bound texture, so glBindTexture() below won't
		// affect anything)
		gl.glPushAttrib(GL2.GL_TEXTURE_BIT);
		// 'select' the new texture by it's handle
		gl.glBindTexture(GL2.GL_TEXTURE_2D, textureHandle);
		// set texture parameters
		// gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_WRAP_S,
		// GL2.GL_REPEAT);
		// gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_WRAP_T,
		// GL2.GL_REPEAT);
		gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MAG_FILTER, GL2.GL_LINEAR); // GL2.GL_NEAREST);
		gl.glTexParameteri(GL2.GL_TEXTURE_2D, GL2.GL_TEXTURE_MIN_FILTER, GL2.GL_LINEAR); // GL2.GL_NEAREST);

		// make texture "anisotropic" so it will minify more gracefully
		if (anisotropic && extensionExists("GL_EXT_texture_filter_anisotropic")) {
			// Due to LWJGL buffer check, you can't use smaller sized buffers
			// (min_size = 16 for glGetFloat()).
			FloatBuffer max_a = Buffers.newDirectFloatBuffer(16);
			// Grab the maximum anisotropic filter.
			gl.glGetFloatv(GL2.GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, max_a);
			// Set up the anisotropic filter.
			gl.glTexParameterf(GL2.GL_TEXTURE_2D, GL2.GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, max_a.get(0));
		}

		// Create the texture from pixels
		gl.glTexImage2D(GL2.GL_TEXTURE_2D, 0, GL2.GL_RGBA, w, h, 0, GL2.GL_RGBA, GL2.GL_UNSIGNED_BYTE, pixels);

		// restore previous texture settings
		gl.glPopAttrib();

		return textureHandle;
	}

	/**
	 * Return a texture containing a character set with the given Font arranged
	 * in a 10x10 grid of printable characters.
	 * 
	 * @param f
	 *            the font to draw characters
	 * @param fgColor
	 *            foreground (text) color as rgb or rgba values in range 0-1
	 * @param bgColor
	 *            background color as rgb or rgba values in range 0-1 (set alpha
	 *            to 0 to make transparent)
	 * @see createFontImage()
	 * @see print()
	 */
	public int makeFontTexture(float[] fgColor, float[] bgColor) {
		int texture = 0;
		try {
			// Create a BufferedImage containing a 10x10 grid of printable
			// characters
			BufferedImage image = createFontImage( // the font
			        fgColor, // text color
			        bgColor); // background color
			// make a texture with the buffered image
			int[] pixels = getImagePixels(image);
			texture = makeTexture(pixels, image.getWidth(), image.getHeight(), false);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return texture;
	}

	/**
	 * Return a BufferedImage containing MAX_CHARS printable characters drawn
	 * with the given font. Characters will be arranged in a 10x10 grid.
	 * 
	 * @param text
	 * @param font
	 * @param imgSize
	 *            a power of two (32 64 256 etc)
	 * @param fgColor
	 *            foreground (text) color as rgb or rgba values in range 0-1
	 * @param bgColor
	 *            background color as rgb or rgba values in range 0-1 (set alpha
	 *            to 0 to make transparent)
	 * @return
	 */
	public BufferedImage createFontImage(float[] fgColor, float[] bgColor) {
		Color bg = bgColor == null ? new Color(0, 0, 0, 0) : (bgColor.length == 3 ? new Color(bgColor[0], bgColor[1],
		        bgColor[2], 1) : new Color(bgColor[0], bgColor[1], bgColor[2], bgColor[3]));
		Color fg = fgColor == null ? new Color(1, 1, 1, 1) : (fgColor.length == 3 ? new Color(fgColor[0], fgColor[1],
		        fgColor[2], 1) : new Color(fgColor[0], fgColor[1], fgColor[2], fgColor[3]));
		boolean isAntiAliased = true;
		boolean usesFractionalMetrics = false;

		// get size of texture image neaded to hold 10x10 character grid
		textureSize = getPowerOfTwoBiggerThan(fontSize * 10);
//		System.out.println("GLFont.getFontImageSize(): build font with fontsize=" + fontSize + " gridsize="
//		        + (fontSize * 10) + " texturesize=" + textureSize);
		if (textureSize > 2048) {
			System.err.println("GLFont.createFontImage(): texture size will be too big (" + textureSize
			        + ") Make the font size smaller.");
			return null;
		}

		// create a buffered image to hold charset
		BufferedImage image = new BufferedImage(textureSize, textureSize, BufferedImage.TYPE_INT_ARGB);
		Graphics2D g = image.createGraphics();

		// Clear image with background color (make transparent if color has
		// alpha value)
		if (bg.getAlpha() < 255) {
			g.setComposite(AlphaComposite.getInstance(AlphaComposite.CLEAR, (float) bg.getAlpha() / 255f));
		}
		g.setColor(bg);
		g.fillRect(0, 0, textureSize, textureSize);

		// prepare to draw characters in foreground color
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f));
		g.setColor(fg);
		g.setFont(font);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, isAntiAliased ? RenderingHints.VALUE_TEXT_ANTIALIAS_ON
		        : RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		g.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
		        usesFractionalMetrics ? RenderingHints.VALUE_FRACTIONALMETRICS_ON
		                : RenderingHints.VALUE_FRACTIONALMETRICS_OFF);
		g.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);

		// get font measurements
		int ascent = fontMetrics.getMaxAscent();

		// draw the grid of MAX_CHARS characters
		for (int r = 0; r < 10; r++) {
			for (int c = 0; c < 10; c++) {
				char ch = (char) (32 + ((r * 10) + c));
				int x = c * fontSize;
				int chWidth = fontMetrics.charWidth(ch);
				x += fontSize / 2 - chWidth / 2;
				g.drawString(String.valueOf(ch), x, (r * fontSize) + ascent);
				charwidths[(r * 10) + c] = chWidth;
			}
		}
		g.dispose();

//		Frame f = new Test(image);
//		f.setSize(textureSize, textureSize + 20);
//		f.setVisible(true);
		return image;
	}

	/**
	 * Return the maximum character size of the given Font. This will be the max
	 * of the vertical and horizontal font dimensions, so can be used to create
	 * a square image large enough to hold any character rendered with this
	 * Font.
	 * <P>
	 * Creates a BufferedImage and Graphics2D graphics context to get font sizes
	 * (is there a more efficient way to do this?).
	 * <P>
	 * 
	 * @param font
	 *            Font object describes the font to render with
	 * @return power-of-two texture size large enough to hold the character set
	 */
	public int getFontSize() {
		boolean isAntiAliased = true;
		boolean usesFractionalMetrics = false;

		// just a dummy image so we can get a graphics context
		BufferedImage image = new BufferedImage(64, 64, BufferedImage.TYPE_INT_ARGB);
		Graphics2D g = image.createGraphics();

		// prepare to draw character
		g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 1.0f));
		g.setFont(font);
		g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, isAntiAliased ? RenderingHints.VALUE_TEXT_ANTIALIAS_ON
		        : RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		g.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS,
		        usesFractionalMetrics ? RenderingHints.VALUE_FRACTIONALMETRICS_ON
		                : RenderingHints.VALUE_FRACTIONALMETRICS_OFF);
		g.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);

		// get character measurements
		fontMetrics = g.getFontMetrics();
		int ascent = fontMetrics.getMaxAscent();
		int descent = fontMetrics.getMaxDescent();
		int leading = fontMetrics.getLeading();
		int fontWidth = fontMetrics.charWidth('W'); // width of widest char, more reliable
		// than getMaxAdvance();
		
		// calculate size of 10x10 character grid
		int fontHeight = ascent + descent + (leading / 2);
		int maxCharSize = Math.max(fontHeight, fontWidth);
		return maxCharSize;
	}

	/**
	 * Build the character set display list from the given texture. Creates one
	 * quad for each character, with one letter textured onto each quad. Assumes
	 * the texture is a 256x256 image containing every character of the charset
	 * arranged in a 16x16 grid. Each character is 16x16 pixels. Call
	 * destroyFont() to release the display list memory.
	 * 
	 * Should be in ORTHO (2D) mode to render text (see setOrtho()).
	 * 
	 * Special thanks to NeHe and Giuseppe D'Agata for the "2D Texture Font"
	 * tutorial (http://nehe.gamedev.net).
	 * 
	 * @param charSetImage
	 *            texture image containing MAX_CHARS characters in a 10x10 grid
	 * @param fontWidth
	 *            how many pixels to allow per character on screen
	 * 
	 * @see destroyFont()
	 */
	public void buildFont(int fontTxtrHandle, int textureSize, int fontSize) {
		fontListBase = gl.glGenLists(MAX_CHARS);

		float fsize = 1.f;
		float txSize = textureSize;
		float usize = (float) fontSize / txSize;
		float halfUsize = usize * .5f;
		float txSize2 = txSize * 2.f;
		float p = halfUsize - (float) charwidths['W'-32] / txSize2 - .002f;
		float q = (fontSize - font.getSize2D()) / txSize - .001f;
		float r = Math.min(p, q);
		
		
		for (int i = 0; i < MAX_CHARS; i++) {
			int x = (i % 10);
			int y = (i / 10);

			float chU = (float) (x * fontSize) / txSize;
			float chV = (float) (textureSize - (y * fontSize) - fontSize) / txSize;

			gl.glNewList(fontListBase + i, GL2.GL_COMPILE);


			gl.glColor3f(1, 1, 1);
			gl.glBegin(GL2.GL_QUADS);

			gl.glTexCoord2f(chU + r, chV + r);
			gl.glVertex2f(0, 0);

			gl.glTexCoord2f(chU + usize - r, chV + r);
			gl.glVertex2f(fsize, 0);

			gl.glTexCoord2f(chU + usize - r, chV + usize - r);
			gl.glVertex2f(fsize, fsize);

			gl.glTexCoord2f(chU + r, chV + usize - r);
			gl.glVertex2f(0, fsize);

			gl.glEnd();
			gl.glEndList();
		}
	}

	/**
	 * Clean up the allocated display lists for the character set.
	 */
	public void destroyFont() {
		if (fontListBase != -1) {
			gl.glDeleteLists(fontListBase, 256);
			fontListBase = -1;
		}
	}

	/**
	 * Set OpenGL to render in flat 2D (no perspective) on top of current scene.
	 * Preserve current projection and model views, and disable depth testing.
	 * Ortho world size will be same as viewport size, so any ortho drawing
	 * (drawQuad(), drawImageFullscreen(), etc.) will be scaled to fit viewport.
	 * <P>
	 * NOTE: if the viewport is the same size as the window (by default it is),
	 * then setOrtho() will make the world coordinates exactly match the screen
	 * pixel positions. This is convenient for mouse interaction, but be warned:
	 * if you setViewport() to something other than fullscreen, then you need to
	 * use getWorldCoordsAtScreen() to convert screen xy to world xy.
	 * <P>
	 * Once Ortho is on, glTranslate() will take pixel coords as arguments, with
	 * the lower left corner 0,0 and the upper right corner 1024,768 (or
	 * whatever your screen size is). !!!
	 * 
	 * @see setOrthoOff()
	 * @see setViewport(int,int,int,int)
	 */
	public void setOrthoOn() {
		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPushMatrix();
		gl.glLoadIdentity();
		gl.glOrtho(0, viewPortW, 0, viewPortH, -1, 1);
		gl.glMatrixMode(GL2.GL_MODELVIEW);
		gl.glPushMatrix();
		gl.glLoadIdentity();
		gl.glDisable(GL2.GL_DEPTH_TEST);
	}

	/**
	 * Turn 2D mode off. Return the projection and model views to their
	 * preserved state that was saved when setOrthoOn() was called, and
	 * re-enable depth testing.
	 * 
	 * @see setOrthoOn()
	 */
	public void setOrthoOff() {
		gl.glMatrixMode(GL2.GL_PROJECTION);
		gl.glPopMatrix();
		gl.glMatrixMode(GL2.GL_MODELVIEW);
		gl.glPopMatrix();
		gl.glEnable(GL2.GL_DEPTH_TEST);
	}

	/**
	 * Render a text string in 2D over the scene, using the character set
	 * created by this GLFont object.
	 * 
	 * @see makeFont()
	 */
	public void print(int x, int y, String msg) {
		if (msg != null) {
			gl.glBindTexture(GL2.GL_TEXTURE_2D, fontTextureHandle);
			int len = msg.length();
			for (int i = 0; i < len; i++) {
				gl.glPushMatrix();
				char chIdx = (char)(msg.charAt(i) - (char)32);
				gl.glTranslatef(i - .5f*len, - .5f, 0);
				gl.glCallList(fontListBase + chIdx);
				gl.glPopMatrix();
			}
		}
	}
	
}
