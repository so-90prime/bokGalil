#pragma once

#include "bokGalil.h"
#include "libindi/indifilterwheel.h"

namespace Connection
{
    class TCP;
}

class indi_GalilFilterWheel : public INDI::FilterWheel
{
public:
                            indi_GalilFilterWheel();
    virtual                ~indi_GalilFilterWheel() = default;
    
protected:
    virtual const char     *getDefaultName() override;
    
    // virtual bool Connect(); 
    virtual bool            Handshake() override;
    
    virtual bool            initProperties() override;   // init properties to be visible before connecting
    virtual bool            updateProperties() override; // init properties visible upon connecting
    // see DefaultDevice::ISGetProperties() comments
    
    virtual void            TimerHit() override;
    
    virtual bool            saveConfigItems(FILE *fp) override;

    //virtual bool          QueryLoaded();
    //virtual bool          SelectLoaded(bool target_pos);
    //virtual void          SelectLoadedDone(bool target_pos); // analagous to SelectFilterDone()
    
    virtual int             QueryFilter() override;
    virtual bool            SelectFilter(int) override;
    
    /* /////////////////////////////////////////////////////////////////////////
    // Variables related to the custom (not inherited) LoadFiltersSP switch
    // (CurrentFilter and TargetFilter are inherited from INDI::FilterWheel)
    ///////////////////////////////////////////////////////////////////////// */
    INDI::PropertySwitch      LoadFiltersSP {2};
    
    enum class FiltersLoaded
    {
        FALSE,
        TRUE,
        UNKNOWN
    };

    FiltersLoaded currentFL = FiltersLoaded::FALSE;
    FiltersLoaded targetFL  = FiltersLoaded::FALSE;
    FiltersLoaded prevFL    = FiltersLoaded::FALSE; // track previous in order to return state after filter change

    /* /////////////////////////////////////////////////////////////////////////
    // Use state machine in TimerHit to control hardware
    // no need for onUpdate on each property
    ///////////////////////////////////////////////////////////////////////// */
    IPState GalilFWStateMachine();

};
