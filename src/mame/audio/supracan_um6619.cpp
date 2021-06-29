// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz,David Haywood

/*
	The UM6619 integrates:

	6502 CPU
	DMA Controller
	Pad Interface for 2 controllers
	Custom audio hardware (see audiuo/acan.cpp)

	--------------

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

	should this be merged with acan.cpp for the sound, as it's an all-in-one integrated part (currently not done as they use different licenses)
*/

#include "emu.h"
#include "audio/supracan_um6619.h"

#define LOG_UNKNOWNS    (1 << 0)
#define LOG_DMA         (1 << 1)
#define LOG_IRQS        (1 << 9)
#define LOG_SOUND       (1 << 10)
#define LOG_68K_SOUND   (1 << 12)
#define LOG_CONTROLS    (1 << 13)

#define VERBOSE         (LOG_SOUND)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SUPRACAN_UM6619_AUDIOSOC, supracan_um6619_audiosoc_device, "umc6619", "UM6619 Audio System on a Chip")


supracan_um6619_audiosoc_device::supracan_um6619_audiosoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SUPRACAN_UM6619_AUDIOSOC, tag, owner, clock)
	, m_soundcpu(*this, "soundcpu")
	, m_sound(*this, "acansnd")
	, m_soundram(*this, "soundram")
	, m_pads(*this, "P%u", 1U)
	, read_cpu_space(*this)
	, write_cpu_space(*this)
	, read_cpu_space16(*this)
	, write_cpu_space16(*this)
{
}


uint8_t supracan_um6619_audiosoc_device::_6502_soundmem_r(offs_t offset)
{
	uint8_t data = m_soundram[offset];

	switch (offset)
	{
	case 0x300: // Boot OK status
		break;
	case 0x402:
	case 0x403:
	{
		const uint8_t index = offset - 0x402;
		data = m_sound_cpu_shift_regs[index];
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_SOUND, "%s: 6502_soundmem_r: Shift register %d read: %02x\n", machine().describe_context(), index, data);
		}
		break;
	}
	case 0x410:
		data = m_soundcpu_irq_enable;
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_SOUND, "%s: 6502_soundmem_r: IRQ enable read: %02x\n", machine().describe_context(), data);
		}
		break;
	case 0x411:
		data = m_soundcpu_irq_source;
		m_soundcpu_irq_source = 0;
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_r: Sound IRQ source read + clear: %02x\n", machine().describe_context(), machine().time().to_string(), data);
			m_soundcpu->set_input_line(0, CLEAR_LINE);
		}
		break;
	case 0x420:
		if (!machine().side_effects_disabled())
		{
			data = m_sound_status;
			LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_r: Sound hardware status read:       0420 = %02x\n", machine().describe_context(), machine().time().to_string(), m_sound_status);
		}
		break;
	case 0x422:
		if (!machine().side_effects_disabled())
		{
			data = m_sound->read(m_sound_reg_addr);
			LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_r: Sound hardware reg data read:     0422 = %02x\n", machine().describe_context(), machine().time().to_string(), data);
		}
		break;
	case 0x404:
	case 0x405:
	case 0x409:
	case 0x414:
	case 0x416:
		// Intentional fall-through
	default:
		if (offset >= 0x400 && offset < 0x500)
		{
			if (!machine().side_effects_disabled())
			{
				LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: 6502_soundmem_r: Unknown register %04x (%02x)\n", machine().describe_context(), offset, data);
			}
		}
		break;
	}

	return data;
}


void supracan_um6619_audiosoc_device::_68k_soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_soundram[offset * 2 + 1] = data & 0xff;
	m_soundram[offset * 2] = data >> 8;

	if (offset * 2 < 0x500 && offset * 2 >= 0x400)
	{
		if (ACCESSING_BITS_8_15)
		{
			_6502_soundmem_w(offset * 2, data >> 8);
		}
		if (ACCESSING_BITS_0_7)
		{
			_6502_soundmem_w(offset * 2 + 1, data & 0xff);
		}
	}
	LOGMASKED(LOG_68K_SOUND, "%s: 68k sound RAM write: %04x & %04x = %04x\n", machine().describe_context(), offset << 1, mem_mask, (uint16_t)data);
}

