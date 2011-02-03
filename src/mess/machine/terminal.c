#include "emu.h"
#include "machine/terminal.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
enum
{
	START = 0,
	STOP = 9
};

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 24
typedef struct _terminal_state terminal_state;
struct _terminal_state
{
	UINT8 framecnt;
	UINT8 buffer[TERMINAL_WIDTH*TERMINAL_HEIGHT];
	UINT8 x_pos;
	UINT8 y_pos;
	UINT8 last_code;
	UINT8 scan_line;
	int rx_state;
	int tx_state;
	UINT8 rx_shift;
	UINT8 tx_shift;
	devcb_resolved_write8 terminal_keyboard_func;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/
INLINE terminal_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GENERIC_TERMINAL);

	return (terminal_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const terminal_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GENERIC_TERMINAL);
	return (const terminal_interface *) device->baseconfig().static_config();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static const UINT8 terminal_font[256*16] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x92, 0x44, 0x92, 0x44, 0x92, 0x44, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x3e, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xf0, 0x80, 0xe0, 0x80, 0x9e, 0x10, 0x1c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x78, 0x80, 0x80, 0x78, 0x3c, 0x22, 0x3c, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x80, 0x80, 0xf8, 0x3e, 0x20, 0x3c, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x38, 0x44, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x10, 0x10, 0xfe, 0x10, 0x10, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x88, 0xc8, 0xa8, 0x98, 0x88, 0x20, 0x20, 0x20, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x88, 0x88, 0x50, 0x20, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02, 0x08, 0x20, 0x80, 0x20, 0x08, 0x02, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x20, 0x08, 0x02, 0x08, 0x20, 0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xfe, 0x24, 0x24, 0x24, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02, 0x04, 0xfe, 0x10, 0xfe, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1c, 0x22, 0x20, 0xf8, 0x20, 0x78, 0xa6, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x48, 0x48, 0xfc, 0x48, 0xfc, 0x48, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x7c, 0x90, 0x7c, 0x12, 0x7c, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xc2, 0xc4, 0x08, 0x10, 0x20, 0x46, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x70, 0x88, 0x88, 0x70, 0x8a, 0x84, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x18, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x44, 0x28, 0xfe, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x10, 0x10, 0xfe, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x78, 0x84, 0x02, 0x1c, 0x60, 0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x04, 0x08, 0x1c, 0x02, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x18, 0x28, 0x48, 0xfe, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x80, 0xbc, 0xc2, 0x02, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x3c, 0x42, 0x80, 0xbc, 0xc2, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x78, 0x84, 0x86, 0x7a, 0x02, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02, 0x08, 0x20, 0x80, 0x20, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x20, 0x08, 0x02, 0x08, 0x20, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x82, 0x0c, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x82, 0x8a, 0x92, 0x9c, 0x80, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x28, 0x44, 0x82, 0xfe, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfc, 0x42, 0x42, 0x7c, 0x42, 0x42, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x3c, 0x42, 0x80, 0x80, 0x80, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xf8, 0x44, 0x42, 0x42, 0x42, 0x44, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x80, 0x80, 0xf8, 0x80, 0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x80, 0x80, 0xf8, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x3c, 0x42, 0x80, 0x80, 0x8e, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0e, 0x04, 0x04, 0x04, 0x04, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x8c, 0xb0, 0xc0, 0xb0, 0x8c, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0xc6, 0xaa, 0x92, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0xc2, 0xa2, 0x92, 0x8a, 0x86, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfc, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x82, 0x82, 0x82, 0x8a, 0x84, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfc, 0x82, 0x82, 0xfc, 0x88, 0x84, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7c, 0x82, 0x80, 0x7c, 0x02, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x82, 0x82, 0x92, 0x92, 0xaa, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xfe, 0x04, 0x08, 0x10, 0x20, 0x40, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x3c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x30, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7c, 0x02, 0x7e, 0x82, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x80, 0xbc, 0xc2, 0x82, 0xc2, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x42, 0x80, 0x40, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02, 0x02, 0x7a, 0x86, 0x82, 0x86, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7c, 0x82, 0xfe, 0x80, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1c, 0x22, 0x20, 0xf8, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7c, 0x84, 0x84, 0x7c, 0x04, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//g
	0x00, 0x80, 0x80, 0xbc, 0xc2, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x04, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x80, 0x88, 0x90, 0xe0, 0x88, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xc4, 0xaa, 0x92, 0x92, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xbc, 0xc2, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xbc, 0xc2, 0xc2, 0xbc, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7a, 0x86, 0x86, 0x7a, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x9c, 0x62, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7c, 0x80, 0x7c, 0x02, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x20, 0x20, 0xf8, 0x20, 0x20, 0x24, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x84, 0x84, 0x84, 0x84, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x82, 0x82, 0x44, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x82, 0x82, 0x92, 0xaa, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x84, 0x48, 0x30, 0x48, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x84, 0x84, 0x8c, 0x74, 0x04, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xfe, 0x04, 0x18, 0x20, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0e, 0x10, 0x10, 0x60, 0x10, 0x10, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xe0, 0x10, 0x10, 0x0c, 0x10, 0x10, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x62, 0x92, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void terminal_scroll_line(device_t *device)
{
	terminal_state *term = get_safe_token(device);

	memcpy(term->buffer,term->buffer+TERMINAL_WIDTH,(TERMINAL_HEIGHT-1)*TERMINAL_WIDTH);
	memset(term->buffer + TERMINAL_WIDTH*(TERMINAL_HEIGHT-1),0x20,TERMINAL_WIDTH);
}

