package pl.swmud.ns.swaedit.map;

import java.math.BigInteger;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;

import javax.media.opengl.GLException;

import pl.swmud.ns.swaedit.gui.MapWidget;
import pl.swmud.ns.swmud._1_0.area.Area;
import pl.swmud.ns.swmud._1_0.area.Rooms.Room;
import pl.swmud.ns.swmud._1_0.area.Rooms.Room.Exits.Exit;

public class Mapper {
	private Area area;
	private int islandNo;
	private SortedMap<BigInteger, MapRoom> mapRooms = new TreeMap<BigInteger, MapRoom>();
	private SortedMap<Integer, List<MapRoom>> islandRooms = new TreeMap<Integer, List<MapRoom>>();
	private int[] layers;

	public Mapper(Area area) {
		this.area = area;
	}

	public MapWidget makeMap() {
		for (Room room : area.getRooms().getRoom()) {
			if (!isAlreadyMade(room)) {
				RoomCoords coords = new RoomCoords(0, 0, 0, islandNo++);
				// System.out.println("making room: " + room.getVnum());
				MapRoom mr = new MapRoom(room, coords);
				mapRooms.put(room.getVnum(), mr);
				makeMapRoom(mr, room);
			}
		}

		assignRevExits();

		createIslands();
		
		checkCoords();

		MapWidget mw = null;
		try {
			mw = new MapWidget(islandRooms, 0, islandNo, false);
			mw.show(0, 0);
        } catch (GLException e) {
        	if (mw != null) {
            	mw.close();
            }
        	try {
            	mw = new MapWidget(islandRooms, 0, islandNo, true);
    			mw.show(0, 0);
            } catch (GLException e1) {
            	e1.printStackTrace();
            	if (mw != null) {
                	mw.close();
                }
            	mw = null;
            }
        }
		return mw;
	}

	private void makeMapRoom(MapRoom parent, Room room) {
		for (Exit exit : room.getExits().getExit()) {
			Room childRoom = findRoom(exit);
			/* omit loop exits */
			if (childRoom != null && !childRoom.getVnum().equals(room.getVnum())) {
				// System.out.println("found child room: " +
				// childRoom.getVnum());
				ExitWrapper exWrapper = new ExitWrapper(exit);
				MapRoom mr = new MapRoom(childRoom, getCoords(exit, parent.getCoords()), exWrapper);
				parent.addChildRoom(mr, exWrapper);
				if (!isAlreadyMade(childRoom)) {
					mapRooms.put(childRoom.getVnum(), mr);
					makeMapRoom(mr, childRoom);
				}
			}
		}
	}

	private void checkCoords() {
		Map<RoomCoords, MapRoom> coordsRooms = new HashMap<RoomCoords, MapRoom>();
		layers = new int[islandNo];

		for (MapRoom attempted : mapRooms.values()) {
			MapRoom current = coordsRooms.get(attempted.getCoords()); 
			if (current != null) {
//				System.out.println("replace attempt, current: "+current.getRoom().getVnum() + ", attempted: "+attempted.getRoom().getVnum());
				RoomCoords rc = attempted.getCoords(); 
				rc.setLayer(++layers[rc.getIslandNo()]);
			}
			coordsRooms.put(attempted.getCoords(), attempted);
		}
	}

	private void assignRevExits() {
		for (MapRoom mr : mapRooms.values()) {
			for (ExitWrapper exit : mr.getMapRooms().keySet()) {
				MapRoom toRoom = mapRooms.get(exit.getVnum());
				if (toRoom != null && exit.getRevExit() == null) {
					exit.setRevExit(getRevExit(exit, mr, toRoom));
					if (exit.getRevExit() != null) {
//						System.out.println("rev exit set to: " + mr.getRoom().getVnum() + ","
//						        + exit.getRevExit().getVnum());
					}
				}
				
				/* set distnace */
				if (mr.getDistance(toRoom) > 1) {
	                exit.setDistant();
                }
			}
		}
	}

	private ExitWrapper getRevExit(Exit exit, MapRoom fromRoom, MapRoom toRoom) {
		for (ExitWrapper revExit : toRoom.getMapRooms().keySet()) {
			if (fromRoom.getRoom().getVnum().equals(revExit.getVnum())
			        && exit.getDirection() == revExit.getRevDirection()) {
				return revExit;
			}
		}

		return null;
	}

	private boolean isAlreadyMade(Room room) {
		return mapRooms.get(room.getVnum()) != null;
	}

	private Room findRoom(Exit exit) {
		for (Room room : area.getRooms().getRoom()) {
			if (room.getVnum().equals(exit.getVnum())) {
				return room;
			}
		}

		return null;
	}

	private RoomCoords getCoords(Exit exit, RoomCoords crds) {
		if (exit == null) {
			System.err.println("exit==null");
			return new RoomCoords(-1, -1, 0, islandNo);
		}

		RoomCoords coords = (RoomCoords) crds.clone();
		switch (exit.getDirection()) {
		case 0: /* north */
			coords.setZ(coords.getZ() - 1);
			break;

		case 1: /* east */
			coords.setX(coords.getX() + 1);
			break;

		case 2: /* south */
			coords.setZ(coords.getZ() + 1);
			break;

		case 3: /* west */
			coords.setX(coords.getX() - 1);
			break;

		case 4: /* up */
			coords.setY(coords.getY() + 1);
			break;

		case 5: /* down */
			coords.setY(coords.getY() - 1);
			break;

		case 6: /* north-east */
			coords.setZ(coords.getZ() - 1);
			coords.setX(coords.getX() + 1);
			break;

		case 7: /* north-west */
			coords.setZ(coords.getZ() - 1);
			coords.setX(coords.getX() - 1);
			break;

		case 8: /* south-east */
			coords.setZ(coords.getZ() + 1);
			coords.setX(coords.getX() + 1);
			break;

		case 9: /* south-west */
			coords.setZ(coords.getZ() + 1);
			coords.setX(coords.getX() - 1);
			break;

		default: /* somewhere: 10 */
			System.err.println("virtual exits not yet supported!");
			/* FIXME: find a way to mark that the exit is virtual */
		}

		return coords;
	}

	private void createIslands() {
		for (int i = 0; i < islandNo; ++i) {
			List<MapRoom> iRooms = new LinkedList<MapRoom>();
			for (MapRoom mr : mapRooms.values()) {
				if (mr.getCoords().getIslandNo() == i) {
					iRooms.add(mr);
				}
			}
			islandRooms.put(i, iRooms);
		}
	}

	public int getWidth(int islandNo) {
		return new RoomSpread(islandRooms.get(islandNo)) {
			@Override
			public int getValue(RoomCoords coords) {
				return coords.getX();
			}
		}.getMiddle();
	}

	public int getHeight(int islandNo) {
		return new RoomSpread(islandRooms.get(islandNo)) {
			@Override
			public int getValue(RoomCoords coords) {
				return coords.getY();
			}
		}.getMiddle();
	}

	public int getDepth(int islandNo) {
		return new RoomSpread(islandRooms.get(islandNo)) {
			@Override
			public int getValue(RoomCoords coords) {
				return coords.getZ();
			}
		}.getMiddle();
	}
}
