/*
 * $FILE: arm.c
 *
 * architecture dependent stuff
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE file for the license terms.
 */
/*
 * Changelog:
 * - [08/09/16:XM-ARM-2:SPR-160908-01:#123] Allow access to entire Memory Map. Some PTL2 are not required.
 * - [09/02/16:XM-ARM-2:SPR-151125-01:#97] Add PTDL2 table to allow TZ registers mapping.
 * - [21/11/16:XM-ARM-2:SPR-161121-01:#130] XM NonSecure sections require a PTL2 for each partition.
 */

#define _RSV_IO_PORTS_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <xm_inc/arch/paging.h>
#include <xm_inc/guest.h>
#include <xm_inc/arch/ginfo.h>

#include "conv.h"
#include "common.h"
#include "parser.h"
#include "checks.h"
#include "xmc.h"

xm_u32_t ToRegionFlags(char *s) {
	xm_u32_t flags=0;
	if(!strcasecmp(s, "stram")) {
		flags=XMC_REG_FLAG_PGTAB;
	} else if(!strcasecmp(s, "sdram")) {
		flags=XMC_REG_FLAG_PGTAB;
	} else if(!strcasecmp(s, "rom")) {
		flags=XM_MEM_AREA_UNMAPPED|XMC_REG_FLAG_ROM;
	} else {
		EPrintF("Expected valid region type (%s)\n", s);
	}
	return flags;
}

static void BaseRange_AH(xmlNodePtr node, const xmlChar *val) {
	xmcIoPortTab[C_IOPORT].range.base=ToU32((char *)val, 16);
	xmcIoPortTabNoL[C_IOPORT].range.base=node->line;
	xmcIoPortTab[C_IOPORT].type=XM_IOPORT_RANGE;
}

static struct attrXml baseRange_A={BAD_CAST"base", BaseRange_AH};

static void NoPortsRange_AH(xmlNodePtr node, const xmlChar *val) {
	xmcIoPortTab[C_IOPORT].range.noPorts=ToU32((char *)val, 10);
	xmcIoPortTabNoL[C_IOPORT].range.noPorts=node->line;
}

static struct attrXml noPortsRange_A={BAD_CAST"noPorts", NoPortsRange_AH};

static void IoPort_NH0(xmlNodePtr node) {
	xmcPartitionTab[C_PARTITION].noIoPorts++;
	xmc.noIoPorts++;
	DO_REALLOC(xmcIoPortTab, xmc.noIoPorts*sizeof(struct xmcIoPort));
	DO_REALLOC(xmcIoPortTabNoL, xmc.noIoPorts*sizeof(struct xmcIoPortNoL));
	memset(&xmcIoPortTab[C_IOPORT], 0, sizeof(struct xmcIoPort));
	memset(&xmcIoPortTabNoL[C_IOPORT], 0, sizeof(struct xmcIoPortNoL));
}

static struct nodeXml rangeIoPort_N={BAD_CAST"Range", IoPort_NH0, 0, 0, (struct attrXml *[]){&baseRange_A, &noPortsRange_A, 0}, 0};

static void AddressRestricted_AH(xmlNodePtr node, const xmlChar *val) {
	xmcIoPortTab[C_IOPORT].restricted.address=ToU32((char *)val, 16);
	xmcIoPortTabNoL[C_IOPORT].restricted.address=node->line;
	xmcIoPortTab[C_IOPORT].type=XM_RESTRICTED_IOPORT;
}

static struct attrXml addressRestricted_A={BAD_CAST"address", AddressRestricted_AH};

static void MaskRestricted_AH(xmlNodePtr node, const xmlChar *val) {
	xmcIoPortTab[C_IOPORT].restricted.mask=ToU32((char *)val, 16);
	xmcIoPortTabNoL[C_IOPORT].restricted.mask=node->line;
}

static struct attrXml maskRestricted_A={BAD_CAST"mask", MaskRestricted_AH};

static struct nodeXml restrictedIoPort_N={BAD_CAST"Restricted", IoPort_NH0, 0, 0, (struct attrXml *[]){&addressRestricted_A, &maskRestricted_A, 0}, 0};

