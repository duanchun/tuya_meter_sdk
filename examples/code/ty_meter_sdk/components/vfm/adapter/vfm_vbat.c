#include "vfm_base.h"
#include "vfm_vbat.h"

static tyVFMVbatIntf_t *s_intf = NULL;

int ty_vfm_vbat_reg(tyVFMVbatIntf_t *info)
{
    s_intf = info;
    return 0;
}

int ty_vfmk_vbat_event_post(VBAT_TYPE_E type,VBAT_EVENT_T event)
{
    return tuya_vfm_base_event_post(event,type);
}

int ty_vfm_cellular_read_rsoc(VBAT_TYPE_E type,uint8_t *rsoc)
{
    return s_intf->read_rsoc(type,rsoc);
}

int ty_vfm_vbt_read_voltage(VBAT_TYPE_E type,int *mV)
{
    return s_intf->read_voltage(type,mV);
}

int ty_vfm_vbt_read_current(VBAT_TYPE_E type,int *mA)
{

    return s_intf->read_current(type,mA);
}

int ty_vfm_vbt_read_temperature(VBAT_TYPE_E type,int *temp)
{
    return s_intf->read_temperature(type,temp);
}
