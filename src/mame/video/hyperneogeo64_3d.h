// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner, Andrew Zaferakis

#ifndef MAME_VIDEO_HYPERNEOGEO64_3D_H
#define MAME_VIDEO_HYPERNEOGEO64_3D_H

#pragma once

DECLARE_DEVICE_TYPE(HYPERNEOGEO64_3D, hyperneogeo64_3d)


class hyperneogeo64_3d : public device_t
/*  public device_video_interface */
{
public:
	// construction/destruction
	hyperneogeo64_3d(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

#endif // MAME_VIDEO_HYPERNEOGEO64_3D_H