static void terminal_write_char(device_t *device,UINT8 data) {
	terminal_state *term = get_safe_token(device);

	term->buffer[term->y_pos*TERMINAL_WIDTH+term->x_pos] = data;
	term->x_pos++;
	if (term->x_pos >= TERMINAL_WIDTH)
	{
		term->x_pos = 0;
		term->y_pos++;
		if (term->y_pos >= TERMINAL_HEIGHT)
		{
			terminal_scroll_line(device);
			term->y_pos = TERMINAL_HEIGHT-1;
		}
	}
}

static void terminal_clear(device_t *device) {
	terminal_state *term = get_safe_token(device);

	memset(term->buffer,0x20,TERMINAL_WIDTH*TERMINAL_HEIGHT);
	term->x_pos = 0;
	term->y_pos = 0;
}

READ_LINE_DEVICE_HANDLER( terminal_serial_r )
{
	terminal_state *term = get_safe_token(device);

	UINT8 data = 1;

	switch (term->tx_state)
	{
	case STOP:
		data = 1;
		break;

	case START:
		data = 0;
		term->tx_state++;
		break;

	default:
		data = BIT(term->tx_shift, 0);
		term->tx_shift >>= 1;
		term->tx_state++;
		break;
	}

	return data;
}

WRITE_LINE_DEVICE_HANDLER( terminal_serial_w )
{
	terminal_state *term = get_safe_token(device);

	switch (term->rx_state)
	{
	case START:
		if (!state) term->rx_state++;
		break;

	case STOP:
		terminal_write(device, 0, term->rx_shift);
		term->rx_state = START;
		break;

	default:
		term->rx_shift >>= 1;
		term->rx_shift |= (state << 7);
		term->rx_state++;
		break;
	}
}