void addAPBMemoryArea(unsigned int addrss, unsigned int line)
{
	extern void incCPartitionNoMemArea(void);
	incCPartitionNoMemArea();
    xmc.noPhysicalMemoryAreas++;
    DO_REALLOC(xmcMemAreaTab, xmc.noPhysicalMemoryAreas*sizeof(struct xmcMemoryArea));
    DO_REALLOC(xmcMemAreaTabNoL, xmc.noPhysicalMemoryAreas*sizeof(struct xmcMemoryAreaNoL));
    memset(&xmcMemAreaTab[C_PHYSMEMAREA], 0, sizeof(struct xmcMemoryArea));
    memset(&xmcMemAreaTabNoL[C_PHYSMEMAREA], 0, sizeof(struct xmcMemoryAreaNoL));

    xmcMemAreaTabNoL[C_PHYSMEMAREA].flags = XM_MEM_AREA_DEVICE_AREA;
    xmcMemAreaTabNoL[C_PHYSMEMAREA].line = line;
    xmcMemAreaTabNoL[C_PHYSMEMAREA].mappedAt=addrss;
    //xmcMemAreaTabNoL[C_PHYSMEMAREA].name;
    extern int getCPartition(void);
    xmcMemAreaTabNoL[C_PHYSMEMAREA].partitionId=getCPartition();
    xmcMemAreaTabNoL[C_PHYSMEMAREA].size=PAGE_SIZE;
    xmcMemAreaTabNoL[C_PHYSMEMAREA].startAddr=addrss;

    xmcMemAreaTab[C_PHYSMEMAREA].flags = XM_MEM_AREA_DEVICE_AREA;
    xmcMemAreaTab[C_PHYSMEMAREA].mappedAt=addrss;
    //xmcMemAreaTab[C_PHYSMEMAREA].nameOffset;
    xmcMemAreaTab[C_PHYSMEMAREA].size=PAGE_SIZE;
    xmcMemAreaTab[C_PHYSMEMAREA].startAddr=addrss;
    xmcMemAreaTab[C_PHYSMEMAREA].mappedAt=addrss;
    xmcMemAreaTab[C_PHYSMEMAREA].memoryRegionOffset = CheckDevMemArea(C_PHYSMEMAREA);
}

#define FILL_TAB(x,y) [x]={#x,#y}
enum APBLIST{APBLIST_NAME, APBLIST_ADDRS, APBLIST_COLUMNS};

char *apbDevList[ZYNQ_N_DEV][APBLIST_COLUMNS]={
    FILL_TAB(ZYNQ_DEV_CAN0, 0xE0008000),
    FILL_TAB(ZYNQ_DEV_CAN1, 0xE0009000),
    FILL_TAB(ZYNQ_DEV_QSPI, 0xE000D000),
    FILL_TAB(ZYNQ_DEV_GPIO, 0xE000A000),
    FILL_TAB(ZYNQ_DEV_I2C0, 0xE0004000),
    FILL_TAB(ZYNQ_DEV_I2C1, 0xE0005000),
#ifndef CONFIG_DEV_UART_0
    FILL_TAB(ZYNQ_DEV_UA0, 0xE0000000),
#endif
#ifndef CONFIG_DEV_UART_1
    FILL_TAB(ZYNQ_DEV_UA1, 0xE0001000),
#endif
    FILL_TAB(ZYNQ_DEV_SPI0, 0xE0006000),
    FILL_TAB(ZYNQ_DEV_SPI1, 0xE0007000),
    FILL_TAB(ZYNQ_DEV_SMC, 0xE000E000),
    FILL_TAB(ZYNQ_DEV_GEM0, 0xE000B000),
    FILL_TAB(ZYNQ_DEV_GEM1, 0xE000C000),
    FILL_TAB(ZYNQ_DEV_USB0, 0xE0002000),
    FILL_TAB(ZYNQ_DEV_USB1, 0xE0003000),
    FILL_TAB(ZYNQ_DEV_TTC1, 0xF8002000),
};

