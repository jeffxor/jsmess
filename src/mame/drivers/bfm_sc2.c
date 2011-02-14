/****************************************************************************************

    bfm_sc2.c

    Bellfruit scorpion2/3 driver, (under heavy construction !!!)

*****************************************************************************************

  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware setup.
  18-01-2006: Cleaned up for MAME inclusion
  19-08-2005: Re-Animator

Standard scorpion2 memorymap


   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000-1FFF  |R/W| D D D D D D D D | RAM (8k) battery backed up
-----------+---+-----------------+-----------------------------------------
2000-20FF  | W | D D D D D D D D | Reel 1 + 2 stepper latch
-----------+---+-----------------+-----------------------------------------
2000       | R | D D D D D D D D | vfd status
-----------+---+-----------------+-----------------------------------------
2100-21FF  | W | D D D D D D D D | Reel 3 + 4 stepper latch
-----------+---+-----------------+-----------------------------------------
2200-22FF  | W | D D D D D D D D | Reel 5 + 6 stepper latch
-----------+---+-----------------+-----------------------------------------
2300-231F  | W | D D D D D D D D | output mux
-----------+---+-----------------+-----------------------------------------
2300-230B  | R | D D D D D D D D | input mux
-----------+---+-----------------+-----------------------------------------
2320       |R/W| D D D D D D D D | dimas0 ?
-----------+---+-----------------+-----------------------------------------
2321       |R/W| D D D D D D D D | dimas1 ?
-----------+---+-----------------+-----------------------------------------
2322       |R/W| D D D D D D D D | dimas2 ?
-----------+---+-----------------+-----------------------------------------
2323       |R/W| D D D D D D D D | dimas3 ?
-----------+---+-----------------+-----------------------------------------
2324       |R/W| D D D D D D D D | expansion latch
-----------+---+-----------------+-----------------------------------------
2325       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2326       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2327       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2328       |R/W| D D D D D D D D | muxena
-----------+---+-----------------+-----------------------------------------
2329       | W | D D D D D D D D | Timer IRQ enable
-----------+---+-----------------+-----------------------------------------
232A       |R/W| D D D D D D D D | blkdiv ?
-----------+---+-----------------+-----------------------------------------
232B       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
232C       |R/W| D D D D D D D D | dimena ?
-----------+---+-----------------+-----------------------------------------
232D       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
232E       | R | D D D D D D D D | chip status b0 = IRQ status
-----------+---+-----------------+-----------------------------------------
232F       | W | D D D D D D D D | coin inhibits
-----------+---+-----------------+-----------------------------------------
2330       | W | D D D D D D D D | payout slide latch
-----------+---+-----------------+-----------------------------------------
2331       | W | D D D D D D D D | payout triac latch
-----------+---+-----------------+-----------------------------------------
2332       |R/W| D D D D D D D D | Watchdog timer
-----------+---+-----------------+-----------------------------------------
2333       | W | D D D D D D D D | electro mechanical meters
-----------+---+-----------------+-----------------------------------------
2334       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2335       | ? | D D D D D D D D | ???
-----------+---+-----------------+-----------------------------------------
2336       |?/W| D D D D D D D D | dimcnt ?
-----------+---+-----------------+-----------------------------------------
2337       | W | D D D D D D D D | volume override
-----------+---+-----------------+-----------------------------------------
2338       | W | D D D D D D D D | payout chip select
-----------+---+-----------------+-----------------------------------------
2339       | W | D D D D D D D D | clkden ?
-----------+---+-----------------+-----------------------------------------
2400       |R/W| D D D D D D D D | uart1 (MC6850 compatible) control/status
-----------+---+-----------------+-----------------------------------------
2500       |R/W| D D D D D D D D | uart1 (MC6850 compatible) data
-----------+---+-----------------+-----------------------------------------
2600       |R/W| D D D D D D D D | uart2 (MC6850 compatible) control/status
-----------+---+-----------------+-----------------------------------------
2700       |R/W| D D D D D D D D | uart2 (MC6850 compatible) data
-----------+---+-----------------+-----------------------------------------
2800       |R/W| D D D D D D D D | vfd1
-----------+---+-----------------+-----------------------------------------
2900       |R/W| D D D D D D D D | reset vfd1 + vfd2
-----------+---+-----------------+-----------------------------------------
2D00       |R/W| D D D D D D D D | ym2413 control
-----------+---+-----------------+-----------------------------------------
2D01       |R/W| D D D D D D D D | ym2413 data
-----------+---+-----------------+-----------------------------------------
2E00       |R/W| D D D D D D D D | ROM page latch
-----------+---+-----------------+-----------------------------------------
2F00       |R/W| D D D D D D D D | vfd2
-----------+---+-----------------+-----------------------------------------
3FFE       | R | D D D D D D D D | direct input1
-----------+---+-----------------+-----------------------------------------
3FFF       | R | D D D D D D D D | direct input2
-----------+---+-----------------+-----------------------------------------
2A00       | W | D D D D D D D D | NEC uPD7759 data
-----------+---+-----------------+-----------------------------------------
2B00       | W | D D D D D D D D | NEC uPD7759 reset
-----------+---+-----------------+-----------------------------------------
4000-5FFF  | R | D D D D D D D D | ROM (8k)
-----------+---+-----------------+-----------------------------------------
6000-7FFF  | R | D D D D D D D D | Paged ROM (8k)
           |   |                 |   page 0 : rom area 0x0000 - 0x1FFF
           |   |                 |   page 1 : rom area 0x2000 - 0x3FFF
           |   |                 |   page 2 : rom area 0x4000 - 0x5FFF
           |   |                 |   page 3 : rom area 0x6000 - 0x7FFF
-----------+---+-----------------+-----------------------------------------
8000-FFFF  | R | D D D D D D D D | ROM (32k)
-----------+---+-----------------+-----------------------------------------

Adder hardware:
    Games supported:
        * Quintoon (2 sets Dutch, 2 sets UK)
        * Pokio (1 set)
        * Paradice (1 set)
        * Pyramid (1 set)
        * Slots (1 set Dutch, 2 sets Belgian)
        * Golden Crown (1 Set)

    Known issues:
        * Need to find the 'missing' game numbers
        * Fix RS232 protocol
***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"

#include "machine/nvram.h"

#include "video/bfm_adr2.h"

#include "sound/2413intf.h"
#include "sound/upd7759.h"

/* fruit machines only */
#include "video/awpvid.h"
#include "machine/steppers.h" // stepper motor

#include "machine/bfm_bd1.h"  // vfd
#include "machine/meters.h"

#include "bfm_sc2.lh"
#include "gldncrwn.lh"
#include "paradice.lh"
#include "pokio.lh"
#include "pyramid.lh"
#include "quintoon.lh"
#include "sltblgpo.lh"
#include "sltblgtk.lh"
#include "slots.lh"

class bfm_sc2_state : public driver_device
{
public:
	bfm_sc2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int sc2gui_update_mmtr;
	UINT8 *nvram;
	UINT8 key[16];
	UINT8 e2ram[1024];
	int mmtr_latch;
	int triac_latch;
	int vfd1_latch;
	int vfd2_latch;
	int irq_status;
	int optic_pattern;
	int uart1_data;
	int uart2_data;
	int data_to_uart1;
	int data_to_uart2;
	int locked;
	int is_timer_enabled;
	int reel_changed;
	int coin_inhibits;
	int irq_timer_stat;
	int expansion_latch;
	int global_volume;
	int volume_override;
	int sc2_show_door;
	int sc2_door_state;
	int reel12_latch;
	int reel34_latch;
	int reel56_latch;
	int pay_latch;
	int slide_states[6];
	int slide_pay_sensor[6];
	int has_hopper;
	int triac_select;
	int hopper_running;
	int hopper_coin_sense;
	int timercnt;
	int watchdog_cnt;
	int watchdog_kicked;
	UINT8 Lamps[256];
	UINT8 sc2_Inputs[64];
	UINT8 input_override[64];
	int e2reg;
	int e2state;
	int e2cnt;
	int e2data;
	int e2address;
	int e2rw;
	int e2data_pin;
	int e2dummywrite;
	int e2data_to_read;
	UINT8 codec_data[256];
};


#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

// log serial communication between mainboard (scorpion2) and videoboard (adder2)
#define LOG_SERIAL(x) do { if (VERBOSE) logerror x; } while (0)
#define UART_LOG(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define MASTER_CLOCK		(XTAL_8MHz)

// local prototypes ///////////////////////////////////////////////////////

static int  read_e2ram(running_machine *machine);
static void e2ram_reset(running_machine *machine);

// global vars ////////////////////////////////////////////////////////////


// local vars /////////////////////////////////////////////////////////////











// user interface stuff ///////////////////////////////////////////////////



/*      INPUTS layout

     b7 b6 b5 b4 b3 b2 b1 b0

     82 81 80 04 03 02 01 00  0
     92 91 90 14 13 12 11 10  1
     A2 A1 A0 24 23 22 21 20  2
     B2 B1 B0 34 33 32 31 30  3
     -- 84 83 44 43 42 41 40  4
     -- 94 93 54 53 52 51 50  5
     -- A4 A3 64 63 62 61 60  6
     -- B4 B3 74 73 72 71 70  7

     B7 B6 B5 B4 B3 B2 B1 B0
      0  1  1  0  0  0

*/
///////////////////////////////////////////////////////////////////////////

static void send_to_adder(running_machine *machine, int data)
{
	adder2_data_from_sc2 = 1;
	adder2_sc2data       = data;

	adder2_acia_triggered = 1;
	cputag_set_input_line(machine, "adder2", M6809_IRQ_LINE, HOLD_LINE );

	LOG_SERIAL(("sadder  %02X  (%c)\n",data, data ));
}

///////////////////////////////////////////////////////////////////////////

static int receive_from_adder(void)
{
	int data = adder2_data;
	adder2_data_to_sc2 = 0;

	LOG_SERIAL(("radder:  %02X(%c)\n",data, data ));

	return data;
}

///////////////////////////////////////////////////////////////////////////

static int get_scorpion2_uart_status(running_machine *machine)
{
	int status = 0;

	if ( adder2_data_to_sc2  ) status |= 0x01;	// receive  buffer full
	if ( !adder2_data_from_sc2) status |= 0x02; // transmit buffer empty

	return status;
}

///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void on_scorpion2_reset(running_machine *machine)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	state->vfd1_latch        = 0;
	state->vfd2_latch        = 0;
	state->mmtr_latch        = 0;
	state->triac_latch       = 0;
	state->irq_status        = 0;
	state->is_timer_enabled  = 1;
	state->coin_inhibits     = 0;
	state->irq_timer_stat    = 0;
	state->expansion_latch   = 0;
	state->global_volume     = 0;
	state->volume_override   = 0;
	state->triac_select      = 0;
	state->pay_latch         = 0;

	state->reel12_latch      = 0;
	state->reel34_latch      = 0;
	state->reel56_latch      = 0;

	state->hopper_running    = 0;  // for video games
	state->hopper_coin_sense = 0;

	state->slide_states[0] = 0;
	state->slide_states[1] = 0;
	state->slide_states[2] = 0;
	state->slide_states[3] = 0;
	state->slide_states[4] = 0;
	state->slide_states[5] = 0;

	state->watchdog_cnt    = 0;
	state->watchdog_kicked = 0;


	BFM_BD1_reset(0);	// reset display1
	BFM_BD1_reset(1);	// reset display2

	e2ram_reset(machine);

	devtag_reset(machine, "ymsnd");

  // reset stepper motors /////////////////////////////////////////////////
	{
/* Although the BFM video games don't use stepper motors to control reels,
the connections are still present on the board, and some of the programs still
send data to them, although obviously there's no response. */

		int pattern =0, i;

		for ( i = 0; i < 6; i++)
		{
			stepper_reset_position(i);
			if ( stepper_optic_state(i) ) pattern |= 1<<i;
		}

		state->optic_pattern = pattern;

	}

	state->locked        = 0;

	// make sure no inputs are overidden ////////////////////////////////////
	memset(state->input_override, 0, sizeof(state->input_override));

	// init rom bank ////////////////////////////////////////////////////////

	{
		UINT8 *rom = machine->region("maincpu")->base();

		memory_configure_bank(machine, "bank1", 0, 1, &rom[0x10000], 0);
		memory_configure_bank(machine, "bank1", 1, 3, &rom[0x02000], 0x02000);

		memory_set_bank(machine, "bank1",3);
	}
}

///////////////////////////////////////////////////////////////////////////

