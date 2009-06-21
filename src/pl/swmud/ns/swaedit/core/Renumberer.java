package pl.swmud.ns.swaedit.core;

import java.math.BigInteger;

import pl.swmud.ns.swmud._1_0.area.Area;
import pl.swmud.ns.swmud._1_0.area.Programs;
import pl.swmud.ns.swmud._1_0.area.Mobiles.Mobile;
import pl.swmud.ns.swmud._1_0.area.Objects.Object;
import pl.swmud.ns.swmud._1_0.area.Programs.Program;
import pl.swmud.ns.swmud._1_0.area.Repairs.Repair;
import pl.swmud.ns.swmud._1_0.area.Resets.Reset;
import pl.swmud.ns.swmud._1_0.area.Rooms.Room;
import pl.swmud.ns.swmud._1_0.area.Shops.Shop;

public class Renumberer {

    /**
     * Renumbers only these parts that it is sure to be vnums.
     */
    public static final int RENUMBER_RELIABLE = 0;
    /**
     * Renumbers also numbers in programs' keyword and body.
     */
    public static final int RENUMBER_MUDPROGS = 1;
    
    private Area area;
    private int flags;
    private BigInteger lvnum;
    private BigInteger uvnum;
    private BigInteger newFirstVnum;
    private BigInteger vnumDifference;
    
    public Renumberer(Area area, BigInteger newFirstVnum, int flags) {
        this.area = area;
        this.newFirstVnum = newFirstVnum;
        this.flags = flags;
        lvnum = area.getHead().getVnums().getLvnum();
        uvnum = area.getHead().getVnums().getUvnum();
        vnumDifference = newFirstVnum.subtract(lvnum);
        if (lvnum.equals(BigInteger.ONE)) {
            vnumDifference.add(BigInteger.ONE);
        }
    }
    
    /**
     * Checks if a specified number is in area vnum range (<code>lvnum <= vnum <= uvnum</code>).
     * 
     * @param vnum vnum to be checked
     * @return true if vnum is in area vnum range
     */
    private boolean isAreaVnum(BigInteger vnum) {
        return vnum.compareTo(lvnum) >= 0 && vnum.compareTo(uvnum) <= 0;
    }

    /**
     * Overloaded method provided for convenience.
     */
    private boolean isAreaVnum(long vnum) {
        return isAreaVnum(BigInteger.valueOf(vnum));
    }
    
    private BigInteger getNewVnum(BigInteger vnum) {
        return vnum.add(vnumDifference);
    } 
    
    /**
     * Renumbers the area according to arguments given to the constructor.
     */
    public void renumber() {
        BigInteger vnum;

        /* items */
        for (Object item : area.getObjects().getObject()) {
            vnum = item.getVnum();
            if (isAreaVnum(vnum)) {
                item.setVnum(getNewVnum(vnum));
            }
            
            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(item.getPrograms());
            }
        }
        
        /* mobiles */
        for (Mobile mob : area.getMobiles().getMobile()) {
            vnum = mob.getVnum();
            if (isAreaVnum(vnum)) {
                mob.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(mob.getPrograms());
            }
        }
        
        /* rooms */
        for (Room room : area.getRooms().getRoom()) {
            vnum = room.getVnum();
            if (isAreaVnum(vnum)) {
                room.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(room.getPrograms());
            }
        }
        
        /* resets */
        for (Reset reset : area.getResets().getReset()) {
        }
        
        /* shops */
        for (Shop shop : area.getShops().getShop()) {
            
        }
 
        /* repairs */
        for (Repair repair : area.getRepairs().getRepair()) {
            
        }
    }
    
    private void renumberPrograms(Programs progs) {
        for (Program prog : progs.getProgram()) {
            
        }
    }
}