uint16_t supracan_um6619_audiosoc_device::_68k_soundram_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = m_soundram[offset * 2] << 8;
	data |= m_soundram[offset * 2 + 1];

	if (offset * 2 >= 0x400 && offset * 2 < 0x500)
	{
		data = 0;
		if (ACCESSING_BITS_8_15)
		{
			data |= _6502_soundmem_r(offset * 2) << 8;
		}
		if (ACCESSING_BITS_0_7)
		{
			data |= _6502_soundmem_r(offset * 2 + 1);
		}
	}
	//LOGMASKED(LOG_68K_SOUND, "%s: 68k sound RAM read: %04x & %04x (%04x)\n", machine().describe_context(), offset << 1, mem_mask, data);

	return data;
}



void supracan_um6619_audiosoc_device::_6502_soundmem_w(offs_t offset, uint8_t data)
{
	static attotime s_curr_time = attotime::zero;
	attotime now = machine().time();

	switch (offset)
	{
	case 0x407:
	{
		LOGMASKED(LOG_CONTROLS, "%s: 6502_soundmem_w: Shift register control: %02x\n", machine().describe_context(), data);
		const uint8_t old = m_sound_cpu_shift_ctrl;
		m_sound_cpu_shift_ctrl = data;
		const uint8_t lowered = old & ~m_sound_cpu_shift_ctrl;
		for (uint8_t pad = 0; pad < 2; pad++)
		{
			if (BIT(lowered, pad + 0))
			{
				m_latched_controls[pad] = m_pads[pad]->read();
			}
			if (BIT(lowered, pad + 2))
			{
				m_sound_cpu_shift_regs[pad] <<= 1;
				m_sound_cpu_shift_regs[pad] |= BIT(m_latched_controls[pad], 15);
				m_latched_controls[pad] <<= 1;
			}
			if (BIT(lowered, pad + 4))
			{
				m_sound_cpu_shift_regs[pad] = 0;
			}
		}
		break;
	}
	case 0x410:
		m_soundcpu_irq_enable = data;
		LOGMASKED(LOG_SOUND | LOG_IRQS, "%s: 6502_soundmem_w: IRQ enable: %02x\n", machine().describe_context(), data);
		break;
	case 0x420:
		LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_w: Sound addr write:                 0420 = %02x\n", machine().describe_context(), now.to_string(), data);
		m_sound_reg_addr = data;
		break;
	case 0x422:
	{
		attotime delta = (s_curr_time == attotime::zero ? attotime::zero : (now - s_curr_time));
		s_curr_time = now;
		LOGMASKED(LOG_SOUND, "%s: %s: 6502_soundmem_w: Sound data write:                 0422 = %02x (delta %0.3f)\n", machine().describe_context(), now.to_string(), data, (float)delta.as_double());
		m_sound->write(m_sound_reg_addr, data);
		break;
	}
	default:
		if (offset >= 0x400 && offset < 0x500)
		{
			LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: 6502_soundmem_w: Unknown register %04x = %02x\n", machine().describe_context(), offset, data);
		}
		m_soundram[offset] = data;
		break;
	}
}

void supracan_um6619_audiosoc_device::supracan_sound_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(supracan_um6619_audiosoc_device::_6502_soundmem_r), FUNC(supracan_um6619_audiosoc_device::_6502_soundmem_w)).share("soundram");
}

uint8_t supracan_um6619_audiosoc_device::sound_ram_read(offs_t offset)
{
	return m_soundram[offset];
}


