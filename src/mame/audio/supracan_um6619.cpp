// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Sound chip is completely custom, and partially implemented.

    There are 6 interrupt sources on the 6502 side, all of which use the IRQ line.
    The register at 0x411 is bitmapped to indicate what source(s) are active.
    In priority order from most to least important, they are:

    411 value  How acked                     Notes
    0x40       read reg 0x16 of sound chip   used for DMA-driven sample playback. Register 0x16 may contain which DMA-driven samples are active.
    0x04       read at 0x405                 latch 1?  0xcd is magic value
    0x08       read at 0x404                 latch 2?  0xcd is magic value
    0x10       read at 0x409                 unknown, dispatched but not used in startup 6502 code
    0x20       read at 0x40a                 IRQ request from 68k, flags data available in shared-RAM mailbox
    0x80       read reg 0x14 of sound chip   depends on reg 0x14 of sound chip & 0x40: if not set writes 0x8f to reg 0x14,
                                             otherwise writes 0x4f to reg 0x14 and performs additional processing
*/

#include "emu.h"
#include "audio/supracan_um6619.h"

DEFINE_DEVICE_TYPE(SUPRACAN_UM6619_AUDIOSOC, supracan_um6619_audiosoc_device, "umc6619", "UM6619 Audio System on a Chip")


supracan_um6619_audiosoc_device::supracan_um6619_audiosoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SUPRACAN_UM6619_AUDIOSOC, tag, owner, clock)
	, m_soundcpu(*this, "soundcpu")
	, m_sound(*this, "acansnd")
{
}

void supracan_um6619_audiosoc_device::supracan_sound_mem(address_map &map)
{
//	map(0x0000, 0xffff).rw(FUNC(supracan_state::_6502_soundmem_r), FUNC(supracan_state::_6502_soundmem_w)).share("soundram");
}

uint8_t supracan_um6619_audiosoc_device::sound_ram_read(offs_t offset)
{
	return 0x00;
	//return m_soundram[offset];
}

void supracan_um6619_audiosoc_device::sound_timer_irq(int state)
{
	//set_sound_irq(7, state);
}

void supracan_um6619_audiosoc_device::sound_dma_irq(int state)
{
	//set_sound_irq(6, state);
}


void supracan_um6619_audiosoc_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_soundcpu, XTAL(3'579'545));     /* TODO: Verify actual clock */
	m_soundcpu->set_addrmap(AS_PROGRAM, &supracan_um6619_audiosoc_device::supracan_sound_mem);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ACANSND(config, m_sound, XTAL(3'579'545));
	m_sound->ram_read().set(FUNC(supracan_um6619_audiosoc_device::sound_ram_read));
	m_sound->timer_irq_handler().set(FUNC(supracan_um6619_audiosoc_device::sound_timer_irq));
	m_sound->dma_irq_handler().set(FUNC(supracan_um6619_audiosoc_device::sound_dma_irq));
	m_sound->add_route(0, "lspeaker", 1.0);
	m_sound->add_route(1, "rspeaker", 1.0);
}

void supracan_um6619_audiosoc_device::device_start()
{
}

void supracan_um6619_audiosoc_device::device_reset()
{
	m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}
