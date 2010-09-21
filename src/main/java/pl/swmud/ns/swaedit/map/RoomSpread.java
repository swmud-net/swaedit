package pl.swmud.ns.swaedit.map;

import java.util.List;

public abstract class RoomSpread {
    private List<MapRoom> island;

    public abstract int getValue(RoomCoords coords);

    public RoomSpread(List<MapRoom> island) {
        this.island = island;
    }
    
    public int getMiddle() {
        if (island.size() < 1) {
            return 0;
        }

        RoomCoords coords = island.get(0).getCoords();
        int min = getValue(coords);
        int max = min;
        
        for (MapRoom mr : island) {
            int value = getValue(mr.getCoords());
            if (value > max) {
                max = value;
            } else if (value < min) {
                min = value;
            }
        }

        return Math.abs(min) - Math.abs(max);
    }
}
