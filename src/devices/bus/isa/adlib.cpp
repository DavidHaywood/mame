// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 8 bit Adlib Sound Card

***************************************************************************/

#include "emu.h"
#include "adlib.h"

#include "sound/spkrdev.h"
#include "speaker.h"


#define ym3812_StdClock 3579545

uint8_t isa8_adlib_device::ym3812_16_r(offs_t offset)
{
	uint8_t retVal = 0xff;
	switch(offset)
	{
		case 0 : retVal = m_ym3812->status_r(); break;
	}
	return retVal;
}

void isa8_adlib_device::ym3812_16_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0 : m_ym3812->address_w(data); break;
		case 1 : m_ym3812->data_w(data); break;
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_ADLIB, isa8_adlib_device, "isa_adlib", "Ad Lib Sound Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_adlib_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	YM3812(config, m_ym3812, ym3812_StdClock).add_route(ALL_OUTPUTS, "mono", 3.00);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_adlib_device - constructor
//-------------------------------------------------

isa8_adlib_device::isa8_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_ADLIB, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_ym3812(*this, "ym3812")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_adlib_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0388, 0x0389, read8sm_delegate(*this, FUNC(isa8_adlib_device::ym3812_16_r)), write8sm_delegate(*this, FUNC(isa8_adlib_device::ym3812_16_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_adlib_device::device_reset()
{
}
