package pl.swmud.ns.swaedit.core;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.math.BigInteger;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import pl.swmud.ns.swaedit.gui.SWAEdit;
import pl.swmud.ns.swmud._1_0.area.Area;
import pl.swmud.ns.swmud._1_0.area.Programs;
import pl.swmud.ns.swmud._1_0.area.Head.Vnums;
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
    /**
     * Program owner type used for warnings. In this case type is: item. 
     */
    private static String TYPE_ITEM = "item";
    /**
     * Program owner type used for warnings. In this case type is: mobile. 
     */
    private static String TYPE_MOBILE = "mobile";
    /**
     * Program owner type used for warnings. In this case type is: room. 
     */
    private static String TYPE_ROOM = "room";
    
    private Area area;
    private int flags;
    HashMap<String, pl.swmud.ns.swaedit.resets.Reset> resetsMap = new HashMap<String, pl.swmud.ns.swaedit.resets.Reset>();
    private BigInteger lvnum;
    private BigInteger uvnum;
    private BigInteger vnumDifference;
    private List<String> warnings = new LinkedList<String>();

    public Renumberer(Area area, BigInteger newFirstVnum, Integer flags,
            HashMap<String, pl.swmud.ns.swaedit.resets.Reset> resetsMap) {
        this.area = area;
        this.flags = flags.intValue();
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

    private BigInteger getNewVnum(BigInteger vnum) {
        return vnum.add(vnumDifference);
    }

    /**
     * Renumbers the area according to arguments given to the constructor.
     */
    public void renumber() {
        BigInteger vnum;

        /* head */
        {
            Vnums vnums = area.getHead().getVnums();
            vnums.setLvnum(getNewVnum(lvnum));
            vnums.setUvnum(getNewVnum(uvnum));
        }

        /* items */
        for (Object item : area.getObjects().getObject()) {
            if (isAreaVnum(vnum = item.getVnum())) {
                item.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(TYPE_ITEM, item.getVnum(), item.getPrograms());
            }
        }

        /* mobiles */
        for (Mobile mob : area.getMobiles().getMobile()) {
            if (isAreaVnum(vnum = mob.getVnum())) {
                mob.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(TYPE_MOBILE, mob.getVnum(), mob.getPrograms());
            }
        }

        /* rooms */
        for (Room room : area.getRooms().getRoom()) {
            if (isAreaVnum(vnum = room.getVnum())) {
                room.setVnum(getNewVnum(vnum));
            }

            if ((flags & RENUMBER_MUDPROGS) == RENUMBER_MUDPROGS) {
                renumberPrograms(TYPE_ROOM, room.getVnum(), room.getPrograms());
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
    
    public List<String> getWarnings() {
        return warnings;
    }

    private void renumberPrograms(String type, BigInteger ownerVnum, Programs progs) {
        int progNo = 0;
        for (Program prog : progs.getProgram()) {
            progNo++;
            BufferedReader br = new BufferedReader(new StringReader(prog.getComlist()));
            String line;
            int lineNo = 1;
            StringBuilder comlist = new StringBuilder();
            try {
                while ((line = br.readLine()) != null) {
                    Pattern p = Pattern.compile("\\b[mio]?[1-9][0-9]*\\b", Pattern.CASE_INSENSITIVE);
                    Matcher m = p.matcher(line);
                    String strVnum;
                    BigInteger vnum;
                    while (m.find()) {
                        strVnum =  m.group().replaceFirst("^[mio]", "");
                        if (isAreaVnum(vnum = BigInteger.valueOf(Long.parseLong(strVnum)))) {
                            BigInteger newVnum = getNewVnum(vnum);
                            int offset = m.start();
                            line = line.replaceFirst("\\b" + strVnum + "\\b", newVnum.toString());
                            warnings.add("changed " + type + "'s: "+ ownerVnum + " program's: "
                                    + progNo + " vnum: " + vnum + " to: " + newVnum
                                    + " in line: " + lineNo + " at offset: " + offset);
                        }
                    }
                    comlist.append(line+"\n");
                    lineNo++;
                }
            } catch (IOException ignored) {
            } finally {
                try {
                    if (br != null) {
                        br.close();
                    }
                } catch (IOException ignored) {
                }
            }
            prog.setComlist(comlist.toString());
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
    
    public boolean saveWarnings(String path) {
        BufferedWriter bw = null;
        try {
            bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(path)));
            for (String warning : warnings) {
                bw.write(warning+System.getProperty("line.separator"));
                bw.flush();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            if (bw != null) {
                try {
                    bw.close();
                } catch (IOException ignored) {
                }
            }
        }
        return true;
    }
}
