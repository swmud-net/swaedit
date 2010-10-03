package pl.swmud.ns.swaedit.map;

import pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit;

public class ExitWrapper extends Exit implements Comparable<ExitWrapper> {
	ExitWrapper revExit;
	boolean drawn = false;
	boolean distant;

	public ExitWrapper(Exit exit, ExitWrapper revExit) {
		this.revExit = revExit;
		setDescription(exit.getDescription());
		setDirection(exit.getDirection());
		setDistance(exit.getDistance());
		setFlags(exit.getFlags());
		setKey(exit.getKey());
		setKeyword(exit.getKeyword());
		setVnum(exit.getVnum());
	}

	public ExitWrapper(Exit exit) {
		this(exit, null);
	}

	@Override
	public int hashCode() {
		return getDirection();
	}

	@Override
	public boolean equals(Object exit) {
		if (exit instanceof Exit) {
			return getDirection() == ((Exit) exit).getDirection();
		}

		return false;
	}

	public int compareTo(ExitWrapper exWrapper) {
		return getDirection() - exWrapper.getDirection();
	}

	public short getRevDirection() {
		short direction;
		switch (getDirection()) {
		case 0: /* north */
			direction = 2;
			break;

		case 1: /* east */
			direction = 3;
			break;

		case 2: /* south */
			direction = 0;
			break;

		case 3: /* west */
			direction = 1;
			break;

		case 4: /* up */
			direction = 5;
			break;

		case 5: /* down */
			direction = 4;
			break;

		case 6: /* north-east */
			direction = 9;
			break;

		case 7: /* north-west */
			direction = 8;
			break;

		case 8: /* south-east */
			direction = 7;
			break;

		case 9: /* south-west */
			direction = 6;
			break;

		default: /* somewhere: 10 */
			direction = 10;
		}

		return direction;
	}

	public String getDirectionName() {
		String name;
		switch (getDirection()) {
		case 0: /* north */
			name = "north";
			break;

		case 1: /* east */
			name = "east";
			break;

		case 2: /* south */
			name = "south";
			break;

		case 3: /* west */
			name = "west";
			break;

		case 4: /* up */
			name = "up";
			break;

		case 5: /* down */
			name = "down";
			break;

		case 6: /* north-east */
			name = "north-east";
			break;

		case 7: /* north-west */
			name = "north-west";
			break;

		case 8: /* south-east */
			name = "south-east";
			break;

		case 9: /* south-west */
			name = "south-west";
			break;

		default: /* somewhere: 10 */
			name = "somewhere";
		}

		return name;
	}

	public ExitWrapper getRevExit() {
		return revExit;
	}

	public boolean isTwoWay() {
		return revExit != null;
	}

	public boolean isDrawn() {
		return drawn;
	}

	public void setDrawn(boolean drawn) {
		this.drawn = drawn;
	}

	public void setDrawn() {
		setDrawn(true);
	}
	
	public void setRevExit(ExitWrapper revExit) {
		this.revExit = revExit;
	}
	
	public void setDistant() {
		distant = true;
	}
	
	public boolean isDistant() {
		return distant;
	}
	
}