void Scorpion2_SetSwitchState(running_machine *machine, int strobe, int data, int state)
{
	bfm_sc2_state *drvstate = machine->driver_data<bfm_sc2_state>();
	if ( strobe < 11 && data < 8 )
	{
		if ( strobe < 8 )
		{
			drvstate->input_override[strobe] |= (1<<data);

			if ( state ) drvstate->sc2_Inputs[strobe] |=  (1<<data);
			else		 drvstate->sc2_Inputs[strobe] &= ~(1<<data);
		}
		else
		{
			if ( data > 2 )
			{
				drvstate->input_override[strobe-8+4] |= (1<<(data+2));

				if ( state ) drvstate->sc2_Inputs[strobe-8+4] |=  (1<<(data+2));
				else		 drvstate->sc2_Inputs[strobe-8+4] &= ~(1<<(data+2));
			}
			else
			{
				drvstate->input_override[strobe-8] |= (1<<(data+5));

				if ( state ) drvstate->sc2_Inputs[strobe-8] |=  (1 << (data+5));
				else		 drvstate->sc2_Inputs[strobe-8] &= ~(1 << (data+5));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

int Scorpion2_GetSwitchState(running_machine *machine, int strobe, int data)
{
	bfm_sc2_state *drvstate = machine->driver_data<bfm_sc2_state>();
	int state = 0;

	if ( strobe < 11 && data < 8 )
	{
		if ( strobe < 8 )
		{
			state = (drvstate->sc2_Inputs[strobe] & (1<<data) ) ? 1 : 0;
		}
		else
		{
			if ( data > 2 )
			{
				state = (drvstate->sc2_Inputs[strobe-8+4] & (1<<(data+2)) ) ? 1 : 0;
			}
			else
			{
				state = (drvstate->sc2_Inputs[strobe-8] & (1 << (data+5)) ) ? 1 : 0;
			}
		}
	}
	return state;
}

///////////////////////////////////////////////////////////////////////////

static NVRAM_HANDLER( bfm_sc2 )
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	static const UINT8 init_e2ram[10] = { 1, 4, 10, 20, 0, 1, 1, 4, 10, 20 };
	if ( read_or_write )
	{	// writing
		file->write(state->e2ram,sizeof(state->e2ram));
	}
	else
	{ // reading
		if ( file )
		{
			file->read(state->e2ram,sizeof(state->e2ram));
		}
		else
		{
			memset(state->e2ram,0x00,sizeof(state->e2ram));
			memcpy(state->e2ram,init_e2ram,sizeof(init_e2ram));
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( watchdog_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->watchdog_kicked = 1;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( bankswitch_w )
{
	memory_set_bank(space->machine, "bank1",data & 0x03);
}

///////////////////////////////////////////////////////////////////////////

static INTERRUPT_GEN( timer_irq )
{
	bfm_sc2_state *state = device->machine->driver_data<bfm_sc2_state>();
	state->timercnt++;

	if ( state->watchdog_kicked )
	{
		state->watchdog_cnt    = 0;
		state->watchdog_kicked = 0;
	}
	else
	{
		state->watchdog_cnt++;
		if ( state->watchdog_cnt > 2 )	// this is a hack, i don't know what the watchdog timeout is, 3 IRQ's works fine
		{  // reset board
			device->machine->schedule_soft_reset();		// reset entire machine. CPU 0 should be enough, but that doesn't seem to work !!
			on_scorpion2_reset(device->machine);
			return;
		}
	}

	if ( state->is_timer_enabled )
	{
		state->irq_timer_stat = 0x01;
		state->irq_status     = 0x02;

		generic_pulse_irq_line(device, M6809_IRQ_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel12_vid_w )  // in a video cabinet this is used to drive a hopper
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->reel12_latch = data;

	if ( state->has_hopper )
	{
		int oldhop = state->hopper_running;

		if ( data & 0x01 )
		{ // hopper power
			if ( data & 0x02 )
			{
				state->hopper_running    = 1;
			}
			else
			{
				state->hopper_running    = 0;
			}
		}
		else
		{
			//state->hopper_coin_sense = 0;
			state->hopper_running    = 0;
		}

		if ( oldhop != state->hopper_running )
		{
			state->hopper_coin_sense = 0;
			oldhop = state->hopper_running;
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel34_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->reel34_latch = data;

	if ( stepper_update(2, data   ) ) state->reel_changed |= 0x04;
	if ( stepper_update(3, data>>4) ) state->reel_changed |= 0x08;

	if ( stepper_optic_state(2) ) state->optic_pattern |=  0x04;
	else                          state->optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) state->optic_pattern |=  0x08;
	else                          state->optic_pattern &= ~0x08;

	awp_draw_reel(2);
	awp_draw_reel(3);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( reel56_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->reel56_latch = data;

	if ( stepper_update(4, data   ) ) state->reel_changed |= 0x10;
	if ( stepper_update(5, data>>4) ) state->reel_changed |= 0x20;

	if ( stepper_optic_state(4) ) state->optic_pattern |=  0x10;
	else                          state->optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) state->optic_pattern |=  0x20;
	else                          state->optic_pattern &= ~0x20;

	awp_draw_reel(4);
	awp_draw_reel(5);
}

///////////////////////////////////////////////////////////////////////////
// mechanical meters //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mmtr_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int i;
	int  changed = state->mmtr_latch ^ data;

	state->mmtr_latch = data;

	for (i = 0; i<8; i++)
	{
		if ( changed & (1 << i) )
		{
			MechMtr_update(i, data & (1 << i) );
		}
	}
	if ( data & 0x1F ) cputag_set_input_line(space->machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE );
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( mux_output_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int i;
	int off = offset<<3;

	for (i=0; i<8; i++)
	{
		state->Lamps[ off+i ] = (data & (1 << i)) != 0;
	}
	if (offset == 0) // update all lamps after strobe 0 has been updated (HACK)
	{
		for ( i = 0; i < 256; i++ )
		{
			output_set_lamp_value(i, state->Lamps[i]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( mux_input_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int result = 0xFF,t1,t2;
	static const char *const port[] = { "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7", "STROBE8", "STROBE9", "STROBE10", "STROBE11" };

	if (offset < 8)
	{
		int idx = (offset & 4) ? 4 : 8;
		t1 = state->input_override[offset];	// strobe 0-7 data 0-4
		t2 = state->input_override[offset+idx];	// strobe 8-B data 0-4

		t1 = (state->sc2_Inputs[offset]   & t1) | ( ( input_port_read(space->machine, port[offset])   & ~t1) & 0x1F);
		if (idx == 8)
			t2 = (state->sc2_Inputs[offset+8] & t2) | ( ( input_port_read(space->machine, port[offset+8]) & ~t2) << 5);
		else
			t2 =  (state->sc2_Inputs[offset+4] & t2) | ( ( ( input_port_read(space->machine, port[offset+4]) & ~t2) << 2) & 0x60);

		state->sc2_Inputs[offset]   = (state->sc2_Inputs[offset]   & ~0x1F) | t1;
		state->sc2_Inputs[offset+idx] = (state->sc2_Inputs[offset+idx] & ~0x60) | t2;
		result = t1 | t2;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( unlock_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( dimas_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( dimcnt_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( unknown_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( volume_override_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int old = state->volume_override;

	state->volume_override = data?1:0;

	if ( old != state->volume_override )
	{
		ym2413_device *ym = space->machine->device<ym2413_device>("ymsnd");
		upd7759_device *upd = space->machine->device<upd7759_device>("upd");
		float percent = state->volume_override? 1.0f : (32-state->global_volume)/32.0f;

		ym->set_output_gain(0, percent);
		ym->set_output_gain(1, percent);
		upd->set_output_gain(0, percent);
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_DEVICE_HANDLER( nec_reset_w )
{
	upd7759_start_w(device, 0);
	upd7759_reset_w(device, data);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_DEVICE_HANDLER( nec_latch_w )
{
	bfm_sc2_state *state = device->machine->driver_data<bfm_sc2_state>();
	int bank = 0;

	if ( data & 0x80 )         bank |= 0x01;
	if ( state->expansion_latch & 2 ) bank |= 0x02;

	upd7759_set_bank_base(device, bank*0x20000);

	upd7759_port_w(device, 0, data&0x3F);	// setup sample
	upd7759_start_w(device, 0);
	upd7759_start_w(device, 1);
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vfd_status_hop_r )	// on video games, hopper inputs are connected to this
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	// b7 = NEC busy
	// b6 = alpha busy (also matrix board)
	// b5 - b0 = reel optics

	int result = 0;

	if ( state->has_hopper )
	{
		result |= 0x04; // hopper high level
		result |= 0x08; // hopper low  level

		result |= 0x01|0x02;

		if ( state->hopper_running )
		{
			result &= ~0x01;								  // set motor running input

			if ( state->timercnt & 0x04 ) state->hopper_coin_sense ^= 1;	  // toggle coin seen

			if ( state->hopper_coin_sense ) result &= ~0x02;		  // update coin seen input
		}
	}

	if ( !upd7759_busy_r(space->machine->device("upd")) ) result |= 0x80;			  // update sound busy input

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( expansion_latch_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int changed = state->expansion_latch^data;

	state->expansion_latch = data;

	// bit0,  1 = lamp mux disabled, 0 = lamp mux enabled
	// bit1,  ? used in Del's millions
	// bit2,  digital volume pot meter, clock line
	// bit3,  digital volume pot meter, direction line
	// bit4,  ?
	// bit5,  ?
	// bit6,  ? used in Del's millions
	// bit7   ?

	if ( changed & 0x04)
	{ // digital volume clock line changed
		if ( !(data & 0x04) )
		{ // changed from high to low,
			if ( !(data & 0x08) )
			{
				if ( state->global_volume < 31 ) state->global_volume++; //0-31 expressed as 1-32
			}
			else
			{
				if ( state->global_volume > 0  ) state->global_volume--;
			}

			{
				ym2413_device *ym = space->machine->device<ym2413_device>("ymsnd");
				upd7759_device *upd = space->machine->device<upd7759_device>("upd");
				float percent = state->volume_override ? 1.0f : (32-state->global_volume)/32.0f;

				ym->set_output_gain(0, percent);
				ym->set_output_gain(1, percent);
				upd->set_output_gain(0, percent);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( expansion_latch_r )
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( muxena_w )
{
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( timerirq_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->is_timer_enabled = data & 1;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( timerirqclr_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->irq_timer_stat = 0;
	state->irq_status     = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( irqstatus_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int result = state->irq_status | state->irq_timer_stat | 0x80;	// 0x80 = ~MUXERROR

	state->irq_timer_stat = 0;

	return result;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( coininhib_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int changed = state->coin_inhibits^data,i,p;

	state->coin_inhibits = data;

	p = 0x01;
	i = 0;

	while ( i < 8 && changed )
	{
		if ( changed & p )
		{ // this inhibit line has changed
			coin_lockout_w(space->machine, i, (~data & p) ); // update lockouts
			changed &= ~p;
		}

		p <<= 1;
		i++;
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( coin_input_r )
{
	return input_port_read(space->machine, "COINS");
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( payout_latch_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->pay_latch = data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( payout_triac_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	if ( state->triac_select == 0x57 )
	{
		int slide = 0;

		switch ( state->pay_latch )
		{
			case 0x01: slide = 1;
				break;

			case 0x02: slide = 2;
				break;

			case 0x04: slide = 3;
				break;

			case 0x08: slide = 4;
				break;

			case 0x10: slide = 5;
				break;

			case 0x20: slide = 6;
				break;
		}

		if ( slide )
		{
			if ( data == 0x4D )
			{
				if ( !state->slide_states[slide] )
				{
					if ( state->slide_pay_sensor[slide] )
					{
						int strobe = state->slide_pay_sensor[slide]>>4, data = state->slide_pay_sensor[slide]&0x0F;

						Scorpion2_SetSwitchState(space->machine, strobe, data, 0);
					}
					state->slide_states[slide] = 1;
				}
			}
			else
			{
				if ( state->slide_states[slide] )
				{
					if ( state->slide_pay_sensor[slide] )
					{
						int strobe = state->slide_pay_sensor[slide]>>4, data = state->slide_pay_sensor[slide]&0x0F;

						Scorpion2_SetSwitchState(space->machine, strobe, data, 1);
					}
					state->slide_states[slide] = 0;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( payout_select_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->triac_select = data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd1_data_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->vfd1_latch = data;
	BFM_BD1_newdata(0, data);
	BFM_BD1_draw(0);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd2_data_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->vfd2_latch = data;
	BFM_BD1_newdata(1, data);
	BFM_BD1_draw(1);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vfd_reset_w )
{
	BFM_BD1_reset(0);	  // reset both VFD's
	BFM_BD1_reset(1);
	BFM_BD1_draw(0);
	BFM_BD1_draw(1);
}

///////////////////////////////////////////////////////////////////////////
// serial port ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart1stat_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int status = 0x06;

	if ( state->data_to_uart1  ) status |= 0x01;
	if ( !state->data_to_uart2 ) status |= 0x02;

	return status;
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart1data_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	return state->uart1_data;
}

//////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart1ctrl_w )
{
	UART_LOG(("uart1ctrl:%x\n", data));
}
///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart1data_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->data_to_uart2 = 1;
	state->uart1_data    = data;
	UART_LOG(("uart1:%x\n", data));
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart2stat_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int status = 0x06;

	if ( state->data_to_uart2  ) status |= 0x01;
	if ( !state->data_to_uart1 ) status |= 0x02;

	return status;
}
///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( uart2data_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	return state->uart2_data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart2ctrl_w )
{
	UART_LOG(("uart2ctrl:%x\n", data));
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( uart2data_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->data_to_uart1 = 1;
	state->uart2_data    = data;
	UART_LOG(("uart2:%x\n", data));
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_tx_w )
{
	send_to_adder(space->machine, data);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( vid_uart_ctrl_w )
{
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_rx_r )
{
	return receive_from_adder();
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( vid_uart_ctrl_r )
{
	return get_scorpion2_uart_status(space->machine);
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( key_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	int result = state->key[ offset ];

	if ( offset == 7 )
	{
		result = (result & 0xFE) | read_e2ram(space->machine);
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
/*

The X24C08 is a CMOS 8,192 bit serial EEPROM,
internally organized 1024 x 8. The X24C08 features a
serial interface and software protocol allowing operation
on a simple two wire bus.

*/




#define SCL 0x01	//SCL pin (clock)
#define SDA	0x02	//SDA pin (data)


static void e2ram_reset(running_machine *machine)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	state->e2reg   = 0;
	state->e2state = 0;
	state->e2address = 0;
	state->e2rw    = 0;
	state->e2data_pin = 0;
	state->e2data  = (SDA|SCL);
	state->e2dummywrite = 0;
	state->e2data_to_read = 0;
}

static int recdata(bfm_sc2_state *state, int changed, int data)
{
	int res = 1;

	if ( state->e2cnt < 8 )
	{
		res = 0;

		if ( (changed & SCL) && (data & SCL) )
		{ // clocked in new data
			int pattern = 1 << (7-state->e2cnt);

			if ( data & SDA ) state->e2data |=  pattern;
			else              state->e2data &= ~pattern;

			state->e2data_pin = state->e2data_to_read & 0x80 ? 1 : 0;

			state->e2data_to_read <<= 1;

			LOG(("e2d pin= %d\n", state->e2data_pin));

			state->e2cnt++;
			if ( state->e2cnt >= 8 )
			{
				res++;
			}
		}
	}

	return res;
}

static int recAck(int changed, int data)
{
	int result = 0;

	if ( (changed & SCL) && (data & SCL) )
	{
		if ( data & SDA )
		{
			result = 1;
		}
		else
		{
			result = -1;
		}
	}
	return result;
}

//
static WRITE8_HANDLER( e2ram_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>(); // b0 = clock b1 = data

	int changed, ack;

	data ^= (SDA|SCL);  // invert signals

	changed  = (state->e2reg^data) & 0x03;

	state->e2reg = data;

	if ( changed )
	{
		while ( 1 )
		{
			if ( (  (changed & SDA) && !(data & SDA))	&&  // 1->0 on SDA  AND
				( !(changed & SCL) && (data & SCL) )    // SCL=1 and not changed
				)
			{	// X24C08 Start condition (1->0 on SDA while SCL=1)
				state->e2dummywrite = ( state->e2state == 5 );

				LOG(("e2ram:   c:%d d:%d Start condition dummywrite=%d\n", (data & SCL)?1:0, (data&SDA)?1:0, state->e2dummywrite ));

				state->e2state = 1; // ready for commands
				state->e2cnt   = 0;
				state->e2data  = 0;
				break;
			}

			if ( (  (changed & SDA) && (data & SDA))	&&  // 0->1 on SDA  AND
				( !(changed & SCL) && (data & SCL) )     // SCL=1 and not changed
				)
			{	// X24C08 Stop condition (0->1 on SDA while SCL=1)
				LOG(("e2ram:   c:%d d:%d Stop condition\n", (data & SCL)?1:0, (data&SDA)?1:0 ));
				state->e2state = 0;
				state->e2data  = 0;
				break;
			}

			switch ( state->e2state )
			{
				case 1: // Receiving address + R/W bit

					if ( recdata(state, changed, data) )
					{
						state->e2address = (state->e2address & 0x00FF) | ((state->e2data>>1) & 0x03) << 8;
						state->e2cnt   = 0;
						state->e2rw    = state->e2data & 1;

						LOG(("e2ram: Slave address received !!  device id=%01X device adr=%01d high order adr %0X RW=%d) %02X\n",
							state->e2data>>4, (state->e2data & 0x08)?1:0, (state->e2data>>1) & 0x03, state->e2rw , state->e2data ));

						state->e2state = 2;
					}
					break;

				case 2: // Receive Acknowledge

					ack = recAck(changed,data);
					if ( ack )
					{
						state->e2data_pin = 0;

						if ( ack < 0 )
						{
							LOG(("ACK = 0\n"));
							state->e2state = 0;
						}
						else
						{
							LOG(("ACK = 1\n"));
							if ( state->e2dummywrite )
							{
								state->e2dummywrite = 0;

								state->e2data_to_read = state->e2ram[state->e2address];

								if ( state->e2rw & 1 ) state->e2state = 7; // read data
								else		  state->e2state = 0; //?not sure
							}
							else
							{
								if ( state->e2rw & 1 ) state->e2state = 7; // reading
								else            state->e2state = 3; // writing
							}
							switch ( state->e2state )
							{
								case 7:
									LOG(("read address %04X\n",state->e2address));
									state->e2data_to_read = state->e2ram[state->e2address];
									break;
								case 3:
									LOG(("write, awaiting address\n"));
									break;
								default:
									LOG(("?unknow action %04X\n",state->e2address));
									break;
							}
						}
						state->e2data = 0;
					}
					break;

				case 3: // writing data, receiving address

					if ( recdata(state, changed, data) )
					{
						state->e2data_pin = 0;
						state->e2address = (state->e2address & 0xFF00) | state->e2data;

						LOG(("write address = %04X waiting for ACK\n", state->e2address));
						state->e2state = 4;
						state->e2cnt   = 0;
						state->e2data  = 0;
					}
					break;

				case 4: // wait ack, for write address

					ack = recAck(changed,data);
					if ( ack )
					{
						state->e2data_pin = 0;	// pin=0, no error !!

						if ( ack < 0 )
						{
							state->e2state = 0;
							LOG(("ACK = 0, cancel write\n" ));
						}
						else
						{
							state->e2state = 5;
							LOG(("ACK = 1, awaiting data to write\n" ));
						}
					}
					break;

				case 5: // receive data to write
					if ( recdata(state, changed, data) )
					{
						LOG(("write data = %02X received, awaiting ACK\n", state->e2data));
						state->e2cnt   = 0;
						state->e2state = 6;  // wait ack
					}
					break;

				case 6: // Receive Acknowlede after writing

					ack = recAck(changed,data);
					if ( ack )
					{
						if ( ack < 0 )
						{
							state->e2state = 0;
							LOG(("ACK=0, write canceled\n"));
						}
						else
						{
							LOG(("ACK=1, writing %02X to %04X\n", state->e2data, state->e2address));

							state->e2ram[state->e2address] = state->e2data;

							state->e2address = (state->e2address & ~0x000F) | ((state->e2address+1)&0x0F);

							state->e2state = 5; // write next address
						}
					}
					break;

				case 7: // receive address from read

					if ( recdata(state, changed, data) )
					{
						//state->e2data_pin = 0;

						LOG(("address read, data = %02X waiting for ACK\n", state->e2data ));

						state->e2state = 8;
					}
					break;

				case 8:

					if ( recAck(changed, data) )
					{
						state->e2state = 7;

						state->e2address = (state->e2address & ~0x0F) | ((state->e2address+1)&0x0F); // lower 4 bits wrap around

						state->e2data_to_read = state->e2ram[state->e2address];

						LOG(("ready for next address %04X\n", state->e2address));

						state->e2cnt   = 0;
						state->e2data  = 0;
					}
					break;

				case 0:

					LOG(("e2ram: ? c:%d d:%d\n", (data & SCL)?1:0, (data&SDA)?1:0 ));
					break;
			}
			break;
		}
	}
}

static int read_e2ram(running_machine *machine)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	LOG(("e2ram: r %d (%02X) \n", state->e2data_pin, state->e2data_to_read ));

	return state->e2data_pin;
}

static const UINT16 AddressDecode[]=
{
	0x0800,0x1000,0x0001,0x0004,0x0008,0x0020,0x0080,0x0200,
	0x0100,0x0040,0x0002,0x0010,0x0400,0x2000,0x4000,0x8000,
	0
};

static const UINT8 DataDecode[]=
{
	0x02,0x08,0x20,0x40,0x10,0x04,0x01,0x80,
	0
};


///////////////////////////////////////////////////////////////////////////
static void decode_mainrom(running_machine *machine, const char *rom_region)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	UINT8 *tmp, *rom;

	rom = machine->region(rom_region)->base();

	tmp = auto_alloc_array(machine, UINT8, 0x10000);
	{
		int i;
		long address;

		memcpy(tmp, rom, 0x10000);

		for ( i = 0; i < 256; i++ )
		{
			UINT8 data,pattern,newdata,*tab;
			data    = i;

			tab     = (UINT8*)DataDecode;
			pattern = 0x01;
			newdata = 0;

			do
			{
				newdata |= data & pattern ? *tab : 0;
				pattern <<= 1;
			} while ( *(++tab) );

			state->codec_data[i] = newdata;
		}

		for ( address = 0; address < 0x10000; address++)
		{
			int	newaddress,pattern;
			UINT16 *tab;

			tab      = (UINT16*)AddressDecode;
			pattern  = 0x0001;
			newaddress = 0;
			do
			{
				newaddress |= address & pattern ? *tab : 0;
				pattern <<= 1;
			} while ( *(++tab) );

			rom[newaddress] = state->codec_data[ tmp[address] ];
		}
		auto_free(machine, tmp);
	}
}

// machine init (called only once) ////////////////////////////////////////

static MACHINE_RESET( init )
{
	// reset adder2
	MACHINE_RESET_CALL(adder2);

	// reset the board //////////////////////////////////////////////////////

	on_scorpion2_reset(machine);
	BFM_BD1_init(0);
	BFM_BD1_init(1);
	//BFM_dm01_reset(machine); No known video based game has a Matrix board
}

static VIDEO_UPDATE( addersc2 )
{
	bfm_sc2_state *state = screen->machine->driver_data<bfm_sc2_state>();
	if ( state->sc2_show_door )
	{
		output_set_value("door",( Scorpion2_GetSwitchState(screen->machine,state->sc2_door_state>>4, state->sc2_door_state & 0x0F) ) );
	}

	return VIDEO_UPDATE_CALL(adder2);
}

// memory map for scorpion2 board video addon /////////////////////////////

static ADDRESS_MAP_START( memmap_vid, ADDRESS_SPACE_PROGRAM, 8 )

	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k RAM
	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_hop_r)		// vfd status register
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_vid_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITE(reel34_w)
	AM_RANGE(0x2200, 0x22FF) AM_WRITE(reel56_w)

	AM_RANGE(0x2300, 0x230B) AM_READ(mux_input_r)			// mux inputs
	AM_RANGE(0x2300, 0x231F) AM_WRITE(mux_output_w)			// mux outputs
	AM_RANGE(0x2320, 0x2323) AM_WRITE(dimas_w)				// ?unknown dim related

	AM_RANGE(0x2324, 0x2324) AM_READWRITE(expansion_latch_r, expansion_latch_w)
	AM_RANGE(0x2325, 0x2327) AM_WRITE(unknown_w)			// ?unknown
	AM_RANGE(0x2328, 0x2328) AM_WRITE(muxena_w)				// mux enable
	AM_RANGE(0x2329, 0x2329) AM_READWRITE(timerirqclr_r, timerirq_w)
	AM_RANGE(0x232A, 0x232D) AM_WRITE(unknown_w)			// ?unknown
	AM_RANGE(0x232E, 0x232E) AM_READ(irqstatus_r)
	AM_RANGE(0x232F, 0x232F) AM_WRITE(coininhib_w)			// coin inhibits
	AM_RANGE(0x2330, 0x2330) AM_WRITE(payout_latch_w)
	AM_RANGE(0x2331, 0x2331) AM_WRITE(payout_triac_w)
	AM_RANGE(0x2332, 0x2332) AM_WRITE(watchdog_w)			// kick watchdog
	AM_RANGE(0x2333, 0x2333) AM_WRITE(mmtr_w)				// mechanical meters
	AM_RANGE(0x2334, 0x2335) AM_WRITE(unknown_w)
	AM_RANGE(0x2336, 0x2336) AM_WRITE(dimcnt_w)				// ?unknown dim related
	AM_RANGE(0x2337, 0x2337) AM_WRITE(volume_override_w)
	AM_RANGE(0x2338, 0x2338) AM_WRITE(payout_select_w)
	AM_RANGE(0x2339, 0x2339) AM_WRITE(unknown_w)			// ?unknown
	AM_RANGE(0x2400, 0x2400) AM_READWRITE(uart1stat_r, uart1ctrl_w)	// mc6850 compatible uart
	AM_RANGE(0x2500, 0x2500) AM_READWRITE(uart1data_r, uart1data_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(uart2stat_r, uart2ctrl_w)	// mc6850 compatible uart
	AM_RANGE(0x2700, 0x2700) AM_READWRITE(uart2data_r, uart2data_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(vfd1_data_w)			// vfd1 data
	AM_RANGE(0x2900, 0x2900) AM_WRITE(vfd_reset_w)			// vfd1+vfd2 reset line

	AM_RANGE(0x2A00, 0x2AFF) AM_DEVWRITE("upd", nec_latch_w)			// this is where it reads?
	AM_RANGE(0x2B00, 0x2BFF) AM_DEVWRITE("upd", nec_reset_w)			// upd7759 reset line
	AM_RANGE(0x2C00, 0x2C00) AM_WRITE(unlock_w)				// custom chip unlock
	AM_RANGE(0x2D00, 0x2D01) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x2E00, 0x2E00) AM_WRITE(bankswitch_w)			// write bank (rom page select for 0x6000 - 0x7fff )
	AM_RANGE(0x2F00, 0x2F00) AM_WRITE(vfd2_data_w)			// vfd2 data

	AM_RANGE(0x3C00, 0x3C07) AM_READ(  key_r   )
	AM_RANGE(0x3C80, 0x3C80) AM_WRITE( e2ram_w )

	AM_RANGE(0x3E00, 0x3E00) AM_READWRITE(vid_uart_ctrl_r, vid_uart_ctrl_w)		// video uart control reg
	AM_RANGE(0x3E01, 0x3E01) AM_READWRITE(vid_uart_rx_r, vid_uart_tx_w)			// video uart data  reg
	AM_RANGE(0x3FFF, 0x3FFF) AM_READ(coin_input_r)
	AM_RANGE(0x4000, 0x5fff) AM_ROM							// 8k  fixed ROM
	AM_RANGE(0x4000, 0xFFFF) AM_WRITE(unknown_w)			// contains unknown I/O registers
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")					// 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0xFFFF) AM_ROM							// 32k ROM

ADDRESS_MAP_END

// input ports for pyramid ////////////////////////////////////////

static INPUT_PORTS_START( pyramid )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.50")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER)     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "Coin 1 Lockout")PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin 2 Lockout")PORT_DIPLOCATION("DIL:!03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin 3 Lockout")PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin 4 Lockout")PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPNAME( 0x02, 0x00, "Attract mode language" ) PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, "Dutch"       )
	PORT_DIPNAME( 0x0C, 0x00, "Skill Level" ) PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x04, "Medium-Low" )
	PORT_DIPSETTING(    0x08, "Medium-High")
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!11" )

	PORT_START("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" ) PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Stake" ) PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "4 credits per game"  )
	PORT_DIPSETTING(    0x08, "1 credit  per round" )
	PORT_DIPSETTING(    0x10, "2 credit  per round" )
	PORT_DIPSETTING(    0x18, "4 credits per round" )
INPUT_PORTS_END

// input ports for golden crown ///////////////////////////////////

static INPUT_PORTS_START( gldncrwn )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME( "Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME( "Reel 1" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME( "Reel 2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME( "Reel 3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME( "Reel 4" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME( "Reel 5" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME( "Reel 6" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME( "Hall Of Fame" ) PORT_CODE( KEYCODE_J )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Attract mode language" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, "Dutch")
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x00, "Max number of spins" )PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, "99")
	PORT_DIPSETTING(    0x02, "50")
	PORT_DIPNAME( 0x0C, 0x00, "Skill Level" )PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ))
	PORT_DIPSETTING(    0x04, "Medium-Low"  )
	PORT_DIPSETTING(    0x08, "Medium-High" )
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, "Base Pricing on:" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, "Full Game")
	PORT_DIPSETTING(    0x10, "Individual Rounds")

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Credits required:" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, "4 credits per game")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x01, "2 credits per game")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 credit  per round")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x01, "4 credits per round")PORT_CONDITION("STROBE10",0x10,PORTCOND_EQUALS,0x10)

/*
          Type1 Type2
              0     0   4 credits per game
              0     1   2 credits per game
              1     0   1 credit  per round
              1     1   4 credits per round
 */

	PORT_DIPNAME( 0x02, 0x00, "Attract Mode" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Time bar" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Time bar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "1 (fast)" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4 (slow)" )
INPUT_PORTS_END

// input ports for dutch quintoon /////////////////////////////////

static INPUT_PORTS_START( qntoond )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.50")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)	  PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hand 1" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hand 2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hand 3" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hand 4" )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hand 5" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x1e, 0x1c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIL:!02,!03,!04,!05")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, "C1=2.5 C2=1.25 C3=0.5 C4=2C_0.25C" )
	PORT_DIPSETTING(    0x04, "C1=10 C2=5 C3=2 C4=0.5" )
	PORT_DIPSETTING(    0x06, "C1=1.5/3.25/5 C2=0.75/1.5/3.25 C3=0.25/0.5/1 C4=3C_0.25C" )
	PORT_DIPSETTING(    0x08, "C1=20 C2=10 C3=4 C4=1" )
	PORT_DIPSETTING(    0x0a, "C1=2 C2=1 C3=0.25/0.75/1/1.5/2 C4=3C_0.25C 5C_0.5C" )
	PORT_DIPSETTING(    0x0c, "C1=5 C2=2.5 C3=1 C4=0.25" )
	PORT_DIPSETTING(    0x0e, "C1=1.25 C2=0.5/1.25/1.75 C3=0.25 C4=0.25" )
	//PORT_DIPSETTING(    0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x12, "C1=3 C2=1.5 C3=0.5 C4=0.25" )
	PORT_DIPSETTING(    0x14, "C1=12 C2=6 C3=2 C4=0.5" )
	PORT_DIPSETTING(    0x16, "C1=2 C2=1 C3=0.25 C4=3C_0.25C" )
	PORT_DIPSETTING(    0x18, "C1=24 C2=12 C3=4 C4=1" )
	PORT_DIPSETTING(    0x1a, "C1=2.25/4.75 C2=1/2.25/3.5/4.75/6 C3=0.25/0.75/1/1.5/2 C4=3C_0.25C 5C_0.5C" )
	PORT_DIPSETTING(    0x1c, "C1=6 C2=3 C3=1 C4=0.25" )
	PORT_DIPSETTING(    0x1e, "C1=1.5 C2=0.75 C3=0.25 C4=4C_0.25C" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin 1 Lockout")PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Coin 2 Lockout")PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin 3 Lockout")PORT_DIPLOCATION("DIL:!08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin 4 Lockout")PORT_DIPLOCATION("DIL:!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Time bar" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Clear credits on reset" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode language" )PORT_DIPLOCATION("DIL:!15")
	PORT_DIPSETTING(    0x00, DEF_STR( English  ) )
	PORT_DIPSETTING(    0x08, "Dutch"    )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!16" )
INPUT_PORTS_END

// input ports for UK quintoon ////////////////////////////////////////////

static INPUT_PORTS_START( quintoon )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Collect") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hand 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hand 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hand 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hand 4")

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hand 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("?1") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("?2") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("?3") PORT_CODE(KEYCODE_O)

	PORT_MODIFY("STROBE5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SPECIAL) //Payout opto

	PORT_MODIFY("STROBE9")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!02" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!04" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "DIL:!05" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin Lockout")PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) //Will activate coin lockout when Credit >= 1 Play
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Stake per Game / Jackpot" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, "20p / 6 Pounds" )
	PORT_DIPSETTING(    0x10, "50p / 20 Pounds" )

	PORT_MODIFY("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x1C, 0x00, "Target percentage" )PORT_DIPLOCATION("DIL:!14,!15,!16")
	PORT_DIPSETTING(    0x1C, "50%")
	PORT_DIPSETTING(    0x0C, "55%")
	PORT_DIPSETTING(    0x08, "60%")
	PORT_DIPSETTING(    0x18, "65%")
	PORT_DIPSETTING(    0x10, "70%")
	PORT_DIPSETTING(    0x00, "75%")
	PORT_DIPSETTING(    0x04, "80%")
	PORT_DIPSETTING(    0x14, "85%")
INPUT_PORTS_END

// input ports for slotsnl  ///////////////////////////////////////////////

static INPUT_PORTS_START( slotsnl )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Slot 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Slot 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_NAME("Slot 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_NAME("Slot 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )   PORT_NAME("Enter") PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!12" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x18, "4" )
INPUT_PORTS_END

// input ports for sltblgtk  //////////////////////////////////////////////

static INPUT_PORTS_START( sltblgtk )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20 BFr")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50 BFr")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Slot 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Slot 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Slot 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Slot 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("Enter") PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SPECIAL ) //Tube 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SPECIAL ) //Tube 2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "CashMeters in refill menu" )PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Token Lockout" )PORT_DIPLOCATION("DIL:!03")
	PORT_DIPSETTING(    0x00, DEF_STR( No  ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "20 Bfr Lockout" )PORT_DIPLOCATION("DIL:!04")
	PORT_DIPSETTING(    0x00, DEF_STR( No  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "50 Bfr Lockout" )PORT_DIPLOCATION("DIL:!05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPNAME( 0x0E, 0x00, "Payout Percentage" )PORT_DIPLOCATION("DIL:!07,!08,!10")
	PORT_DIPSETTING(    0x00, "60%")
	PORT_DIPSETTING(    0x08, "65%")
	PORT_DIPSETTING(    0x04, "70%")
	PORT_DIPSETTING(    0x0C, "75%")
	PORT_DIPSETTING(    0x02, "80%")
	PORT_DIPSETTING(    0x0A, "84%")
	PORT_DIPSETTING(    0x06, "88%")
	PORT_DIPSETTING(    0x0E, "90%")
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Timebar" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Clear credits" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On   ) )
	PORT_DIPNAME( 0x08, 0x00, "Show hints" )PORT_DIPLOCATION("DIL:!15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Pay win to credits" )PORT_DIPLOCATION("DIL:!16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

// input ports for sltblgpo  //////////////////////////////////////////////

static INPUT_PORTS_START( sltblgpo )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Bfr 20")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Bfr 50")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hand 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hand 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_NAME("Hand 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_NAME("Hand 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_NAME("Stake")  PORT_CODE( KEYCODE_O )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Collect") PORT_CODE(KEYCODE_C)

	PORT_MODIFY("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "Hopper Limit" )PORT_DIPLOCATION("DIL:!02")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!03" )
	PORT_DIPNAME( 0x18, 0x00, "Attendant payout" )PORT_DIPLOCATION("DIL:!04,!05")
	PORT_DIPSETTING(    0x00, "1000 Bfr" )
	PORT_DIPSETTING(    0x08, "1250 Bfr" )
	PORT_DIPSETTING(    0x10, "1500 Bfr" )
	PORT_DIPSETTING(    0x18, "1750 Bfr" )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Bfr 20 Lockout" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Clear credits on reset?" )PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Attract Mode" )PORT_DIPLOCATION("DIL:!13")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Target Percentage" )PORT_DIPLOCATION("DIL:!14,!15,!16")
	PORT_DIPSETTING(    0x14, "80%")
	PORT_DIPSETTING(    0x04, "82%")
	PORT_DIPSETTING(    0x1C, "84%")
	PORT_DIPSETTING(    0x0C, "86%")
	PORT_DIPSETTING(    0x10, "90%")
	PORT_DIPSETTING(    0x00, "92%")
	PORT_DIPSETTING(    0x18, "94%")
	PORT_DIPSETTING(    0x08, "96%")
INPUT_PORTS_END

// input ports for paradice ///////////////////////////////////////////////

static INPUT_PORTS_START( paradice )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME( "1 Player Start (Left)" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME( "2 Player Start (Right)" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME( "A" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME( "B" )

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME( "C" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME( "Enter" ) PORT_CODE( KEYCODE_E )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Joker" )PORT_DIPLOCATION("DIL:!06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Language ) )PORT_DIPLOCATION("DIL:!07")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x02, "Dutch"    )
	PORT_DIPNAME( 0x0C, 0x00, "Payout level" )PORT_DIPLOCATION("DIL:!08,!10")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x08, "Medium-Low"  )
	PORT_DIPSETTING(    0x04, "Medium-High" )
	PORT_DIPSETTING(    0x0C, DEF_STR( High ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Difficulty ) )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x03, 0x00, "Winlines to go" )PORT_DIPLOCATION("DIL:!12,!13")
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x03, "9" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
INPUT_PORTS_END

// input ports for pokio //////////////////////////////////////////////////

static INPUT_PORTS_START( pokio )
	PORT_INCLUDE( pyramid )

	PORT_MODIFY("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")

	PORT_MODIFY("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Hand 1 Left" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME( "Hand 2 Left" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME( "Hand 3 Left" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME( "1 Player Start (Left)" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME( "Enter" ) PORT_CODE( KEYCODE_SPACE )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME( "2 Player Start (Right)" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 )PORT_NAME( "Hand 3 Right" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )PORT_NAME( "Hand 2 Right" )

	PORT_MODIFY("STROBE3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )PORT_NAME( "Hand 1 Right" )

	PORT_MODIFY("STROBE10")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "DIL:!06" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!07" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "DIL:!08" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "DIL:!10" )
	PORT_DIPNAME( 0x10, 0x00, "Coin Jam Alarm" )PORT_DIPLOCATION("DIL:!11")
	PORT_DIPSETTING(    0x10, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On   ) )

	PORT_MODIFY("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "Time bar" ) PORT_DIPLOCATION("DIL:!12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "DIL:!13" )
	PORT_DIPNAME( 0x04, 0x00, "Attract mode" )PORT_DIPLOCATION("DIL:!14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x18, 0x00, "Timebar speed" )PORT_DIPLOCATION("DIL:!15,!16")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
INPUT_PORTS_END


///////////////////////////////////////////////////////////////////////////
// machine driver for scorpion2 board + adder2 expansion //////////////////
///////////////////////////////////////////////////////////////////////////

static MACHINE_CONFIG_START( scorpion2_vid, bfm_sc2_state )
	MCFG_MACHINE_RESET( init )							// main scorpion2 board initialisation
	MCFG_QUANTUM_TIME(attotime::from_hz(960))									// needed for serial communication !!
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )	// 6809 CPU at 2 Mhz
	MCFG_CPU_PROGRAM_MAP(memmap_vid)					// setup scorpion2 board memorymap
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000)				// generate 1000 IRQ's per second

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_HANDLER(bfm_sc2)
	MCFG_DEFAULT_LAYOUT(layout_bfm_sc2)

	MCFG_SCREEN_ADD("adder", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 400, 280)
	MCFG_SCREEN_VISIBLE_AREA(  0, 400-1, 0, 280-1)
	MCFG_SCREEN_REFRESH_RATE(50)

	MCFG_VIDEO_START( adder2)
	MCFG_VIDEO_RESET( adder2)
	MCFG_VIDEO_UPDATE(addersc2)

	MCFG_PALETTE_LENGTH(16)
	MCFG_PALETTE_INIT(adder2)
	MCFG_GFXDECODE(adder2)

	MCFG_CPU_ADD("adder2", M6809, MASTER_CLOCK/4 )	// adder2 board 6809 CPU at 2 Mhz
	MCFG_CPU_PROGRAM_MAP(adder2_memmap)				// setup adder2 board memorymap
	MCFG_CPU_VBLANK_INT("adder", adder2_vbl)			// board has a VBL IRQ

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static void sc2_common_init(running_machine *machine, int decrypt)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	UINT8 *rom;

	if (decrypt) decode_mainrom(machine, "maincpu");		  // decode main rom

	rom = machine->region("maincpu")->base();
	if ( rom )
	{
		memcpy(&rom[0x10000], &rom[0x00000], 0x2000);
	}

	memset(state->sc2_Inputs, 0, sizeof(state->sc2_Inputs));  // clear all inputs
}

static void adder2_common_init(running_machine *machine)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	UINT8 *pal;

	pal = machine->region("proms")->base();
	if ( pal )
	{
		memcpy(state->key, pal, 8);
	}
}

// UK quintoon initialisation ////////////////////////////////////////////////

static DRIVER_INIT (quintoon)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);
	MechMtr_config(machine,8);					// setup mech meters

	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	Scorpion2_SetSwitchState(machine,5,2,1);
	Scorpion2_SetSwitchState(machine,6,4,1);

	state->sc2_show_door   = 1;
	state->sc2_door_state  = 0x41;
}

// dutch pyramid intialisation //////////////////////////////////////////////

static DRIVER_INIT( pyramid )
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->has_hopper = 1;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	state->sc2_show_door   = 1;
	state->sc2_door_state  = 0x41;
}
// belgian slots initialisation /////////////////////////////////////////////

static DRIVER_INIT( sltsbelg )
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->has_hopper = 1;

	state->sc2_show_door   = 1;
	state->sc2_door_state  = 0x41;
}

// other dutch adder games ////////////////////////////////////////////////

static DRIVER_INIT( adder_dutch )
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	state->sc2_show_door   = 1;
	state->sc2_door_state  = 0x41;
}

// golden crown //////////////////////////////////////////////////////////

static DRIVER_INIT( gldncrwn )
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2_common_init(machine, 1);
	adder2_decode_char_roms(machine);			// decode GFX roms
	adder2_common_init(machine);

	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);	// tube1 level switch
	Scorpion2_SetSwitchState(machine,3,1,1);	// tube2 level switch
	Scorpion2_SetSwitchState(machine,3,2,1);	// tube3 level switch

	state->sc2_show_door   = 0;
	state->sc2_door_state  = 0x41;
}

// ROM definition UK Quintoon ////////////////////////////////////////////

ROM_START( quintoon )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750206.p1",	0x00000, 0x10000,  CRC(05f4bfad) SHA1(22751573f3a51a9fd2d2a75a7d1b20d78112e0bb))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",		0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",	0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",		0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition UK Quintoon (older) ////////////////////////////////////

ROM_START( quintono )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750203.bin",	0x00000, 0x10000,  CRC(037ef2d0) SHA1(6958624e29629a7639a80e8929b833a8b0201833))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",		0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",	0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",		0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition UK Quintoon (data) /////////////////////////////////////

ROM_START( quintond )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95751206.bin",	0x00000, 0x10000,  CRC(63def707) SHA1(d016df74f4f83cd72b16f9ccbe78cc382bf056c8))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("quinp132",		0x00000, 0x20000,  CRC(63896a7f) SHA1(81aa56874a15faa3aabdfc0fc524b2e25b751f22))

	ROM_REGION( 0x20000, "upd", 0 ) // using Dutch samples, need to check a UK Quintoon PCB
	ROM_LOAD("95001016.snd",	0x00000, 0x20000, BAD_DUMP CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("quinp233",		0x00000, 0x20000, CRC(3d4ebecf) SHA1(b339cf16797ccf7a1ec20fcebf52b6edad9a1047))
ROM_END

// ROM definition Dutch Quintoon ///////////////////////////////////////////

ROM_START( qntoond )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750243.bin", 0x00000, 0x10000, CRC(36a8dcd1) SHA1(ab21301312fbb6609f850e1cf6bcda5a2b7f66f5))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770024.vid", 0x00000, 0x20000, CRC(5bc7ac55) SHA1(b54e9684f750b73c357d41b88ca8c527258e2a10))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770025.chr", 0x00000, 0x20000, CRC(f59748ea) SHA1(f0f7f914fdf72db8eb60717b95e7d027c0081339))
ROM_END

// ROM definition Dutch Quintoon alternate set /////////////////////////////

ROM_START( qntoondo )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750136.bin", 0x00000, 0x10000, CRC(839ea01d) SHA1(d7f77dbaea4e87c3d782408eb50d10f44b6df5e2))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770024.vid", 0x00000, 0x20000, CRC(5bc7ac55) SHA1(b54e9684f750b73c357d41b88ca8c527258e2a10))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(cf097d41) SHA1(6712f93896483360256d8baffc05977c8e532ef1))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770025.chr", 0x00000, 0x20000, CRC(f59748ea) SHA1(f0f7f914fdf72db8eb60717b95e7d027c0081339))
ROM_END

// ROM definition dutch golden crown //////////////////////////////////////

ROM_START( gldncrwn )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95752011.bin", 0x00000, 0x10000, CRC(54f7cca0) SHA1(835727d88113700a38060f880b4dfba2ded41487))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770117.vid", 0x00000, 0x20000, CRC(598ba7cb) SHA1(ab518d7df24b0b453ec3fcddfc4db63e0391fde7))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001039.snd", 0x00000, 0x20000, CRC(6af26157) SHA1(9b3a85f5dd760c4430e38e2844928b74aadc7e75))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770118.ch1", 0x00000, 0x20000, CRC(9c9ac946) SHA1(9a571e7d00f6654242aface032c2fb186ef44aba))
	ROM_LOAD("95770119.ch2", 0x20000, 0x20000, CRC(9e0fdb2e) SHA1(05e8257285b0009df4fcc73e93490876358a8be8))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("gcrpal.bin", 0, 8 , CRC(4edd5a1d) SHA1(d6fe38377d5f2291d33ee8ed808548871e63c4d7))
ROM_END

// ROM definition Dutch Paradice //////////////////////////////////////////

ROM_START( paradice )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750615.bin", 0x00000, 0x10000, CRC(f51192e5) SHA1(a1290e32bba698006e83fd8d6075202586232929))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770084.vid", 0x00000, 0x20000, CRC(8f27bd34) SHA1(fccf7283b5c952b74258ee6e5138c1ca89384e24))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001037.snd", 0x00000, 0x20000, CRC(82f74276) SHA1(c51c3caeb7bf514ec7a1b452c8effc4c79186062))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770085.ch1", 0x00000, 0x20000, CRC(4d1fb82f) SHA1(054f683d1d7c884911bd2d0f85aab4c59ddf9930))
	ROM_LOAD("95770086.ch2", 0x20000, 0x20000, CRC(7b566e11) SHA1(f34c82ad75a0f88204ac4ae83a00801215c46ca9))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD( "pdcepal.bin", 0, 8 , CRC(64020c97) SHA1(9371841e2df950c1f2e5b5a4b52621beb6f60945))
ROM_END

// ROM definition Dutch Pokio /////////////////////////////////////////////

ROM_START( pokio )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750278.bin", 0x00000, 0x10000, CRC(5124b24d) SHA1(9bc63891a8e9283c2baa64c264a5d6d1625d44b2))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770044.vid", 0x00000, 0x20000, CRC(46d7a6d8) SHA1(01f58e735621661b57c61491b3769ae99e92476a))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001016.snd", 0x00000, 0x20000, CRC(98aaff76) SHA1(4a59cf83daf018d93f1ff7805e06309d2f3d7252))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770045.chr", 0x00000, 0x20000, CRC(dd30da90) SHA1(b4f5a229d88613c0c7d43adf3f325c619abe38a3))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("pokiopal.bin", 0, 8 , CRC(53535184) SHA1(c5c98085e39ca3671dca72c21a8466d7d70cd341))
ROM_END

// ROM definition pyramid prototype  //////////////////////////////////////

ROM_START( pyramid )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750898.bin", 0x00000, 0x10000,  CRC(3b0df16c) SHA1(9af599fe604f86c72986aa1610d74837852e023f))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770108.vid", 0x00000, 0x20000,  CRC(216ff683) SHA1(227764771600ce88c5f36bed9878e6bb9988ae8f))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001038.snd", 0x00000, 0x20000, CRC(f885c42e) SHA1(4d79fc5ae4c58247740d78d81302bfbb43331c43))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770106.ch1", 0x00000, 0x20000, CRC(a83c27ae) SHA1(f61ca3cdf19a933bae18c1b32a5fb0a2204dde78))
	ROM_LOAD("95770107.ch2", 0x20000, 0x20000, CRC(52e59f64) SHA1(ea4828c2cfb72cd77c92c60560b4d5ee424f7dca))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("pyrmdpal.bin", 0, 8 , CRC(1c7c37bb) SHA1(fe0276603fee8f58e4318f91645260368212b78b))
ROM_END

// ROM definition Dutch slots /////////////////////////////////////////////

ROM_START( slotsnl )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750368.bin", 0x00000, 0x10000, CRC(3a43048c) SHA1(13728e05b334cba90ea9cc51ea00c4384baa8614))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("video.vid",	 0x00000, 0x20000, CRC(cc760208) SHA1(cc01b1e31335b26f2d0f3470d8624476b153655f))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("95001029.snd", 0x00000, 0x20000, CRC(7749c724) SHA1(a87cce0c99e392f501bba44b3936a7059d682c9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("charset.chr",	 0x00000, 0x20000,  CRC(ef4300b6) SHA1(a1f765f38c2f146651fc685ea6195af72465f559))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD( "slotspal.bin", 0, 8 , CRC(ee5421f0) SHA1(21bdcbf11dda8b1a93c49ae1c706954bba53c917))
ROM_END

// ROM definition Belgian Slots (Token pay per round) Payslide ////////////

ROM_START( sltblgtk )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750943.bin", 0x00000, 0x10000, CRC(c9fb8153) SHA1(7c1d0660c15f05b1e0784d8322c62981fe8dc4c9))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("adder121.bin", 0x00000, 0x20000, CRC(cedbbf28) SHA1(559ae341b55462feea771127394a54fc65266818))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound029.bin", 0x00000, 0x20000, CRC(7749c724) SHA1(a87cce0c99e392f501bba44b3936a7059d682c9c))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("chr122.bin",	 0x00000, 0x20000, CRC(a1e3bdf4) SHA1(f0cabe08dee028e2014cbf0fc3fe0806cdfa60c6))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbtpal.bin", 0, 8 , CRC(20e13635) SHA1(5aa7e7cac8c00ebc193d63d0c6795904f42c70fa))
ROM_END

// ROM definition Belgian Slots (Cash Payout) /////////////////////////////

ROM_START( sltblgp1 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95752008.bin", 0x00000, 0x10000, CRC(3167d3b9) SHA1(a28563f65d55c4d47f3e7fdb41e050d8a733b9bd))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("adder142.bin", 0x00000, 0x20000, CRC(a6f6356b) SHA1(b3d3063155ee3ea888273081f844279b6e33f7d9))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound033.bin", 0x00000, 0x20000, CRC(bb1dfa55) SHA1(442454fccfe03e6f4c3353551cb7459e184a099d))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("chr143.bin",	 0x00000, 0x20000, CRC(a40e91e2) SHA1(87dc76963ea961fcfbe4f3e25df9162348d39d79))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbcpal.bin", 0, 8 , CRC(c63bcab6) SHA1(238841165d5b3241b0bcc5c1792e9c0be1fc0177))
ROM_END

// ROM definition Belgian Slots (Cash Payout) /////////////////////////////

ROM_START( sltblgpo )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95770938.bin", 0x00000, 0x10000, CRC(7e802634) SHA1(fecf86e632546649d5e647c42a248b39fc2cf982))

	ROM_REGION( 0x20000, "adder2", 0 )
	ROM_LOAD("95770120.chr", 0x00000, 0x20000, CRC(ad505138) SHA1(67ccd8dc30e76283247ab5a62b22337ebaff74cd))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("sound033.bin", 0x00000, 0x20000, CRC(bb1dfa55) SHA1(442454fccfe03e6f4c3353551cb7459e184a099d))

	ROM_REGION( 0x40000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD("95770110.add", 0x00000, 0x20000, CRC(64b03284) SHA1(4b1c17b75e449c9762bb949d7cde0694a3aaabeb))

	ROM_REGION( 0x10, "proms", 0 )
	ROM_LOAD("stsbcpal.bin", 0, 8 , CRC(c63bcab6) SHA1(238841165d5b3241b0bcc5c1792e9c0be1fc0177))
ROM_END

//     year, name,     parent,    machine,       input,     init,       monitor, company,    fullname
GAMEL( 1993, qntoondo, qntoond,	  scorpion2_vid, qntoond,   adder_dutch,0,       "BFM/ELAM", "Quintoon (Dutch, Game Card 95-750-136)",		GAME_SUPPORTS_SAVE,layout_quintoon )
GAMEL( 1993, quintoon, 0,		  scorpion2_vid, quintoon,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-750-206)",			GAME_SUPPORTS_SAVE|GAME_IMPERFECT_SOUND,layout_quintoon ) //Current samples need verification
GAMEL( 1993, quintond, quintoon,  scorpion2_vid, quintoon,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-751-206, Datapak)",GAME_SUPPORTS_SAVE|GAME_IMPERFECT_SOUND|GAME_NOT_WORKING,layout_quintoon ) //Current samples need verification
GAMEL( 1993, quintono, quintoon,  scorpion2_vid, quintoon,  quintoon,   0,       "BFM",      "Quintoon (UK, Game Card 95-750-203)",			GAME_SUPPORTS_SAVE|GAME_IMPERFECT_SOUND,layout_quintoon ) //Current samples need verification
GAMEL( 1993, qntoond,  0,		  scorpion2_vid, qntoond,   adder_dutch,0,       "BFM/ELAM", "Quintoon (Dutch, Game Card 95-750-243)",		GAME_SUPPORTS_SAVE,layout_quintoon )
GAMEL( 1994, pokio,    0,		  scorpion2_vid, pokio,     adder_dutch,0,       "BFM/ELAM", "Pokio (Dutch, Game Card 95-750-278)",			GAME_SUPPORTS_SAVE,layout_pokio )
GAMEL( 1995, slotsnl,  0,		  scorpion2_vid, slotsnl,   adder_dutch,0,       "BFM/ELAM", "Slots (Dutch, Game Card 95-750-368)",			GAME_SUPPORTS_SAVE,layout_slots )
GAMEL( 1995, paradice, 0,		  scorpion2_vid, paradice,  adder_dutch,0,       "BFM/ELAM", "Paradice (Dutch, Game Card 95-750-615)",		GAME_SUPPORTS_SAVE,layout_paradice )
GAMEL( 1996, pyramid,  0,		  scorpion2_vid, pyramid,   pyramid,	0,       "BFM/ELAM", "Pyramid (Dutch, Game Card 95-750-898)",		GAME_SUPPORTS_SAVE,layout_pyramid )

GAMEL( 1996, sltblgtk, 0,		  scorpion2_vid, sltblgtk,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Token, Game Card 95-750-943)",	GAME_SUPPORTS_SAVE,layout_sltblgtk )
GAMEL( 1996, sltblgpo, 0,		  scorpion2_vid, sltblgpo,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Cash, Game Card 95-750-938)",	GAME_SUPPORTS_SAVE,layout_sltblgpo )
GAMEL( 1996, sltblgp1, sltblgpo,  scorpion2_vid, sltblgpo,  sltsbelg,   0,       "BFM/ELAM", "Slots (Belgian Cash, Game Card 95-752-008)",	GAME_SUPPORTS_SAVE,layout_sltblgpo )
GAMEL( 1997, gldncrwn, 0,		  scorpion2_vid, gldncrwn,  gldncrwn,   0,       "BFM/ELAM", "Golden Crown (Dutch, Game Card 95-752-011)",	GAME_SUPPORTS_SAVE,layout_gldncrwn )

/**************************************************************************

    Mechanical Scorpion 2 Games
        AGEMAME driver

***************************************************************************

  30-12-2006: J Wallace: Fixed init routines.
  07-03-2006: El Condor: Recoded to more accurately represent the hardware
              setup.
  18-01-2006: Cleaned up for MAME inclusion
  19-08-2005: Re-Animator

***************************************************************************/

/* fruit machines only */
#include "video/bfm_dm01.h"
#include "awpdmd.lh"
#include "drwho.lh"
#include "awpvid14.lh"
#include "awpvid16.lh"


/* Reels 1 and 2 */
static WRITE8_HANDLER( reel12_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->reel12_latch = data;

	if ( stepper_update(0, data   ) ) state->reel_changed |= 0x01;
	if ( stepper_update(1, data>>4) ) state->reel_changed |= 0x02;

	if ( stepper_optic_state(0) ) state->optic_pattern |=  0x01;
	else                          state->optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) state->optic_pattern |=  0x02;
	else                          state->optic_pattern &= ~0x02;

	awp_draw_reel(0);
	awp_draw_reel(1);
}


/* VFD Status */
static READ8_HANDLER( vfd_status_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	/* b7 = NEC busy */
	/* b6 = alpha busy (also matrix board) */
	/* b5 - b0 = reel optics */

	int result = state->optic_pattern;

	if ( !upd7759_busy_r(space->machine->device("upd")) ) result |= 0x80;

	return result;
}

/* VFD Status and data */
static READ8_HANDLER( vfd_status_dm01_r )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	/* b7 = NEC busy */
	/* b6 = alpha busy (also matrix board) */
	/* b5 - b0 = reel optics */

	int result = state->optic_pattern;

	if ( !upd7759_busy_r(space->machine->device("upd")) ) result |= 0x80;

	if ( BFM_dm01_busy() ) result |= 0x40;

	return result;
}


static WRITE8_HANDLER( vfd1_data_dm01_w )
{
	bfm_sc2_state *state = space->machine->driver_data<bfm_sc2_state>();
	state->vfd1_latch = data;
	BFM_dm01_writedata(space->machine,data);
}


static READ8_HANDLER( direct_input_r )
{
	return 0;
}


#ifdef UNUSED_FUNCTION
/* Scorpion 3 expansion */
static READ8_HANDLER( sc3_expansion_r )
{
    int result = 0;

    switch ( offset )
    {
        case 0: result = 0;
        break;
        case 1: result = input_port_read_indexed(machine,0);  /* coin input */
    }

    return result;
}


static WRITE8_HANDLER( sc3_expansion_w )
{
    switch ( offset )
    {
	    case 0:
    	break;
	    case 1:
    	break;
    }
}
#endif


/* machine init (called only once) */
static MACHINE_RESET( awp_init )
{
	on_scorpion2_reset(machine);
	BFM_BD1_init(0);
	BFM_BD1_init(1);
}


static MACHINE_RESET( dm01_init )
{
	on_scorpion2_reset(machine);
	BFM_dm01_reset(machine);
}


static ADDRESS_MAP_START( sc2_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k
	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_r)
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITE(reel34_w)
	AM_RANGE(0x2200, 0x22FF) AM_WRITE(reel56_w)

	AM_RANGE(0x2300, 0x230B) AM_READ(mux_input_r)
	AM_RANGE(0x2300, 0x231F) AM_WRITE(mux_output_w)
	AM_RANGE(0x2320, 0x2323) AM_WRITE(dimas_w)				/* ?unknown dim related */

	AM_RANGE(0x2324, 0x2324) AM_READWRITE(expansion_latch_r, expansion_latch_w)
	AM_RANGE(0x2325, 0x2327) AM_WRITE(unknown_w)
	AM_RANGE(0x2328, 0x2328) AM_WRITE(muxena_w)
	AM_RANGE(0x2329, 0x2329) AM_READWRITE(timerirqclr_r, timerirq_w)
	AM_RANGE(0x232A, 0x232D) AM_WRITE(unknown_w)
	AM_RANGE(0x232E, 0x232E) AM_READ(irqstatus_r)
	AM_RANGE(0x232F, 0x232F) AM_WRITE(coininhib_w)
	AM_RANGE(0x2330, 0x2330) AM_WRITE(payout_latch_w)
	AM_RANGE(0x2331, 0x2331) AM_WRITE(payout_triac_w)
	AM_RANGE(0x2332, 0x2332) AM_WRITE(watchdog_w)
	AM_RANGE(0x2333, 0x2333) AM_WRITE(mmtr_w)
	AM_RANGE(0x2334, 0x2335) AM_WRITE(unknown_w)
	AM_RANGE(0x2336, 0x2336) AM_WRITE(dimcnt_w)
	AM_RANGE(0x2337, 0x2337) AM_WRITE(volume_override_w)
	AM_RANGE(0x2338, 0x2338) AM_WRITE(payout_select_w)
	AM_RANGE(0x2339, 0x2339) AM_WRITE(unknown_w)
	AM_RANGE(0x2400, 0x2400) AM_READWRITE(uart1stat_r, uart1ctrl_w)	/* mc6850 compatible uart */
	AM_RANGE(0x2500, 0x2500) AM_READWRITE(uart1data_r, uart1data_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(uart2stat_r, uart2ctrl_w)	/* mc6850 compatible uart */
	AM_RANGE(0x2700, 0x2700) AM_READWRITE(uart2data_r, uart2data_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(vfd1_data_w)					/* vfd1 data */
	AM_RANGE(0x2900, 0x2900) AM_WRITE(vfd_reset_w)					/* vfd1+vfd2 reset line */

	AM_RANGE(0x2A00, 0x2AFF) AM_DEVWRITE("upd", nec_latch_w)
	AM_RANGE(0x2B00, 0x2BFF) AM_DEVWRITE("upd", nec_reset_w)
	AM_RANGE(0x2C00, 0x2C00) AM_WRITE(unlock_w)						/* custom chip unlock */
	AM_RANGE(0x2D00, 0x2D01) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x2E00, 0x2E00) AM_WRITE(bankswitch_w)					/* write bank (rom page select for 0x6000 - 0x7fff ) */
	AM_RANGE(0x2F00, 0x2F00) AM_WRITE(vfd2_data_w)					/* vfd2 data */

	AM_RANGE(0x3FFF, 0x3FFF) AM_READ( coin_input_r)
	AM_RANGE(0x4000, 0x5FFF) AM_ROM									/* 8k  fixed ROM */
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")							/* 8k  paged ROM (4 pages) */
	AM_RANGE(0x8000, 0xFFFF) AM_ROM									/* 32k ROM */
ADDRESS_MAP_END


/* memory map for scorpion3 board */
static ADDRESS_MAP_START( sc3_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k
	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_r)
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITE(reel34_w)
	AM_RANGE(0x2200, 0x22FF) AM_WRITE(reel56_w)

	AM_RANGE(0x2300, 0x230B) AM_READ(mux_input_r)
	AM_RANGE(0x2300, 0x231F) AM_WRITE(mux_output_w)
	AM_RANGE(0x2320, 0x2323) AM_WRITE(dimas_w)

	AM_RANGE(0x2324, 0x2324) AM_READWRITE(expansion_latch_r, expansion_latch_w)
	AM_RANGE(0x2325, 0x2327) AM_WRITE(unknown_w)
	AM_RANGE(0x2328, 0x2328) AM_WRITE(muxena_w)
	AM_RANGE(0x2329, 0x2329) AM_READWRITE(timerirqclr_r, timerirq_w)
	AM_RANGE(0x232A, 0x232D) AM_WRITE(unknown_w)
	AM_RANGE(0x232E, 0x232E) AM_READ(irqstatus_r)
	AM_RANGE(0x232F, 0x232F) AM_WRITE(coininhib_w)
	AM_RANGE(0x2330, 0x2330) AM_WRITE(payout_latch_w)
	AM_RANGE(0x2331, 0x2331) AM_WRITE(payout_triac_w)
	AM_RANGE(0x2332, 0x2332) AM_WRITE(watchdog_w)
	AM_RANGE(0x2333, 0x2333) AM_WRITE(mmtr_w)
	AM_RANGE(0x2334, 0x2335) AM_WRITE(unknown_w)
	AM_RANGE(0x2336, 0x2336) AM_WRITE(dimcnt_w)
	AM_RANGE(0x2337, 0x2337) AM_WRITE(volume_override_w)
	AM_RANGE(0x2338, 0x2338) AM_WRITE(payout_select_w)
	AM_RANGE(0x2339, 0x2339) AM_WRITE(unknown_w)
	AM_RANGE(0x2400, 0x2400) AM_READWRITE(uart1stat_r, uart1ctrl_w)
	AM_RANGE(0x2500, 0x2500) AM_READWRITE(uart1data_r, uart1data_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(uart2stat_r, uart2ctrl_w)
	AM_RANGE(0x2700, 0x2700) AM_READWRITE(uart2data_r, uart2data_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(vfd1_data_w)
	AM_RANGE(0x2900, 0x2900) AM_WRITE(vfd_reset_w)

	AM_RANGE(0x2A00, 0x2AFF) AM_DEVWRITE("upd", nec_latch_w)
	AM_RANGE(0x2B00, 0x2BFF) AM_DEVWRITE("upd", nec_reset_w)
	AM_RANGE(0x2C00, 0x2C00) AM_WRITE(unlock_w)
	AM_RANGE(0x2D00, 0x2D01) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x2E00, 0x2E00) AM_WRITE(bankswitch_w)
	AM_RANGE(0x2F00, 0x2F00) AM_WRITE(vfd2_data_w)
	AM_RANGE(0x3FFF, 0x3FFF) AM_READ( coin_input_r)
	AM_RANGE(0x4000, 0x5FFF) AM_ROM
//  AM_RANGE(0x4000, 0xFFFF) AM_WRITE(unknown_w)
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xFFFF) AM_ROM
ADDRESS_MAP_END


/* memory map for scorpion2 board + dm01 dot matrix board */
static ADDRESS_MAP_START( memmap_sc2_dm01, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram") //8k
	AM_RANGE(0x2000, 0x2000) AM_READ(vfd_status_dm01_r)
	AM_RANGE(0x2000, 0x20FF) AM_WRITE(reel12_w)
	AM_RANGE(0x2100, 0x21FF) AM_WRITE(reel34_w)
	AM_RANGE(0x2200, 0x22FF) AM_WRITE(reel56_w)

	AM_RANGE(0x2300, 0x230B) AM_READ(mux_input_r)
	AM_RANGE(0x2300, 0x231F) AM_WRITE(mux_output_w)
	AM_RANGE(0x2320, 0x2323) AM_WRITE(dimas_w)

	AM_RANGE(0x2324, 0x2324) AM_READWRITE(expansion_latch_r, expansion_latch_w)
	AM_RANGE(0x2325, 0x2327) AM_WRITE(unknown_w)
	AM_RANGE(0x2328, 0x2328) AM_WRITE(muxena_w)
	AM_RANGE(0x2329, 0x2329) AM_READWRITE(timerirqclr_r, timerirq_w)
	AM_RANGE(0x232A, 0x232D) AM_WRITE(unknown_w)
	AM_RANGE(0x232E, 0x232E) AM_READ(irqstatus_r)
	AM_RANGE(0x232F, 0x232F) AM_WRITE(coininhib_w)
	AM_RANGE(0x2330, 0x2330) AM_WRITE(payout_latch_w)
	AM_RANGE(0x2331, 0x2331) AM_WRITE(payout_triac_w)
	AM_RANGE(0x2332, 0x2332) AM_WRITE(watchdog_w)
	AM_RANGE(0x2333, 0x2333) AM_WRITE(mmtr_w)
	AM_RANGE(0x2334, 0x2335) AM_WRITE(unknown_w)
	AM_RANGE(0x2336, 0x2336) AM_WRITE(dimcnt_w)
	AM_RANGE(0x2337, 0x2337) AM_WRITE(volume_override_w)
	AM_RANGE(0x2338, 0x2338) AM_WRITE(payout_select_w)
	AM_RANGE(0x2339, 0x2339) AM_WRITE(unknown_w)
	AM_RANGE(0x2400, 0x2400) AM_READWRITE(uart1stat_r, uart1ctrl_w)
	AM_RANGE(0x2500, 0x2500) AM_READWRITE(uart1data_r, uart1data_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(uart2stat_r, uart2ctrl_w)
	AM_RANGE(0x2700, 0x2700) AM_READWRITE(uart2data_r, uart2data_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(vfd1_data_dm01_w)
	AM_RANGE(0x2900, 0x2900) AM_WRITE(vfd_reset_w)

	AM_RANGE(0x2A00, 0x2AFF) AM_DEVWRITE("upd", nec_latch_w)
	AM_RANGE(0x2B00, 0x2BFF) AM_DEVWRITE("upd", nec_reset_w)
	AM_RANGE(0x2C00, 0x2C00) AM_WRITE(unlock_w)
	AM_RANGE(0x2D00, 0x2D01) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x2E00, 0x2E00) AM_WRITE(bankswitch_w)
	AM_RANGE(0x2F00, 0x2F00) AM_WRITE(vfd2_data_w)
	AM_RANGE(0x3FFE, 0x3FFE) AM_READ( direct_input_r)
	AM_RANGE(0x3FFF, 0x3FFF) AM_READ( coin_input_r)
	AM_RANGE(0x4000, 0x5FFF) AM_ROM
//  AM_RANGE(0x4000, 0xFFFF) AM_WRITE(unknown_w)
	AM_RANGE(0x6000, 0x7FFF) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xFFFF) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( scorpion2 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("I10")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("I11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("I12")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("I13")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("I14")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("I20")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("I21")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("I22")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("I23")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("I24")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED  )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I30")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I31")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I32")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I33")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )	PORT_NAME("I34")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)	PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)	PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("I43")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("I44")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I50")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I51")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I52")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I53")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I54")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I60")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I61")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I62")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I63")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I64")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I70")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I71")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I72")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I73")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I74")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I80")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I81")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I82")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I83")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("I84")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bbrkfst )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Take Big Breakfast")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Take Feature")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED  )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)	PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)	PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )		PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( drwho )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("GBP 1.00")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK)PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK)PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )  PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for 1 Pound*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for 20p*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for Token Front*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status Low switch for Token Rear*/
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")/*Certain combinations give different percentages*/
	PORT_DIPSETTING(    0x00, "No key") /*Some day, I'll work all these values out.*/
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for 1 Pound*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for 20p*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for Token Front*/
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )/*Tube status High switch for Token Rear*/
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END


