// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Known unemulated graphical effects and issues:
    - All: Sprite sizing is still imperfect.
    - All: Sprites need to be converted to use scanline rendering for proper clipping.
    - All: Improperly-emulated 1bpp ROZ mode, used by the Super A'Can BIOS logo.
    - All: Unimplemented ROZ scaling tables, used by the Super A'Can BIOS logo and Speedy Dragon intro, among others.
    - All: Priorities are largely unknown.
    - C.U.G.: Gameplay backgrounds are broken.
    - Sango Fighter: Possible missing masking on the upper edges of the screen during gameplay.
    - Sango Fighter: Raster effects off by 1 line
    - Sango Fighter: Specifies tiles out of range of video ram??
    - Speedy Dragon: Backgrounds are broken (wrong tile bank/region).
    - Super Taiwanese Baseball League: Does not boot, uses an unemulated DMA type
    - Super Taiwanese Baseball League: Missing window effect applied on tilemaps?
    - The Son of Evil: Many graphical issues.
    - Visible area, looks like it should be 224 pixels high at most, most games need 8 off the top and 8 off the bottom (or a global scroll)
      Sango looks like it needs 16 off the bottom instead
      Visible area is almost certainly 224 as Son of Evil has an explicit check in the vblank handler
*/

#include "emu.h"
#include "video/supracan_um6618.h"

DEFINE_DEVICE_TYPE(SUPRACAN_UM6618_VIDEO, supracan_um6618_video_device, "umc6618", "UM6618 Video Controller")


supracan_um6618_video_device::supracan_um6618_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SUPRACAN_UM6618_VIDEO, tag, owner, clock)
	, m_gfxdecode(*this, "gfxdecode")
	, m_screen(*this, "screen")
{
}

void supracan_um6618_video_device::device_start()
{
}

void supracan_um6618_video_device::device_reset()
{
}

void supracan_um6618_video_device::palette_init(palette_device &palette) const
{
	// Used for debugging purposes for now
	for (int i = 0; i < 32768; i++)
	{
		int const r = (i & 0x1f) << 3;
		int const g = ((i >> 5) & 0x1f) << 3;
		int const b = ((i >> 10) & 0x1f) << 3;
		palette.set_pen_color(i, r, g, b);
	}
}

uint32_t supracan_um6618_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* gfxdecode is retained for reference purposes but not otherwise used by the driver */
static const gfx_layout supracan_gfx8bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ STEP8(0, 8*8) },
	8*8*8
};



static const gfx_layout supracan_gfx4bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout supracan_gfx2bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	2,
	{ 0, 1 },
	{ 0*2, 1*2, 2*2, 3*2, 4*2, 5*2, 6*2, 7*2 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const uint32_t xtexlayout_xoffset[64] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
												24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
												45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63 };

static const uint32_t xtexlayout_yoffset[64] = {  0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,
												9*64,10*64,11*64,12*64,13*64,14*64,15*64,
												16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
												24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64,
												32*64,33*64,34*64,35*64,36*64,37*64,38*64,39*64,
												40*64,41*64,42*64,43*64,44*64,45*64,46*64,47*64,
												48*64,49*64,50*64,51*64,52*64,53*64,54*64,55*64,
												56*64,57*64,58*64,59*64,60*64,61*64,62*64,63*64 };
static const gfx_layout supracan_gfx1bpp =
{
	64, 64,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	64*64,
	xtexlayout_xoffset,
	xtexlayout_yoffset
};

static const gfx_layout supracan_gfx1bpp_alt =
{
	8, 8,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( gfx_supracan )
	GFXDECODE_RAM( "vram", 0, supracan_gfx8bpp, 0, 1 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx4bpp, 0, 0x10 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx2bpp, 0, 0x40 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx1bpp, 0, 0x80 )
	GFXDECODE_RAM( "vram", 0, supracan_gfx1bpp_alt, 0, 0x80 )
GFXDECODE_END


void supracan_um6618_video_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(10'738'635)/2, 348, 0, 256, 256, 0, 240);  /* No idea if this is correct */
	m_screen->set_screen_update(FUNC(supracan_um6618_video_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(supracan_um6618_video_device::palette_init)).set_format(palette_device::xBGR_555, 32768);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_supracan);
}
