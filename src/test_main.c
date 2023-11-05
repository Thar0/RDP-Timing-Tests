
// Test Configuration

#define WIDTH 320
#define HEIGHT 240
#define TOTAL_RUNS 1000

// Test

#include <stdlib.h>
#include <malloc.h>
#include <libdragon.h>

#include "rdp.h"
#include "vi.h"

#define ARRLEN(arr) (sizeof(arr) / (sizeof((arr)[0])))

#define MI_BASE_REG         0x04300000
#define MI_INTR_REG         (MI_BASE_REG + 0x08)
#define MI_INTR_MASK_REG    (MI_BASE_REG + 0x0C)

#define MI_CLR_DP_INTR      (1 << 11)

static volatile bool dp_done_sig = false;

static void
dp_done (void)
{
    dp_done_sig = true;
    IO_WRITE(MI_INTR_REG, MI_CLR_DP_INTR);
}

static void
rdp_init_ (void)
{
    static bool rdp_is_init = false;

    if (rdp_is_init)
        return;

    IO_WRITE(MI_INTR_REG, MI_CLR_DP_INTR);
    register_DP_handler(dp_done);
    set_DP_interrupt(1);
    rdp_is_init = true;
}

typedef struct {
    uint32_t buf;
    uint32_t pipe;
    uint32_t tmem;
} rdp_times_t;

static void
rdp_exec (rdp_times_t* out, Gfx* dl, size_t length)
{
    data_cache_hit_writeback_invalidate(dl, length);

    dp_done_sig = false;

    // Try to approximately sync with the VI
    while (IO_READ(VI_CURRENT_REG) != 0)
        ;

    IO_WRITE(DPC_START_REG, dl);
    IO_WRITE(DPC_STATUS_REG, DPC_CLR_TMEM_CTR | DPC_CLR_PIPE_CTR | DPC_CLR_CMD_CTR | DPC_CLR_CLOCK_CTR);
    IO_WRITE(DPC_END_REG, (uint8_t*)dl + length);

    while (!dp_done_sig)
        ;

    dp_done_sig = false;

    // Wait for counters to fully settle
    wait_ms(2);

    if (out != NULL) {
        out->buf = IO_READ(DPC_BUFBUSY_REG);
        out->pipe = IO_READ(DPC_PIPEBUSY_REG);
        out->tmem = IO_READ(DPC_TMEM_REG);
    }
}

typedef struct {
    // pipeline
    bool two_cycle;
    // fbuffer
    bool color_read;
    // zbuffer
    bool depth_read;
    bool depth_write;
    bool depth_pass;
    bool zb_same_bank;
    // alpha cmp
    bool alpha_compare;
    uint8_t alpha_compare_threshold;
    uint8_t rectangle_alpha;
    // vi
    bool vi_on;
    bool vi_same_bank;
    // desc
    const char *desc;
} rdp_timing_spec_t;

// Reserve a full MB for RDRAM banking purposes
__attribute__((aligned(0x100000))) static uint8_t fb_region[0x100000];

#define FB_ADDR      (&fb_region[0 * WIDTH * HEIGHT * 2])
#define ZB_ADDR_SAME (&fb_region[1 * WIDTH * HEIGHT * 2])
#define VI_ADDR_SAME (&fb_region[2 * WIDTH * HEIGHT * 2])
#define ZB_ADDR_DIFF ((void*)(0xA0400000 + 0 * 0x100000))
#define VI_ADDR_DIFF ((void*)(0xA0400000 + 1 * 0x100000))