static INPUT_PORTS_START( cpeno1 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Stop/Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract Hi/Lo reel" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceptor type" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin play" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, "Multi coin" )
	PORT_DIPSETTING(    0x01, "Single coin" )
	PORT_DIPNAME( 0x02, 0x00, "CashPot Freq" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High) )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Jam Alarm" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Percentage setting" ) PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x0C, "72%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x10, "81%" )
	PORT_DIPSETTING(    0x18, "85%" )
INPUT_PORTS_END


static INPUT_PORTS_START( luvjub )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Take win")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME( "Yes!" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME( "No!" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9)   PORT_NAME("Answer the phone")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bfmcgslm )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN6 ) PORT_IMPULSE(3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN7 ) PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN8 ) PORT_IMPULSE(3)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER)     PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL03" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL04" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "DIL06" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL07" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL10" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL14" ) PORT_DIPLOCATION("DIL:14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "DIL15" ) PORT_DIPLOCATION("DIL:15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL16" ) PORT_DIPLOCATION("DIL:16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
INPUT_PORTS_END


static INPUT_PORTS_START( scorpion3 )
	PORT_START("COINS")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("Fl 5.00")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("Fl 2.50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("Fl 1.00")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("Fl 0.25")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hold 1?")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Stop/Exchange")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Front Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  )   PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE6")
	PORT_DIPNAME( 0x07, 0x07, "PERCENTAGE KEY" ) PORT_DIPLOCATION("STROBE6:01,02,03")
	PORT_DIPSETTING(    0x00, "No key" )
	PORT_DIPSETTING(    0x01, "Key 1" )
	PORT_DIPSETTING(    0x02, "Key 2" )
	PORT_DIPSETTING(    0x03, "Key 3" )
	PORT_DIPSETTING(    0x04, "Key 4" )
	PORT_DIPSETTING(    0x05, "Key 5" )
	PORT_DIPSETTING(    0x06, "Key 6" )
	PORT_DIPSETTING(    0x07, "Key 7" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE9")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_DIPNAME( 0x02, 0x00, "DIL02" ) PORT_DIPLOCATION("DIL:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Attract Hi/Lo reel" ) PORT_DIPLOCATION("DIL:03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Acceptor type" ) PORT_DIPLOCATION("DIL:04")
	PORT_DIPSETTING(    0x00, "Mars" )
	PORT_DIPSETTING(    0x08, "Sentinel" )
	PORT_DIPNAME( 0x10, 0x00, "DIL05" ) PORT_DIPLOCATION("DIL:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE10")
	PORT_DIPNAME( 0x01, 0x00, "Coin play" ) PORT_DIPLOCATION("DIL:06")
	PORT_DIPSETTING(    0x00, "Multi coin" )
	PORT_DIPSETTING(    0x01, "Single coin" )
	PORT_DIPNAME( 0x02, 0x00, "CashPot Freq" ) PORT_DIPLOCATION("DIL:07")
	PORT_DIPSETTING(    0x00, DEF_STR( High) )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL08" ) PORT_DIPLOCATION("DIL:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Coin Jam Alarm" ) PORT_DIPLOCATION("DIL:10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL11" ) PORT_DIPLOCATION("DIL:11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )

	PORT_START("STROBE11")
	PORT_DIPNAME( 0x01, 0x00, "DIL12" ) PORT_DIPLOCATION("DIL:12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL13" ) PORT_DIPLOCATION("DIL:13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x1C, 0x00, "Percentage setting" ) PORT_DIPLOCATION("DIL:14,15,16")
	PORT_DIPSETTING(    0x0C, "72%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPSETTING(    0x10, "81%" )
	PORT_DIPSETTING(    0x18, "85%" )

INPUT_PORTS_END


/* machine driver for scorpion2 board */

static MACHINE_CONFIG_START( scorpion2, bfm_sc2_state )
	MCFG_MACHINE_RESET(awp_init)
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(sc2_memmap)
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000 )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ymsnd",YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_HANDLER(bfm_sc2)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_awpvid14)
MACHINE_CONFIG_END


/* machine driver for scorpion3 board */
static MACHINE_CONFIG_DERIVED( scorpion3, scorpion2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sc3_memmap)
MACHINE_CONFIG_END


/* machine driver for scorpion2 board + matrix board */
static MACHINE_CONFIG_START( scorpion2_dm01, bfm_sc2_state )
	MCFG_MACHINE_RESET(dm01_init)
	MCFG_QUANTUM_TIME(attotime::from_hz(960))									// needed for serial communication !!
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4 )
	MCFG_CPU_PROGRAM_MAP(memmap_sc2_dm01)
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000 )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd",YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_NVRAM_HANDLER(bfm_sc2)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_awpdmd)
	MCFG_CPU_ADD("matrix", M6809, 2000000 )				/* matrix board 6809 CPU at 2 Mhz ?? I don't know the exact freq.*/
	MCFG_CPU_PROGRAM_MAP(bfm_dm01_memmap)
	MCFG_CPU_PERIODIC_INT(bfm_dm01_vbl, 1500 )			/* generate 1500 NMI's per second ?? what is the exact freq?? */
