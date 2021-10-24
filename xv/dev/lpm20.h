#ifndef _DEV_LPM20_H_
#define _DEV_LPM20_H_ 1
#include <vcpu16.h>

#define LPM20_FREQUENCY         50
#define LPM20_MAX_MEMORY        0x1000
#define LPM20_IOPORT_TEXT_OFF   0x1F01
#define LPM20_IOPORT_CUR_POS    0x1F02
#define LPM20_IOPORT_SCR_DIMS   0x1F03

void init_lpm20(void);
void shutdown_lpm20(void);
void lpm20_draw(const struct vcpu *cpu);
int lpm20_ioread(struct vcpu *cpu, unsigned short port, unsigned short *value);
int lpm20_iowrite(struct vcpu *cpu, unsigned short port, unsigned short value);

#endif