void supracan_um6619_audiosoc_device::set_sound_irq(uint8_t bit, uint8_t state)
{
	const uint8_t old = m_soundcpu_irq_source;
	if (state)
		m_soundcpu_irq_source |= 1 << bit;
	else
		m_soundcpu_irq_source &= ~(1 << bit);
	const uint8_t changed = old ^ m_soundcpu_irq_source;
	if (changed)
	{
		m_soundcpu->set_input_line(0, (m_soundcpu_irq_enable & m_soundcpu_irq_source) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void supracan_um6619_audiosoc_device::sound_timer_irq(int state)
{
	set_sound_irq(7, state);
}

void supracan_um6619_audiosoc_device::sound_dma_irq(int state)
{
	set_sound_irq(6, state);
}


uint16_t supracan_um6619_audiosoc_device::sound_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0x04/2:
		data = (m_soundram[0x40c] << 8) | m_soundram[0x40d];
		LOGMASKED(LOG_SOUND, "%s: sound_r: DMA Request address from 6502, %08x: %04x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), data, mem_mask);
		break;

	case 0x0c/2:
		data = m_soundram[0x40a];
		LOGMASKED(LOG_SOUND, "%s: sound_r: DMA Request flag from 6502, %08x: %04x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), data, mem_mask);
		machine().debug_break();
		break;

	default:
		LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: sound_r: Unknown register: %08x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), mem_mask);
		break;
	}

	return data;
}

void supracan_um6619_audiosoc_device::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0x000a/2:  /* Sound cpu IRQ request. */
		LOGMASKED(LOG_SOUND, "%s: Sound CPU IRQ request: %04x\n", machine().describe_context(), data);
		set_sound_irq(5, 1);
		//m_soundcpu->set_input_line(0, ASSERT_LINE);
		break;
	case 0x001c/2:  /* Sound cpu control. Bit 0 tied to sound cpu RESET line */
	{
		const uint16_t old = m_sound_cpu_ctrl;
		m_sound_cpu_ctrl = data;
		const uint16_t changed = old ^ m_sound_cpu_ctrl;
		if (BIT(changed, 0))
		{
			if (BIT(m_sound_cpu_ctrl, 0))
			{
				/* Reset and enable the sound cpu */
				m_soundcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_soundcpu->reset();
			}
			else
			{
				/* Halt the sound cpu */
				m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			}
		}
		LOGMASKED(LOG_SOUND, "%s: Sound CPU ctrl write: %04x\n", machine().describe_context(), data);
		break;
	}
	default:
		LOGMASKED(LOG_SOUND | LOG_UNKNOWNS, "%s: sound_w: Unknown register: %08x = %04x & %04x\n", machine().describe_context(), 0xe90000 + (offset << 1), data, mem_mask);
		break;
	}
}


void supracan_um6619_audiosoc_device::dma_w(int offset, uint16_t data, uint16_t mem_mask, int ch)
{
	//address_space& mem = 0;// *nullptr;// m_maincpu->space(AS_PROGRAM);

	switch (offset)
	{
	case 0x00/2: // Source address MSW
		LOGMASKED(LOG_DMA, "dma_w: source msw %d: %04x\n", ch, data);
		m_dma_regs.source[ch] &= 0x0000ffff;
		m_dma_regs.source[ch] |= data << 16;
		break;
	case 0x02/2: // Source address LSW
		LOGMASKED(LOG_DMA, "dma_w: source lsw %d: %04x\n", ch, data);
		m_dma_regs.source[ch] &= 0xffff0000;
		m_dma_regs.source[ch] |= data;
		break;
	case 0x04/2: // Destination address MSW
		LOGMASKED(LOG_DMA, "dma_w: dest msw %d: %04x\n", ch, data);
		m_dma_regs.dest[ch] &= 0x0000ffff;
		m_dma_regs.dest[ch] |= data << 16;
		break;
	case 0x06/2: // Destination address LSW
		LOGMASKED(LOG_DMA, "dma_w: dest lsw %d: %04x\n", ch, data);
		m_dma_regs.dest[ch] &= 0xffff0000;
		m_dma_regs.dest[ch] |= data;
		break;
	case 0x08/2: // Byte count
		LOGMASKED(LOG_DMA, "dma_w: count %d: %04x\n", ch, data);
		m_dma_regs.count[ch] = data;
		break;
	case 0x0a/2: // Control
		LOGMASKED(LOG_DMA, "dma_w: control %d: %04x\n", ch, data);
		if (data & 0x8800)
		{
			//if (data & 0x2000)
			//	m_dma_regs.source-=2;
			LOGMASKED(LOG_DMA, "dma_w: Kicking off a DMA from %08x to %08x, %d bytes (%04x)\n", m_dma_regs.source[ch], m_dma_regs.dest[ch], m_dma_regs.count[ch] + 1, data);

			for (int i = 0; i <= m_dma_regs.count[ch]; i++)
			{
				if (data & 0x1000)
				{
					write_cpu_space16(m_dma_regs.dest[ch], read_cpu_space16(m_dma_regs.source[ch]));

					m_dma_regs.dest[ch] += 2;
					m_dma_regs.source[ch] += 2;
					if (data & 0x0100)
						if ((m_dma_regs.dest[ch] & 0xf) == 0)
							m_dma_regs.dest[ch] -= 0x10;
				}
				else
				{
					write_cpu_space(m_dma_regs.dest[ch], read_cpu_space(m_dma_regs.source[ch]));

					m_dma_regs.dest[ch]++;
					m_dma_regs.source[ch]++;
				}
			}
		}
		else if (data != 0x0000) // fake DMA, used by C.U.G.
		{
			LOGMASKED(LOG_UNKNOWNS | LOG_DMA, "dma_w: Unknown DMA kickoff value of %04x (other regs %08x, %08x, %d)\n", data, m_dma_regs.source[ch], m_dma_regs.dest[ch], m_dma_regs.count[ch] + 1);
			fatalerror("dma_w: Unknown DMA kickoff value of %04x (other regs %08x, %08x, %d)\n",data, m_dma_regs.source[ch], m_dma_regs.dest[ch], m_dma_regs.count[ch] + 1);
		}
		break;
	default:
		LOGMASKED(LOG_UNKNOWNS | LOG_DMA, "dma_w: Unknown register: %08x = %04x & %04x\n", 0xe90020 + (offset << 1), data, mem_mask);
		break;
	}
}