static void
exec_timing (rdp_times_t* fullsync_out, rdp_times_t* out, rdp_timing_spec_t *spec)
{
    static Gfx gfx_fullsync[] = {
        gsDPFullSync(),
    };

    // Set buffer addresses depending on current spec
    void* fb_addr = FB_ADDR;
    void* zb_addr = (spec->zb_same_bank) ? ZB_ADDR_SAME : ZB_ADDR_DIFF;
    void* vi_addr = (spec->vi_same_bank) ? VI_ADDR_SAME : VI_ADDR_DIFF;

    if (spec->vi_on) {
        // Switch on the VI for NTSC video
        IO_WRITE(VI_CONTROL_REG, VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON |
                                 VI_CTRL_GAMMA_ON | VI_CTRL_DIVOT_ON |
                                 VI_CTRL_ANTIALIAS_MODE_1 | VI_CTRL_PIXEL_ADV(3));
        IO_WRITE(VI_ORIGIN_REG, vi_addr);
        IO_WRITE(VI_WIDTH_REG, VI_WIDTH(320));
        IO_WRITE(VI_INTR_REG, 1024-1);
        IO_WRITE(VI_CURRENT_REG, 0);
        IO_WRITE(VI_BURST_REG, VI_BURST(57, 34, 5, 62));
        IO_WRITE(VI_V_SYNC_REG, VI_VSYNC(525));
        IO_WRITE(VI_H_SYNC_REG, VI_HSYNC(3093, 0));
        IO_WRITE(VI_LEAP_REG, VI_LEAP(3093, 3093));
        IO_WRITE(VI_H_START_REG, VI_START(108, 748));
        IO_WRITE(VI_V_START_REG, VI_START(37, 511));
        IO_WRITE(VI_V_BURST_REG, VI_V_BURST(14, 516));
        IO_WRITE(VI_X_SCALE_REG, VI_SCALE(2.0, 0));
        IO_WRITE(VI_Y_SCALE_REG, VI_SCALE(1.0, 0));
    } else {
        // Stop the VI
        IO_WRITE(VI_CONTROL_REG, 0);
        IO_WRITE(VI_INTR_REG, 1024-1);
        IO_WRITE(VI_H_START_REG, 0);
        IO_WRITE(VI_V_START_REG, 0);
        IO_WRITE(VI_CURRENT_REG, 0);
        IO_WRITE(VI_ORIGIN_REG, VI_ADDR_DIFF);
    }
    // Wait for VI to settle
    wait_ms(20);

    static Gfx gfx_setup[25];
    Gfx* gdl = &gfx_setup[0];

    // Initial
    gDPSetScissorFrac(gdl++, G_SC_NON_INTERLACE, qu102(0), qu102(0), qu102(WIDTH), qu102(HEIGHT));
    gDPSetOtherMode(gdl++, G_CYC_FILL, 0);

    // Clear the fb
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, WIDTH, fb_addr);
    gDPSetFillColor(gdl++, (GPACK_RGBA5551(0, 0, 0, 255) << 16) | GPACK_RGBA5551(0, 0, 0, 255));
    gDPFillRectangle(gdl++, 0, 0, WIDTH, HEIGHT);
    gDPPipeSync(gdl++);

    // Clear the zb
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, WIDTH, zb_addr);
    gDPSetFillColor(gdl++, (GPACK_ZDZ(G_MAXFBZ, 0) << 16) | GPACK_ZDZ(G_MAXFBZ, 0));
    gDPFillRectangle(gdl++, 0, 0, WIDTH, HEIGHT);
    gDPPipeSync(gdl++);

    // Setup
    gDPSetColorImage(gdl++, G_IM_FMT_RGBA, G_IM_SIZ_16b, WIDTH, fb_addr);
    gDPSetDepthImage(gdl++, zb_addr);
    gDPSetCombineLERP(gdl++, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE,
                             0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE);

    // Build othermode
    uint32_t om0 = ((spec->two_cycle) ? G_CYC_2CYCLE : G_CYC_1CYCLE) |
                   G_AD_DISABLE | G_CD_DISABLE |
                   G_CK_NONE |
                   G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_NONE |
                   G_PM_NPRIMITIVE;
    uint32_t om1 = ((spec->alpha_compare) ? G_AC_THRESHOLD : G_AC_NONE) |
                   G_ZS_PRIM |
                   CVG_DST_FULL | ZMODE_OPA |
                   ((spec->color_read) ? IM_RD : 0) |
                   ((spec->depth_read) ? Z_CMP : 0) |
                   ((spec->depth_write) ? Z_UPD : 0) |
                   G_RM_NOOP | G_RM_NOOP2;

    if (!spec->depth_pass) {
        // Inhibit depth passes with a 0 depth fillrect
        gDPSetOtherMode(gdl++, om0, om1 | Z_UPD);
        gDPSetPrimColor(gdl++, 0,0, 255,0,0,255);
        gDPSetPrimDepth(gdl++, 0, 0);
        gDPFillRectangle(gdl++, 0, 0, WIDTH, HEIGHT);
        gDPPipeSync(gdl++);
    }

    // Configure
    gDPSetOtherMode(gdl++, om0, om1);
    gDPSetBlendColor(gdl++, 0,0,0, spec->alpha_compare_threshold);
    gDPSetPrimColor(gdl++, 0,0, 0,255,0,spec->rectangle_alpha);
    gDPSetPrimDepth(gdl++, 0x7FFF, 0);
    gDPFullSync(gdl++);

    // Run the setup dl
    rdp_exec(NULL, gfx_setup, (uintptr_t)gdl - (uintptr_t)gfx_setup);

    // Run a single fullsync for baseline timing
    rdp_exec(fullsync_out, gfx_fullsync, sizeof(gfx_fullsync));

    // Run fillrects, vary depth from far -> closer
    for (size_t i = 0; i < TOTAL_RUNS; i++) {
        // debugf("%u\n", i);
        Gfx gfx_run[3] = {
            gsDPFillRectangle(0, 0, WIDTH, HEIGHT),
            gsDPSetPrimDepth(0x7FFF - i, 0),
            gsDPFullSync(),
        };

        // Random wait to try and break up phase patterns
        wait_ms((rand() >> 28) & 0xF);
        rdp_exec(&out[i], gfx_run, sizeof(gfx_run));
    }
}

