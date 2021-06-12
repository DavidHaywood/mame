// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner, Andrew Zaferakis

#include "emu.h"
#include "video/hyperneogeo64_3d.h"


DEFINE_DEVICE_TYPE(HYPERNEOGEO64_3D, hyperneogeo64_3d, "hng64_3d", "SNK HyperNeoGeo 64 3D")


hyperneogeo64_3d::hyperneogeo64_3d(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HYPERNEOGEO64_3D, tag, owner, clock)
{
}

void hyperneogeo64_3d::device_start()
{
}

void hyperneogeo64_3d::device_reset()
{
}
