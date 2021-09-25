#include "ty_timer.h"
#include "os_timer.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
#define TY_TIMER_MAX_NUM                    20
            
/*********************************************************************
 * LOCAL STRUCT
 */
//#pragma pack(1)
typedef struct {
    os_timer_t os_timer;
    uint8_t    is_occupy;
    uint32_t   ms;
    uint8_t    mode;
} ty_timer_item_t;
//#pragma pack()

/*********************************************************************
 * LOCAL VARIABLE
 */
static ty_timer_item_t ty_timer_pool[TY_TIMER_MAX_NUM] = {0};

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
static ty_timer_item_t* acquire_timer(uint32_t ms, uint8_t mode)
{
    for(uint8_t i=0; i<TY_TIMER_MAX_NUM; i++) {
        if (ty_timer_pool[i].is_occupy == 0) {
            ty_timer_pool[i].is_occupy = 1;
            ty_timer_pool[i].ms = ms;
            ty_timer_pool[i].mode = mode;
            return (void*)&ty_timer_pool[i].os_timer;
        }
    }
    return NULL;
}

/*********************************************************
FN: 
*/
uint32_t ty_timer_create(void** p_timer_id, uint32_t ms, ty_timer_mode_t mode, ty_timer_handler_t handler)
{
    ty_timer_item_t* timer_item = acquire_timer(ms, mode);

    os_timer_init(&timer_item->os_timer, handler, NULL);

    *p_timer_id = timer_item;

    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_timer_delete(void* timer_id)
{  
    ty_timer_item_t* timer_item = timer_id;
    os_timer_destroy(&timer_item->os_timer);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_timer_start(void* timer_id)
{
    ty_timer_item_t* timer_item = timer_id;
  
    if(timer_item->mode == TUYA_BLE_TIMER_SINGLE_SHOT) {
        os_timer_start(&timer_item->os_timer, timer_item->ms, false);
    } else if(timer_item->mode == TUYA_BLE_TIMER_REPEATED) {
        os_timer_start(&timer_item->os_timer, timer_item->ms, true);
    }
    
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_timer_stop(void* timer_id)
{
    ty_timer_item_t* timer_item = timer_id;
    os_timer_stop(&timer_item->os_timer);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_timer_restart(void* timer_id, uint32_t ms)
{
    return 0;
}