#define CYC1    false
#define CYC2    true

#define NCRD    false
#define CRD     true

                 // r,    w,    pass, bnk
#define ZB_DISABLED false,false,false,false
#define ZB_R        true, false
#define ZB_W        false,true, true
#define ZB_RW       true, true

#define AC_DISABLED false,128,255
#define AC_ENABLED  true, 128, 96

#define ZB_SAME true
#define ZB_DIFF false

#define Z_FAIL false
#define Z_PASS true

#define VI_SAME true
#define VI_DIFF false

#define VI_ON  true
#define VI_OFF false,false

#define GROUP(settings...)                                  \
    { CYC1, NCRD, settings ", image_read off, 1-cycle" },   \
    { CYC2, NCRD, settings ", image_read off, 2-cycle" },   \
    { CYC1,  CRD, settings ", image_read on,  1-cycle" },   \
    { CYC2,  CRD, settings ", image_read on,  2-cycle" }

#define AC_GROUP(im_rd, z_cmp, zb_loc, desc) \
    { CYC1, im_rd, z_cmp, false, false, zb_loc, AC_ENABLED, VI_OFF, desc ", 1-cycle" }, \
    { CYC2, im_rd, z_cmp, false, false, zb_loc, AC_ENABLED, VI_OFF, desc ", 2-cycle" }

