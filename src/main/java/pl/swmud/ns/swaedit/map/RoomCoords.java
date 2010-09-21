package pl.swmud.ns.swaedit.map;

public class RoomCoords implements Cloneable {
	private int x;
	private int y;
	private int z;
	private int islandNo;
	private int layer;

	public RoomCoords(int x, int y, int z, int islandNo) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.islandNo = islandNo;
	}

	public RoomCoords(int x, int y, int z) {
		this(x, y, z, 0);
	}

	public int getX() {
		return x;
	}

	public void setX(int x) {
		this.x = x;
	}

	public int getY() {
		return y;
	}

	public void setY(int y) {
		this.y = y;
	}

	public int getZ() {
		return z;
	}

	public void setZ(int z) {
		this.z = z;
	}

	public int getIslandNo() {
		return islandNo;
	}

	public void setIslandNo(int islandNo) {
		this.islandNo = islandNo;
	}

	public int getDistance(RoomCoords c) {
		int dx = c.getX() - x;
		dx *= dx;
		int dy = c.getY() - y;
		dy *= dy;
		int dz = c.getZ() - z;
		dz *= dz;

		return (int) Math.sqrt(dx + dy + dz);
	}
	
	public int getLayer() {
		return layer;
	}
	
	public void setLayer(int layer) {
		this.layer = layer;
	}

	@Override
	protected Object clone() {
		try {
			return super.clone();
		} catch (CloneNotSupportedException e) {
			return new RoomCoords(0, 0, 0);
		}
	}

	@Override
	public String toString() {
		return "(" + x + "," + y + "," + z + ")";
	}

	@Override
	public int hashCode() {
		return x + y + z + (islandNo * 10000);
	}

	@Override
	public boolean equals(Object obj) {
		if (obj instanceof RoomCoords) {
			RoomCoords c = (RoomCoords) obj;
			return x == c.getX() && y == c.getY() && z == c.getZ() && islandNo == c.getIslandNo();
		}

		return false;
	}
}