void supracan_um6619_audiosoc_device::dma_channel0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	dma_w(offset, data, mem_mask, 0);
}

void supracan_um6619_audiosoc_device::dma_channel1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	dma_w(offset, data, mem_mask, 1);
}


static INPUT_PORTS_START( supracan )
	PORT_START("P1")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(1) PORT_NAME("P1 Button R")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(1) PORT_NAME("P1 Button L")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(1) PORT_NAME("P1 Button Y")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1) PORT_NAME("P1 Button X")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_NAME("P1 Joypad Right")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_NAME("P1 Joypad Left")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_NAME("P1 Joypad Down")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_NAME("P1 Joypad Up")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1) PORT_NAME("P1 Button B")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1) PORT_NAME("P1 Button A")

	PORT_START("P2")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_PLAYER(2) PORT_NAME("P2 Button R")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_PLAYER(2) PORT_NAME("P2 Button L")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_PLAYER(2) PORT_NAME("P2 Button Y")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2) PORT_NAME("P2 Button X")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_NAME("P2 Joypad Right")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_NAME("P2 Joypad Left")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_NAME("P2 Joypad Down")
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_NAME("P2 Joypad Up")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2) PORT_NAME("P2 Button B")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2) PORT_NAME("P2 Button A")
INPUT_PORTS_END

ioport_constructor supracan_um6619_audiosoc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(supracan);
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
	read_cpu_space.resolve();
	write_cpu_space.resolve_safe();
	read_cpu_space16.resolve();
	write_cpu_space16.resolve_safe();

	save_item(NAME(m_soundcpu_irq_enable));
	save_item(NAME(m_soundcpu_irq_source));
	save_item(NAME(m_sound_cpu_ctrl));
	save_item(NAME(m_sound_cpu_shift_ctrl));
	save_item(NAME(m_sound_cpu_shift_regs));
	save_item(NAME(m_sound_status));
	save_item(NAME(m_sound_reg_addr));
	save_item(NAME(m_latched_controls));

	save_item(NAME(m_dma_regs.source));
	save_item(NAME(m_dma_regs.dest));
	save_item(NAME(m_dma_regs.count));
	save_item(NAME(m_dma_regs.control));
}

void supracan_um6619_audiosoc_device::device_reset()
{
	m_soundcpu_irq_enable = 0;
	m_soundcpu_irq_source = 0;
	m_sound_cpu_ctrl = 0;

	m_sound_cpu_shift_ctrl = 0;
	std::fill(std::begin(m_sound_cpu_shift_regs), std::end(m_sound_cpu_shift_regs), 0);

	m_sound_status = 0;
	m_sound_reg_addr = 0;

	std::fill(std::begin(m_latched_controls), std::end(m_latched_controls), 0);

	m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}
