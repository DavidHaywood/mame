// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_SUPRACAN_UM6618_H
#define MAME_VIDEO_SUPRACAN_UM6618_H

#pragma once

DECLARE_DEVICE_TYPE(SUPRACAN_UM6618_VIDEO, supracan_um6618_video_device)

class supracan_um6618_video_device : public device_t
{
public:
	supracan_um6618_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
};

#endif // MAME_VIDEO_SUPRACAN_UM6618_H