WRITE8_DEVICE_HANDLER ( terminal_write )
{
	terminal_state *term = get_safe_token(device);
	if (data > 0x1f)
	{
		// printable char
		if (data!=0x7f) terminal_write_char(device,data);
	}
	else
	{
		switch(data)
		{
			case 0x07 : // bell
					break;

			case 0x08:	if (term->x_pos) term->x_pos--;
					break;

			case 0x09:	term->x_pos ++;
					term->x_pos += term->x_pos % 8;
					if (term->x_pos >= TERMINAL_WIDTH)
						term->x_pos = TERMINAL_WIDTH-1;
					break;

			case 0x0a:	term->y_pos++;
					term->x_pos = 0;
					if (term->y_pos >= TERMINAL_HEIGHT)
					{
						terminal_scroll_line(device);
						term->y_pos = TERMINAL_HEIGHT-1;
					}
					break;

			case 0x0b:	if (term->y_pos) term->y_pos--;
					break;

			case 0x0d:	term->x_pos = 0;
					break;

			case 0x1e:	term->x_pos = 0;
					term->y_pos = 0;
					break;
		}
	}
}

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/
static void generic_terminal_update(device_t *device, bitmap_t *bitmap, const rectangle *cliprect)
{
	terminal_state *term = get_safe_token(device);
	UINT8 options = input_port_read(device->machine, "TERM_CONF");
	UINT16 cursor = term->y_pos * TERMINAL_WIDTH + term->x_pos;
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	term->framecnt++;

	for (y = 0; y < TERMINAL_HEIGHT; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16  *p = BITMAP_ADDR16(bitmap, sy++, 0);

			for (x = ma; x < ma + TERMINAL_WIDTH; x++)
			{
				chr = term->buffer[x];
				gfx = terminal_font[(chr<<4) | ra ];

				if ((x == cursor) && (options & 1)) // at cursor position and want a cursor
				{
					if ((options & 2) || (ra == 9)) // block, or underline & at bottom line
					{
						if ((options & 4) && (term->framecnt & 8)) // want blink & time to blink
						{
						}
						else
						{
							if (options & 8)
								gfx ^= 0xff; // invert
							else
								gfx |= 0xff; // overwrite
						}
					}
				}
						
				/* Display a scanline of a character */
				*p++ = ( gfx & 0x80 ) ? 1 : 0;
				*p++ = ( gfx & 0x40 ) ? 1 : 0;
				*p++ = ( gfx & 0x20 ) ? 1 : 0;
				*p++ = ( gfx & 0x10 ) ? 1 : 0;
				*p++ = ( gfx & 0x08 ) ? 1 : 0;
				*p++ = ( gfx & 0x04 ) ? 1 : 0;
				*p++ = ( gfx & 0x02 ) ? 1 : 0;
				*p++ = ( gfx & 0x01 ) ? 1 : 0;
			}
		}
		ma+=TERMINAL_WIDTH;
	}
}

static UINT8 row_number(UINT8 code) {
	if BIT(code,0) return 0;
	if BIT(code,1) return 1;
	if BIT(code,2) return 2;
	if BIT(code,3) return 3;
	if BIT(code,4) return 4;
	if BIT(code,5) return 5;
	if BIT(code,6) return 6;
	if BIT(code,7) return 7;
	return 0;
}

