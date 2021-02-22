/*
 * $FILE: xmcbuild.c
 *
 * Compile the c code
 *
 * $VERSION$
 *
 * Authors: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license terms.
 */

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

#include <endianess.h>
#include <xm_inc/digest.h>
#include <xm_inc/xmconf.h>

//#define CMD_TMPL "%s/xmcbuild.sh %s %s"
//#define CMD2_TMPL "xmcbuild.sh %s %s"

#define CONFIG_FILE "xmconfig"

#define BUFFER_SIZE 256

#ifdef CONFIG_ARM
static char ldsContent[]=						\
    "SECTIONS\n"								\
    "{\n" 										\
    "         . = 0x0;\n"						\
    "         .data ALIGN (8) : {\n"			\
    "      	     *(.rodata.hdr)\n"				\
    "    	     *(.rodata)\n"					\
    "    	     *(.data)\n"					\
    "                . = . + 0x100;\n"			\
    "                _memObjTab = .;\n"			\
    "                *(.data.memobj)\n"			\
    "                LONG(0);\n"				\
    "        }\n"								\
    "\n"										\
    "        _dataSize = .;\n"					\
	"\n"										\
    "        .bss ALIGN (8) : {\n"				\
    "            *(.bss)\n"						\
    "    	     *(.bss.mempool)\n"				\
    "    	}\n"								\
    "\n"										\
    "    	_xmcSize = .;\n"					\
    "\n"										\
    " 	/DISCARD/ : {\n"						\
    "	   	*(.text)\n"							\
    "    	*(.note)\n"							\
    "	    *(.comment*)\n"						\
    "	}\n"									\
    "}\n";

/*TBI:ARMPorting:
"    	     . = ALIGN(64*1024*1024);\n"	\
"    	     *(.bss.nsmempool)\n"			\
*/


#else
static char ldsContent[]="OUTPUT_FORMAT(\"binary\")\n" \
    "SECTIONS\n" \
    "{\n" \
    "         . = 0x0;\n" \
    "         .data ALIGN (8) : {\n" \
    "      	     *(.rodata.hdr)\n" \
    "    	     *(.rodata)\n" \
    "    	     *(.data)\n" \
    "                _memObjTab = .;\n" \
    "                *(.data.memobj)\n" \
    "                LONG(0);\n" \
    "        }\n" \
    "\n" \
    "        _dataSize = .;\n"   \
   "\n" \
    "        .bss ALIGN (8) : {\n" \
    "                *(.bss)\n" \
    "    	     *(.bss.mempool)\n" \
    "    	}\n" \
    "\n" \
    "    	_xmcSize = .;\n" \
    "\n" \
    " 	/DISCARD/ : {\n" \
    "	   	*(.text)\n" \
    "    	*(.note)\n" \
    "	    	*(.comment*)\n" \
    "	}\n" \
    "}\n";
#endif

static void WriteLdsFile(char *ldsFile) {    
    int fd=mkstemp(ldsFile);

    if ((fd<0)||(write(fd, ldsContent, strlen(ldsContent))!=strlen(ldsContent)))
	EPrintF("unable to create the lds file\n");
    close(fd);
}

#define DEV_INC_PATH "/user/libxm/include"
#define INST_INC_PATH "/include"

