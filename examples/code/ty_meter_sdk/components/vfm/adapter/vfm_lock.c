#include "vfm_base.h"
#include "vfm_lock.h"


static tyVFMLockIntf_t *s_vfm_lock_intf = NULL;

int ty_vfm_lock_reg(tyVFMLockIntf_t *intf)
{
    s_vfm_lock_intf = intf;
    return 0;
}

int ty_vfm_lock_ctrl(VFM_LOCK_TYPE_E type,LOCK_ACTION_E action)
{
    return s_vfm_lock_intf->ctrl(type,action);
}

int ty_vfm_lock_read(VFM_LOCK_TYPE_E type,LOCK_ACTION_E *action)
{
    return s_vfm_lock_intf->read(type,action);
}
