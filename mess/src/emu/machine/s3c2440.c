/*******************************************************************************

    Samsung S3C2440

    (c) 2010 Tim Schuerewegen

*******************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/s3c2440.h"
#include "sound/dac.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

#define DEVICE_S3C2440
#include "machine/s3c24xx.c"
#undef DEVICE_S3C2440

VIDEO_START( s3c2440 )
{
	device_t *device = machine.device( S3C2440_TAG);
	s3c24xx_video_start( device, machine);
}

SCREEN_UPDATE( s3c2440 )
{
	device_t *device = screen->machine().device( S3C2440_TAG);
	return s3c24xx_video_update( device, screen, bitmap, cliprect);
}

DEVICE_START( s3c2440 )
{
	address_space *space = device->machine().device( "maincpu")->memory().space( AS_PROGRAM);
	space->install_legacy_readwrite_handler( *device, 0x48000000, 0x4800003b, FUNC(s3c24xx_memcon_r), FUNC(s3c24xx_memcon_w));
	space->install_legacy_readwrite_handler( *device, 0x49000000, 0x4900005b, FUNC(s3c24xx_usb_host_r), FUNC(s3c24xx_usb_host_w));
	space->install_legacy_readwrite_handler( *device, 0x4a000000, 0x4a00001f, FUNC(s3c24xx_irq_r), FUNC(s3c24xx_irq_w));
	space->install_legacy_readwrite_handler( *device, 0x4b000000, 0x4b000023, FUNC(s3c24xx_dma_0_r), FUNC(s3c24xx_dma_0_w));
	space->install_legacy_readwrite_handler( *device, 0x4b000040, 0x4b000063, FUNC(s3c24xx_dma_1_r), FUNC(s3c24xx_dma_1_w));
	space->install_legacy_readwrite_handler( *device, 0x4b000080, 0x4b0000a3, FUNC(s3c24xx_dma_2_r), FUNC(s3c24xx_dma_2_w));
	space->install_legacy_readwrite_handler( *device, 0x4b0000c0, 0x4b0000e3, FUNC(s3c24xx_dma_3_r), FUNC(s3c24xx_dma_3_w));
	space->install_legacy_readwrite_handler( *device, 0x4c000000, 0x4c00001b, FUNC(s3c24xx_clkpow_r), FUNC(s3c24xx_clkpow_w));
	space->install_legacy_readwrite_handler( *device, 0x4d000000, 0x4d000063, FUNC(s3c2440_lcd_r), FUNC(s3c24xx_lcd_w));
	space->install_legacy_readwrite_handler( *device, 0x4d000400, 0x4d0007ff, FUNC(s3c24xx_lcd_palette_r), FUNC(s3c24xx_lcd_palette_w));
	space->install_legacy_readwrite_handler( *device, 0x4e000000, 0x4e00003f, FUNC(s3c24xx_nand_r), FUNC(s3c24xx_nand_w));
	space->install_legacy_readwrite_handler( *device, 0x4f000000, 0x4f0000a3, FUNC(s3c24xx_cam_r), FUNC(s3c24xx_cam_w));
	space->install_legacy_readwrite_handler( *device, 0x50000000, 0x5000002b, FUNC(s3c24xx_uart_0_r), FUNC(s3c24xx_uart_0_w));
	space->install_legacy_readwrite_handler( *device, 0x50004000, 0x5000402b, FUNC(s3c24xx_uart_1_r), FUNC(s3c24xx_uart_1_w));
	space->install_legacy_readwrite_handler( *device, 0x50008000, 0x5000802b, FUNC(s3c24xx_uart_2_r), FUNC(s3c24xx_uart_2_w));
	space->install_legacy_readwrite_handler( *device, 0x51000000, 0x51000043, FUNC(s3c24xx_pwm_r), FUNC(s3c24xx_pwm_w));
	space->install_legacy_readwrite_handler( *device, 0x52000140, 0x5200026f, FUNC(s3c24xx_usb_device_r), FUNC(s3c24xx_usb_device_w));
	space->install_legacy_readwrite_handler( *device, 0x53000000, 0x5300000b, FUNC(s3c24xx_wdt_r), FUNC(s3c24xx_wdt_w));
	space->install_legacy_readwrite_handler( *device, 0x54000000, 0x54000013, FUNC(s3c24xx_iic_r), FUNC(s3c24xx_iic_w));
	space->install_legacy_readwrite_handler( *device, 0x55000000, 0x55000013, FUNC(s3c24xx_iis_r), FUNC(s3c24xx_iis_w));
	space->install_legacy_readwrite_handler( *device, 0x56000000, 0x560000df, FUNC(s3c24xx_gpio_r), FUNC(s3c24xx_gpio_w));
	space->install_legacy_readwrite_handler( *device, 0x57000040, 0x5700008b, FUNC(s3c24xx_rtc_r), FUNC(s3c24xx_rtc_w));
	space->install_legacy_readwrite_handler( *device, 0x58000000, 0x58000017, FUNC(s3c24xx_adc_r), FUNC(s3c24xx_adc_w));
	space->install_legacy_readwrite_handler( *device, 0x59000000, 0x59000017, FUNC(s3c24xx_spi_0_r), FUNC(s3c24xx_spi_0_w));
	space->install_legacy_readwrite_handler( *device, 0x59000020, 0x59000037, FUNC(s3c24xx_spi_1_r), FUNC(s3c24xx_spi_1_w));
	space->install_legacy_readwrite_handler( *device, 0x5a000000, 0x5a000043, FUNC(s3c24xx_sdi_r), FUNC(s3c24xx_sdi_w));
	space->install_legacy_readwrite_handler( *device, 0x5b000000, 0x5b00001f, FUNC(s3c24xx_ac97_r), FUNC(s3c24xx_ac97_w));
	DEVICE_START_CALL(s3c24xx);
}

DEVICE_GET_INFO( s3c2440 )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START: info->start = DEVICE_START_NAME(s3c2440); break;
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME: strcpy(info->s, "Samsung S3C2440"); break;
		/* --- default --- */
		default: DEVICE_GET_INFO_CALL(s3c24xx); break;
	}
}

void s3c2440_uart_fifo_w( device_t *device, int uart, UINT8 data)
{
	s3c24xx_uart_fifo_w( device, uart, data);
}

void s3c2440_touch_screen( device_t *device, int state)
{
	s3c24xx_touch_screen( device, state);
}

void s3c2440_request_irq( device_t *device, UINT32 int_type)
{
	s3c24xx_request_irq( device, int_type);
}

void s3c2440_request_eint( device_t *device, UINT32 number)
{
	s3c24xx_request_eint( device, number);
}

WRITE_LINE_DEVICE_HANDLER( s3c2440_pin_frnb_w )
{
	s3c24xx_pin_frnb_w( device, state);
}

DEFINE_LEGACY_DEVICE(S3C2440, s3c2440);
