/*
 * $FILE: hm.c
 *
 * Health Monitor
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <xm.h>
#include <hm.h>
#include <xm_inc/objects/hm.h>

xm_s32_t XM_hm_read(xmHmLog_t *hmLogPtr, xm_s32_t noLogs) {
    xm_s32_t ret;

    if (!hmLogPtr){
        return XM_INVALID_PARAM;
    }
    /* SPR-150416-09 */
    ret=XM_read_object(OBJDESC_BUILD(OBJ_CLASS_HM, XM_HYPERVISOR_ID, 0), hmLogPtr, noLogs*sizeof(xmHmLog_t), 0);
    return (ret>0) ? (ret/sizeof(xmHmLog_t)) : ret;

}

//@ \void{<track id="using-proc">}
xm_s32_t XM_hm_status(xmHmStatus_t *hmStatusPtr) {

    if (!hmStatusPtr) {
        return XM_INVALID_PARAM;
    }
    return XM_ctrl_object(OBJDESC_BUILD(OBJ_CLASS_HM, XM_HYPERVISOR_ID, 0), XM_HM_GET_STATUS, hmStatusPtr);
}
//@ \void{</track id="using-proc">}

/* SPR-150415-04 */
xm_s32_t XM_hm_raise_event(xm_u32_t event) {
    /* SPR-150414-14 */
    xm_s32_t ret = XM_write_object(OBJDESC_BUILD(OBJ_CLASS_HM, XM_PARTITION_SELF, 0), &event, 1, 0);
    return (ret >= 0)?XM_OK: ret;
}
