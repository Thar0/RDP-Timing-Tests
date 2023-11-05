#ifndef VI_H_
#define VI_H_

#define VI_BASE_REG         (0x04400000)
#define VI_CONTROL_REG      (VI_BASE_REG + 0x00)
#define VI_ORIGIN_REG       (VI_BASE_REG + 0x04)
#define VI_WIDTH_REG        (VI_BASE_REG + 0x08)
#define VI_INTR_REG         (VI_BASE_REG + 0x0C)
#define VI_CURRENT_REG      (VI_BASE_REG + 0x10)
#define VI_BURST_REG        (VI_BASE_REG + 0x14)
#define VI_V_SYNC_REG       (VI_BASE_REG + 0x18)
#define VI_H_SYNC_REG       (VI_BASE_REG + 0x1C)
#define VI_LEAP_REG         (VI_BASE_REG + 0x20)
#define VI_H_START_REG      (VI_BASE_REG + 0x24)
#define VI_V_START_REG      (VI_BASE_REG + 0x28)
#define VI_V_BURST_REG      (VI_BASE_REG + 0x2C)
#define VI_X_SCALE_REG      (VI_BASE_REG + 0x30)
#define VI_Y_SCALE_REG      (VI_BASE_REG + 0x34)

#define VI_CTRL_TYPE_16             0x00002 // [1:0] pixel size: 16 bit
#define VI_CTRL_TYPE_32             0x00003 // [1:0] pixel size: 32 bit
#define VI_CTRL_GAMMA_DITHER_ON     0x00004 // 2: default = on
#define VI_CTRL_GAMMA_ON            0x00008 // 3: default = on
#define VI_CTRL_DIVOT_ON            0x00010 // 4: default = on
#define VI_CTRL_SERRATE_ON          0x00040 // 6: on if interlaced
#define VI_CTRL_ANTIALIAS_MASK      0x00300 // [9:8] anti-alias mode
#define VI_CTRL_ANTIALIAS_MODE_0    0x00000 // Bit [9:8] anti-alias mode: AA enabled, resampling enabled, always fetch extra lines
#define VI_CTRL_ANTIALIAS_MODE_1    0x00100 // Bit [9:8] anti-alias mode: AA enabled, resampling enabled, fetch extra lines as-needed
#define VI_CTRL_ANTIALIAS_MODE_2    0x00200 // Bit [9:8] anti-alias mode: AA disabled, resampling enabled, operate as if everything is covered
#define VI_CTRL_ANTIALIAS_MODE_3    0x00300 // Bit [9:8] anti-alias mode: AA disabled, resampling disabled, replicate pixels
#define VI_CTRL_PIXEL_ADV_MASK      0x0F000 // [15:12] pixel advance mode
#define VI_CTRL_PIXEL_ADV(n)        (((n) << 12) & VI_CTRL_PIXEL_ADV_MASK) // Bit [15:12] pixel advance mode: Always 3 on N64
#define VI_CTRL_DITHER_FILTER_ON    0x10000 // 16: dither-filter mode

#define VI_BURST(hsync_width, color_width, vsync_width, color_start) \
    ((hsync_width) | ((color_width) << 8) | ((vsync_width) << 16) | ((color_start) << 20))
#define VI_V_BURST(start, end) (((start) << 16) | (end))
#define VI_WIDTH(v) (v)
#define VI_VSYNC(v) (v)
#define VI_HSYNC(duration, leap) ((duration) | ((leap) << 16))
#define VI_LEAP(upper, lower) (((upper) << 16) | (lower))
#define VI_START(start, end) (((start) << 16) | (end))

#define FTOFIX(val, i, f) ((unsigned)(val * (float)(1 << f)) & ((1 << (i + f)) - 1))

#define F210(val) FTOFIX(val, 2, 10)
#define VI_SCALE(scaleup, off) (F210((1.0f / (float)scaleup)) | (F210((float)off) << 16))

#endif
