/*
 * $FILE: logstream.h
 *
 * Log Stream
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#ifndef _XM_LOGSTREAM_H_
#define _XM_LOGSTREAM_H_

#include <assert.h>
#include <kdevice.h>
#include <stdc.h>

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif
#if 0
struct logStreamHdr {
#define LOGSTREAM_MAGIC1 0xF9E8D7C6
    xm_u32_t magic1;
    struct logStreamInfo {
        xm_s32_t elemSize, maxNoElem, lock, cHdr;
    } info;
    struct logStreamCtrl {
        xm_s32_t tail, elem, head, d;
    } ctrl[2];
#define LOGSTREAM_MAGIC2 0x1A2B3C4D
    xm_u32_t magic2;
};

struct logStream {
    struct logStreamInfo info;
    struct logStreamCtrl ctrl;
    const kDevice_t *kDev;
};

static inline void LogStreamCommit(struct logStream *lS) {
    if (lS->info.maxNoElem) {
        KDevSeek(lS->kDev, OFFSETOF(struct logStreamHdr, ctrl[lS->info.cHdr]), DEV_SEEK_START);
        KDevWrite(lS->kDev, &lS->ctrl, sizeof(struct logStreamCtrl));
        KDevSeek(lS->kDev, OFFSETOF(struct logStreamHdr, info.cHdr), DEV_SEEK_START);
        KDevWrite(lS->kDev, &lS->info.cHdr, sizeof(xm_s32_t));        
        lS->info.cHdr=!(lS->info.cHdr)?1:0;
    }
}

static inline void LogStreamInit(struct logStream *lS, const kDevice_t *kDev, xm_s32_t elemSize) {
    struct logStreamHdr hdr;
    ASSERT(elemSize>0);
    memset(lS, 0, sizeof(struct logStream));
    lS->kDev=kDev;
    KDevReset(kDev);
    lS->info.elemSize=elemSize;
    lS->info.maxNoElem=(xm_s32_t)KDevSeek(lS->kDev, 0, DEV_SEEK_END)-sizeof(struct logStreamHdr);
    lS->info.maxNoElem=(lS->info.maxNoElem>0)?lS->info.maxNoElem/elemSize:0;
    if (lS->info.maxNoElem) {
        KDevSeek(lS->kDev, 0, DEV_SEEK_START);
        KDevRead(lS->kDev, &hdr, sizeof(struct logStreamHdr));
        if ((hdr.magic1!=LOGSTREAM_MAGIC1)||(hdr.magic2!=LOGSTREAM_MAGIC2))
            goto INIT_HDR;
        if ((hdr.info.maxNoElem!=lS->info.maxNoElem)||
            (hdr.info.elemSize!=lS->info.elemSize))
            goto INIT_HDR;
        lS->info.lock=hdr.info.lock;
        lS->info.cHdr=!(hdr.info.cHdr)?1:0;
        memcpy(&lS->ctrl, &hdr.ctrl[hdr.info.cHdr], sizeof(struct logStreamCtrl));
    }

    return;
INIT_HDR:
    memset(&hdr, 0, sizeof(struct logStreamHdr));
    hdr.magic1=LOGSTREAM_MAGIC1;
    hdr.magic2=LOGSTREAM_MAGIC2;
    hdr.info.maxNoElem=lS->info.maxNoElem;
    hdr.info.elemSize=lS->info.elemSize;
    KDevSeek(lS->kDev, 0, DEV_SEEK_START);
    KDevWrite(lS->kDev, &hdr, sizeof(struct logStreamHdr));
}

static inline xm_s32_t LogStreamInsert(struct logStream *lS, void *log) {
    xm_s32_t smashed=0;
 
    ASSERT(lS->info.maxNoElem>=0);
    if (lS->info.maxNoElem) {
        if (lS->ctrl.elem>=lS->info.maxNoElem) {
            if (lS->info.lock)
                return -1;

            lS-> ctrl.head=((lS-> ctrl.head+1)<lS->info.maxNoElem)?lS-> ctrl.head+1:0;
            if (lS->ctrl.d>0)
                lS->ctrl.d--;
            lS->ctrl.elem--;
            smashed ++;
	}
	ASSERT(lS->ctrl.elem<lS->info.maxNoElem);
        KDevSeek(lS->kDev, sizeof(struct logStreamHdr)+lS->ctrl.tail*lS->info.elemSize, DEV_SEEK_START);
        KDevWrite(lS->kDev, log, lS->info.elemSize);
        lS->ctrl.tail=((lS->ctrl.tail+1)<lS->info.maxNoElem)?lS->ctrl.tail+1:0;
        lS->ctrl.elem++;
    } else {
	KDevWrite(lS->kDev, log, lS->info.elemSize);
    }
    LogStreamCommit(lS);
    return smashed;
}

static inline xm_s32_t LogStreamGet(struct logStream *lS, void *log) {
    xm_s32_t ptr;
    if ((lS->ctrl.elem)>0&&(lS->ctrl.d<lS->ctrl.elem)) {
        ptr=(lS->ctrl.d+lS-> ctrl.head)%lS->info.maxNoElem;
        lS->ctrl.d++;
	KDevSeek(lS->kDev, sizeof(struct logStreamHdr)+ptr*lS->info.elemSize, DEV_SEEK_START);
	KDevRead(lS->kDev, log, lS->info.elemSize);
        LogStreamCommit(lS);
        return 0;
    }
    return -1;
}

// These values must match with the ones defined in hypercall.h
#define XM_LOGSTREAM_CURRENT 0x0
#define XM_LOGSTREAM_START 0x1
#define XM_LOGSTREAM_END 0x2

static inline xm_s32_t LogStreamSeek(struct logStream *lS,  xm_s32_t offset,  xm_u32_t whence) {
    xm_s32_t off=offset;
    switch((whence)) {
    case XM_LOGSTREAM_START:
	break;
    case XM_LOGSTREAM_CURRENT:
	off+=lS->ctrl.d;
	break;
    case XM_LOGSTREAM_END:
	off+=lS->ctrl.elem;
	break;
    default:
	return -1;
    }

    if (off>lS->ctrl.elem) off=lS->ctrl.elem;
    if (off<0) off=0;
    lS->ctrl.d=off;
    LogStreamCommit(lS);
    return off;
}

static inline void LogStreamLock(struct logStream *lS) {
    lS->info.lock=1;
}

static inline void LogStreamUnlock(struct logStream *lS) {
    lS->info.lock=0;
}
#endif
#endif