MACHINE_CONFIG_END

static void sc2awp_common_init(running_machine *machine,int reels, int decrypt)
{
	int n;
	sc2_common_init(machine, decrypt);
	/* setup n default 96 half step reels */
	for ( n = 0; n < reels; n++ )
	{
		stepper_config(machine, n, &starpoint_interface_48step);
	}
	if (reels)
	{
		awp_reel_setup();
	}
}

static DRIVER_INIT (bbrkfst)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,5, 1);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);

	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,4,0, 1);	  /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,1, 1);	  /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,2, 1);	  /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,3, 1);	  /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,4, 1);
	Scorpion2_SetSwitchState(machine,6,0, 0);
	Scorpion2_SetSwitchState(machine,6,1, 1);
	Scorpion2_SetSwitchState(machine,6,2, 0);
	Scorpion2_SetSwitchState(machine,6,3, 1);

}

static DRIVER_INIT (drwho)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,4, 1);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);

	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,4,0, 0);	  /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,1, 0);	  /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,2, 0);	  /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,3, 0);	  /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(machine,7,0, 0);	  /* GBP1 High Level Switch */
	Scorpion2_SetSwitchState(machine,7,1, 0);	  /* 20P High Level Switch */
	Scorpion2_SetSwitchState(machine,7,2, 0);	  /* Token Front High Level Switch */
	Scorpion2_SetSwitchState(machine,7,3, 0);	  /* Token Rear High Level Switch */
}

