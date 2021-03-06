class strnskil_state : public driver_device
{
public:
	strnskil_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_xscroll;
	UINT8 m_scrl_ctrl;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/strnskil.c -----------*/

WRITE8_HANDLER( strnskil_videoram_w );
WRITE8_HANDLER( strnskil_scrl_ctrl_w );

PALETTE_INIT( strnskil );
VIDEO_START( strnskil );
SCREEN_UPDATE( strnskil );
