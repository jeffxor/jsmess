/*******************************************************************************************

Bingo Circus (c) 1989 Sega

A Bingo machine with a terminal for each player,maximum 8 players can play together.

preliminary driver by David Haywood & Angelo Salese

TODO:
-terminal pcb(s) roms aren't dumped,so no video can be shown,a cabinet snap is here ->
 http://www.system16.com/hardware.php?id=840&page=1#2743 ,every player should have his own
 screen.
-inconsistant (likely wrong) sound banking.

============================================================================================
BINGO CIRCUS (MAIN PCB)
(c)SEGA

CPU   : MAIN 68000 SOUND Z-80
SOUND : YM2151 uPD7759C

12635A.EPR  ; MAIN PROGRAM
12636A.EPR  ;  /
12637.EPR   ; VOICE DATA
12638.EPR   ;  /
12639.EPR   ; SOUND PRG

*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/upd7759.h"


class bingoc_state : public driver_device
{
public:
	bingoc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_x;
};


#define SOUND_TEST 0

static VIDEO_START(bingoc)
{

}

static SCREEN_UPDATE_IND16(bingoc)
{
	return 0;
}

static READ16_HANDLER( bingoc_rand_r )
{
	return 0xffff;
}

#if SOUND_TEST
/*dirty code to test z80 + bgm/sfx*/
/*
0x00-0x7f controls u7759 samples (command 0xff->n)
0x80-0x85 ym2151 bgm
0x90-0x9b ym2151 sfx
*/
static READ8_HANDLER( sound_test_r )
{
	bingoc_state *state = space->machine().driver_data<bingoc_state>();

	if(space->machine().input().code_pressed_once(KEYCODE_Z))
		state->m_x++;

	if(space->machine().input().code_pressed_once(KEYCODE_X))
		state->m_x--;

	if(space->machine().input().code_pressed_once(KEYCODE_A))
		return 0xff;

	popmessage("%02x",state->m_x);
	return state->m_x;
}
#else
static WRITE16_HANDLER( main_sound_latch_w )
{
	soundlatch_w(space,0,data&0xff);
	cputag_set_input_line(space->machine(), "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
}
#endif

static WRITE8_DEVICE_HANDLER( bingoc_play_w )
{
	/*
    ---- --x- sound rom banking
    ---- ---x start-stop sample
    */
	UINT8 *upd = device->machine().region("upd")->base();
	memcpy(&upd[0x00000], &upd[0x20000 + (((data & 2)>>1) * 0x20000)], 0x20000);
	upd7759_start_w(device, data & 1);
//  printf("%02x\n",data);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, bingoc_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10007f) AM_READ_LEGACY(bingoc_rand_r) //comms? lamps?
	AM_RANGE(0x180000, 0x18007f) AM_READ_LEGACY(bingoc_rand_r) //comms? lamps?
#if !SOUND_TEST
	AM_RANGE(0x180010, 0x180011) AM_WRITE_LEGACY(main_sound_latch_w) //WRONG there...
#endif
	AM_RANGE(0xff8000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, bingoc_state )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, AS_IO, 8, bingoc_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE_LEGACY("upd", bingoc_play_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE_LEGACY("upd", upd7759_port_w)
#if !SOUND_TEST
	AM_RANGE(0xc0, 0xc0) AM_READ_LEGACY(soundlatch_r) //soundlatch
#else
	AM_RANGE(0xc0, 0xc0) AM_READ_LEGACY(sound_test_r)
#endif
ADDRESS_MAP_END


static INPUT_PORTS_START( bingoc )
INPUT_PORTS_END


static MACHINE_CONFIG_START( bingoc, bingoc_state )

	MCFG_CPU_ADD("maincpu", M68000,8000000)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80,4000000)		 /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io)
#if SOUND_TEST
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)
#endif

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_STATIC(bingoc)

	MCFG_PALETTE_LENGTH(0x100)

	MCFG_VIDEO_START(bingoc)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker") //might just be mono...

	MCFG_SOUND_ADD("ymsnd", YM2151, 7159160/2)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START( bingoc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "12636a.epr", 0x00000, 0x20000, CRC(ef8dccff) SHA1(9eb6e55e2000b252647fc748cbbeedf4f119aed7) )
	ROM_LOAD16_BYTE( "12635a.epr", 0x00001, 0x20000, CRC(a94cd74e) SHA1(0c3e157a5ddf34f4f1a2d30b9758bf067896371c) )

	ROM_REGION( 0x10000, "ter_1", 0 ) //just as a re-dump reminder,might be either one sub-board or eight of them...
	ROM_LOAD( "terminal.rom", 0x00000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "12639.epr", 0x00000, 0x10000, CRC(4307f6ba) SHA1(f568930191cd31a2112ef8d4cf5ff340826d5877) )

	ROM_REGION( 0x60000, "upd", 0 )
	ROM_LOAD( "12637.epr", 0x40000, 0x20000, CRC(164ac43f) SHA1(90160df8e927a25ea08badedb3fcd818c314b388) )
	ROM_LOAD( "12638.epr", 0x20000, 0x20000, CRC(ef52ab73) SHA1(d14593ef88ac2acd00daaf522008405f65f67548) )
	ROM_COPY( "upd",       0x20000, 0x00000, 0x20000 )
ROM_END

GAME( 1989, bingoc,  0,    bingoc, bingoc,  0, ROT0, "Sega", "Bingo Circus (Rev. A 891001)", GAME_NOT_WORKING )
