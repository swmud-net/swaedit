package pl.swmud.ns.swaedit.map;

import java.util.SortedMap;
import java.util.TreeMap;

import pl.swmud.ns.swmud._1_0.area.Rooms.Room;

public class MapRoom {
    private Room room;

    private RoomCoords coords;

    /**
     * Specifies the parent exit direction that lead to this room. Can be <code>null</code>.
     */
    private ExitWrapper parentExit;

    private SortedMap<ExitWrapper, MapRoom> mapRooms = new TreeMap<ExitWrapper, MapRoom>();
    
    public boolean drawn = false;

    public MapRoom(Room room, RoomCoords coords, ExitWrapper parentExit) {
        this.room = room;
        this.coords = coords;
        this.parentExit = parentExit;
    }

    public MapRoom(Room room, RoomCoords coords) {
        this(room, coords, null);
    }

    public Room getRoom() {
        return room;
    }

    public void setRoom(Room room) {
        this.room = room;
    }

    public RoomCoords getCoords() {
        return coords;
    }

    public void setCoords(RoomCoords coords) {
        this.coords = coords;
    }

    public void addChildRoom(MapRoom mapRoom, ExitWrapper exWrapper) {
        mapRooms.put(exWrapper, mapRoom);
    }

    public boolean hasChild(MapRoom mapRoom) {
        return mapRooms.get(mapRoom.getRoom().getVnum()) != null;
    }

    public ExitWrapper getParentExit() {
        return parentExit;
    }

    public void setParentExit(ExitWrapper parentExit) {
        this.parentExit = parentExit;
    }

    public SortedMap<ExitWrapper, MapRoom> getMapRooms() {
        return mapRooms;
    }

    public void setMapRooms(SortedMap<ExitWrapper, MapRoom> mapRooms) {
        this.mapRooms = mapRooms;
    }
    
    public int getDistance(MapRoom mr) {
    	return mr.getCoords().getDistance(coords);
    }
}