static rdp_timing_spec_t timing_specs[] = {
    // No Z-Buffer, No VI
    // 0 - 3
    GROUP(ZB_DISABLED,          AC_DISABLED, VI_OFF,        "No ZB, No VI"),

    // No Z-Buffer, VI
    // 4 - 7
    GROUP(ZB_DISABLED,          AC_DISABLED, VI_ON,VI_SAME, "No ZB, VI, FB + VI same    "),
    // 8 - 11
    GROUP(ZB_DISABLED,          AC_DISABLED, VI_ON,VI_DIFF, "No ZB, VI, FB + VI separate"),

    // Z-Buffer Read-Only, No VI
    // 12 - 15
    GROUP(ZB_R ,Z_FAIL,ZB_SAME, AC_DISABLED, VI_OFF,        "ZB Read-Only, No VI, Z Fail, FB + ZB same    "),
    // 16 - 19
    GROUP(ZB_R ,Z_FAIL,ZB_DIFF, AC_DISABLED, VI_OFF,        "ZB Read-Only, No VI, Z Fail, FB + ZB separate"),
    // 20 - 23
    GROUP(ZB_R ,Z_PASS,ZB_SAME, AC_DISABLED, VI_OFF,        "ZB Read-Only, No VI, Z Pass, FB + ZB same    "),
    // 24 - 27
    GROUP(ZB_R ,Z_PASS,ZB_DIFF, AC_DISABLED, VI_OFF,        "ZB Read-Only, No VI, Z Pass, FB + ZB separate"),

    // Z-Buffer Write-Only, No VI
    // 28 - 31
    GROUP(ZB_W ,       ZB_SAME, AC_DISABLED, VI_OFF,        "ZB Write-Only, No VI, FB + ZB same    "),
    // 32 - 35
    GROUP(ZB_W ,       ZB_DIFF, AC_DISABLED, VI_OFF,        "ZB Write-Only, No VI, FB + ZB separate"),

    // Z-Buffer Read/Write, No VI, Z Fail
    // 36 - 39
    GROUP(ZB_RW,Z_FAIL,ZB_SAME, AC_DISABLED, VI_OFF,        "ZB Read/Write, No VI, Z Fail, FB + ZB same    "),
    // 40 - 43
    GROUP(ZB_RW,Z_FAIL,ZB_DIFF, AC_DISABLED, VI_OFF,        "ZB Read/Write, No VI, Z Fail, FB + ZB separate"),
    // 44 - 47
    GROUP(ZB_RW,Z_PASS,ZB_SAME, AC_DISABLED, VI_OFF,        "ZB Read/Write, No VI, Z Pass, FB + ZB same    "),
    // 48 - 51
    GROUP(ZB_RW,Z_PASS,ZB_DIFF, AC_DISABLED, VI_OFF,        "ZB Read/Write, No VI, Z Pass, FB + ZB separate"),

    // Z-Buffer Read/Write, VI, Z Fail
    // 52 - 55
    GROUP(ZB_RW,Z_FAIL,ZB_DIFF, AC_DISABLED, VI_ON,VI_DIFF, "ZB Read/Write, VI, Z Fail, FB + ZB + VI separate    "),
    // 56 - 59
    GROUP(ZB_RW,Z_FAIL,ZB_DIFF, AC_DISABLED, VI_ON,VI_SAME, "ZB Read/Write, VI, Z Fail, FB + VI same, ZB separate"),
    // 60 - 63
    GROUP(ZB_RW,Z_FAIL,ZB_SAME, AC_DISABLED, VI_ON,VI_DIFF, "ZB Read/Write, VI, Z Fail, FB + ZB same, VI separate"),
    // 64 - 67
    GROUP(ZB_RW,Z_FAIL,ZB_SAME, AC_DISABLED, VI_ON,VI_SAME, "ZB Read/Write, VI, Z Fail, FB + ZB + VI same        "),
    // 68 - 71
    GROUP(ZB_RW,Z_PASS,ZB_DIFF, AC_DISABLED, VI_ON,VI_DIFF, "ZB Read/Write, VI, Z Pass, FB + ZB + VI separate    "),
    // 72 - 75
    GROUP(ZB_RW,Z_PASS,ZB_DIFF, AC_DISABLED, VI_ON,VI_SAME, "ZB Read/Write, VI, Z Pass, FB + VI same, ZB separate"),
    // 76 - 79
    GROUP(ZB_RW,Z_PASS,ZB_SAME, AC_DISABLED, VI_ON,VI_DIFF, "ZB Read/Write, VI, Z Pass, FB + ZB same, VI separate"),
    // 80 - 83
    GROUP(ZB_RW,Z_PASS,ZB_SAME, AC_DISABLED, VI_ON,VI_SAME, "ZB Read/Write, VI, Z Pass, FB + ZB + VI same        "),

    // Alpha Compare
    // 84 - 85
    AC_GROUP(false, false, ZB_SAME, "Alpha Compare, image_read off, z_compare off, FB + ZB same    "),
    // 86 - 87
    AC_GROUP(false, true,  ZB_SAME, "Alpha Compare, image_read off, z_compare on,  FB + ZB same    "),
    // 88 - 89
    AC_GROUP(true,  false, ZB_SAME, "Alpha Compare, image_read on,  z_compare off, FB + ZB same    "),
    // 90 - 91
    AC_GROUP(true,  true,  ZB_SAME, "Alpha Compare, image_read on,  z_compare on,  FB + ZB same    "),
    // 92 - 93
    AC_GROUP(false, false, ZB_DIFF, "Alpha Compare, image_read off, z_compare off, FB + ZB separate"),
    // 94 - 95
    AC_GROUP(false, true,  ZB_DIFF, "Alpha Compare, image_read off, z_compare on,  FB + ZB separate"),
    // 96 - 97
    AC_GROUP(true,  false, ZB_DIFF, "Alpha Compare, image_read on,  z_compare off, FB + ZB separate"),
    // 98 - 99
    AC_GROUP(true,  true,  ZB_DIFF, "Alpha Compare, image_read on,  z_compare on,  FB + ZB separate"),
};

static void
reset_callback (void)
{
    // For those of us testing without video display
    debugf("RESET Pressed!\n");
}

int
main ()
{
    srand(C0_COUNT());

    debug_init_usblog();
    register_RESET_handler(reset_callback);
    set_SI_interrupt(0);
    rdp_init_();

    // Fence for analysis script
    debugf("!!BEGIN!!\n");

    for (size_t i = 0; i < ARRLEN(timing_specs); i++) {
        static rdp_times_t all_times[TOTAL_RUNS];
        static rdp_times_t fullsync_time;

        debugf("%s\n", timing_specs[i].desc);

        // Ensure PI idle
        dma_wait();
        // Run timing for this spec
        exec_timing(&fullsync_time, all_times, &timing_specs[i]);

        debugf("BUF = [\n    ");
        for (size_t j = 0; j < TOTAL_RUNS; j++)
            debugf("%lu, ", all_times[j].buf - fullsync_time.buf - 1);
        debugf("\n]\n");

        debugf("PIPE = [\n    ");
        for (size_t j = 0; j < TOTAL_RUNS; j++)
            debugf("%lu, ", all_times[j].pipe - fullsync_time.pipe - 1);
        debugf("\n]\n");
    }

    // Multiple times incase the first isn't flushed properly
    debugf("!!DONE!!\n\n\n");
    debugf("!!DONE!!\n\n\n");
    debugf("!!DONE!!\n\n\n");

    fflush(stdout);
    fflush(stderr);
    return 0;
}
