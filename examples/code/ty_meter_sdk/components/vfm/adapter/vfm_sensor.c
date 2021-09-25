#include "vfm_base.h"
#include "vfm_sensor.h"

static tyVFMSensorIntf_t *s_intf = NULL;

int ty_vfm_sensor_reg(tyVFMSensorIntf_t *intf)
{
    s_intf = intf;
    return 0;
}

int ty_vfm_sensor_event_post(SENSOR_EVENT_E event)
{
    return tuya_vfm_base_event_post(event,0);
}

int ty_vfm_sensor_read_speed(uint32_t *speed)
{

    return s_intf->read_speed(speed);
}

int ty_vfm_sensor_read_mileage(float *mileage)
{

    return s_intf->read_mileage(mileage);
}

int ty_vfm_sensor_read_total_mileage(float *allmileage)
{
    

    return s_intf->read_total_mileage(allmileage);
}

int ty_vfm_sensor_read_error(uint32_t *error)
{
   

    return s_intf->read_error(error);
}

int ty_vfm_acc_ctrl(ACC_POWER_E action)
{

    return s_intf->ctrl_acc(action);
}

int ty_vfm_acc_read(ACC_POWER_E *action)
{

    return s_intf->read_acc(action);
}
