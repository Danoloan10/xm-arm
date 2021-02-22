/*
 * $FILE: trace.c
 *
 * Tracing functionality
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
#include <xm_inc/objdir.h>
#include <xm_inc/objects/trace.h>
extern void *memcpy(void *dest, const void *src, unsigned long n);

/* SPR-150415-01 */
static inline xm_s32_t __trace_open(xmId_t id) {
    return (id>OBJDESC_ID_MASK)?XM_INVALID_PARAM:OBJDESC_BUILD(OBJ_CLASS_TRACE, id, 0);
}

xm_s32_t XM_trace_event(xm_u8_t *trace) {
    xm_s32_t ret;
    xm_s32_t size = sizeof(xmTraceEvent_t);
    if (!trace)
	return XM_INVALID_PARAM;
    ret = XM_write_object(OBJDESC_BUILD(OBJ_CLASS_TRACE, XM_PARTITION_SELF, 0), trace, sizeof(xmTraceEvent_t), 0);
    return (ret<size)?XM_INVALID_PARAM:XM_OK;
}

xm_s32_t XM_trace_read(xmId_t id, xmTraceEvent_t *traceEventPtr, xm_s32_t noTraces) {
    xm_u32_t traceStream=__trace_open(id);
/* SPR-150415-01 */
    if (traceStream==XM_INVALID_PARAM)
        return XM_INVALID_PARAM;
    xm_s32_t ret;
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;
    if (!traceEventPtr)
	return XM_INVALID_PARAM;
    
    ret=XM_read_object(traceStream, traceEventPtr, noTraces*sizeof(xmTraceEvent_t), 0);
    return (ret>0)?(ret/sizeof(xmTraceEvent_t)):ret;
}

xm_s32_t XM_trace_status(xmId_t id, xmTraceStatus_t *traceStatusPtr) {
    xm_u32_t traceStream=__trace_open(id);
/* SPR-150415-01 */
    if (traceStream==XM_INVALID_PARAM)
        return XM_INVALID_PARAM;
    if (OBJDESC_GET_CLASS(traceStream)!=OBJ_CLASS_TRACE)
        return XM_INVALID_PARAM;

    if (!traceStatusPtr)
	return XM_INVALID_PARAM;
    return XM_ctrl_object(traceStream, XM_TRACE_GET_STATUS, traceStatusPtr);
}
