/*
 *
 *
 */

#ifndef _CL_H
#define _CL_H

#define DEFAULT_CONTROL_PORT 4660
#define DEFAULT_DATA_PORT 24

#define CL_VERSION	0x0000
#define CL_ID		0x04
#define CL_SCR		0x08
#define CL_RCR		0x09
#define CL_FCR		0x0c
#define CL_KEYWORD	0x0d
#define CL_FFTH		0x10

#define CL_SCR_VMESYSRST 0x80
#define CL_SCR_RECONFIG 0x40

#define CL_RCR_FINESSE 0x80

#define CL_FCR_ENABLE_A 0x01
#define CL_FCR_ENABLE_B 0x02
#define CL_FCR_ENABLE_C 0x04
#define CL_FCR_ENABLE_D 0x08

#define CL_FINESSE	0x400
#define CL_FINESSE_ALL	0x600

#endif
