// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz, David Haywood
/***************************************************************************


    Funtech Super A'Can
    -------------------

    Preliminary driver by Angelo Salese
    Improvements by Ryan Holtz


*******************************************************************************

INFO:

    The system unit contains a reset button.

    Controllers:
    - 4 directional buttons
    - A, B, X, Y, buttons
    - Start, select buttons
    - L, R shoulder buttons

STATUS:




DEBUG TRICKS:

    baseball game debug trick:
    wpset e90020,1f,w
    do pc=5ac40
    ...
    do pc=5acd4
    wpclear
    bp 0269E4
    [ff7be4] <- 0x269ec
    bpclear

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

#include "audio/supracan_um6619.h"
#include "video/supracan_um6618.h"

namespace {



class supracan_state : public driver_device
{
public:
	supracan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_um6618_vid(*this, "um6618_vid")
		, m_um6619_audio(*this, "um6619_audio")
	{
	}

	void supracan(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void supracan_mem(address_map &map);

	uint8_t read_cpu_byte(offs_t offset);
	void write_cpu_byte(offs_t offset, uint8_t data);

	uint16_t read_cpu_byte16(offs_t offset, uint16_t mem_mask = ~0);
	void write_cpu_byte16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	DECLARE_WRITE_LINE_MEMBER(vblank_w);
	DECLARE_WRITE_LINE_MEMBER(hblank_w);
	DECLARE_WRITE_LINE_MEMBER(lineirq_w);

	DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<supracan_um6618_video_device> m_um6618_vid;
	required_device<supracan_um6619_audiosoc_device> m_um6619_audio;
};


void supracan_state::supracan_mem(address_map &map)
{

	// 0x000000..0x3fffff is mapped by the cartslot
	map(0xe80000, 0xe8ffff).rw(m_um6619_audio, FUNC(supracan_um6619_audiosoc_device::_68k_soundram_r), FUNC(supracan_um6619_audiosoc_device::_68k_soundram_w));
	map(0xe90000, 0xe9001f).rw(m_um6619_audio, FUNC(supracan_um6619_audiosoc_device::sound_r), FUNC(supracan_um6619_audiosoc_device::sound_w));
	map(0xe90020, 0xe9002f).w(m_um6619_audio, FUNC(supracan_um6619_audiosoc_device::dma_channel0_w));
	map(0xe90030, 0xe9003f).w(m_um6619_audio, FUNC(supracan_um6619_audiosoc_device::dma_channel1_w));

	map(0xf00000, 0xf001ff).rw(m_um6618_vid, FUNC(supracan_um6618_video_device::video_r), FUNC(supracan_um6618_video_device::video_w));
	map(0xf00200, 0xf003ff).ram().w("um6618_vid:palette", FUNC(palette_device::write16)).share("um6618_vid:palette");
	map(0xf40000, 0xf5ffff).ram().w(m_um6618_vid, FUNC(supracan_um6618_video_device::vram_w)).share("um6618_vid:vram");
	map(0xfc0000, 0xfcffff).mirror(0x30000).ram(); /* System work ram */

}

static INPUT_PORTS_START( supracan )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(supracan_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size > 0x400000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


void supracan_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x000000, 0x3fffff, read16s_delegate(*m_cart, FUNC(generic_slot_device::read16_rom)));
}


void supracan_state::machine_reset()
{
}

uint8_t supracan_state::read_cpu_byte(offs_t offset)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	return mem.read_byte(offset);
}

void supracan_state::write_cpu_byte(offs_t offset, uint8_t data)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	mem.write_byte(offset, data);
}

uint16_t supracan_state::read_cpu_byte16(offs_t offset, uint16_t mem_mask)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	return mem.read_word(offset, mem_mask);
}

void supracan_state::write_cpu_byte16(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	mem.write_word(offset, data, mem_mask);
}


WRITE_LINE_MEMBER( supracan_state::vblank_w )
{
	// NMI
	m_maincpu->set_input_line(M68K_IRQ_7, state ? HOLD_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( supracan_state::hblank_w )
{
	m_maincpu->set_input_line(M68K_IRQ_3, state ? HOLD_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( supracan_state::lineirq_w )
{
	m_maincpu->set_input_line(M68K_IRQ_5, state ? HOLD_LINE : CLEAR_LINE);
}



void supracan_state::supracan(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(10'738'635));        /* Correct frequency unknown */
	m_maincpu->set_addrmap(AS_PROGRAM, &supracan_state::supracan_mem);

	SUPRACAN_UM6618_VIDEO(config, m_um6618_vid, 0);
	m_um6618_vid->set_read_cpu_space().set(FUNC(supracan_state::read_cpu_byte));
	m_um6618_vid->set_write_cpu_space().set(FUNC(supracan_state::write_cpu_byte));
	m_um6618_vid->set_vblank_irq().set(FUNC(supracan_state::vblank_w));
	m_um6618_vid->set_hblank_irq().set(FUNC(supracan_state::hblank_w));
	m_um6618_vid->set_line_irq().set(FUNC(supracan_state::lineirq_w));

	SUPRACAN_UM6619_AUDIOSOC(config, m_um6619_audio, 0); 
	m_um6619_audio->set_read_cpu_space().set(FUNC(supracan_state::read_cpu_byte));
	m_um6619_audio->set_write_cpu_space().set(FUNC(supracan_state::write_cpu_byte));
	m_um6619_audio->set_read_cpu_space16().set(FUNC(supracan_state::read_cpu_byte16));
	m_um6619_audio->set_write_cpu_space16().set(FUNC(supracan_state::write_cpu_byte16));

	config.set_perfect_quantum("um6619_audio:soundcpu");

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "supracan_cart"));
	cartslot.set_width(GENERIC_ROM16_WIDTH);
	cartslot.set_endian(ENDIANNESS_BIG);
	cartslot.set_device_load(FUNC(supracan_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("supracan");
}

ROM_START( supracan )
ROM_END

} // Anonymous namespace


/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     STATE           INIT        COMPANY                  FULLNAME        FLAGS */
CONS( 1995, supracan, 0,      0,      supracan, supracan, supracan_state, empty_init, "Funtech Entertainment", "Super A'Can",  MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
