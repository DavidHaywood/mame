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
{
}

void supracan_um6618_video_device::device_start()
{
}

void supracan_um6618_video_device::device_reset()
{
}
