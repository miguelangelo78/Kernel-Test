/*
 * mouse.h
 *
 *  Created on: 21/07/2016
 *      Author: Miguel
 */

#ifndef SRC_MODULES_MOUSE_MOUSE_H_
#define SRC_MODULES_MOUSE_MOUSE_H_

/********************************/
/**** Mouse Macro Constants: ****/
/********************************/
#define MOUSE_MAGIC       0xFEED1234

#define MOUSE_PACKETS_IN_PIPE 1024
#define MOUSE_DISCARD_POINT   32

#define MOUSE_IRQ         Kernel::CPU::IRQ::IRQ_MOUSE

#define MOUSE_PORT        0x60
#define MOUSE_STATUS      0x64
#define MOUSE_ABIT        0x02
#define MOUSE_BBIT        0x01
#define MOUSE_WRITE       0xD4
#define MOUSE_F_BIT       0x20
#define MOUSE_V_BIT       0x08

#define MOUSE_DEFAULT     0
#define MOUSE_SCROLLWHEEL 1
#define MOUSE_BUTTONS     2
/********************************/

/********************************************/
/**** Mouse Struct and Enum Definitions: ****/
/********************************************/
typedef enum {
	LEFT_CLICK        = 0x01,
	RIGHT_CLICK       = 0x02,
	MIDDLE_CLICK      = 0x04,
	MOUSE_SCROLL_UP   = 0x10,
	MOUSE_SCROLL_DOWN = 0x20,
} mouse_click_t;

typedef struct {
	uint32_t magic;
	int32_t x_difference;
	int32_t y_difference;
	mouse_click_t buttons;
} mouse_device_packet_t;
/********************************************/

#endif /* SRC_MODULES_MOUSE_MOUSE_H_ */
