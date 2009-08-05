package pl.swmud.ns.swaedit.core;

import java.math.BigInteger;

import pl.swmud.ns.swaedit.resets.Arg;
import pl.swmud.ns.swmud._1_0.area.Resets.Reset;

public class ResetWrapper {
    
    private Reset reset;
    private pl.swmud.ns.swaedit.resets.Reset res;
    private Arg[] args = new Arg[5];
    private String[] names = new String[5];
    private int current;
    

    public ResetWrapper(Reset reset, pl.swmud.ns.swaedit.resets.Reset res, int current) {
        this.reset = reset;
        this.res = res;
        this.current = current;
        
        names[0] = "extra";
        names[1] = "arg1";
        names[2] = "arg2";
        names[3] = "arg3";
        names[4] = "arg4";
        
        args[0] = res.getExtra();
        args[1] = res.getArg1();
        args[2] = res.getArg2();
        args[3] = res.getArg3();
        args[4] = res.getArg4();
    }

    public Reset getReset() {
        return reset;
    }

    public void setReset(Reset reset) {
        this.reset = reset;
    }

    public pl.swmud.ns.swaedit.resets.Reset getRes() {
        return res;
    }

    public void setRes(pl.swmud.ns.swaedit.resets.Reset res) {
        this.res = res;
    }
    
    public String getName(int i) {
        return names[i];
    }
    
    public Arg getArg(int i) {
        return args[i];
    }
    
    public Arg getCurrentArg() {
        return args[current];
    }
    
    public String getCurrentName() {
        return names[current];
    }

    public int getCurrent() {
        return current;
    }
    
    public void setCurrent(int current) {
        this.current = current;
    }
    
    public Long getCurrentValue() {
        return getValue(current).longValue();
    }

    public void setCurrentValue(Long value) {
        setCurrentValue(BigInteger.valueOf(value));
    }

    public void setCurrentValue(BigInteger value) {
        switch (current) {
        case 1:
            reset.setArg1(value);
            break;

        case 2:
            reset.setArg2(value);
            break;

        case 3:
            reset.setArg3(value);
            break;

        case 4:
            reset.setArg4(value);
            break;

        default: /* 0 */
            reset.setExtra(value);
        }
    }
    
    public BigInteger getValue(int idx) {
        switch (idx) {
        case 1:
            return reset.getArg1();

        case 2:
            return reset.getArg2();

        case 3:
            return reset.getArg3();

        case 4:
            return reset.getArg4();

        default: /* 0 */
            return reset.getExtra();
        }
    }


}
