// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner, Andrew Zaferakis

#ifndef MAME_VIDEO_HYPERNEOGEO64_SPRITES_H
#define MAME_VIDEO_HYPERNEOGEO64_SPRITES_H

#pragma once

DECLARE_DEVICE_TYPE(HYPERNEOGEO64_SPRITES, hyperneogeo64_sprites)

#include "screen.h"

class hyperneogeo64_sprites : public device_t
{
public:
	// construction/destruction
	hyperneogeo64_sprites(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t* spriteram, uint32_t* spriteregs, gfxdecode_device* gfxdecode);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector< std::pair <int, uint32_t *> > m_spritelist;

};

#endif // MAME_VIDEO_HYPERNEOGEO64_SPRITES_H
