package pl.swmud.ns.swaedit.core;

import java.math.BigInteger;
import java.util.HashMap;

import com.trolltech.qt.gui.QComboBox;

import pl.swmud.ns.swaedit.gui.SWAEdit;
import pl.swmud.ns.swaedit.resets.Arg;
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
    private BigInteger newFirstVnum;
    private int flags;
    HashMap<String, pl.swmud.ns.swaedit.resets.Reset> resetsMap = new HashMap<String, pl.swmud.ns.swaedit.resets.Reset>();
    private BigInteger lvnum;
    private BigInteger uvnum;
    private BigInteger vnumDifference;

    public Renumberer(Area area, BigInteger newFirstVnum, int flags,
            HashMap<String, pl.swmud.ns.swaedit.resets.Reset> resetsMap) {
        this.area = area;
        this.newFirstVnum = newFirstVnum;
        this.flags = flags;
        this.resetsMap = resetsMap;
        lvnum = area.getHead().getVnums().getLvnum();
        uvnum = area.getHead().getVnums().getUvnum();
        vnumDifference = newFirstVnum.subtract(lvnum);
        if (lvnum.equals(BigInteger.ONE)) {
            vnumDifference.add(BigInteger.ONE);
        }
    }

    /**
     * Checks if a specified number is in area vnum range (
     * <code>lvnum <= vnum <= uvnum</code>).
     * 
     * @param vnum
     *            vnum to be checked
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
            if (isAreaVnum(vnum = item.getVnum())) {
                item.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(item.getPrograms());
            }
        }

        /* mobiles */
        for (Mobile mob : area.getMobiles().getMobile()) {
            if (isAreaVnum(vnum = mob.getVnum())) {
                mob.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(mob.getPrograms());
            }
        }

        /* rooms */
        for (Room room : area.getRooms().getRoom()) {
            if (isAreaVnum(vnum = room.getVnum())) {
                room.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(room.getPrograms());
            }
        }

        /* resets */
        for (Reset reset : area.getResets().getReset()) {
            renumberReset(reset);
        }

        /* shops */
        for (Shop shop : area.getShops().getShop()) {
            if (isAreaVnum(vnum = shop.getKeeper())) {
                shop.setKeeper(getNewVnum(vnum));
            }
        }

        /* repairs */
        for (Repair repair : area.getRepairs().getRepair()) {
            if (isAreaVnum(vnum = repair.getKeeper())) {
                repair.setKeeper(getNewVnum(vnum));
            }
        }
    }

    private void renumberPrograms(Programs progs) {
        for (Program prog : progs.getProgram()) {
            //TODO: renumber progs
        }
    }

    private void renumberReset(Reset reset) {
        ResetWrapper wrapper = new ResetWrapper(reset, resetsMap.get(reset
                .getCommand()), 0);
        BigInteger vnum;
        String type;

        for (int i = 0; i < SWAEdit.MAX_RESET_ARGS; i++) {
            wrapper.setCurrent(i);
            type = wrapper.getArg(i).getType();
            if (type.equals("room") || type.equals("room_other") || type.equals("mob")
                    || type.equals("mob_other") || type.equals("item") || type.equals("item_other")) {
                if (isAreaVnum(vnum = wrapper.getValue(i))) {
                    wrapper.setCurrentValue(getNewVnum(vnum));
                }
            }
        }
    }
}
