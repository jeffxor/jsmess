/***************************************************************************

        Science of Cambridge MK-14

        20/11/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/scmp/scmp.h"
#include "machine/ins8154.h"
#include "mk14.lh"
/*
000-1FF  512 byte SCIOS ROM  Decoded by 0xxx
200-3FF  ROM Shadow / Expansion RAM
400-5FF  ROM Shadow / Expansion RAM
600-7FF  ROM Shadow / Expansion RAM
800-87F  128 bytes I/O chip RAM  Decoded by 1xx0
880-8FF  I/O Ports  Decoded by 1xx0
900-9FF  Keyboard & Display  Decoded by 1x01
A00-AFF  I/O Port & RAM Shadow
B00-BFF  256 bytes RAM (Extended) / VDU RAM  Decoded by 1011
C00-CFF  I/O Port & RAM Shadow
D00-DFF  Keyboard & Display Shadow
E00-EFF  I/O Port & RAM Shadow
F00-FFF  256 bytes RAM (Standard) / VDU RAM  Decoded by 1111

*/
static const char *const keynames[] = {
	"LINE0", "LINE1", "LINE2", "LINE3",
	"LINE4", "LINE5", "LINE6", "LINE7"
};

static READ8_HANDLER(keyboard_r)
{
	if (offset < 8) {
		return input_port_read(space->machine, keynames[offset]);
	} else {
		return 0xff;
	}
}

static WRITE8_HANDLER(display_w)
{
	if (offset < 8 ) {
		output_set_digit_value(7-offset, data);
	} else {
		//logerror("write %02x to %02x\n",data,offset);
	}
}

static ADDRESS_MAP_START(mk14_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x0fff)
	AM_RANGE(0x000, 0x1ff) AM_ROM AM_MIRROR(0x600) // ROM
	AM_RANGE(0x800, 0x87f) AM_RAM AM_MIRROR(0x600) // 128 I/O chip RAM
	AM_RANGE(0x880, 0x8ff)  AM_DEVREADWRITE("ic8", ins8154_r, ins8154_w) AM_MIRROR(0x600) // I/O
	AM_RANGE(0x900, 0x9ff) AM_READWRITE(keyboard_r, display_w) AM_MIRROR(0x400)
	AM_RANGE(0xb00, 0xbff) AM_RAM // VDU RAM
	AM_RANGE(0xf00, 0xfff) AM_RAM // Standard RAM
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( mk14 )
	PORT_START("LINE0")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_START("LINE1")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_START("LINE2")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_START("LINE3")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MEM") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_START("LINE4")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AB") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_START("LINE5")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_START("LINE6")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_START("LINE7")
		PORT_BIT(0x0F, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TERM") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
INPUT_PORTS_END


static MACHINE_RESET(mk14)
{
}

static const ins8154_interface mk14_ins8154 =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_DRIVER_START( mk14 )
    /* basic machine hardware */
	// IC1 1SP-8A/600 (8060) SC/MP Microprocessor
    MDRV_CPU_ADD("maincpu", INS8060, XTAL_4_433619MHz)
    MDRV_CPU_PROGRAM_MAP(mk14_mem)

    MDRV_MACHINE_RESET(mk14)

	/* video hardware */
	MDRV_DEFAULT_LAYOUT(layout_mk14)

	/* devices */
	MDRV_INS8154_ADD("ic8", mk14_ins8154)
MACHINE_DRIVER_END

/* ROM definition */
ROM_START( mk14 )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// IC2,3 74S571 512 x 4 bit ROM
	ROM_LOAD( "scios.bin", 0x0000, 0x0200, CRC(8b667daa) SHA1(802dc637ce5391a2a6627f76f919b12a869b56ef))
ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                   FULLNAME     FLAGS */
COMP( 1977, mk14,  0,       0,		mk14,		mk14,	 0, 	"Science of Cambridge",   "MK-14",		GAME_NO_SOUND)