void ExecXmcBuild(char *path, char *in, char *out) {
    char *xmPath, *sysCmd, *target_cc, *target_cflags_arch, *target_cflags_arch2, *cflags;
    char ldsFile[]="ldsXXXXXX";
    char *objFile;
#ifdef CONFIG_ARM
    objFile=strdup("objf");
#else
    objFile=strdup(out);
#endif

    if (!(xmPath=getenv("XTRATUM_PATH")))
            EPrintF("The XTRATUM_PATH enviroment variable must be set\n");

    if (!(target_cc=getenv("TARGET_CC")))
            EPrintF("The TARGET_CC enviroment variable must be set\n");

    if (!(target_cflags_arch2=getenv("TARGET_CFLAGS_ARCH")))
            EPrintF("The TARGET_CFLAGS_ARCH enviroment variable must be set\n");
//    extern char *replace_char (char *str, char find, char *replace);
    target_cflags_arch = strdup(target_cflags_arch2);
//    target_cflags_arch = replace_char(target_cflags_arch, '\'', " ");


#define FILE_CONFIG_H "xm_inc/config.h"
#define FILE_ARCH_TYPES_H "xm_inc/arch/arch_types.h"
#define ENTRY_ADDRESS (0x0)

#define INCLUDE_PATH_LIBXM "/user/libxm/include"
#define INCLUDE_PATH "/include"

#define CMD "%s %s -x c %s --include %s --include %s %s -o %s -Wl,--entry=0x%08x,-T%s\n"
#define CFLAGS "-O2 -Wall -I%s%s -I%s%s -nostdlib -nostdinc"

    DO_MALLOC(cflags, strlen(CFLAGS)
    		+strlen(xmPath)*2
    		+strlen(INCLUDE_PATH_LIBXM)
    		+strlen(INCLUDE_PATH)
    		+1);
    sprintf(cflags, CFLAGS, xmPath, INCLUDE_PATH_LIBXM, xmPath, INCLUDE_PATH);

    WriteLdsFile(ldsFile);

    DO_MALLOC(sysCmd, strlen(CMD)
    			+strlen(target_cc)
    			+strlen(target_cflags_arch)
    			+strlen(cflags)
    			+strlen(FILE_CONFIG_H)
    			+strlen(FILE_ARCH_TYPES_H)
    			+strlen(in)
    			+strlen(objFile)
    			+8
    			+strlen(ldsFile)
    			+1);
    sprintf(sysCmd, CMD,
    		target_cc,
    		target_cflags_arch,
    		cflags,
    		FILE_CONFIG_H,
    		FILE_ARCH_TYPES_H,
    		in,
    		objFile,
    		ENTRY_ADDRESS,
    		ldsFile);

    fprintf(stderr, "command=[\n%s]\n", sysCmd);

    if (system(sysCmd))
    	fprintf(stderr, "Error building xmc file\n");

   	free(sysCmd);
#ifdef CONFIG_ARM
#define CMD_OBJCOPY "%s -O binary %s %s\n"
    char * objcopy;
    if (!(objcopy=getenv("TARGET_OBJCOPY")))
            EPrintF("The TARGET_OBJCOPY enviroment variable must be set\n");

    DO_MALLOC(sysCmd,
    		strlen(CMD_OBJCOPY)+
    		strlen(objcopy)+
//    		strlen(configPath)+
    		strlen(objFile)+
    		strlen(out)+
    		1);
    sprintf(sysCmd, CMD_OBJCOPY,
//    		configPath,
    		objcopy,
    		objFile,
    		out);

    fprintf(stderr, "command=[%s]\n", sysCmd);

    if (system(sysCmd))
    	fprintf(stderr, "Error building xmc file\n");

    free(sysCmd);

    unlink(objFile);
#endif
    unlink(ldsFile);
}

void CalcDigest(char *out) {
    xm_u8_t digest[XM_DIGEST_BYTES];
    struct digestCtx digestState;
    xm_u32_t signature;
    xmSize_t dataSize;
    xm_u8_t *buffer;
    int fd;

    memset(digest, 0, XM_DIGEST_BYTES);
    if ((fd=open(out, O_RDWR))<0)
        EPrintF("File %s cannot be opened\n", out);

    fsync(fd);
    DO_READ(fd, &signature, sizeof(xm_u32_t));
    if (RWORD(signature)!=XMC_SIGNATURE)
        EPrintF("File signature unknown (%x)\n", signature);

    lseek(fd, offsetof(struct xmc, dataSize), SEEK_SET);
    
    DO_READ(fd, &dataSize, sizeof(xmSize_t));
    dataSize=RWORD(dataSize);
    DO_MALLOC(buffer, dataSize);
    lseek(fd, 0, SEEK_SET);
    DO_READ(fd, buffer, dataSize);
    DigestInit(&digestState);
    DigestUpdate(&digestState, buffer, offsetof(struct xmc, digest));
    DigestUpdate(&digestState, (xm_u8_t *)digest, XM_DIGEST_BYTES);
    DigestUpdate(&digestState, &buffer[offsetof(struct xmc, dataSize)], dataSize-offsetof(struct xmc, dataSize));
    DigestFinal(digest, &digestState);
    free(buffer);

    lseek(fd, offsetof(struct xmc, digest), SEEK_SET);
    DO_WRITE(fd, digest, XM_DIGEST_BYTES);
    close(fd);
}
