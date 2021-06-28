// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_AUDIO_SUPRACAN_UM6619_H
#define MAME_AUDIO_SUPRACAN_UM6619_H

#pragma once

#include "audio/acan.h"

#include "cpu/m6502/m6502.h"

#include "speaker.h"


DECLARE_DEVICE_TYPE(SUPRACAN_UM6619_AUDIOSOC, supracan_um6619_audiosoc_device)

class supracan_um6619_audiosoc_device : public device_t
{
public:
	supracan_um6619_audiosoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t _68k_soundram_r(offs_t offset, uint16_t mem_mask = ~0);
	void _68k_soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_w(int offset, uint16_t data, uint16_t mem_mask, int ch);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:

	struct dma_regs_t
	{
		uint32_t source[2];
		uint32_t dest[2];
		uint16_t count[2];
		uint16_t control[2];
	};

	void supracan_sound_mem(address_map &map);

	uint8_t sound_ram_read(offs_t offset);
	void sound_timer_irq(int state);
	void sound_dma_irq(int state);

	uint16_t sound_r(offs_t offset, uint16_t mem_mask);
	void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void _6502_soundmem_w(offs_t offset, uint8_t data);
	uint8_t _6502_soundmem_r(offs_t offset);

	void set_sound_irq(uint8_t bit, uint8_t state);

	void dma_channel0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_channel1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	uint8_t m_soundcpu_irq_enable;
	uint8_t m_soundcpu_irq_source;
	uint16_t m_sound_cpu_ctrl;
	uint8_t m_sound_cpu_shift_ctrl;
	uint8_t m_sound_cpu_shift_regs[2];
	uint8_t m_sound_status;
	uint8_t m_sound_reg_addr;
	uint16_t m_latched_controls[2];

	dma_regs_t m_dma_regs;

	required_device<cpu_device> m_soundcpu;
	required_device<acan_sound_device> m_sound;
	required_shared_ptr<uint8_t> m_soundram;
	required_ioport_array<2> m_pads;
};

#endif // MAME_AUDIO_SUPRACAN_UM6619_H