static unsigned int previouslyReservedAPBDevs = 0;
static void APBDEV_AH(xmlNodePtr node, const xmlChar *val)
{
	const char token[2] = " ";
	char *curStr;
	int e;
	unsigned int found = 0;
	unsigned int curr;
	char errorline[70];

	curStr = strtok(((char*)(val)), ((char*)(token)));
	while (curStr != NULL)
	{
		for (e = 0; e < ZYNQ_N_DEV; e++)
		{
			if (apbDevList[e][APBLIST_NAME] && !strcmp(((char *) (curStr)), apbDevList[e][APBLIST_NAME]))
			{
			    curr = (1<<e);
				if(curr & previouslyReservedAPBDevs)
				{
				    sprintf(errorline, "APB Device \"%s\" is used in more than one partition.", apbDevList[e][APBLIST_NAME]);
		            LineError(node->line, errorline);
				}
				unsigned int address=-1;
				sscanf(apbDevList[e][APBLIST_ADDRS], "%x", &address);
				if(address != -1)
					addAPBMemoryArea(address, node->line);
				found |= curr;
				previouslyReservedAPBDevs |= curr;
				break;
			}
		}
		if(e>=ZYNQ_N_DEV)
		{
		    LineError(node->line,
	                "Expected a valid APB Device\n"
	                "  <HwResources>"
	                "      <APBDev device=\"ZYNQ_DEV_UA1 ZYNQ_DEV_USB0\"/>"
	                "  </HwResources>\n"
		        );
		}
		curStr = strtok(NULL, token);
	}
	if( !found )
	{	LineError(node->line, "Expected a valid APB Device"); }

	xmcPartitionTab[C_PARTITION].apbDevices = found;
}

static struct attrXml apbDevs_A={BAD_CAST"device", APBDEV_AH};

struct nodeXml apbDevs_N={BAD_CAST"APBDev",  0, 0, 0, (struct attrXml *[]){&apbDevs_A, 0}, 0};

static void IoPorts_NH0(xmlNodePtr node) {
	xmcPartitionTab[C_PARTITION].ioPortsOffset=xmc.noIoPorts;
}

struct nodeXml ioPorts_N={BAD_CAST"IoPorts", IoPorts_NH0, 0, 0, 0, (struct nodeXml *[]){&rangeIoPort_N, &restrictedIoPort_N, 0}};

void GenerateIoPortTab(FILE *oFile) {
	int i;
	fprintf(oFile, "const struct xmcIoPort xmcIoPortTab[] = {\n");
	for (i=0; i<xmc.noIoPorts; i++) {
		fprintf(oFile, ADDNTAB(1, "[%d] = {\n"), i);
		fprintf(oFile, ADDNTAB(2, ".type = "));
		if (xmcIoPortTab[i].type==XM_IOPORT_RANGE) {
			fprintf(oFile, "XM_IOPORT_RANGE,\n");
			fprintf(oFile, ADDNTAB(2, "{.range.base = 0x%x,\n"), xmcIoPortTab[i].range.base);
			fprintf(oFile, ADDNTAB(2, ".range.noPorts = %d, },\n"), xmcIoPortTab[i].range.noPorts);
		} else {
			fprintf(oFile, "XM_RESTRICTED_IOPORT,\n");
			fprintf(oFile, ADDNTAB(3, "{.restricted.address = 0x%x,\n"), xmcIoPortTab[i].restricted.address);
			fprintf(oFile, ADDNTAB(3, ".restricted.mask = 0x%x,\n"), xmcIoPortTab[i].restricted.mask);
			fprintf(oFile, ADDNTAB(2, "},\n"));
		}
		fprintf(oFile, ADDNTAB(1, "},\n"));
	}
	fprintf(oFile, "};\n\n");
}

