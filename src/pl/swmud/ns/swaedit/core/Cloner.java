package pl.swmud.ns.swaedit.core;

import java.math.BigInteger;

import pl.swmud.ns.swmud._1_0.area.Extradescs;
import pl.swmud.ns.swmud._1_0.area.ObjectFactory;
import pl.swmud.ns.swmud._1_0.area.Programs;
import pl.swmud.ns.swmud._1_0.area.Extradescs.Extradesc;
import pl.swmud.ns.swmud._1_0.area.Programs.Program;
import pl.swmud.ns.swmud._1_0.area.Specials.Special;

public final class Cloner {
    public static Extradescs clone(Extradescs extraDescs) {
        ObjectFactory of = new ObjectFactory();
        Extradescs eds =  of.createExtradescs();
        for (Extradesc ed : extraDescs.getExtradesc()) {
            Extradesc newEd = of.createExtradescsExtradesc();
            newEd.setKeyword(new String(ed.getKeyword()));
            newEd.setDescription(new String(ed.getDescription()));
            eds.getExtradesc().add(newEd);
        }
        return eds;
    }

    public static Programs clone(Programs programs) {
        ObjectFactory of = new ObjectFactory();
        Programs progs =  of.createPrograms();
        for (Program prog : programs.getProgram()) {
            Program newProg = of.createProgramsProgram();
            newProg.setArgs(new String(prog.getArgs()));
            newProg.setComlist(new String(prog.getComlist()));
            newProg.setType(new String(prog.getType()));
            progs.getProgram().add(newProg);
        }
        return progs;
    }
    
    public static Special clone(Special special) {
        ObjectFactory of = new ObjectFactory();
        Special newSpec = of.createSpecialsSpecial();
        newSpec.setVnum(new BigInteger(special.getVnum().toString()));
        newSpec.setFunction((special.getFunction() == null) ? "" : new String(special.getFunction()));
        newSpec.setFunction2((special.getFunction2() == null) ? "" : new String(special.getFunction2()));

        return newSpec;
    }
}
