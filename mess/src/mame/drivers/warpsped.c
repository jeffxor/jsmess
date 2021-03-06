/*

Meadows Warp Speed

Preliminary driver by Mariusz Wojcieszek

Hardware registers:
0x00 - 0x1f control register for discs generator (8 bytes each)
            0x00, 0x01  disc radius (?)
            0x02, 0x03  disc middle point
            0x04, 0x05  disc middle point
            0x06        disc colour (0-7)
            0x07        unused
0x20        ?
0x21        sound (intro screens have bit 1 toggled for click effect)
0x22        ?
0x23        ?
0x24        ?
0x25        ?
0x26        ?
0x27        ?


Board etched...
    MEADOWS 024-0084
    MADE IN USA

Empty socket at .E3
Z80 processor
6 2102 memory chips
2 2112 memory chips
5Mhz crystal
All PROMS are SN74s474

.L18    no sticker
.L17    stickered       L9, L13
                L17, G17
.L15    stickered       L10, L15
                L18, G18
.L13    stickered       L9, L13
                L17, G17
.L10    stickered       L10, L15
                L18, G18
.L9 stickered (damaged) xxx, L13
                xxx, G1y
.K1 stickered       K1
.G2 no sticker      K1
.E4 no sticker
.E5 stickered       M16
                PRO
                1
.E6 stickered       M16
                PRO
                3
.C3 can't read
.C4 stickered       M16
                PRO
                4
.C5 stickered       M16
                PRO
                0
.C6 stickered       M16
                PRO
                2
.E8 stickered       E8
.E10    stickered       E10
.C12    stickered       C12
.G17    stickered       L9, L13
                L17, G17
.G18    stickered       L10, L15
                L18, G18

L9, L13, L17 and G17 all read the same
L10, L15, L18 and G18 all read the same

*/

#include "emu.h"
#include "cpu/z80/z80.h"

class warpspeed_state : public driver_device
{
public:
	warpspeed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8		*m_videoram;
	tilemap_t	*m_text_tilemap;
	tilemap_t	*m_starfield_tilemap;
	UINT8		*m_workram;
	UINT8		m_regs[0x28];
};

static READ8_HANDLER( warpspeed_hardware_r )
{
//  warpspeed_state *state = space->machine().driver_data<warpspeed_state>();

	switch( offset )
	{
		case 0: return input_port_read(space->machine(), "IN1" );
		/* dipswitches? bit 6 when set causes jump to $e01 during vblank (rom missing) is this test mode? */
		case 2: return 0;
		case 3: return input_port_read(space->machine(), "IN0" );
	}
	return 0;
}

static WRITE8_HANDLER( warpspeed_hardware_w )
{
	warpspeed_state *state = space->machine().driver_data<warpspeed_state>();
	state->m_regs[offset] = data;
}

static TILE_GET_INFO( get_warpspeed_text_tile_info )
{
	warpspeed_state *state = machine.driver_data<warpspeed_state>();

	UINT8 code = state->m_videoram[tile_index] & 0x3f;
	SET_TILE_INFO(0, code, 0, 0);
}

static TILE_GET_INFO( get_warpspeed_starfield_tile_info )
{
	UINT8 code = 0x3f;
	if ( tile_index & 1 )
	{
		code = machine.region("starfield")->base()[tile_index >> 1] & 0x3f;
	}
	SET_TILE_INFO(1, code, 0, 0);
}

static WRITE8_HANDLER( warpspeed_vidram_w )
{
	warpspeed_state *state = space->machine().driver_data<warpspeed_state>();

	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_text_tilemap, offset);
}

static VIDEO_START( warpspeed )
{
	warpspeed_state *state = machine.driver_data<warpspeed_state>();
	state->m_text_tilemap = tilemap_create(machine, get_warpspeed_text_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->m_text_tilemap, 0);
	state->m_starfield_tilemap = tilemap_create(machine, get_warpspeed_starfield_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_mark_all_tiles_dirty(state->m_starfield_tilemap);
}

static SCREEN_UPDATE( warpspeed )
{
	warpspeed_state *state = screen->machine().driver_data<warpspeed_state>();

	tilemap_draw(bitmap, cliprect, state->m_starfield_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->m_text_tilemap, 0, 0);
	return 0;
}

static ADDRESS_MAP_START( warpspeed_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0dff) AM_ROM
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE( warpspeed_vidram_w ) AM_BASE_MEMBER(warpspeed_state, m_videoram)
	AM_RANGE(0x1c00, 0x1cff) AM_RAM AM_BASE_MEMBER(warpspeed_state, m_workram)
ADDRESS_MAP_END

static ADDRESS_MAP_START ( warpspeed_io_map, AS_IO, 8)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_READ( warpspeed_hardware_r )
	AM_RANGE(0x00, 0x27) AM_WRITE( warpspeed_hardware_w )
ADDRESS_MAP_END

static INPUT_PORTS_START( warpspeed )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x7e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Accelerate" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Brake" )
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout warpspeed_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	8*8
};

