/******************************************************************************

    drivers/compis.c
    machine driver

    Per Ola Ingvarsson
    Tomas Karlsson

    Hardware:
        - Intel 80186 CPU 8MHz, integrated DMA(8237?), PIC(8259?), PIT(8253?)
                - Intel 80130 OSP Operating system processor (PIC 8259, PIT 8254)
        - Intel 8274 MPSC Multi-protocol serial communications controller (NEC 7201)
        - Intel 8255 PPI Programmable peripheral interface
        - Intel 8253 PIT Programmable interval timer
        - Intel 8251 USART Universal synchronous asynchronous receiver transmitter
        - National 58174 Real-time clock (compatible with 58274)
    Peripheral:
        - Intel 82720 GDC Graphic display processor (NEC uPD 7220)
        - Intel 8272 FDC Floppy disk controller (Intel iSBX-218A)
        - Western Digital WD1002-05 Winchester controller

    Memory map:

    00000-3FFFF RAM LMCS (Low Memory Chip Select)
    40000-4FFFF RAM MMCS 0 (Midrange Memory Chip Select)
    50000-5FFFF RAM MMCS 1 (Midrange Memory Chip Select)
    60000-6FFFF RAM MMCS 2 (Midrange Memory Chip Select)
    70000-7FFFF RAM MMCS 3 (Midrange Memory Chip Select)
    80000-EFFFF NOP
    F0000-FFFFF ROM UMCS (Upper Memory Chip Select)

 ******************************************************************************/

#include "driver.h"
#include "machine/8255ppi.h"
#include "includes/compis.h"
#include "video/i82720.h"
#include "devices/mflopimg.h"
#include "devices/printer.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/mm58274c.h"
#include "formats/cpis_dsk.h"


static ADDRESS_MAP_START( compis_mem , ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE( 0x00000, 0x3ffff) AM_RAM
	AM_RANGE( 0x40000, 0x4ffff) AM_RAM
	AM_RANGE( 0x50000, 0x5ffff) AM_RAM
	AM_RANGE( 0x60000, 0x6ffff) AM_RAM
	AM_RANGE( 0x70000, 0x7ffff) AM_RAM
	AM_RANGE( 0x80000, 0xeffff) AM_NOP
	AM_RANGE( 0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( compis_io, ADDRESS_SPACE_IO, 16)
	AM_RANGE( 0x0000, 0x0007) AM_DEVREADWRITE(PPI8255, "ppi8255", compis_ppi_r, compis_ppi_w )	/* PPI 8255         */
	AM_RANGE( 0x0080, 0x0087) AM_DEVREADWRITE(PIT8253, "pit8253", compis_pit_r, compis_pit_w )	/* PIT 8253         */
	AM_RANGE( 0x0100, 0x011b) AM_READWRITE( compis_rtc_r, compis_rtc_w ) 	/* RTC 58174            */
	AM_RANGE( 0x0280, 0x0283) AM_DEVREADWRITE(PIC8259, "pic8259_master", compis_osp_pic_r, compis_osp_pic_w ) /* PIC 8259 (80150/80130)  */
//  AM_RANGE( 0x0288, 0x028e) AM_DEVREADWRITE(PIT8254, "pit8254", compis_osp_pit_r, compis_osp_pit_w ) /* PIT 8254 (80150/80130)  */
	AM_RANGE( 0x0310, 0x031f) AM_READWRITE( compis_usart_r, compis_usart_w )	/* USART 8251 Keyboard      */
	AM_RANGE( 0x0330, 0x033f) AM_READWRITE( compis_gdc_r, compis_gdc_w )	/* GDC 82720 PCS6:6     */
	AM_RANGE( 0x0340, 0x0343) AM_READWRITE( compis_fdc_r, compis_fdc_w )	/* iSBX0 (J8) FDC 8272      */
	AM_RANGE( 0x0350, 0x0351) AM_READ( compis_fdc_dack_r)	/* iSBX0 (J8) DMA ACK       */
	AM_RANGE( 0xff00, 0xffff) AM_READWRITE( i186_internal_port_r, i186_internal_port_w)/* CPU 80186         */
//{ 0x0100, 0x017e, compis_null_r },    /* RTC              */
//{ 0x0180, 0x01ff, compis_null_r },    /* PCS3?            */
//{ 0x0200, 0x027f, compis_null_r },    /* Reserved         */
//{ 0x0280, 0x02ff, compis_null_r },    /* 80150 not used?      */
//{ 0x0300, 0x0300, compis_null_r },    /* Cassette  motor      */
//{ 0x0301, 0x030f, compis_null_r},     /* DMA ACK Graphics     */
//{ 0x0310, 0x031e, compis_null_r },    /* SCC 8274 Int Ack     */
//{ 0x0320, 0x0320, compis_null_r },    /* SCC 8274 Serial port     */
//{ 0x0321, 0x032f, compis_null_r },    /* DMA Terminate        */
//{ 0x0331, 0x033f, compis_null_r },    /* DMA Terminate        */
//{ 0x0341, 0x034f, compis_null_r },    /* J8 CS1 (16-bit)      */
//{ 0x0350, 0x035e, compis_null_r },    /* J8 CS1 (8-bit)       */
//{ 0x0360, 0x036e, compis_null_r },    /* J9 CS0 (8/16-bit)        */
//{ 0x0361, 0x036f, compis_null_r },    /* J9 CS1 (16-bit)      */
//{ 0x0370, 0x037e, compis_null_r },    /* J9 CS1 (8-bit)       */
//{ 0x0371, 0x037f, compis_null_r },    /* J9 CS1 (8-bit)       */
//{ 0xff20, 0xffff, compis_null_r },    /* CPU 80186            */
ADDRESS_MAP_END

/* COMPIS Keyboard */

/* 2008-05 FP: 
Small note about natural keyboard: currently,
- Both "SShift" keys (left and right) are not mapped
- Keypad '00' and '000' are not mapped
- "Compis !" is mapped to 'F3'
- "Compis ?" is mapped to 'F4'
- "Compis |" is mapped to 'F5'
- "Compis S" is mapped to 'F6'
- "Avbryt" is mapped to 'F7'
- "Inpassa" is mapped to 'Insert'
- "S�k" is mapped to "Print Screen"
- "Utpl�na"is mapped to 'Delete'
- "Start / Stop" is mapped to 'Pause'
- "TabL" is mapped to 'Page Up'
- "TabR" is mapped to 'Page Down'
*/

static INPUT_PORTS_START (compis)
	PORT_START("ROW0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)		PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)			PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)			PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)			PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)			PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)			PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)			PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)			PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)			PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)			PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)			PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC2\xB4 `") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('`')
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)	PORT_CHAR(8)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)		PORT_CHAR('\t')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)			PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)			PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)			PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)			PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)			PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)			PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)			PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)			PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)			PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)			PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC3\xA5 \xC3\x85") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC3\xBC \xC3\x9C") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00FC) PORT_CHAR(0x00DC)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)		PORT_CHAR(13)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)			PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)			PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)			PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)			PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)			PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)			PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)			PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)			PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)			PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC3\xB6 \xC3\x96") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC3\xA4 \xC3\x84") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'' *") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('*')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)			PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)			PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)			PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)			PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)			PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)			PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)			PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)		PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)		PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)		PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SShift (Left)") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)	PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)		PORT_CHAR(' ') 
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)	PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SShift (Right)") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INPASSA") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S\xC3\x96K") PORT_CODE(KEYCODE_PRTSCR) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UTPL\xC3\x85NA") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("START-STOP") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x91") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AVBRYT") PORT_CODE(KEYCODE_SCRLOCK) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x92") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TABL") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xE2\x86\x93") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TABR") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS !") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS ?") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS |") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)		PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)		PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COMPIS S") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)		PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)		PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)		PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)		PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)		PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)		PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)		PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)		PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)		PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)		PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 000") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Enter") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))

	PORT_START("DSW0")
	PORT_DIPNAME( 0x18, 0x00, "S8 Test mode")
	PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x08, "Remote" )
	PORT_DIPSETTING( 0x10, "Stand alone" )
	PORT_DIPSETTING( 0x18, "Reserved" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "iSBX-218A DMA")
	PORT_DIPSETTING( 0x01, "Enabled" )
	PORT_DIPSETTING( 0x00, "Disabled" )