static DRIVER_INIT (drwhon)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,4, 0);
	MechMtr_config(machine,8);

	BFM_BD1_init(0);
	BFM_BD1_init(1);

	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,4,0, 0);	  /* GBP1 Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,1, 0);	  /* 20p Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,2, 0);	  /* Token Front Low Level Switch */
	Scorpion2_SetSwitchState(machine,4,3, 0);	  /* Token Rear  Low Level Switch */
	Scorpion2_SetSwitchState(machine,7,0, 0);	  /* GBP1 High Level Switch */
	Scorpion2_SetSwitchState(machine,7,1, 0);	  /* 20P High Level Switch */
	Scorpion2_SetSwitchState(machine,7,2, 0);	  /* Token Front High Level Switch */
	Scorpion2_SetSwitchState(machine,7,3, 0);	  /* Token Rear High Level Switch */
}


static DRIVER_INIT (focus)
{
	sc2awp_common_init(machine,6, 1);
	MechMtr_config(machine,5);

	BFM_BD1_init(0);
}

static DRIVER_INIT (cpeno1)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,6, 1);

	MechMtr_config(machine,5);

	Scorpion2_SetSwitchState(machine,3,3,1);	/*  5p play */
	Scorpion2_SetSwitchState(machine,3,4,1);	/* 20p play */

	Scorpion2_SetSwitchState(machine,4,0,1);	/* pay tube low (1 pound front) */
	Scorpion2_SetSwitchState(machine,4,1,1);	/* pay tube low (20p) */
	Scorpion2_SetSwitchState(machine,4,2,1);	/* pay tube low (?1 right) */
	Scorpion2_SetSwitchState(machine,4,3,1);	/* pay tube low (?1 left) */

	Scorpion2_SetSwitchState(machine,5,0,1);	/* pay sensor (GBP1 front) */
	Scorpion2_SetSwitchState(machine,5,1,1);	/* pay sensor (20 p) */
	Scorpion2_SetSwitchState(machine,5,2,1);	/* pay sensor (1 right) */
	Scorpion2_SetSwitchState(machine,5,3,1);	/* pay sensor (?1 left) */
	Scorpion2_SetSwitchState(machine,5,4,1);	/* payout unit present */

	state->slide_pay_sensor[0] = 0x50;
	state->slide_pay_sensor[1] = 0x51;
	state->slide_pay_sensor[2] = 0x52;
	state->slide_pay_sensor[3] = 0x53;
	state->slide_pay_sensor[4] = 0;
	state->slide_pay_sensor[5] = 0;

	Scorpion2_SetSwitchState(machine,6,0,1);	/* ? percentage key */
	Scorpion2_SetSwitchState(machine,6,1,1);
	Scorpion2_SetSwitchState(machine,6,2,1);
	Scorpion2_SetSwitchState(machine,6,3,1);
	Scorpion2_SetSwitchState(machine,6,4,1);

	Scorpion2_SetSwitchState(machine,7,0,0);	/* GBP1 High Level Switch  */
	Scorpion2_SetSwitchState(machine,7,1,0);	/* 20P High Level Switch */
	Scorpion2_SetSwitchState(machine,7,2,0);	/* Token Front High Level Switch */
	Scorpion2_SetSwitchState(machine,7,3,0);	/* Token Rear High Level Switch */

	state->sc2_show_door   = 1;
	state->sc2_door_state  = 0x31;

	state->has_hopper = 0;
}