static GFXDECODE_START( warpspeed )
	GFXDECODE_ENTRY( "gfx1", 0, warpspeed_charlayout,   0, 1  )
	GFXDECODE_ENTRY( "gfx2", 0, warpspeed_charlayout,   0, 1  )
GFXDECODE_END

static MACHINE_CONFIG_START( warpspeed, warpspeed_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_5MHz/2)
	MCFG_CPU_PROGRAM_MAP(warpspeed_map)
	MCFG_CPU_IO_MAP(warpspeed_io_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE((32)*8, (32)*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 32*8-1, 8*8, 32*8-1)

	MCFG_VIDEO_START(warpspeed)
	MCFG_SCREEN_UPDATE(warpspeed)

	MCFG_GFXDECODE(warpspeed)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

MACHINE_CONFIG_END

ROM_START( warpsped )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "m16 pro 0.c5",  0x0000, 0x0200, CRC(81f33dfb) SHA1(5c4adef88e1e7f1a9f7c156f17b98ba522ddee81) )
	ROM_LOAD( "m16 pro 1.e5",  0x0200, 0x0200, CRC(135f7421) SHA1(0cabe9a2590fe4f81976a4d10e9b2a223a2d875e) )
	ROM_LOAD( "m16 pro 2.c6",  0x0400, 0x0200, CRC(0a36d152) SHA1(83a2e8f78a36e512da81c44a5ceca7f865bc3508) )
	ROM_LOAD( "m16 pro 3.e6",  0x0600, 0x0200, CRC(ba416cca) SHA1(afb831a79a4a4334fa4caf4e2244c2d4e2f25853) )
	ROM_LOAD( "m16 pro 4.c4",  0x0800, 0x0200, CRC(fc44f25b) SHA1(185f593f2ec6075fd68869d87ed584908031100a) )
	ROM_LOAD( "m16 pro 5.e4",  0x0a00, 0x0200, CRC(7a16bc2b) SHA1(48f58f0c7469da24a3ebee9183b5aae8c676e6ea) )
	ROM_LOAD( "m16 pro 6.c3",  0x0c00, 0x0200, CRC(e2e7940f) SHA1(78c9df32580784c278675d09b89095781893d48f) )

	ROM_REGION(0x1000, "sprite", 0)
	ROM_LOAD( "l9 l13 l17 g17.g17",  0x0000, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.g18", 0x0200, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )
	ROM_LOAD( "l9 l13 l17 g17.l9",   0x0400, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.l10", 0x0600, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )
	ROM_LOAD( "l9 l13 l17 g17.l13",  0x0800, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.l15", 0x0a00, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )
	ROM_LOAD( "l9 l13 l17 g17.l17",  0x0c00, 0x0200, CRC(7449aae9) SHA1(1f49dad6f60103da6093592efa2087bc24dc0283) )
	ROM_LOAD( "l10 l15 l18 g18.l18", 0x0e00, 0x0200, CRC(5829699c) SHA1(20ffa7b81a0de159408d2668005b9ee8a2e588d1) )

	ROM_REGION(0x200, "starfield", 0)
	ROM_LOAD( "e10.e10", 0x0000, 0x0200, CRC(e0d4b72c) SHA1(ae5fae0df9e0bfc67f586649474ff8a69abd7579) )

	ROM_REGION(0x400, "unknown", 0)
	ROM_LOAD( "c12.c12", 0x0000, 0x0200, CRC(88a8db15) SHA1(3fdf4e23cf75cf5dd4d3bad08e9e71c0268f8d79) )
	ROM_LOAD( "e8.e8",   0x0200, 0x0200, CRC(3ef3a576) SHA1(905f9b8d3cabab944a0f6f0736c5c26d0e36107f) )

	ROM_REGION(0x0200, "gfx1", 0)
	ROM_LOAD( "k1.g1",  0x0000, 0x0200, CRC(63d4fa84) SHA1(3465ce27497e2d4fcae994c022480e37e1345686) )

	ROM_REGION(0x0200, "gfx2", 0)
	ROM_LOAD( "k1.k1",  0x0000, 0x0200, CRC(76b10d47) SHA1(e644a50df06535fe1fbfb8754cfc7b4a49fcb05e) )

ROM_END

static DRIVER_INIT( warpspeed )
{
}

GAME( 197?, warpsped,  0,      warpspeed, warpspeed, warpspeed, ROT0, "Meadows Games, Inc.", "Warp Speed (prototype)", GAME_NO_SOUND | GAME_NOT_WORKING )
