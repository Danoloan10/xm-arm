/*
 * $FILE: extractinfo.c
 *
 * Extracts information from the XM binary
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */
/*
 * Changelog:
 * - [15/02/16:XM-ARM-2:SPR-160215-02:#110] Get XM code and data areas size.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define TMPFILE "/tmp/info.tmp"

#if (((__BYTE_ORDER== __LITTLE_ENDIAN) && defined(CONFIG_TARGET_LITTLE_ENDIAN)) ||((__BYTE_ORDER==__BIG_ENDIAN) && defined(CONFIG_TARGET_BIG_ENDIAN)))
#define SET_BYTE_ORDER(i) (i)
#else
#define SET_BYTE_ORDER(i) ((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff)
#endif

#define USAGE \
    "USAGE: extractinfo <xmfile>\n"

#define C_HEADER \
"/*\n" \
" * $VERSION$\n" \
" *\n" \
" * Author: Miguel Masmano <mmasmano@ai2.upv.es>\n" \
" */\n\n" \
"#ifndef _XM_INFO_H_\n" \
"#define _XM_INFO_H_\n\n"

#define C_FOOT \
"#endif\n"

int main(int argc, char *argv[]) {
    char cmd[256];
    int fd, val1, val2, noElem;
    FILE * fs;
    
    if (argc!=2) {
	fprintf(stderr, USAGE);
	exit(-1);
    }

    sprintf(cmd, TARGET_OBJCOPY" -O binary -j .rsv_hwirqs %s "TMPFILE, argv[1]);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
	
    fd=open(TMPFILE, O_RDONLY);
    fprintf(stderr, C_HEADER);
    fprintf(stderr, "#ifdef _RSV_HW_IRQS_\n");
    fprintf(stderr, "static xm_u32_t rsvHwIrqs[]={\n");
    noElem=0;
    while (read(fd, &val1, 4)>0) {
	fprintf(stderr, "    0x%x,\n", SET_BYTE_ORDER(val1));
	noElem++;
    }    
    fprintf(stderr, "};\n\n");
    fprintf(stderr, "static xm_s32_t noRsvHwIrqs=%d;\n\n", noElem);
    fprintf(stderr, "#endif\n");
    sprintf(cmd, TARGET_OBJCOPY" -O binary -j .rsv_ioports %s "TMPFILE, argv[1]);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
    close(fd);
    fd=open(TMPFILE, O_RDONLY);
    fprintf(stderr, "#ifdef _RSV_IO_PORTS_\n");
    fprintf(stderr, "static struct {\n"
	    "    xm_u32_t base;\n"
	    "    xm_s32_t offset;\n"
	    "} rsvIoPorts[]={\n");
    noElem=0;
    while ((read(fd, &val1, 4)>0)&&(read(fd, &val2, 4)>0)) {
	fprintf(stderr, "    {.base=0x%x, .offset=%d,},\n", SET_BYTE_ORDER(val1), SET_BYTE_ORDER(val2));
	noElem++;
    }
    fprintf(stderr, "};\n\n");
    fprintf(stderr, "static xm_s32_t noRsvIoPorts=%d;\n\n", noElem);
    fprintf(stderr, "#endif\n");
    sprintf(cmd, TARGET_OBJCOPY" -O binary -j .rsv_physpages %s "TMPFILE, argv[1]);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
    close(fd);
    fd=open(TMPFILE, O_RDONLY);
    fprintf(stderr, "#ifdef _RSV_PHYS_PAGES_\n");
    fprintf(stderr, "static struct {\n"
	    "    xm_u32_t address;\n"
	    "    xm_s32_t noPag;\n"
	    "} rsvPhysPages[]={\n");
    noElem=0;
    while ((read(fd, &val1, 4)>0)&&(read(fd, &val2, 4)>0)) {
	fprintf(stderr, "    {.address=0x%x, .noPag=%d,},\n", SET_BYTE_ORDER(val1), SET_BYTE_ORDER(val2));
	noElem++;
    }
    fprintf(stderr, "};\n\n");
    fprintf(stderr, "static xm_s32_t noRsvPhysPages=%d;\n\n", noElem);
    fprintf(stderr, "#endif\n");
    close(fd);

    char *target_size;
	if (!(target_size=getenv("TARGET_SIZE")))
		fprintf(stderr, "The TARGET_SIZE enviroment variable must be set\n");

    sprintf(cmd, "%s -A -x %s > %s", target_size, argv[1], TMPFILE);
    if (WEXITSTATUS(system(cmd))) {
	fprintf(stderr, USAGE);
	exit(-1);
    }
    fs=fopen(TMPFILE, "r");
    char buffer[100], c;
    while((fread(&c, 1, 1, fs)>0)&&(c!='\n'));
    while((fread(&c, 1, 1, fs)>0)&&(c!='\n'));

#	define DISCARD ((void*)(-1))
#	define XM_CODE ((void*)(0))
#	define XM_DATA ((void*)(1))
    const void * sections[20][2]={
    		{"boot", XM_CODE},
    		{"text", XM_CODE},
    		{"xmHdr", XM_CODE},
    		{"rsv_ioports", XM_CODE},
    		{"rodata", XM_CODE},
    		{"build_info", XM_CODE},
    		{"ARM.exidx", XM_CODE},
    		{"data", XM_DATA},
    		{"bss", XM_DATA},
    		{"ARM.attributes", DISCARD},
    		{"debug_info", DISCARD},
    		{"debug_abbrev", DISCARD},
    		{"debug_aranges", DISCARD},
    		{"debug_line", DISCARD},
    		{"debug_str", DISCARD},
    		{"debug_frame", DISCARD},
    		{"debug_loc", DISCARD},
    		{"debug_ranges", DISCARD},
			{0,0},
    };

    unsigned int sizes[2]={0,0};
    while(fscanf(fs,".%s%x%x\n",buffer, &val1, &val2)>0)
    {
    	unsigned int s=-1;
    	while(sections[++s][0])
    	{
    		if(strcmp(buffer, ((char*)(sections[s][0])))==0)
    		{
    			if(sections[s][1] != DISCARD)
    			{
    				sizes[((unsigned long)(sections[s][1]))] += val1;
    			}
    			break;
    		}
    	}
    }
    fclose(fs);

    fprintf(stderr, "\n\n#define XM_CORE_SIZE 0x%x\t\t/*%dKB*/\n", sizes[((unsigned long)(XM_CODE))] , sizes[((unsigned long)(XM_CODE))]/1024);
    fprintf(stderr, "#define XM_CORE_DATA_SIZE 0x%x\t/*%dKB*/\n\n", sizes[((unsigned long)(XM_DATA))], sizes[((unsigned long)(XM_DATA))]/1024);

    unlink(TMPFILE);
    fprintf(stderr, C_FOOT);

    return 0;
}
