#ifndef __ARM_H__
#define __ARM_H__

#define PSR_MODE_USR 0x10
#define PSR_MODE_FIQ 0x11
#define PSR_MODE_IRQ 0x12
#define PSR_MODE_SVC 0x13
#define PSR_MODE_ABT 0x17
#define PSR_MODE_UND 0x1B
#define PSR_MODE_SYS 0x1F

#define PSR_INT_DISABLED    0x80
#define PSR_FINT_DISABLED   0x40
#define PSR_THUMB_EXECUTION 0x20

#define DEFAULT_SYSMODE (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SYS)
#define DEFAULT_IRQMODE (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_IRQ)
#define DEFAULT_SVCMODE (PSR_INT_DISABLED | PSR_FINT_DISABLED | PSR_MODE_SVC)

#define SWI_HANDLER_ADDR 0x28
#define HWI_HANDLER_ADDR 0x38

#endif /*__ARM_H__*/
