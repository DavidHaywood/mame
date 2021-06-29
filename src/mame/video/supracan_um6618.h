// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz,David Haywood

#ifndef MAME_VIDEO_SUPRACAN_UM6618_H
#define MAME_VIDEO_SUPRACAN_UM6618_H

#pragma once

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

DECLARE_DEVICE_TYPE(SUPRACAN_UM6618_VIDEO, supracan_um6618_video_device)

class supracan_um6618_video_device : public device_t
{
public:
	supracan_um6618_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	auto set_read_cpu_space() { return read_cpu_space.bind(); }
	auto set_write_cpu_space() { return write_cpu_space.bind(); }
	auto set_vblank_irq() { return vblank_irq.bind(); }
	auto set_line_irq() { return line_irq.bind(); }
	auto set_hblank_irq() { return hblank_irq.bind(); }

	uint16_t video_r(offs_t offset, uint16_t mem_mask = 0);
	void video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void palette_init(palette_device &palette) const;


	struct sprdma_regs_t
	{
		uint32_t src;
		uint16_t src_inc;
		uint32_t dst;
		uint16_t dst_inc;
		uint16_t count;
		uint16_t control;
	};

	sprdma_regs_t m_sprdma_regs;

	emu_timer *m_video_timer;
	emu_timer *m_hbl_timer;
	emu_timer *m_line_on_timer;
	emu_timer *m_line_off_timer;

	std::vector<uint8_t> m_vram_addr_swapped;


	uint16_t m_sprite_count;
	uint32_t m_sprite_base_addr;
	uint8_t m_sprite_flags;

	uint32_t m_tilemap_base_addr[3];
	int m_tilemap_scrollx[3];
	int m_tilemap_scrolly[3];
	uint16_t m_video_flags;
	uint16_t m_tilemap_flags[3];
	uint16_t m_tilemap_mode[3];
	uint16_t m_irq_mask;

	uint32_t m_roz_base_addr;
	uint16_t m_roz_mode;
	uint32_t m_roz_scrollx;
	uint32_t m_roz_scrolly;
	uint16_t m_roz_tile_bank;
	uint32_t m_roz_unk_base0;
	uint32_t m_roz_unk_base1;
	uint32_t m_roz_unk_base2;
	uint16_t m_roz_coeffa;
	uint16_t m_roz_coeffb;
	uint16_t m_roz_coeffc;
	uint16_t m_roz_coeffd;
	int32_t m_roz_changed;
	uint16_t m_unk_1d0;

	uint16_t m_video_regs[256];

	tilemap_t *m_tilemap_sizes[4][4];
	bitmap_ind16 m_sprite_final_bitmap;
	bitmap_ind8 m_sprite_mask_bitmap;
	bitmap_ind8 m_prio_bitmap;

	void write_swapped_byte(int offset, uint8_t byte);
	TILE_GET_INFO_MEMBER(get_tilemap0_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap1_tile_info);
	TILE_GET_INFO_MEMBER(get_tilemap2_tile_info);
	TILE_GET_INFO_MEMBER(get_roz_tile_info);

	TIMER_CALLBACK_MEMBER(hbl_callback);
	TIMER_CALLBACK_MEMBER(line_on_callback);
	TIMER_CALLBACK_MEMBER(line_off_callback);
	TIMER_CALLBACK_MEMBER(video_callback);

	int get_tilemap_region(int layer);
	void get_tilemap_info_common(int layer, tile_data &tileinfo, int count);
	void get_roz_tilemap_info(int layer, tile_data &tileinfo, int count);
	int get_tilemap_dimensions(int &xsize, int &ysize, int layer);
	void draw_sprite_tile(bitmap_ind16 &dst, bitmap_ind8 &priomap, const rectangle &cliprect, gfx_element *gfx, int tile, int palette, bool xflip, bool yflip, int dstx, int dsty, int prio);
	void draw_sprite_tile_mask(bitmap_ind8 &dst, const rectangle &cliprect, gfx_element *gfx, int tile, bool xflip, bool yflip, int dstx, int dsty);
	void draw_sprite_tile_masked(bitmap_ind16 &dst, bitmap_ind8 &mask, bitmap_ind8 &priomap, const rectangle &cliprect, gfx_element *gfx, int tile, int palette, bool xflip, bool yflip, int dstx, int dsty, int prio);
	void draw_sprites(bitmap_ind16 &bitmap, bitmap_ind8 &maskmap, bitmap_ind8 &priomap, const rectangle &cliprect);
	void mark_active_tilemap_all_dirty(int layer);
	void draw_roz_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy, int wraparound/*, int columnscroll, uint32_t* scrollram*/, int transmask);

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_vram;

	devcb_read16 read_cpu_space;
	devcb_write16 write_cpu_space;
	devcb_write_line vblank_irq;
	devcb_write_line line_irq;
	devcb_write_line hblank_irq;
};

#endif // MAME_VIDEO_SUPRACAN_UM6618_H