static DRIVER_INIT (bfmcgslm)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,6, 1);
	MechMtr_config(machine,8);
	BFM_BD1_init(0);
	state->has_hopper = 0;
}

static DRIVER_INIT (luvjub)
{
	bfm_sc2_state *state = machine->driver_data<bfm_sc2_state>();
	sc2awp_common_init(machine,6, 1);
	MechMtr_config(machine,8);
	state->has_hopper = 0;

	Scorpion2_SetSwitchState(machine,3,0,1);
	Scorpion2_SetSwitchState(machine,3,1,1);

	Scorpion2_SetSwitchState(machine,4,0,1);
	Scorpion2_SetSwitchState(machine,4,1,1);
	Scorpion2_SetSwitchState(machine,4,2,1);
	Scorpion2_SetSwitchState(machine,4,3,1);

	Scorpion2_SetSwitchState(machine,6,0,1);
	Scorpion2_SetSwitchState(machine,6,1,1);
	Scorpion2_SetSwitchState(machine,6,2,1);
	Scorpion2_SetSwitchState(machine,6,3,0);

	Scorpion2_SetSwitchState(machine,7,0,0);
	Scorpion2_SetSwitchState(machine,7,1,0);
	Scorpion2_SetSwitchState(machine,7,2,0);
	Scorpion2_SetSwitchState(machine,7,3,0);
}