void CheckIoPorts(void) {
	xmAddress_t a0, a1, b0, b1;
	xm_u32_t m0, m1;
	int e, line0, line1, iP;

	for (iP=0; iP<xmc.noIoPorts; iP++) {
		if (xmcIoPortTab[iP].type==XM_IOPORT_RANGE) {
			b0=xmcIoPortTab[iP].range.base;
			b1=b0+xmcIoPortTab[iP].range.noPorts*sizeof(xm_u32_t);
			line0=xmcIoPortTabNoL[iP].range.base;
			m0=~0;
		} else {
			b0=xmcIoPortTab[iP].restricted.address;
			m0=xmcIoPortTab[iP].restricted.mask;
			line0=xmcIoPortTabNoL[iP].restricted.address;
			b1=b0+sizeof(xm_u32_t);
		}

		for (e=0; e<noRsvIoPorts; e++) {
			a0=rsvIoPorts[e].base;
			a1=a0+rsvIoPorts[e].offset*sizeof(xm_u32_t);
			if (!((a0>=b1)||(a1<=b0))) {
				LineError(line0, "io-port [0x%lx:%d] reserved by XM",
						rsvIoPorts[e].base, rsvIoPorts[e].offset);
			}
		}

		for (e=iP+1; e<xmc.noIoPorts; e++) {
			if (xmcIoPortTab[e].type==XM_IOPORT_RANGE) {
				a0=xmcIoPortTab[e].range.base;
				a1=a0+xmcIoPortTab[e].range.noPorts*sizeof(xm_u32_t);
				line1=xmcIoPortTabNoL[e].range.base;
				m1=~0;
			} else {
				a0=xmcIoPortTab[e].restricted.address;
				a1=a0+sizeof(xm_u32_t);
				m1=xmcIoPortTab[e].restricted.mask;
				line1=xmcIoPortTabNoL[e].restricted.address;
			}
			if (!((a0>=b1)||(a1<=b0)))
				if (m0&m1)
					LineError(line1, "io-port [0x%lx:%d] already assigned (line %d)", b0, b1-b0, line0);
		}
	}
}

#ifdef CONFIG_MPU
void ArchMpuRsvMem(FILE *oFile) {
}
#endif

#ifdef CONFIG_MMU
//#define PTDL1_SHIFT 20
//#define PTDL2_SHIFT 12
//#define PTDL3_SHIFT 12
//#define PTDL1SIZE 4*1024
//#define PTDL2SIZE 256
//#define PTDL3SIZE 256

#define PTDL1ENTRIES (PTDL1SIZE>>2)
#define PTDL2ENTRIES (PTDL2SIZE>>2)
//#define PTDL3ENTRIES (PTDL3SIZE>>2)
#define XM_VMAPEND 0xfeffffff
#define VA2PtdL1(x) (((x)&0xff000000)>>PTDL1_SHIFT)
#define VA2PtdL2(x) (((x)&0xfc0000)>>PTDL2_SHIFT)
//#define VA2PtdL3(x) (((x)&0x3f000)>>PTDL3_SHIFT)
#define ROUNDUP(r, v) ((((~(r)) + 1)&((v)-1))+(r))

unsigned int** AllocPTL2(xm_s32_t* l2c)
{
    unsigned int** ptdL2;
    (*l2c)++;
    DO_MALLOC(ptdL2, PTDL2SIZE);
    memset(ptdL2, 0, PTDL2SIZE);
    return ptdL2;
}
#define MMap_HYP  2
#define MMap_PART 3
xmAddress_t MMap(xmAddress_t vStartAddr, xmAddress_t vEndAddr, xmAddress_t pStartAddr, unsigned int** ptdL1[PTDL1ENTRIES], xm_s32_t* l2c, unsigned int memType)
{
    xmAddress_t addr;
    xm_s32_t l1e, l2e, i;

    for (addr = vStartAddr; (vStartAddr <= addr) && (addr < vEndAddr);)
    {
        l1e = VA2PtdL1(addr);
        l2e = VA2PtdL2(addr);

        if (!ptdL1[l1e])
        {
            if (!l2e && ((addr + (LPAGE_SIZE)) <= vEndAddr) && !(pStartAddr & ((LPAGE_SIZE) - 1)))
            {
                addr += LPAGE_SIZE;
                pStartAddr += LPAGE_SIZE;
                continue;
            }
          	ptdL1[l1e] = AllocPTL2(l2c);;
            if(memType==MMap_PART)
                for (i = 1; i < (xmc.noPartitions+1); i++)
                	(*l2c)++;
        }

        addr += PAGE_SIZE;
        pStartAddr += PAGE_SIZE;
    }
    return addr;
}