UINT8 terminal_keyboard_handler(running_machine *machine, devcb_resolved_write8 *callback, UINT8 last_code, UINT8 *scan_line, UINT8 *tx_shift, int *tx_state)
{
	static const char *const keynames[] = { "TERM_LINE0", "TERM_LINE1", "TERM_LINE2", "TERM_LINE3", "TERM_LINE4", "TERM_LINE5", "TERM_LINE6" };
	int i;
	UINT8 code;
	UINT8 key_code = 0;
	UINT8 retVal = 0;
	UINT8 shift = BIT(input_port_read(machine, "TERM_LINEC"),1);
	UINT8 caps  = BIT(input_port_read(machine, "TERM_LINEC"),2);
	UINT8 ctrl  = BIT(input_port_read(machine, "TERM_LINEC"),0);
	i = *scan_line;
	{
		code =	input_port_read(machine, keynames[i]);
		if (code != 0)
		{
			if (i==0 && shift==0) {
				key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
			}
			if (i==0 && shift==1) {
				key_code = 0x20 + row_number(code) + 8*i; // for shifted numbers
			}
			if (i==1 && shift==0) {
				if (row_number(code) < 4) {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i==1 && shift==1) {
				if (row_number(code) < 4) {
					key_code = 0x20 + row_number(code) + 8*i; // for numbers and some signs
				} else {
					key_code = 0x30 + row_number(code) + 8*i; // for numbers and some signs
				}
			}
			if (i>=2 && i<=4 && (shift ^ caps)==0 && ctrl==0) {
				key_code = 0x60 + row_number(code) + (i-2)*8; // for small letters
			}
			if (i>=2 && i<=4 && (shift ^ caps)==1 && ctrl==0) {
				key_code = 0x40 + row_number(code) + (i-2)*8; // for big letters
			}
			if (i>=2 && i<=4 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for CTRL + letters
			}
			if (i==5 && shift==1 && ctrl==0) {
				if (row_number(code)<7) {
					if (row_number(code)<3) {
						key_code = (caps ? 0x60 : 0x40) + row_number(code) + (i-2)*8; // for big letters
					} else {
						key_code = 0x60 + row_number(code) + (i-2)*8; // for upper symbols letters
					}
				} else {
					key_code = 0x40 + row_number(code) + (i-2)*8; // for DEL it is switched
				}
			}
			if (i==5 && shift==0 && ctrl==0) {
				if (row_number(code)<7) {
					if (row_number(code)<3) {
						key_code = (caps ? 0x40 : 0x60) + row_number(code) + (i-2)*8; // for small letters
					} else {
						key_code = 0x40 + row_number(code) + (i-2)*8; // for lower symbols letters
					}
				} else {
					key_code = 0x60 + row_number(code) + (i-2)*8; // for DEL it is switched
				}
			}
			if (i==5 && shift==1 && ctrl==1) {
				key_code = 0x00 + row_number(code) + (i-2)*8; // for letters + ctrl
			}
			if (i==6) {
				switch(row_number(code))
				{
/*                  case 0: key_code = 0x11; break;
                    case 1: key_code = 0x12; break;
                    case 2: key_code = 0x13; break;
                    case 3: key_code = 0x14; break;*/
					case 4: key_code = 0x20; break; // Space
					case 5: key_code = 0x0A; break; // LineFeed
					case 6: key_code = 0x09; break; // TAB
					case 7: key_code = 0x0D; break; // Enter
				}
			}
			if (last_code != key_code ) {
				devcb_call_write8(callback, 0, key_code);
				if (tx_shift) *tx_shift = key_code;
				if (tx_state) *tx_state = START;
			}
			retVal = key_code;
		} else {
			*scan_line += 1;
			if (*scan_line==7) {
				*scan_line = 0;
			}
		}
	}
	return retVal;
}

static TIMER_CALLBACK(keyboard_callback)
{
	terminal_state *term = get_safe_token((device_t *)ptr);
	term->last_code = terminal_keyboard_handler(machine, &term->terminal_keyboard_func, term->last_code, &term->scan_line, &term->tx_shift, &term->tx_state);
}

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/
static VIDEO_START( terminal )
{

}

static VIDEO_UPDATE(terminal )
{
	device_t *devconf = screen->machine->device(TERMINAL_TAG);
	generic_terminal_update( devconf, bitmap, cliprect);
	return 0;
}

MACHINE_CONFIG_FRAGMENT( generic_terminal )
	MCFG_SCREEN_ADD(TERMINAL_SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(TERMINAL_WIDTH*8, TERMINAL_HEIGHT*10)
	MCFG_SCREEN_VISIBLE_AREA(0, TERMINAL_WIDTH*8-1, 0, TERMINAL_HEIGHT*10-1)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_VIDEO_START(terminal)
	MCFG_VIDEO_UPDATE(terminal)
MACHINE_CONFIG_END


/*-------------------------------------------------
    DEVICE_START( terminal )
-------------------------------------------------*/

static DEVICE_START( terminal )
{
	terminal_state *term = get_safe_token(device);
	const terminal_interface *intf = get_interface(device);

	devcb_resolve_write8(&term->terminal_keyboard_func, &intf->terminal_keyboard_func, device);
	timer_pulse(device->machine, attotime::from_hz(2400), (void*)device, 0, keyboard_callback);
}

/*-------------------------------------------------
    DEVICE_RESET( terminal )
-------------------------------------------------*/

static DEVICE_RESET( terminal )
{
	terminal_state *term = get_safe_token(device);
	terminal_clear(device);
	term->last_code = 0;
	term->scan_line = 0;
	term->rx_state = START;
	term->tx_state = STOP;
}

/*-------------------------------------------------
    DEVICE_GET_INFO( terminal )
-------------------------------------------------*/

DEVICE_GET_INFO( terminal )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;												break;
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(terminal_state);							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(terminal);					break;
		case DEVINFO_FCT_STOP:							/* Nothing */												break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(terminal);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Generic Terminal");						break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Terminal");								break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");										break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);									break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright the MESS Team"); 				break;
	}
}

