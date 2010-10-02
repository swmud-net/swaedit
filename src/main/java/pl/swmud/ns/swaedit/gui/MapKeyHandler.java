package pl.swmud.ns.swaedit.gui;

import com.jogamp.newt.event.KeyAdapter;
import com.jogamp.newt.event.KeyEvent;

public class MapKeyHandler extends KeyAdapter {
	private final Minion minion;

	private int currentIsland;

	private final int maxIslands;

	public MapKeyHandler(Minion minion, int currentIsland, int maxIslands) {
		this.minion = minion;
		this.currentIsland = currentIsland;
		this.maxIslands = maxIslands;
	}

	@Override
	public void keyTyped(KeyEvent e) {
		char kc = e.getKeyChar();
		if (kc == KeyEvent.VK_SPACE) {
			minion.rotationStopped();
		} else if (kc == 'd' || kc == KeyEvent.VK_D) {
			minion.drawDistantExits();
		} else if (kc == 'm' || kc == KeyEvent.VK_M) {
			minion.multisampling();
		} else if (kc == 'k' || kc == KeyEvent.VK_K) {
			minion.drawCross();
		} else if (kc == 'c' || kc == KeyEvent.VK_C) {
			minion.center();
		} else if (kc == 'h' || kc == KeyEvent.VK_H) {
			minion.showHelp();
		} else if (kc == 'f' || kc == KeyEvent.VK_F) {
			minion.showFullscreen();
		}
	}

	@Override
	public void keyPressed(KeyEvent e) {
		int kc = e.getKeyCode();
		if (kc == KeyEvent.VK_LEFT) {
			if (currentIsland > 0) {
				minion.setCurrentIsland(--currentIsland);
			}
		} else if (kc == KeyEvent.VK_RIGHT) {
			if (currentIsland < maxIslands - 1) {
				minion.setCurrentIsland(++currentIsland);
			}
		} else if (kc == KeyEvent.VK_UP) {
			minion.incLayer();
		} else if (kc == KeyEvent.VK_DOWN) {
			minion.decLayer();
		} else if (kc == KeyEvent.VK_F12) {
			minion.screenShot();
		} else if (kc == KeyEvent.VK_F11) {
			minion.transparentScreenShot();
		}
	}

	public interface Minion {
		void setCurrentIsland(int islandNo);

		void rotationStopped();

		void drawDistantExits();
		
		void incLayer();
		
		void decLayer();
		
		void multisampling();
		
		void drawCross();
		
		void center();
		
		void showHelp();

		void showFullscreen();
		
		void screenShot();
		
		void transparentScreenShot();
	}

}
