package pl.swmud.ns.swaedit.core;


public final class FlagsWrapper {
    
    private Long flagsValue;
    private IFlagsSetter flagsSetter;
    
    public FlagsWrapper(Long flagsValue, IFlagsSetter flagsSetter) {
        this.flagsValue = flagsValue;
        this.flagsSetter = flagsSetter;
    }
    
    public Long getFlagsValue() {
        return flagsValue;
    }
    
    public void setFlagsValue(Long flagsValue) {
        this.flagsValue = flagsValue;
        flagsSetter.setFlags(flagsValue);
    }
}
