/*
 * $FILE: audit.h
 *
 * Core trace events
 *
 * $VERSION$
 * 
 * Author: Miguel Masmano <mmasmano@fentiss.com>
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _XM_AUDIT_H_
#define _XM_AUDIT_H_

#ifdef CONFIG_AUDIT_EVENTS

#ifdef _XM_KERNEL_
#include <objects/trace.h>
#else
#include <xm_inc/objects/trace.h>
#endif

#ifdef _XM_KERNEL_
#include <assert.h>

#define XM_AUDIT_START_WATCHDOG 0x1
#define XM_AUDIT_END_WATCHDOG 0x2
#define XM_AUDIT_END_PARTITION 0x3
#define XM_AUDIT_BEGIN_CS 0x4
#define XM_AUDIT_END_CS 0x5
#define XM_AUDIT_BEGIN_TRAP 0x6
#define XM_AUDIT_END_TRAP 0x7
#define XM_AUDIT_HYP 0x8
#define XM_AUDIT_BEGIN_ASM_HYP 0x9
#define XM_AUDIT_END_ASM_HYP 0x9

extern void RaiseAuditEvent(xm_u8_t partId, xm_u8_t event);
#endif
#endif

#endif