/*TODO ARMPorting: To be tested*/
void ArchMmuRsvMem(FILE *oFile)
{
    xmAddress_t a, b, addr = 0, c;
    unsigned int **ptdL1[PTDL1ENTRIES];
    xm_s32_t e, i, l2c = 0;

    RsvBlock(xmc.noPartitions*sizeof(xmWord_t), ALIGNMENT, "ptdL1S_list");
    // Memory Areas
    char name[20];
    for (e = 0; e < xmc.noPartitions; e++)
    {
        sprintf(name, "ptdL1S  Part %d", e);
        RsvBlock(PTDL1SIZE, PTDL1ALIGN, name);
        sprintf(name, "ptdL1NS Part %d", e);
        RsvBlock(PTDL1SIZE, PTDL1ALIGN, name);
        memset(ptdL1, 0, sizeof(unsigned int **)*PTDL1ENTRIES);
        for (i = 0; i < xmcPartitionTab[e].noPhysicalMemoryAreas; i++)
        {
            if (xmcMemAreaTab[i + xmcPartitionTab[e].physicalMemoryAreasOffset].flags & XM_MEM_AREA_UNMAPPED)
            {
                continue;
            }
            a = xmcMemAreaTab[i + xmcPartitionTab[e].physicalMemoryAreasOffset].mappedAt;
            b = a + xmcMemAreaTab[i + xmcPartitionTab[e].physicalMemoryAreasOffset].size;
            c = xmcMemAreaTab[i + xmcPartitionTab[e].physicalMemoryAreasOffset].startAddr;
           	MMap(a, b, c, ptdL1, &l2c, MMap_PART);
        }
    }

    /**************************
     * Memory reserved for XM *
     **************************/
    memset(ptdL1, 0, sizeof(unsigned int **)*PTDL1ENTRIES);
    ////////////////////////////////////////////
    for (i = 0; i < xmc.hpv.noPhysicalMemoryAreas; i++)
    {
        a = xmcMemAreaTab[i + xmc.hpv.physicalMemoryAreasOffset].mappedAt;
        b = a + xmcMemAreaTab[i + xmc.hpv.physicalMemoryAreasOffset].size;
        c = xmcMemAreaTab[i + xmc.hpv.physicalMemoryAreasOffset].startAddr;
        MMap(a, b, c, ptdL1, &l2c, MMap_HYP);
    }
    /*PTL2 for Access XM NSAreas.*/
    l2c += xmc.noPartitions;

    /**************************
     * Communication Channels *
     **************************/
    for (i = 0; i < xmc.noCommChannels; i++)
    {
        xmAddress_t physAddress = xmcCommChannelTab[i].q.address;
        xmSize_t size = xmcCommChannelTab[i].q.size;
        if (xmcCommChannelTab[i].type == XM_SAMPLING_CHANNEL)
        {
            physAddress = xmcCommChannelTab[i].s.address;
            size = xmcCommChannelTab[i].s.size;
        }
        if (physAddress && size)
        {
            a = addr;
            b = addr + size;
            c = physAddress;
            MMap(a, b, c, ptdL1, &l2c, MMap_HYP);
        }
    }

    /*********************************
     * Memory reserved for Memblocks *
     *********************************/
    memset(ptdL1, 0, sizeof(unsigned int **)*PTDL1ENTRIES);
    for (i = 0; i < xmc.deviceTab.noMemBlocks; i++)
    {
        a = xmcMemAreaTab[i + xmc.deviceTab.memBlocksOffset].startAddr;
        b = a + xmcMemAreaTab[i + xmc.deviceTab.memBlocksOffset].size;
        c = xmcMemAreaTab[i + xmc.deviceTab.memBlocksOffset].startAddr;
        MMap(a, b, c, ptdL1, &l2c, MMap_HYP);
    }

    /**********
     * PTD L2 *
     **********/
    for (e = 0; e < l2c; e++)
    {
    	RsvBlock(PTDL2SIZE, PTDL2ALIGN, "ptdL2");
    }

    /********
     * PCTs *
     ********/
    for (e = 0; e < xmc.noPartitions; e++)
    {
        RsvBlock(PTDL2SIZE, PTDL2ALIGN, "partition control tables's PTDL2");
        RsvBlock(PTDL2SIZE, PTDL2ALIGN, "partitions Own PTL");
    }

    /***********
     * DEVICES *
     ***********/
    for (e = 0; e < xmc.noPartitions; e++)
    {
    	RsvBlock(PTDL2SIZE, PTDL2ALIGN, "GIC Controler NonSec ptdL2");
    }
}
#endif