/*
Char  Dec  Oct  Hex | Char  Dec  Oct  Hex | Char  Dec  Oct  Hex | Char Dec  Oct   Hex
-------------------------------------------------------------------------------------
(nul)   0 0000 0x00 | (sp)   32 0040 0x20 | @      64 0100 0x40 | `      96 0140 0x60
(soh)   1 0001 0x01 | !      33 0041 0x21 | A      65 0101 0x41 | a      97 0141 0x61
(stx)   2 0002 0x02 | "      34 0042 0x22 | B      66 0102 0x42 | b      98 0142 0x62
(etx)   3 0003 0x03 | #      35 0043 0x23 | C      67 0103 0x43 | c      99 0143 0x63
(eot)   4 0004 0x04 | $      36 0044 0x24 | D      68 0104 0x44 | d     100 0144 0x64
(enq)   5 0005 0x05 | %      37 0045 0x25 | E      69 0105 0x45 | e     101 0145 0x65
(ack)   6 0006 0x06 | &      38 0046 0x26 | F      70 0106 0x46 | f     102 0146 0x66
(bel)   7 0007 0x07 | '      39 0047 0x27 | G      71 0107 0x47 | g     103 0147 0x67
(bs)    8 0010 0x08 | (      40 0050 0x28 | H      72 0110 0x48 | h     104 0150 0x68
(ht)    9 0011 0x09 | )      41 0051 0x29 | I      73 0111 0x49 | i     105 0151 0x69
(nl)   10 0012 0x0a | *      42 0052 0x2a | J      74 0112 0x4a | j     106 0152 0x6a
(vt)   11 0013 0x0b | +      43 0053 0x2b | K      75 0113 0x4b | k     107 0153 0x6b
(np)   12 0014 0x0c | ,      44 0054 0x2c | L      76 0114 0x4c | l     108 0154 0x6c
(cr)   13 0015 0x0d | -      45 0055 0x2d | M      77 0115 0x4d | m     109 0155 0x6d
(so)   14 0016 0x0e | .      46 0056 0x2e | N      78 0116 0x4e | n     110 0156 0x6e
(si)   15 0017 0x0f | /      47 0057 0x2f | O      79 0117 0x4f | o     111 0157 0x6f
(dle)  16 0020 0x10 | 0      48 0060 0x30 | P      80 0120 0x50 | p     112 0160 0x70
(dc1)  17 0021 0x11 | 1      49 0061 0x31 | Q      81 0121 0x51 | q     113 0161 0x71
(dc2)  18 0022 0x12 | 2      50 0062 0x32 | R      82 0122 0x52 | r     114 0162 0x72
(dc3)  19 0023 0x13 | 3      51 0063 0x33 | S      83 0123 0x53 | s     115 0163 0x73
(dc4)  20 0024 0x14 | 4      52 0064 0x34 | T      84 0124 0x54 | t     116 0164 0x74
(nak)  21 0025 0x15 | 5      53 0065 0x35 | U      85 0125 0x55 | u     117 0165 0x75
(syn)  22 0026 0x16 | 6      54 0066 0x36 | V      86 0126 0x56 | v     118 0166 0x76
(etb)  23 0027 0x17 | 7      55 0067 0x37 | W      87 0127 0x57 | w     119 0167 0x77
(can)  24 0030 0x18 | 8      56 0070 0x38 | X      88 0130 0x58 | x     120 0170 0x78
(em)   25 0031 0x19 | 9      57 0071 0x39 | Y      89 0131 0x59 | y     121 0171 0x79
(sub)  26 0032 0x1a | :      58 0072 0x3a | Z      90 0132 0x5a | z     122 0172 0x7a
(esc)  27 0033 0x1b | ;      59 0073 0x3b | [      91 0133 0x5b | {     123 0173 0x7b
(fs)   28 0034 0x1c | <      60 0074 0x3c | \      92 0134 0x5c | |     124 0174 0x7c
(gs)   29 0035 0x1d | =      61 0075 0x3d | ]      93 0135 0x5d | }     125 0175 0x7d
(rs)   30 0036 0x1e | >      62 0076 0x3e | ^      94 0136 0x5e | ~     126 0176 0x7e
(us)   31 0037 0x1f | ?      63 0077 0x3f | _      95 0137 0x5f | (del) 127 0177 0x7f

*/
INPUT_PORTS_START( generic_terminal )
	PORT_START("TERM_LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)  PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("TERM_LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("TERM_LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("TERM_LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("TERM_LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("TERM_LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("TERM_LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("TERM_LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT) PORT_CHAR(10)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("TERM_CONF")
	PORT_CONFNAME( 0x01, 0x01, "Cursor")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
	PORT_CONFNAME( 0x02, 0x02, "Type")
	PORT_CONFSETTING(    0x00, "Underline")
	PORT_CONFSETTING(    0x02, "Block")
	PORT_CONFNAME( 0x04, 0x04, "Blinking")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x04, DEF_STR(Yes))
	PORT_CONFNAME( 0x08, 0x08, "Invert")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x08, DEF_STR(Yes))
INPUT_PORTS_END

DEFINE_LEGACY_DEVICE(GENERIC_TERMINAL, terminal);
