/*
 * $FILE: memcpy.S
 *
 * $VERSION$
 *
 * Author: Javier Coronel <jcoronel@fentiss.com>
 *
 * $LICENSE:
 * (c) Fent Innovative Software Solutions. All rights reserved.
 *     Read LICENSE file for the license terms.
 */

#include <comp.h>
#include <linkage.h>

ASM_ALIGN

.global _memcpy

_memcpy:
        push {r3-r11}
        cmp r2,#0
        beq 7f
        orr r3,r0,r1
        ands r3,r3,#3
        beq 11f
        orr r3,r0,r1
        ands r3,r3,#1
        beq 55f
        b 66f

11: 
        lsrs r3,r2,#5
        beq 22f
        lsl r3,r3,#5
        subs r3,r2,r3
        subne r2,r2,r3
        
1:      /*multiplo de 32*/
        ldmia r1!, {r4-r11}
        stmia r0!, {r4-r11}
        subs r2, r2, #32
        bne 1b
        movs r2,r3
        beq 7f

22:     lsrs r3,r2,#4
        beq 33f
        lsl r3,r3,#4
        subs r3,r2,r3
        subne r2,r2,r3

2:      /*multiplo de 16*/
        ldmia r1!, {r4-r7}
        stmia r0!, {r4-r7}
        subs r2, r2, #16
        bne 2b
        movs r2,r3
        beq 7f

33:     lsrs r3,r2,#3
        beq 44f
        lsl r3,r3,#3
        subs r3,r2,r3
        subne r2,r2,r3

3:      /*multiplo de 8*/
        ldmia r1!, {r4-r5}
        stmia r0!, {r4-r5}
        subs r2, r2, #8
        bne 3b
        movs r2,r3
        beq 7f

44:     lsrs r3,r2,#2
        beq 55f
        lsl r3,r3,#2
        subs r3,r2,r3
        subne r2,r2,r3

4:      /*multiplo de 4*/
        ldr r4,[r1]
        str r4,[r0]
        add r0, r0, #4
        add r1, r1, #4
        subs r2, r2, #4
        bne 3b
        movs r2,r3
        beq 7f

55:     lsrs r3,r2,#1
        beq 66f
        lsl r3,r3,#1
        subs r3,r2,r3
        subne r2,r2,r3

5:      /*multiplo de 2*/
        ldrh r4,[r1]
        strh r4,[r0]
        add r0, r0, #2
        add r1, r1, #2
        subs r2,r2, #2
        bne 5b
        movs r2,r3
        beq 7f

66:     
6:      /*1 byte*/
        ldrb r4, [r1]
        strb r4, [r0]
        add r0, r0, #1
        add r1, r1, #1
        subs r2, r2, #1
        bne 6b

7:
        pop {r3-r11}
        bx lr

