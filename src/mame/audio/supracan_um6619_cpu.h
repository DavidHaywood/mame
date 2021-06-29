// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz,David Haywood

#ifndef MAME_AUDIO_SUPRACAN_UM6619_H
#define MAME_AUDIO_SUPRACAN_UM6619_H

#pragma once

#include "cpu/m6502/m6502.h"

DECLARE_DEVICE_TYPE(SUPRACAN_UM6619_CPU, supracan_um6619_cpu_device)

class supracan_um6619_cpu_device : public m6502_device
{
public:
	supracan_um6619_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t _68k_soundram_r(offs_t offset, uint16_t mem_mask = ~0);
	void _68k_soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t sound_r(offs_t offset, uint16_t mem_mask);
	void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void dma_channel0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dma_channel1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	auto set_read_cpu_space16() { return read_cpu_space16.bind(); }
	auto set_write_cpu_space16() { return write_cpu_space16.bind(); }

protected:
	supracan_um6619_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t sound_read(offs_t offset) { return 0x00; }
	virtual void sound_write(offs_t offset, uint8_t data) {}

	void set_sound_irq(uint8_t bit, uint8_t state);

	required_shared_ptr<uint8_t> m_soundram;

private:

	struct dma_regs_t
	{
		uint32_t source[2];
		uint32_t dest[2];
		uint16_t count[2];
		uint16_t control[2];
	};

	void supracan_sound_mem(address_map &map);

	void _6502_soundmem_w(offs_t offset, uint8_t data);
	uint8_t _6502_soundmem_r(offs_t offset);


	void dma_w(int offset, uint16_t data, uint16_t mem_mask, int ch);

	uint8_t m_soundcpu_irq_enable;
	uint8_t m_soundcpu_irq_source;
	uint16_t m_sound_cpu_ctrl;
	uint8_t m_sound_cpu_shift_ctrl;
	uint8_t m_sound_cpu_shift_regs[2];
	uint8_t m_sound_status;
	uint8_t m_sound_reg_addr;
	uint16_t m_latched_controls[2];

	dma_regs_t m_dma_regs;

	required_ioport_array<2> m_pads;

	devcb_read16 read_cpu_space16;
	devcb_write16 write_cpu_space16;
};

#endif // MAME_AUDIO_SUPRACAN_UM6619_H
