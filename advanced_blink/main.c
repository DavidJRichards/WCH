
// Blink LEDs connected to port P1 (bits 4 to 7)

#include <stdint.h>
#include <ch559.h>

#define MASK_P1_OC 0x0F

static inline void delay() {
    uint32_t i;
    for (i = 0; i < (5 * 12000UL); i++){}
        __asm__("nop");
}

void run_led () {
	uint8_t i;
	for(i=0; i<8; i++) {
		P1 = (1 << i);
		delay();
	}
}

void main() {

	PORT_CFG |= bP1_DRV | (MASK_P1_OC & ~bP1_OC); // Set port P1 driving current and output mode.
        P1_DIR = 0xFF; // Set all bits to output mode

	// Turning all bits to high
	P1 = 0xFF;
	delay();
	delay();
	delay();
	delay();


	// Turning all bits to low
	P1 = 0xAA;
	delay();
	delay();
	delay();
	delay();

	// Turning alternate bits to high
	P1 = 0x55;
	delay();
	delay();
	delay();
	delay();

	// Turning upper half bits to high
	P1 = 0xF0;
	delay();
	delay();
	delay();
	delay();

	// Turning lower half bits to high
	P1 = 0x0F;
	delay();
	delay();
	delay();
	delay();

	while (1) {
		run_led();
	}
}
