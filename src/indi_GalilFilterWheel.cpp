#include <cstring>
#include <iostream>

#include "libindi/indicom.h"

#include "indi_GalilFilterWheel.h"

static std::unique_ptr<indi_GalilFilterWheel> mydriver(new indi_GalilFilterWheel());

indi_GalilFilterWheel::indi_GalilFilterWheel()
{
    // TODO: Use generated version variables
    setVersion(0, 0);
    setFilterConnection(CONNECTION_TCP);
    setCurrentPollingPeriod(100);
}

const char *indi_GalilFilterWheel::getDefaultName()
{
    return "Bok Galil Filter Wheel";
}

bool indi_GalilFilterWheel::initProperties()
{
    // initialize the parent's properties first
    INDI::FilterWheel::initProperties();

    CurrentFilter = 1;
    FilterSlotNP[0].fill("filter_index", "Current Index", "%g", 1, 6, 1, CurrentFilter);
    
    /* ****************************************************************
    *  These properties are inherited from IndiFilterWheel
    *  The updates made here will be applied in onUpdate()
    **************************************************************** */
    FilterNameTP.setPermission(IP_RO);
    FilterNameTP.setLabel("Filter Opts");
    FilterNameTP.resize(6);
    
    FilterNameTP[0].fill("01", "Filter 01", "R"); // name, label, test
    FilterNameTP[1].fill("02", "Filter 02", "G");
    FilterNameTP[2].fill("03", "Filter 03", "B");
    FilterNameTP[3].fill("01", "Filter 04", "X");
    FilterNameTP[4].fill("02", "Filter 05", "Y");
    FilterNameTP[5].fill("03", "Filter 06", "Z");
    /* ************************************************************* */
    
    addAuxControls();

    return true;
}

bool indi_GalilFilterWheel::updateProperties()
{
    INDI::FilterWheel::updateProperties();

    LoadFiltersSP[0].fill("filtersUnloaded", "Unload", ISS_OFF);
    LoadFiltersSP[1].fill("filtersLoaded",   "Load",   ISS_OFF);
    LoadFiltersSP.fill(
        getDeviceName(),
        "loadFilters",
        "Load Filters",
        FILTER_TAB,
        IP_RW,
        ISRule::ISR_ATMOST1,
        5,
        IPS_OK
    );

    defineProperty(LoadFiltersSP);

    return true;
}

void indi_GalilFilterWheel::TimerHit()
{
    if (!isConnected())
        return;

    LOG_INFO("timer hit");

    TargetFilter = FilterSlotNP[0].getValue();
    LOGF_INFO("Target filter: %f", TargetFilter);
    
    targetFL = (LoadFiltersSP.findOnSwitchIndex() == 1) ? FiltersLoaded::TRUE : FiltersLoaded::FALSE;
    LOGF_INFO("Filters Loaded: %d", ((currentFL == FiltersLoaded::TRUE) ? 1 : 0) );

    SetTimer(getCurrentPollingPeriod());
}

bool indi_GalilFilterWheel::saveConfigItems(FILE *fp)
{
    INDI::FilterWheel::saveConfigItems(fp);

    LoadFiltersSP.save(fp);

    return true;
}

bool indi_GalilFilterWheel::Handshake()
{
    bool handshakeSuccess = false;
    if (isSimulation())
    {
        LOGF_INFO("Connected successfuly to simulated %s.", getDeviceName());
        handshakeSuccess = true;
    }
    else
    {
        GReturn initialized = xq_gfwinit();
        if (initialized == G_NO_ERROR)
        {
            LOGF_INFO("Connected successfuly to %s.", getDeviceName());
            handshakeSuccess = true;
        }
        else
        {
            throw; // dependent lib will print error & "close" connection but no exceptions bubble up
        }
    }

    if (handshakeSuccess)
    {
        // NOTE: PortFD is set by the base class.
    
        // TODO: Any initial communciation needed with our filterwheel

    }

    return handshakeSuccess;
}

int indi_GalilFilterWheel::QueryFilter()
{
    // TODO: Query the hardware (or a local variable) to return what index
    // the filter wheel is currently at.

    return CurrentFilter;
}

bool indi_GalilFilterWheel::SelectFilter(int index)
{
    // NOTE: index starts at 1, not 0

    TargetFilter = index;

    // TODO: Tell the hardware to change to the given index.
    // Be sure to call SelectFilterDone when it has finished moving.

    CurrentFilter = TargetFilter;
    SelectFilterDone(index);
    return true;
}
