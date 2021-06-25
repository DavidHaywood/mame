// license:LGPL-2.1+
// copyright-holders:Angelo Salese,Ryan Holtz
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

namespace {

class supracan_state : public driver_device
{
public:
	supracan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
	{
	}

	void supracan(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void supracan_mem(address_map &map);

	DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
};


void supracan_state::supracan_mem(address_map &map)
{
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

void supracan_state::supracan(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(10'738'635));        /* Correct frequency unknown */
	m_maincpu->set_addrmap(AS_PROGRAM, &supracan_state::supracan_mem);

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
