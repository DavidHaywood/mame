// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_AUDIO_SUPRACAN_UM6619_H
#define MAME_AUDIO_SUPRACAN_UM6619_H

#pragma once

#include "cpu/m6502/m6502.h"


DECLARE_DEVICE_TYPE(SUPRACAN_UM6619_AUDIOSOC, supracan_um6619_audiosoc_device)

class supracan_um6619_audiosoc_device : public device_t
{
public:
	supracan_um6619_audiosoc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void supracan_sound_mem(address_map &map);

	required_device<cpu_device> m_soundcpu;
};

#endif // MAME_AUDIO_SUPRACAN_UM6619_H