INPUT_PORTS_END


static const unsigned i86_address_mask = 0x000fffff;

static const mm58274c_interface compis_mm58274c_interface =
{
	0,	/* 	mode 24*/
	1   /*  first day of week */
};

static MACHINE_DRIVER_START( compis )
	/* basic machine hardware */
	MDRV_CPU_ADD("main", I80186, 8000000)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(compis_mem, 0)
	MDRV_CPU_IO_MAP(compis_io, 0)
	MDRV_CPU_VBLANK_INT("main", compis_vblank_int)
	MDRV_CPU_CONFIG(i86_address_mask)

	MDRV_INTERLEAVE(1)

	MDRV_MACHINE_RESET(compis)

	MDRV_PIT8253_ADD( "pit8253", compis_pit8253_config )

	MDRV_PIT8254_ADD( "pit8254", compis_pit8254_config )

	MDRV_PIC8259_ADD( "pic8259_master", compis_pic8259_master_config )

	MDRV_PIC8259_ADD( "pic8259_slave", compis_pic8259_slave_config )

	MDRV_PPI8255_ADD( "ppi8255", compis_ppi_interface )

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(640, 480)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MDRV_PALETTE_LENGTH(COMPIS_PALETTE_SIZE)
	MDRV_PALETTE_INIT(compis_gdc)

	MDRV_VIDEO_START(compis_gdc)
	MDRV_VIDEO_UPDATE(compis_gdc)

	/* printer */
	MDRV_DEVICE_ADD("printer", PRINTER)

	/* uart */
	MDRV_DEVICE_ADD("uart", MSM8251)
	MDRV_DEVICE_CONFIG(compis_usart_interface)
	
	/* rtc */
	MDRV_MM58274C_ADD("mm58274c", compis_mm58274c_interface)	
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START (compis)
     ROM_REGION (0x100000, "main", 0)
     ROM_LOAD ("compis.rom", 0xf0000, 0x10000, CRC(89877688) SHA1(7daa1762f24e05472eafc025879da90fe61d0225))
ROM_END

static void compis_floppy_getinfo(const mess_device_class *devclass, UINT32 state, union devinfo *info)
{
	/* floppy */
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case MESS_DEVINFO_INT_COUNT:							info->i = 2; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case MESS_DEVINFO_PTR_FLOPPY_OPTIONS:				info->p = (void *) floppyoptions_compis; break;

		default:										floppy_device_getinfo(devclass, state, info); break;
	}
}

static SYSTEM_CONFIG_START(compis)
	CONFIG_DEVICE(compis_floppy_getinfo)
SYSTEM_CONFIG_END

/*   YEAR   NAME        PARENT  COMPAT MACHINE  INPUT   INIT    CONFIG  COMPANY     FULLNAME */
COMP(1985,	compis,		0,		0,     compis,	compis,	compis,	compis,	"Telenova", "Compis" , 0)