/*********************************************
The Big Breakfast
*********************************************/

ROM_START( m_brkfst )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ar_var_a.bin",	0x00000, 0x10000, CRC(5f016daa) SHA1(25ee10138bddf453588e3c458268533a88a51217) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( m_brkfs1 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ss_var_a.bin",	0x00000, 0x10000, CRC(08d1fa7d) SHA1(a3dba79eef32835f0b46dbd7b376b797324df904) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( m_brkfs2 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_var_jp-8_a.bin",	0x00000, 0x10000, CRC(2671af1b) SHA1(0a34dd2953a99be9fb2a128f9d1f7ddc0fc8242a) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( m_brkfs3 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_8pnd20p_a.bin",	0x00000, 0x10000, CRC(054c38ad) SHA1(f4ab55f977848e3d2a933bba1ab619ffa3e14db6) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( m_brkfs4 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_var_10pnd-20p_a.bin",	0x00000, 0x10000, CRC(d879feaa) SHA1(2656fbe018fe40194c2b77d289b77fabbc9e537c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

ROM_START( m_brkfs5 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("big-breakfast_std_ac_10pnd-20p_a.bin",	0x00000, 0x10000, CRC(55d7321c) SHA1(0b4a6b66aa64fbb3238539a2167f761d0910b814) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("bigbreakfastsnd.bin", 0x00000, 0x80000, CRC(bf91aa2b) SHA1(40942165e65ff9b027015d500e5a9726c44ba1c5))
ROM_END

/*********************************************
Dr.Who The Timelord
*********************************************/

ROM_START( m_bdrwho )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750288.bin",	0x00000, 0x10000, CRC(fe95b5a5) SHA1(876a812f69903fd99f896b35eeaf132c215b0035) ) // dr-who-time-lord_std_ss_20p_ass.bin

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh1 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750661.p1",	0x00000, 0x10000, CRC(4b5b50eb) SHA1(fe2b820c214b3e967348b99ccff30a4bfe0251dc) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh2 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_dat_ac_ass.bin",	0x00000, 0x10000, CRC(5a467a44) SHA1(d5a3dcdf50e07e36187350072b5d82d620f8f1d8) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh3 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_dat_ss_20p_ass.bin",	0x00000, 0x10000, CRC(8ce06af9) SHA1(adb58507b2b6aae59857384748d59485f1739eaf) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh4 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_std_ac_ass.bin",	0x00000, 0x10000, CRC(053313cc) SHA1(2a52b7edae0ce676255eb347bba17a2e48c1707a) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh5 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("dr-who-time-lord_std_var_20p_ass.bin",	0x00000, 0x10000, CRC(35f4e6ab) SHA1(5e5e35889adb7d3384aae663c667b0251d39aeee) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh6 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(e65717c2) SHA1(9b8db0bcac9fd996de29527440d6af3592102120) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh7 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ac_10pnd-20p-25p_ass.bin",	0x00000, 0x10000, CRC(9a27ac6d) SHA1(d1b0e85d41198c5d2cd1b492e53359a5dc1ac474) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh8 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ac_8pnd-20p_ass.bin",	0x00000, 0x10000, CRC(b6629b5e) SHA1(d20085b4ab9a0786063eb063f7d1df2a6814f40c) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrwh9 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ar_10p_ass.bin",	0x00000, 0x10000, CRC(04653c3b) SHA1(0c23f939103772fac628342074de820ec6b472ce) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw10 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_ar_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(40aaa98f) SHA1(80705e24e419558d8a7b1f886bfc2b3ce5465446) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw11 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_dat_var_no-jp-spin_ass.bin",	0x00000, 0x10000, CRC(bf087547) SHA1(f4b7289a76e814af5fb3affc360a9ac659c09bbe) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw12 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(278f559e) SHA1(d4396df02a5e24b3684c26fcaa57c8e499789332) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw13 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ac_8pnd-20p_ass.bin",	0x00000, 0x10000, CRC(0b2850c8) SHA1(5fac64f35a6b6158d8c15f41e82574768b1c3617) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw14 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ar_10p_ass.bin",	0x00000, 0x10000, CRC(f716a21d) SHA1(340df4cdea3309bfebeba7c419057f1bf5ed5024) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw15 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("time-lord_std_ar_20p_uk94_ass.bin",	0x00000, 0x10000, CRC(8dd0f908) SHA1(2eca748874cc061f9a8145b081d2c097a40e1e47) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END

ROM_START( m_bdrw16 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("tmld5pa",	0x00000, 0x10000, CRC(b9ddfd0d) SHA1(915afd83eab330a0e70635c35f031f2041b9f5ad) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END


/* not encrypted, bootleg? */
ROM_START( m_bdrw17 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("drwho.bin",	0x00000, 0x10000, CRC(9e53a1f7) SHA1(60c6aa226c96678a6e487fbf0f32554fd85ebd66) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("timelordsnd.bin", 0x00000, 0x80000, CRC(57fdaf3a) SHA1(f7cbaddb7f2ab8e1c7b17f187bab263e0dde463b))
ROM_END



/*********************************************
Focus
*********************************************/

ROM_START( m_bfocus )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("focus.bin",	 0x00000, 0x10000, CRC(ddd1a21e) SHA1(cbb467b03642d6de37f6dc204b902f2d7e92230e))

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD("focsound.bin", 0x00000, 0x20000, CRC(fce86700) SHA1(546680dd85234608c1b7e850bad3165400fd981c))
ROM_END

/*********************************************
Club Grandslam
*********************************************/

ROM_START( m_bcgslm )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750843.bin", 0x00000, 0x10000, CRC(e159ddf6) SHA1(c897564a956becbd9d4c155df33b239e899156c0))

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD("gslamsnd.bin", 0x00000, 0x40000, CRC(9afb8b42) SHA1(20e108c0041412fcd7b2969701f47a4a99d3677c))
ROM_END

/*********************************************
Luvvly Jubbly
*********************************************/

ROM_START( m_luvjub )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("95750808.bin", 0x00000, 0x10000, CRC(e6668fc7) SHA1(71dd412114c6386cba72e2b29ea07f2d99d14065))

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("mtx_ass.bin",	 0x00000, 0x10000, CRC(cfdd7bb2) SHA1(90086aaff743a7b2385488af1e8a126029113028))

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("snd.bin",		 0x00000, 0x80000, CRC(19efac32) SHA1(26f901fc11f052a4d3cff67f8f61dcdd04f3dc22))
ROM_END

/*********************************************
Club Public Enemy No.1
*********************************************/

ROM_START( m_cpeno1 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD("ce1std25p.bin", 0x00000, 0x10000, CRC(2fad9a49) SHA1(5ffb53031eef8778363836143c4e8d2a65361d51))

	ROM_REGION( 0x20000, "matrix", 0 )
	ROM_LOAD("cpe1_mtx.bin",  0x00000, 0x10000, CRC(5fd1fd7c) SHA1(7645f8c011be77ac48f4eb2c75c92cc4245fdad4))

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD("cpe1_snd.bin",  0x00000, 0x80000, CRC(ca8a56bb) SHA1(36434dae4369f004fa5b4dd00eb6b1a965be60f9))
ROM_END

GAMEL( 1994, m_bdrwho, 0,         scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 1, UK, Game Card 95-750-288)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh1, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 2, UK, Game Card 95-750-661)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh2, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh3, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 4)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh4, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 5)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh5, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 6)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh6, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 7)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh7, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 8)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh8, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 9)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrwh9, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 10)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw10, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 11)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw11, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 12)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw12, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 13)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw13, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 14)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw14, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 15)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw15, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 16)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw16, m_bdrwho,  scorpion2,	 drwho,		drwho,		0,		 "BFM",      "Dr.Who The Timelord (set 17)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)
GAMEL( 1994, m_bdrw17, m_bdrwho,  scorpion2,	 drwho,		drwhon,		0,		 "BFM",      "Dr.Who The Timelord (set 18, not encrypted)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL,layout_drwho)

GAME( 1994, m_brkfst, 0,          scorpion2,	 bbrkfst,	bbrkfst,	0,		 "BFM",      "The Big Breakfast (Set 1 UK, Game Card 95-750-524)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 1994, m_brkfs1, m_brkfst,	  scorpion2,	 bbrkfst,	bbrkfst,	0,		 "BFM",      "The Big Breakfast (Set 2)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 1994, m_brkfs2, m_brkfst,	  scorpion2,	 bbrkfst,	bbrkfst,	0,		 "BFM",      "The Big Breakfast (Set 3)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 1994, m_brkfs3, m_brkfst,	  scorpion2,	 bbrkfst,	bbrkfst,	0,		 "BFM",      "The Big Breakfast (Set 4)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 1994, m_brkfs4, m_brkfst,	  scorpion2,	 bbrkfst,	bbrkfst,	0,		 "BFM",      "The Big Breakfast (Set 5)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)
GAME( 1994, m_brkfs5, m_brkfst,	  scorpion2,	 bbrkfst,	bbrkfst,	0,		 "BFM",      "The Big Breakfast (Set 6)", GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_NOT_WORKING|GAME_MECHANICAL)

GAME( 1995, m_bfocus, 0,          scorpion3,	 scorpion3,	focus,		0,		 "BFM/ELAM", "Focus (Dutch, Game Card 95-750-347)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 1996, m_bcgslm, 0,          scorpion2,	 bfmcgslm,	bfmcgslm,	0,		 "BFM",      "Club Grandslam (UK, Game Card 95-750-843)", GAME_NOT_WORKING|GAME_SUPPORTS_SAVE|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 1996, m_luvjub, 0,          scorpion2_dm01,luvjub,	luvjub,		0,		 "BFM",      "Luvvly Jubbly (UK Multisite 10/25p, Game Card 95-750-808)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
GAME( 1996, m_cpeno1, 0,          scorpion2_dm01,cpeno1,	cpeno1,		0,		 "BFM",      "Club Public Enemy No.1 (UK, Game Card 95-750-846)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL)
