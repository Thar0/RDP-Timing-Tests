/**
 * RDP command macros more-or-less lifted from f3d for precise control
 */
#ifndef RDP_H_
#define RDP_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PHYS_TO_K1(x)       ((uint32_t)(x) | 0xA0000000)
#define IO_READ(addr)       (*(volatile uint32_t*)PHYS_TO_K1(addr))
#define IO_WRITE(addr,data) (*(volatile uint32_t*)PHYS_TO_K1(addr) = (uint32_t)(data))

#define TMEM_SIZE 0x1000

/**
 * Display Processor Command (DPC) Registers
 */
#define DPC_BASE_REG        0x04100000

// DP CMD DMA start (R/W): [23:0] DMEM/RDRAM start address
#define DPC_START_REG       (DPC_BASE_REG + 0x00)

// DP CMD DMA end (R/W): [23:0] DMEM/RDRAM end address
#define DPC_END_REG         (DPC_BASE_REG + 0x04)

// DP CMD DMA end (R): [23:0] DMEM/RDRAM current address
#define DPC_CURRENT_REG     (DPC_BASE_REG + 0x08)

// DP CMD status (R/W): [9:0] valid bits - see below for definitions
#define DPC_STATUS_REG      (DPC_BASE_REG + 0x0C)

// DP clock counter (R): [23:0] clock counter
#define DPC_CLOCK_REG       (DPC_BASE_REG + 0x10)

// DP buffer busy counter (R): [23:0] clock counter
#define DPC_BUFBUSY_REG     (DPC_BASE_REG + 0x14)

// DP pipe busy counter (R): [23:0] clock counter
#define DPC_PIPEBUSY_REG    (DPC_BASE_REG + 0x18)

// DP TMEM load counter (R): [23:0] clock counter
#define DPC_TMEM_REG        (DPC_BASE_REG + 0x1C)

/*
 * DPC_STATUS_REG: write bits
 */
#define DPC_CLR_XBUS_DMEM_DMA       (1 << 0)
#define DPC_SET_XBUS_DMEM_DMA       (1 << 1)
#define DPC_CLR_FREEZE              (1 << 2)
#define DPC_SET_FREEZE              (1 << 3)
#define DPC_CLR_FLUSH               (1 << 4)
#define DPC_SET_FLUSH               (1 << 5)
#define DPC_CLR_TMEM_CTR            (1 << 6)
#define DPC_CLR_PIPE_CTR            (1 << 7)
#define DPC_CLR_CMD_CTR             (1 << 8)
#define DPC_CLR_CLOCK_CTR           (1 << 9)

/*
 * DPC_STATUS_REG: read bits
 */
#define DPC_STATUS_XBUS_DMEM_DMA    (1 <<  0)
#define DPC_STATUS_FREEZE           (1 <<  1)
#define DPC_STATUS_FLUSH            (1 <<  2)
#define DPC_STATUS_START_GCLK       (1 <<  3)
#define DPC_STATUS_TMEM_BUSY        (1 <<  4)
#define DPC_STATUS_PIPE_BUSY        (1 <<  5)
#define DPC_STATUS_CMD_BUSY         (1 <<  6)
#define DPC_STATUS_CBUF_READY       (1 <<  7)
#define DPC_STATUS_DMA_BUSY         (1 <<  8)
#define DPC_STATUS_END_VALID        (1 <<  9)
#define DPC_STATUS_START_VALID      (1 << 10)



/**
 * Display Processor Span (DPS) Registers
 */
#define DPS_BASE_REG            0x04200000

// DP tmem built-in self-test (R/W): [10:0] BIST status bits
#define DPS_TBIST_REG           (DPS_BASE_REG + 0x00)

// DP span test mode (R/W): [0] Span buffer test access enable
#define DPS_TEST_MODE_REG       (DPS_BASE_REG + 0x04)

// DP span buffer test address (R/W): [6:0] bits
#define DPS_BUFTEST_ADDR_REG    (DPS_BASE_REG + 0x08)

// DP span buffer test data (R/W): [31:0] span buffer data
#define DPS_BUFTEST_DATA_REG    (DPS_BASE_REG + 0x0C)



/**
 * Color macros
 */

#define G_MAXFBZ    0x3FFF

#define GPACK_RGBA5551(r,g,b,a) (gF_(r, 5, 11) | gF_(g, 5, 6) | gF_(b, 5, 1) | gF_(a, 1, 0))

#define GPACK_RGBA8888(r,g,b,a) (gF_(r, 8, 24) | gF_(g, 8, 16) | gF_(b, 8, 8) | gF_(a, 8, 0))

#define GPACK_RGB24A8(rgb,a)    (gF_(rgb, 24, 8) | gF_(a, 8, 0))

#define GPACK_ZDZ(z,dz)         (gF_(z, 14, 2) | gF_(dz, 2, 0))



/**
 * Raw RDP Command Generation
 */

typedef struct __attribute__((aligned(8))) {
    uint32_t w0;
    uint32_t w1;
} Gfx;

/*
 * RDP commands:
 */
#define G_SETCIMG               (0xC0 | 0x3F)
#define G_SETZIMG               (0xC0 | 0x3E)
#define G_SETTIMG               (0xC0 | 0x3D)
#define G_SETCOMBINE            (0xC0 | 0x3C)
#define G_SETENVCOLOR           (0xC0 | 0x3B)
#define G_SETPRIMCOLOR          (0xC0 | 0x3A)
#define G_SETBLENDCOLOR         (0xC0 | 0x39)
#define G_SETFOGCOLOR           (0xC0 | 0x38)
#define G_SETFILLCOLOR          (0xC0 | 0x37)
#define G_FILLRECT              (0xC0 | 0x36)
#define G_SETTILE               (0xC0 | 0x35)
#define G_LOADTILE              (0xC0 | 0x34)
#define G_LOADBLOCK             (0xC0 | 0x33)
#define G_SETTILESIZE           (0xC0 | 0x32)
#define G_LOADTLUT              (0xC0 | 0x30)
#define G_RDPSETOTHERMODE       (0xC0 | 0x2F)
#define G_SETPRIMDEPTH          (0xC0 | 0x2E)
#define G_SETSCISSOR            (0xC0 | 0x2D)
#define G_SETCONVERT            (0xC0 | 0x2C)
#define G_SETKEYR               (0xC0 | 0x2B)
#define G_SETKEYGB              (0xC0 | 0x2A)
#define G_RDPFULLSYNC           (0xC0 | 0x29)
#define G_RDPTILESYNC           (0xC0 | 0x28)
#define G_RDPPIPESYNC           (0xC0 | 0x27)
#define G_RDPLOADSYNC           (0xC0 | 0x26)
#define G_TEXRECTFLIP           (0xC0 | 0x25)
#define G_TEXRECT               (0xC0 | 0x24)

#define G_TRI_FILL              (0xC0 | 0x08)   /* fill triangle:            0b001000 */
#define G_TRI_TXTR              (0xC0 | 0x0A)   /* texture triangle:         0b001010 */
#define G_TRI_SHADE             (0xC0 | 0x0C)   /* shade triangle:           0b001100 */
#define G_TRI_SHADE_TXTR        (0xC0 | 0x0E)   /* shade, texture triangle:  0b001110 */
#define G_TRI_FILL_ZBUFF        (0xC0 | 0x09)   /* fill, zbuff triangle:     0b001001 */
#define G_TRI_TXTR_ZBUFF        (0xC0 | 0x0B)   /* texture, zbuff triangle:  0b001011 */
#define G_TRI_SHADE_ZBUFF       (0xC0 | 0x0D)   /* shade, zbuff triangle:    0b001101 */
#define G_TRI_SHADE_TXTR_ZBUFF  (0xC0 | 0x0F)   /* shade, txtr, zbuff trngl: 0b001111 */

#define G_NOOP                  0x00

#define gsTriBase(cmd,              \
                  lft, level, tile, \
                  xl, yl, dxldy,    \
                  xm, ym, dxmdy,    \
                  xh, yh, dxhdy)    \
    gO_(cmd, gF_(lft,    1, 23) |   \
             gF_(level,  3, 19) |   \
             gF_(tile,   3, 16) |   \
             gF_(yh,    14,  0),    \
             gF_(ym,    14, 16) |   \
             gF_(yl,    14,  0)),   \
    gO_(0, xl, dxldy),              \
    gO_(0, xh, dxhdy),              \
    gO_(0, xm, dxmdy)

#define gsTriShadeCoeffs(r_i, r_f,       \
                         drdx_i, drdx_f, \
                         drde_i, drde_f, \
                         drdy_i, drdy_f, \
                         g_i, g_f,       \
                         dgdx_i, dgdx_f, \
                         dgde_i, dgde_f, \
                         dgdy_i, dgdy_f, \
                         b_i, b_f,       \
                         dbdx_i, dbdx_f, \
                         dbde_i, dbde_f, \
                         dbdy_i, dbdy_f, \
                         a_i, a_f,       \
                         dadx_i, dadx_f, \
                         dade_i, dade_f, \
                         dady_i, dady_f) \
    gO_(0, gF_(r_i, 16, 16) |       \
           gF_(g_i, 16,  0),        \
           gF_(b_i, 16, 16) |       \
           gF_(a_i, 16,  0)),       \
    gO_(0, gF_(drdx_i, 16, 16) |    \
           gF_(dgdx_i, 16,  0),     \
           gF_(dbdx_i, 16, 16) |    \
           gF_(dadx_i, 16,  0)),    \
    gO_(0, gF_(r_f, 16, 16) |       \
           gF_(g_f, 16,  0),        \
           gF_(b_f, 16, 16) |       \
           gF_(a_f, 16,  0)),       \
    gO_(0, gF_(drdx_f, 16, 16) |    \
           gF_(dgdx_f, 16,  0),     \
           gF_(dbdx_f, 16, 16) |    \
           gF_(dadx_f, 16,  0)),    \
    gO_(0, gF_(drde_i, 16, 16) |    \
           gF_(dgde_i, 16,  0),     \
           gF_(dbde_i, 16, 16) |    \
           gF_(dade_i, 16,  0)),    \
    gO_(0, gF_(drdy_i, 16, 16) |    \
           gF_(dgdy_i, 16,  0),     \
           gF_(dbdy_i, 16, 16) |    \
           gF_(dady_i, 16,  0)),    \
    gO_(0, gF_(drde_f, 16, 16) |    \
           gF_(dgde_f, 16,  0),     \
           gF_(dbde_f, 16, 16) |    \
           gF_(dade_f, 16,  0)),    \
    gO_(0, gF_(drdy_f, 16, 16) |    \
           gF_(dgdy_f, 16,  0),     \
           gF_(dbdy_f, 16, 16) |    \
           gF_(dady_f, 16,  0))

#define gsTriTexCoeffs(s_i, s_f,       \
                       dsdx_i, dsdx_f, \
                       dsde_i, dsde_f, \
                       dsdy_i, dsdy_f, \
                       t_i, t_f,       \
                       dtdx_i, dtdx_f, \
                       dtde_i, dtde_f, \
                       dtdy_i, dtdy_f, \
                       w_i, w_f,       \
                       dwdx_i, dwdx_f, \
                       dwde_i, dwde_f, \
                       dwdy_i, dwdy_f) \
    gO_(0, gF_(s_i, 16, 16) |       \
           gF_(t_i, 16,  0),        \
           gF_(w_i, 16, 16) |       \
           gF_(  0, 16,  0)),       \
    gO_(0, gF_(dsdx_i, 16, 16) |    \
           gF_(dtdx_i, 16,  0),     \
           gF_(dwdx_i, 16, 16) |    \
           gF_(     0, 16,  0)),    \
    gO_(0, gF_(s_f, 16, 16) |       \
           gF_(t_f, 16,  0),        \
           gF_(w_f, 16, 16) |       \
           gF_(  0, 16,  0)),       \
    gO_(0, gF_(dsdx_f, 16, 16) |    \
           gF_(dtdx_f, 16,  0),     \
           gF_(dwdx_f, 16, 16) |    \
           gF_(     0, 16,  0)),    \
    gO_(0, gF_(dsde_i, 16, 16) |    \
           gF_(dtde_i, 16,  0),     \
           gF_(dwde_i, 16, 16) |    \
           gF_(     0, 16,  0)),    \
    gO_(0, gF_(dsdy_i, 16, 16) |    \
           gF_(dtdy_i, 16,  0),     \
           gF_(dwdy_i, 16, 16) |    \
           gF_(     0, 16,  0)),    \
    gO_(0, gF_(dsde_f, 16, 16) |    \
           gF_(dtde_f, 16,  0),     \
           gF_(dwde_f, 16, 16) |    \
           gF_(     0, 16,  0)),    \
    gO_(0, gF_(dsdy_f, 16, 16) |    \
           gF_(dtdy_f, 16,  0),     \
           gF_(dwdy_f, 16, 16) |    \
           gF_(     0, 16,  0))

#define gsTriDepthCoeffs(z, dzdx, dzde, dzdy) \
    gO_(0,   z,dzdx),   \
    gO_(0,dzde,dzdy)



#define gsDPTriFill(lft, level, tile,   \
                    xl, yl, dxldy,      \
                    xm, ym, dxmdy,      \
                    xh, yh, dxhdy)      \
    gsTriBase(G_TRI_FILL, lft, level, tile, \
              xl, yl, dxldy,                \
              xm, ym, dxmdy,                \
              xh, yh, dxhdy)

#define gsDPTriFill_S(lft, level, tile, \
                      xl, yl, dxldy,    \
                      xm, ym, dxmdy,    \
                      xh, yh, dxhdy,    \
                      r_i, r_f,         \
                      drdx_i, drdx_f,   \
                      drde_i, drde_f,   \
                      drdy_i, drdy_f,   \
                      g_i, g_f,         \
                      dgdx_i, dgdx_f,   \
                      dgde_i, dgde_f,   \
                      dgdy_i, dgdy_f,   \
                      b_i, b_f,         \
                      dbdx_i, dbdx_f,   \
                      dbde_i, dbde_f,   \
                      dbdy_i, dbdy_f,   \
                      a_i, a_f,         \
                      dadx_i, dadx_f,   \
                      dade_i, dade_f,   \
                      dady_i, dady_f)   \
    gsTriBase(G_TRI_SHADE, lft, level, tile,    \
              xl, yl, dxldy,                    \
              xm, ym, dxmdy,                    \
              xh, yh, dxhdy),                   \
    gsTriShadeCoeffs(r_i, r_f,                  \
                     drdx_i, drdx_f,            \
                     drde_i, drde_f,            \
                     drdy_i, drdy_f,            \
                     g_i, g_f,                  \
                     dgdx_i, dgdx_f,            \
                     dgde_i, dgde_f,            \
                     dgdy_i, dgdy_f,            \
                     b_i, b_f,                  \
                     dbdx_i, dbdx_f,            \
                     dbde_i, dbde_f,            \
                     dbdy_i, dbdy_f,            \
                     a_i, a_f,                  \
                     dadx_i, dadx_f,            \
                     dade_i, dade_f,            \
                     dady_i, dady_f)

#define gsDPTriFill_T(lft, level, tile, \
                      xl, yl, dxldy,    \
                      xm, ym, dxmdy,    \
                      xh, yh, dxhdy,    \
                      s_i, s_f,         \
                      dsdx_i, dsdx_f,   \
                      dsde_i, dsde_f,   \
                      dsdy_i, dsdy_f,   \
                      t_i, t_f,         \
                      dtdx_i, dtdx_f,   \
                      dtde_i, dtde_f,   \
                      dtdy_i, dtdy_f,   \
                      w_i, w_f,         \
                      dwdx_i, dwdx_f,   \
                      dwde_i, dwde_f,   \
                      dwdy_i, dwdy_f)   \
    gsTriBase(G_TRI_TXTR, lft, level, tile, \
              xl, yl, dxldy,                \
              xm, ym, dxmdy,                \
              xh, yh, dxhdy),               \
    gsTriTexCoeffs(s_i, s_f,                \
                   dsdx_i, dsdx_f,          \
                   dsde_i, dsde_f,          \
                   dsdy_i, dsdy_f,          \
                   t_i, t_f,                \
                   dtdx_i, dtdx_f,          \
                   dtde_i, dtde_f,          \
                   dtdy_i, dtdy_f,          \
                   w_i, w_f,                \
                   dwdx_i, dwdx_f,          \
                   dwde_i, dwde_f,          \
                   dwdy_i, dwdy_f)

#define gsDPTriFill_ST(lft, level, tile,    \
                       xl, yl, dxldy,       \
                       xm, ym, dxmdy,       \
                       xh, yh, dxhdy,       \
                       r_i, r_f,            \
                       drdx_i, drdx_f,      \
                       drde_i, drde_f,      \
                       drdy_i, drdy_f,      \
                       g_i, g_f,            \
                       dgdx_i, dgdx_f,      \
                       dgde_i, dgde_f,      \
                       dgdy_i, dgdy_f,      \
                       b_i, b_f,            \
                       dbdx_i, dbdx_f,      \
                       dbde_i, dbde_f,      \
                       dbdy_i, dbdy_f,      \
                       a_i, a_f,            \
                       dadx_i, dadx_f,      \
                       dade_i, dade_f,      \
                       dady_i, dady_f,      \
                       s_i, s_f,            \
                       dsdx_i, dsdx_f,      \
                       dsde_i, dsde_f,      \
                       dsdy_i, dsdy_f,      \
                       t_i, t_f,            \
                       dtdx_i, dtdx_f,      \
                       dtde_i, dtde_f,      \
                       dtdy_i, dtdy_f,      \
                       w_i, w_f,            \
                       dwdx_i, dwdx_f,      \
                       dwde_i, dwde_f,      \
                       dwdy_i, dwdy_f)      \
    gsTriBase(G_TRI_SHADE_TXTR, lft, level, tile,   \
              xl, yl, dxldy,                        \
              xm, ym, dxmdy,                        \
              xh, yh, dxhdy),                       \
    gsTriShadeCoeffs(r_i, r_f,                      \
                     drdx_i, drdx_f,                \
                     drde_i, drde_f,                \
                     drdy_i, drdy_f,                \
                     g_i, g_f,                      \
                     dgdx_i, dgdx_f,                \
                     dgde_i, dgde_f,                \
                     dgdy_i, dgdy_f,                \
                     b_i, b_f,                      \
                     dbdx_i, dbdx_f,                \
                     dbde_i, dbde_f,                \
                     dbdy_i, dbdy_f,                \
                     a_i, a_f,                      \
                     dadx_i, dadx_f,                \
                     dade_i, dade_f,                \
                     dady_i, dady_f),               \
    gsTriTexCoeffs(s_i, s_f,                        \
                   dsdx_i, dsdx_f,                  \
                   dsde_i, dsde_f,                  \
                   dsdy_i, dsdy_f,                  \
                   t_i, t_f,                        \
                   dtdx_i, dtdx_f,                  \
                   dtde_i, dtde_f,                  \
                   dtdy_i, dtdy_f,                  \
                   w_i, w_f,                        \
                   dwdx_i, dwdx_f,                  \
                   dwde_i, dwde_f,                  \
                   dwdy_i, dwdy_f)

#define gsDPTriFill_Z(lft, level, tile,     \
                      xl, yl, dxldy,        \
                      xm, ym, dxmdy,        \
                      xh, yh, dxhdy,        \
                      z, dzdx, dzde, dzdy)  \
    gsTriBase(G_TRI_FILL_ZBUFF, lft, level, tile,   \
              xl, yl, dxldy,                        \
              xm, ym, dxmdy,                        \
              xh, yh, dxhdy),                       \
    gsTriDepthCoeffs(z, dzdx, dzde, dzdy)

#define gsDPTriFill_SZ(lft, level, tile,    \
                       xl, yl, dxldy,       \
                       xm, ym, dxmdy,       \
                       xh, yh, dxhdy,       \
                       r_i, r_f,            \
                       drdx_i, drdx_f,      \
                       drde_i, drde_f,      \
                       drdy_i, drdy_f,      \
                       g_i, g_f,            \
                       dgdx_i, dgdx_f,      \
                       dgde_i, dgde_f,      \
                       dgdy_i, dgdy_f,      \
                       b_i, b_f,            \
                       dbdx_i, dbdx_f,      \
                       dbde_i, dbde_f,      \
                       dbdy_i, dbdy_f,      \
                       a_i, a_f,            \
                       dadx_i, dadx_f,      \
                       dade_i, dade_f,      \
                       dady_i, dady_f,      \
                       z, dzdx, dzde, dzdy) \
    gsTriBase(G_TRI_SHADE_ZBUFF, lft, level, tile,  \
              xl, yl, dxldy,                        \
              xm, ym, dxmdy,                        \
              xh, yh, dxhdy),                       \
    gsTriShadeCoeffs(r_i, r_f,                      \
                     drdx_i, drdx_f,                \
                     drde_i, drde_f,                \
                     drdy_i, drdy_f,                \
                     g_i, g_f,                      \
                     dgdx_i, dgdx_f,                \
                     dgde_i, dgde_f,                \
                     dgdy_i, dgdy_f,                \
                     b_i, b_f,                      \
                     dbdx_i, dbdx_f,                \
                     dbde_i, dbde_f,                \
                     dbdy_i, dbdy_f,                \
                     a_i, a_f,                      \
                     dadx_i, dadx_f,                \
                     dade_i, dade_f,                \
                     dady_i, dady_f),               \
    gsTriDepthCoeffs(z, dzdx, dzde, dzdy)

#define gsDPTriFill_TZ(lft, level, tile,    \
                       xl, yl, dxldy,       \
                       xm, ym, dxmdy,       \
                       xh, yh, dxhdy,       \
                       s_i, s_f,            \
                       dsdx_i, dsdx_f,      \
                       dsde_i, dsde_f,      \
                       dsdy_i, dsdy_f,      \
                       t_i, t_f,            \
                       dtdx_i, dtdx_f,      \
                       dtde_i, dtde_f,      \
                       dtdy_i, dtdy_f,      \
                       w_i, w_f,            \
                       dwdx_i, dwdx_f,      \
                       dwde_i, dwde_f,      \
                       dwdy_i, dwdy_f,      \
                       z, dzdx, dzde, dzdy) \
    gsTriBase(G_TRI_TXTR_ZBUFF, lft, level, tile,   \
              xl, yl, dxldy,                        \
              xm, ym, dxmdy,                        \
              xh, yh, dxhdy),                       \
    gsTriTexCoeffs(s_i, s_f,                        \
                   dsdx_i, dsdx_f,                  \
                   dsde_i, dsde_f,                  \
                   dsdy_i, dsdy_f,                  \
                   t_i, t_f,                        \
                   dtdx_i, dtdx_f,                  \
                   dtde_i, dtde_f,                  \
                   dtdy_i, dtdy_f,                  \
                   w_i, w_f,                        \
                   dwdx_i, dwdx_f,                  \
                   dwde_i, dwde_f,                  \
                   dwdy_i, dwdy_f),                 \
    gsTriDepthCoeffs(z, dzdx, dzde, dzdy)

#define gsDPTriFill_STZ(lft, level, tile,       \
                        xl, yl, dxldy,          \
                        xm, ym, dxmdy,          \
                        xh, yh, dxhdy,          \
                        r_i, r_f,               \
                        drdx_i, drdx_f,         \
                        drde_i, drde_f,         \
                        drdy_i, drdy_f,         \
                        g_i, g_f,               \
                        dgdx_i, dgdx_f,         \
                        dgde_i, dgde_f,         \
                        dgdy_i, dgdy_f,         \
                        b_i, b_f,               \
                        dbdx_i, dbdx_f,         \
                        dbde_i, dbde_f,         \
                        dbdy_i, dbdy_f,         \
                        a_i, a_f,               \
                        dadx_i, dadx_f,         \
                        dade_i, dade_f,         \
                        dady_i, dady_f,         \
                        s_i, s_f,               \
                        dsdx_i, dsdx_f,         \
                        dsde_i, dsde_f,         \
                        dsdy_i, dsdy_f,         \
                        t_i, t_f,               \
                        dtdx_i, dtdx_f,         \
                        dtde_i, dtde_f,         \
                        dtdy_i, dtdy_f,         \
                        w_i, w_f,               \
                        dwdx_i, dwdx_f,         \
                        dwde_i, dwde_f,         \
                        dwdy_i, dwdy_f,         \
                        z, dzdx, dzde, dzdy)    \
    gsTriBase(G_TRI_SHADE_TXTR_ZBUFF, lft, level, tile, \
              xl, yl, dxldy,                            \
              xm, ym, dxmdy,                            \
              xh, yh, dxhdy),                           \
    gsTriShadeCoeffs(r_i, r_f,                          \
                     drdx_i, drdx_f,                    \
                     drde_i, drde_f,                    \
                     drdy_i, drdy_f,                    \
                     g_i, g_f,                          \
                     dgdx_i, dgdx_f,                    \
                     dgde_i, dgde_f,                    \
                     dgdy_i, dgdy_f,                    \
                     b_i, b_f,                          \
                     dbdx_i, dbdx_f,                    \
                     dbde_i, dbde_f,                    \
                     dbdy_i, dbdy_f,                    \
                     a_i, a_f,                          \
                     dadx_i, dadx_f,                    \
                     dade_i, dade_f,                    \
                     dady_i, dady_f),                   \
    gsTriTexCoeffs(s_i, s_f,                            \
                   dsdx_i, dsdx_f,                      \
                   dsde_i, dsde_f,                      \
                   dsdy_i, dsdy_f,                      \
                   t_i, t_f,                            \
                   dtdx_i, dtdx_f,                      \
                   dtde_i, dtde_f,                      \
                   dtdy_i, dtdy_f,                      \
                   w_i, w_f,                            \
                   dwdx_i, dwdx_f,                      \
                   dwde_i, dwde_f,                      \
                   dwdy_i, dwdy_f),                     \
    gsTriDepthCoeffs(z, dzdx, dzde, dzdy)



/*
 * Static macros
 */

#define gsDPSetColorImage(fmt, siz, width, img) \
    gO_(G_SETCIMG,                  \
        gF_(fmt, 3, 21) |           \
        gF_(siz, 2, 19) |           \
        gF_((width) - 1, 12, 0),    \
        img)

#define gsDPSetDepthImage(img)  \
    gO_(G_SETZIMG, 0,   \
        img)

#define gsDPSetTextureImage(fmt, siz, width, img)   \
    gO_(G_SETTIMG,                  \
        gF_(fmt, 3, 21) |           \
        gF_(siz, 2, 19) |           \
        gF_((width) - 1, 12, 0),    \
        img)

#define gsDPSetCombineLERP(a0, b0, c0, d0,      \
                           Aa0, Ab0, Ac0, Ad0,  \
                           a1, b1, c1, d1,      \
                           Aa1, Ab1, Ac1, Ad1)  \
    gO_(G_SETCOMBINE,               \
        gF_(G_CCMUX_##a0,4,20) |    \
        gF_(G_CCMUX_##c0,5,15) |    \
        gF_(G_ACMUX_##Aa0,3,12) |   \
        gF_(G_ACMUX_##Ac0,3,9) |    \
        gF_(G_CCMUX_##a1,4,5) |     \
        gF_(G_CCMUX_##c1,5,0),      \
        gF_(G_CCMUX_##b0,4,28) |    \
        gF_(G_CCMUX_##b1,4,24) |    \
        gF_(G_ACMUX_##Aa1,3,21) |   \
        gF_(G_ACMUX_##Ac1,3,18) |   \
        gF_(G_CCMUX_##d0,3,15) |    \
        gF_(G_ACMUX_##Ab0,3,12) |   \
        gF_(G_ACMUX_##Ad0,3,9) |    \
        gF_(G_CCMUX_##d1,3,6) |     \
        gF_(G_ACMUX_##Ab1,3,3) |    \
        gF_(G_ACMUX_##Ad1,3,0))

#define gsDPSetEnvColor(r, g, b, a) \
    gO_(G_SETENVCOLOR, 0,   \
        gF_(r, 8, 24) |     \
        gF_(g, 8, 16) |     \
        gF_(b, 8,  8) |     \
        gF_(a, 8,  0))

#define gsDPSetPrimColor(m, l, r, g, b, a)  \
    gO_(G_SETPRIMCOLOR, \
        gF_(m, 8,  8) | \
        gF_(l, 8, 0),   \
        gF_(r, 8, 24) | \
        gF_(g, 8, 16) | \
        gF_(b, 8,  8) | \
        gF_(a, 8,  0))

#define gsDPSetBlendColor(r, g, b, a) \
    gO_(G_SETBLENDCOLOR, 0, \
        gF_(r, 8, 24) |     \
        gF_(g, 8, 16) |     \
        gF_(b, 8,  8) |     \
        gF_(a, 8,  0))

#define gsDPSetFogColor(r, g, b, a) \
    gO_(G_SETFOGCOLOR, 0,   \
        gF_(r, 8, 24) |     \
        gF_(g, 8, 16) |     \
        gF_(b, 8,  8) |     \
        gF_(a, 8,  0))

#define gsDPSetFillColor(c) \
    gO_(G_SETFILLCOLOR, 0,  \
        c)

#define gsDPFillRectangle(ulx, uly, lrx, lry)   \
    gO_(G_FILLRECT,         \
        gF_(lrx, 10, 14) |  \
        gF_(lry, 10,  2),   \
        gF_(ulx, 10, 14) |  \
        gF_(uly, 10,  2))

#define gsDPSetTile(fmt, siz, line,         \
                    tmem, tile, palette,    \
                    cmt, maskt, shiftt,     \
                    cms, masks, shifts)     \
    gO_(G_SETTILE,              \
        gF_(fmt,     3, 21) |   \
        gF_(siz,     2, 19) |   \
        gF_(line,    9,  9) |   \
        gF_(tmem,    9,  0),    \
        gF_(tile,    3, 24) |   \
        gF_(palette, 4, 20) |   \
        gF_(cmt,     2, 18) |   \
        gF_(maskt,   4, 14) |   \
        gF_(shiftt,  4, 10) |   \
        gF_(cms,     2,  8) |   \
        gF_(masks,   4,  4) |   \
        gF_(shifts,  4,  0))

#define gsDPLoadTile(tile, uls, ult, lrs, lrt)  \
    gO_(G_LOADTILE,         \
        gF_(uls,  12, 12) | \
        gF_(ult,  12,  0),  \
        gF_(tile,  3, 24) | \
        gF_(lrs,  12, 12) | \
        gF_(lrt,  12,  0))

#define gsDPLoadBlock(tile, uls, ult, lrs, dxt) \
    gO_(G_LOADBLOCK,        \
        gF_(uls,  12, 12) | \
        gF_(ult,  12,  0),  \
        gF_(tile,  3, 24) | \
        gF_(lrs,  12, 12) | \
        gF_(dxt,  12,  0))

#define gsDPSetTileSize(tile, uls, ult, lrs, lrt)   \
    gO_(G_SETTILESIZE,      \
        gF_(uls,  12, 12) | \
        gF_(ult,  12,  0),  \
        gF_(tile,  3, 24) | \
        gF_(lrs,  12, 12) | \
        gF_(lrt,  12,  0))

#define gsDPLoadTLUTCmd(tile, count)    \
    gO_(G_LOADTLUT, 0,          \
        gF_(tile,   3, 24) |    \
        gF_(count, 10, 14))

#define gsDPSetOtherMode(mode0, mode1)  \
    gO_(G_RDPSETOTHERMODE,  \
        gF_(mode0, 24, 0),  \
        mode1)

#define gsDPSetPrimDepth(z, dz) \
    gO_(G_SETPRIMDEPTH, 0,  \
        gF_(z,  16, 16) |   \
        gF_(dz, 16,  0))

#define gsDPSetScissorFrac(mode, ulx, uly, lrx, lry)    \
    gO_(G_SETSCISSOR,       \
        gF_(ulx,  12, 12) | \
        gF_(uly,  12,  0),  \
        gF_(mode,  2, 24) | \
        gF_(lrx,  12, 12) | \
        gF_(lry,  12,  0))

#define gsDPSetConvert(k0, k1, k2, k3, k4, k5)  \
    gO_(G_SETCONVERT,               \
        gF_(    k0,       9, 13) |  \
        gF_(    k1,       9,  4) |  \
        gF_(gI_(k2) >> 5, 4,  0),   \
        gF_(    k2,       5, 27) |  \
        gF_(    k3,       9, 18) |  \
        gF_(    k4,       9,  9) |  \
        gF_(    k5,       9,  0))

#define gsDPSetKeyR(cR, sR, wR) \
    gO_(G_SETKEYR, 0,       \
        gF_(wR, 12, 16) |   \
        gF_(cR,  8,  8) |   \
        gF_(sR,  8,  0))

#define gsDPSetKeyGB(cG,sG,wG,cB,sB,wB)     \
    gO_(G_SETKEYGB,         \
        gF_(wG, 12, 12) |   \
        gF_(wB, 12, 0),     \
        gF_(cG,  8, 24) |   \
        gF_(sG,  8,16) |    \
        gF_(cB,  8,  8) |   \
        gF_(sB,  8, 0))

#define gsDPFullSync()  gO_(G_RDPFULLSYNC, 0, 0)

#define gsDPTileSync()  gO_(G_RDPTILESYNC, 0, 0)

#define gsDPPipeSync()  gO_(G_RDPPIPESYNC, 0, 0)

#define gsDPLoadSync()  gO_(G_RDPLOADSYNC, 0, 0)

#define gsTexRectFlip(ulx, uly, lrx, lry, tile, s, t, dsdx, dtdylrs) \
    gO_(G_TEXRECTFLIP,      \
        gF_(lrx,  12, 12) | \
        gF_(lry,  12,  0),  \
        gF_(tile,  3, 24) | \
        gF_(ulx,  12, 12) | \
        gF_(uly,  12,  0)), \
    gO_(0,                  \
        gF_(s, 16, 16) |    \
        gF_(t, 16,  0),     \
        gF_(dsdx, 16, 16) | \
        gF_(dtdy, 16,  0))

#define gsTexRect(ulx, uly, lrx, lry, tile, s, t, dsdx, dtdy) \
    gO_(G_TEXRECT,          \
        gF_(lrx,  12, 12) | \
        gF_(lry,  12,  0),  \
        gF_(tile,  3, 24) | \
        gF_(ulx,  12, 12) | \
        gF_(uly,  12,  0)), \
    gO_(0,                  \
        gF_(s, 16, 16) |    \
        gF_(t, 16,  0),     \
        gF_(dsdx, 16, 16) | \
        gF_(dtdy, 16,  0))

#define gsDPNoOp()  gO_(G_NOOP, 0, 0)

/*
 * Compound static macros
 */

#define gsDPLoadTLUT(count, tmemaddr, dram, loadtile)               \
    gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, dram),      \
    gsDPTileSync(),                                                 \
    gsDPSetTile(0, 0, 0, tmemaddr, loadtile, 0, 0, 0, 0, 0, 0, 0),  \
    gsDPLoadSync(),                                                 \
    gsDPLoadTLUTCmd(loadtile, (count) - 1),                         \
    gsDPPipeSync()

#define gsDPLoadTLUT_pal16(pal, dram, loadtile)                     \
    gsDPLoadTLUT(16, 0x100 + (((pal) & 0xF) * 0x10), dram, loadtile)

#define gsDPLoadTLUT_pal256(dram, loadtile) \
    gsDPLoadTLUT(256, 0x000, dram, loadtile)

/*
 * Dynamic macros
 */
#define gDPSetColorImage(gdl, ...)      gD_(gdl, gsDPSetColorImage, __VA_ARGS__)
#define gDPSetDepthImage(gdl, ...)      gD_(gdl, gsDPSetDepthImage, __VA_ARGS__)
#define gDPSetTextureImage(gdl, ...)    gD_(gdl, gsDPSetTextureImage, __VA_ARGS__)
#define gDPSetCombineLERP(gdl, ...)     gD_(gdl, gsDPSetCombineLERP, __VA_ARGS__)
#define gDPSetEnvColor(gdl, ...)        gD_(gdl, gsDPSetEnvColor, __VA_ARGS__)
#define gDPSetPrimColor(gdl, ...)       gD_(gdl, gsDPSetPrimColor, __VA_ARGS__)
#define gDPSetBlendColor(gdl, ...)      gD_(gdl, gsDPSetBlendColor, __VA_ARGS__)
#define gDPSetFogColor(gdl, ...)        gD_(gdl, gsDPSetFogColor, __VA_ARGS__)
#define gDPSetFillColor(gdl, ...)       gD_(gdl, gsDPSetFillColor, __VA_ARGS__)
#define gDPFillRectangle(gdl, ...)      gD_(gdl, gsDPFillRectangle, __VA_ARGS__)
#define gDPSetTile(gdl, ...)            gD_(gdl, gsDPSetTile, __VA_ARGS__)
#define gDPLoadTile(gdl, ...)           gD_(gdl, gsDPLoadTile, __VA_ARGS__)
#define gDPLoadBlock(gdl, ...)          gD_(gdl, gsDPLoadBlock, __VA_ARGS__)
#define gDPSetTileSize(gdl, ...)        gD_(gdl, gsDPSetTileSize, __VA_ARGS__)
#define gDPLoadTLUTCmd(gdl, ...)        gD_(gdl, gsDPLoadTLUTCmd, __VA_ARGS__)
#define gDPSetOtherMode(gdl, ...)       gD_(gdl, gsDPSetOtherMode, __VA_ARGS__)
#define gDPSetPrimDepth(gdl, ...)       gD_(gdl, gsDPSetPrimDepth, __VA_ARGS__)
#define gDPSetScissorFrac(gdl, ...)     gD_(gdl, gsDPSetScissorFrac, __VA_ARGS__)
#define gDPSetConvert(gdl, ...)         gD_(gdl, gsDPSetConvert, __VA_ARGS__)
#define gDPSetKeyR(gdl, ...)            gD_(gdl, gsDPSetKeyR, __VA_ARGS__)
#define gDPSetKeyGB(gdl, ...)           gD_(gdl, gsDPSetKeyGB, __VA_ARGS__)
#define gDPFullSync(gdl, ...)           gD_(gdl, gsDPFullSync, __VA_ARGS__)
#define gDPTileSync(gdl, ...)           gD_(gdl, gsDPTileSync, __VA_ARGS__)
#define gDPPipeSync(gdl, ...)           gD_(gdl, gsDPPipeSync, __VA_ARGS__)
#define gDPLoadSync(gdl, ...)           gD_(gdl, gsDPLoadSync, __VA_ARGS__)
#define gTexRectFlip(gdl, ...)          gD_(gdl, gsTexRectFlip, __VA_ARGS__)
#define gTexRect(gdl, ...)              gD_(gdl, gsTexRect, __VA_ARGS__)
#define gDPNoOp(gdl, ...)               gD_(gdl, gsDPNoOp, __VA_ARGS__)
/*
 * Compound dynamic macros
 */
#define gDPLoadTLUT(gdl, ...)           gD_(gdl, gsDPLoadTLUT, __VA_ARGS__)
#define gDPLoadTLUT_pal16(gdl, ...)     gD_(gdl, gsDPLoadTLUT_pal16, __VA_ARGS__)
#define gDPLoadTLUT_pal256(gdl, ...)    gD_(gdl, gsDPLoadTLUT_pal256, __VA_ARGS__)



/*
 * G_SETIMG fmt: set image formats
 */
#define G_IM_FMT_RGBA   0
#define G_IM_FMT_YUV    1
#define G_IM_FMT_CI     2
#define G_IM_FMT_IA     3
#define G_IM_FMT_I      4

/*
 * G_SETIMG siz: set image pixel size
 */
#define G_IM_SIZ_4b     0
#define G_IM_SIZ_8b     1
#define G_IM_SIZ_16b    2
#define G_IM_SIZ_32b    3
#define G_IM_SIZ_DD     5

/*
 * G_SETSCISSOR: interlace mode
 */
#define G_SC_NON_INTERLACE  0
#define G_SC_ODD_INTERLACE  3
#define G_SC_EVEN_INTERLACE 2

/* fixed point data types and conversion macros */

typedef uint8_t   qu08_t;
typedef uint16_t  qu016_t;
typedef int16_t   qs48_t;
typedef int16_t   qs510_t;
typedef uint16_t  qu510_t;
typedef int16_t   qs102_t;
typedef uint16_t  qu102_t;
typedef int16_t   qs105_t;
typedef uint16_t  qu105_t;
typedef int16_t   qs132_t;
typedef int16_t   qs142_t;
typedef int32_t   qs1516_t;
typedef int32_t   qs1616_t;
typedef int32_t   qs205_t;

#define qu08(n)     (  (qu08_t)((n) * (1 <<  8)))
#define qu016(n)    ( (qu016_t)((n) * (1 << 16)))
#define qs48(n)     (  (qs48_t)((n) * (1 <<  8)))
#define qs510(n)    ( (qs510_t)((n) * (1 << 10)))
#define qu510(n)    ( (qu510_t)((n) * (1 << 10)))
#define qs102(n)    ( (qs102_t)((n) * (1 <<  2)))
#define qu102(n)    ( (qu102_t)((n) * (1 <<  2)))
#define qs105(n)    ( (qs105_t)((n) * (1 <<  5)))
#define qu105(n)    ( (qu105_t)((n) * (1 <<  5)))
#define qs132(n)    ( (qs132_t)((n) * (1 <<  2)))
#define qs142(n)    ( (qs142_t)((n) * (1 <<  2)))
#define qs1516(n)   ((qs1516_t)((n) * (1 << 16)))
#define qs1616(n)   ((qs1616_t)((n) * (1 << 16)))
#define qs205(n)    ( (qs205_t)((n) * (1 <<  5)))



/*
 * Command builders
 */

#define gI_(i)          ((uint32_t)(i))
#define gL_(l)          ((uint64_t)(l))
#define gF_(i,n,s)      ((gI_(i)&((gI_(1)<<(n))-1))<<(s))
#define gFL_(l,n,s)     ((gL_(l)&((gL_(1)<<(n))-1))<<(s))
#define gO_(opc,hi,lo)  ((Gfx){ gF_(opc,8,24) | gI_(hi) , gI_(lo)})

#define gDisplayListPut(gdl,...)                                    \
    ({                                                              \
        Gfx Gdl__[] = { __VA_ARGS__ };                              \
        for(size_t Gi__=0;Gi__<sizeof(Gdl__)/sizeof(*Gdl__);++Gi__) \
            *(Gfx*)(gdl)=Gdl__[Gi__];                               \
        (void)0;                                                    \
    })

#define gD_(gdl, m, ...) gDisplayListPut(gdl, m(__VA_ARGS__))



/*
 * G_SETOTHERMODE_L sft: shift count
 */
#define G_MDSFT_ALPHACOMPARE    0
#define G_MDSFT_ZSRCSEL         2
#define G_MDSFT_RENDERMODE      3
#define G_MDSFT_BLENDER         16

/*
 * G_SETOTHERMODE_H sft: shift count
 */
#define G_MDSFT_BLENDMASK       0   /* unsupported */
#define G_MDSFT_ALPHADITHER     4
#define G_MDSFT_RGBDITHER       6

#define G_MDSFT_COMBKEY         8
#define G_MDSFT_TEXTCONV        9
#define G_MDSFT_TEXTFILT        12
#define G_MDSFT_TEXTLUT         14
#define G_MDSFT_TEXTLOD         16
#define G_MDSFT_TEXTDETAIL      17
#define G_MDSFT_TEXTPERSP       19
#define G_MDSFT_CYCLETYPE       20
#define G_MDSFT_COLORDITHER     22  /* unsupported in HW 2.0 */
#define G_MDSFT_PIPELINE        23

/* G_SETOTHERMODE_H gPipelineMode */
#define G_PM_1PRIMITIVE     (1 << G_MDSFT_PIPELINE)
#define G_PM_NPRIMITIVE     (0 << G_MDSFT_PIPELINE)

/* G_SETOTHERMODE_H gSetCycleType */
#define G_CYC_1CYCLE        (0 << G_MDSFT_CYCLETYPE)
#define G_CYC_2CYCLE        (1 << G_MDSFT_CYCLETYPE)
#define G_CYC_COPY          (2 << G_MDSFT_CYCLETYPE)
#define G_CYC_FILL          (3 << G_MDSFT_CYCLETYPE)

/* G_SETOTHERMODE_H gSetTexturePersp */
#define G_TP_NONE           (0 << G_MDSFT_TEXTPERSP)
#define G_TP_PERSP          (1 << G_MDSFT_TEXTPERSP)

/* G_SETOTHERMODE_H gSetTextureDetail */
#define G_TD_CLAMP          (0 << G_MDSFT_TEXTDETAIL)
#define G_TD_SHARPEN        (1 << G_MDSFT_TEXTDETAIL)
#define G_TD_DETAIL         (2 << G_MDSFT_TEXTDETAIL)

/* G_SETOTHERMODE_H gSetTextureLOD */
#define G_TL_TILE           (0 << G_MDSFT_TEXTLOD)
#define G_TL_LOD            (1 << G_MDSFT_TEXTLOD)

/* G_SETOTHERMODE_H gSetTextureLUT */
#define G_TT_NONE           (0 << G_MDSFT_TEXTLUT)
#define G_TT_RGBA16         (2 << G_MDSFT_TEXTLUT)
#define G_TT_IA16           (3 << G_MDSFT_TEXTLUT)

/* G_SETOTHERMODE_H gSetTextureFilter */
#define G_TF_POINT          (0 << G_MDSFT_TEXTFILT)
#define G_TF_AVERAGE        (3 << G_MDSFT_TEXTFILT)
#define G_TF_BILERP         (2 << G_MDSFT_TEXTFILT)

/* G_SETOTHERMODE_H gSetTextureConvert */
#define G_TC_CONV           (0 << G_MDSFT_TEXTCONV)
#define G_TC_FILTCONV       (5 << G_MDSFT_TEXTCONV)
#define G_TC_FILT           (6 << G_MDSFT_TEXTCONV)

/* G_SETOTHERMODE_H gSetCombineKey */
#define G_CK_NONE           (0 << G_MDSFT_COMBKEY)
#define G_CK_KEY            (1 << G_MDSFT_COMBKEY)

/* G_SETOTHERMODE_H gSetColorDither */
#define G_CD_MAGICSQ        (0 << G_MDSFT_RGBDITHER)
#define G_CD_BAYER          (1 << G_MDSFT_RGBDITHER)
#define G_CD_NOISE          (2 << G_MDSFT_RGBDITHER)

#ifndef _HW_VERSION_1
#define G_CD_DISABLE        (3 << G_MDSFT_RGBDITHER)
#define G_CD_ENABLE         G_CD_NOISE  /* HW 1.0 compatibility mode */
#else
#define G_CD_ENABLE         (1 << G_MDSFT_COLORDITHER)
#define G_CD_DISABLE        (0 << G_MDSFT_COLORDITHER)
#endif

/* G_SETOTHERMODE_H gSetAlphaDither */
#define G_AD_PATTERN        (0 << G_MDSFT_ALPHADITHER)
#define G_AD_NOTPATTERN     (1 << G_MDSFT_ALPHADITHER)
#define G_AD_NOISE          (2 << G_MDSFT_ALPHADITHER)
#define G_AD_DISABLE        (3 << G_MDSFT_ALPHADITHER)

/* G_SETOTHERMODE_L gSetAlphaCompare */
#define G_AC_NONE           (0 << G_MDSFT_ALPHACOMPARE)
#define G_AC_THRESHOLD      (1 << G_MDSFT_ALPHACOMPARE)
#define G_AC_DITHER         (3 << G_MDSFT_ALPHACOMPARE)

/* G_SETOTHERMODE_L gSetDepthSource */
#define G_ZS_PIXEL          (0 << G_MDSFT_ZSRCSEL)
#define G_ZS_PRIM           (1 << G_MDSFT_ZSRCSEL)

/* G_SETOTHERMODE_L gSetRenderMode */
#define AA_EN           0x0008
#define Z_CMP           0x0010
#define Z_UPD           0x0020
#define IM_RD           0x0040
#define CLR_ON_CVG      0x0080
#define CVG_DST_CLAMP   0x0000
#define CVG_DST_WRAP    0x0100
#define CVG_DST_FULL    0x0200
#define CVG_DST_SAVE    0x0300
#define ZMODE_OPA       0x0000
#define ZMODE_INTER     0x0400
#define ZMODE_XLU       0x0800
#define ZMODE_DEC       0x0C00
#define CVG_X_ALPHA     0x1000
#define ALPHA_CVG_SEL   0x2000
#define FORCE_BL        0x4000
#define TEX_EDGE        0x0000  /* used to be 0x8000 */

#define G_BL_CLR_IN     0
#define G_BL_CLR_MEM    1
#define G_BL_CLR_BL     2
#define G_BL_CLR_FOG    3
#define G_BL_1MA        0
#define G_BL_A_MEM      1
#define G_BL_A_IN       0
#define G_BL_A_FOG      1
#define G_BL_A_SHADE    2
#define G_BL_1          2
#define G_BL_0          3

#define GBL_c1(m1a, m1b, m2a, m2b)  \
    (m1a) << 30 | (m1b) << 26 | (m2a) << 22 | (m2b) << 18
#define GBL_c2(m1a, m1b, m2a, m2b)  \
    (m1a) << 28 | (m1b) << 24 | (m2a) << 20 | (m2b) << 16

#define RM_AA_ZB_OPA_SURF(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_RA_ZB_OPA_SURF(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | CVG_DST_CLAMP |                     \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_ZB_XLU_SURF(clk)                                  \
    AA_EN | Z_CMP | IM_RD | CVG_DST_WRAP | CLR_ON_CVG |         \
    FORCE_BL | ZMODE_XLU |                                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_OPA_DECAL(clk)                                 \
    AA_EN | Z_CMP | IM_RD | CVG_DST_WRAP | ALPHA_CVG_SEL |      \
    ZMODE_DEC |                                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_RA_ZB_OPA_DECAL(clk)                                 \
    AA_EN | Z_CMP | CVG_DST_WRAP | ALPHA_CVG_SEL |              \
    ZMODE_DEC |                                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_ZB_XLU_DECAL(clk)                                 \
    AA_EN | Z_CMP | IM_RD | CVG_DST_WRAP | CLR_ON_CVG |         \
    FORCE_BL | ZMODE_DEC |                                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_OPA_INTER(clk)                                 \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    ALPHA_CVG_SEL | ZMODE_INTER |                               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_RA_ZB_OPA_INTER(clk)                                 \
    AA_EN | Z_CMP | Z_UPD | CVG_DST_CLAMP |                     \
    ALPHA_CVG_SEL | ZMODE_INTER |                               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_ZB_XLU_INTER(clk)                                 \
    AA_EN | Z_CMP | IM_RD | CVG_DST_WRAP | CLR_ON_CVG |         \
    FORCE_BL | ZMODE_INTER |                                    \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_XLU_LINE(clk)                                  \
    AA_EN | Z_CMP | IM_RD | CVG_DST_CLAMP | CVG_X_ALPHA |       \
    ALPHA_CVG_SEL | FORCE_BL | ZMODE_XLU |                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_DEC_LINE(clk)                                  \
    AA_EN | Z_CMP | IM_RD | CVG_DST_SAVE | CVG_X_ALPHA |        \
    ALPHA_CVG_SEL | FORCE_BL | ZMODE_DEC |                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_TEX_EDGE(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    CVG_X_ALPHA | ALPHA_CVG_SEL | ZMODE_OPA | TEX_EDGE |        \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_ZB_TEX_INTER(clk)                                 \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    CVG_X_ALPHA | ALPHA_CVG_SEL | ZMODE_INTER | TEX_EDGE |      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_ZB_SUB_SURF(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_FULL |              \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_ZB_PCL_SURF(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    ZMODE_OPA | G_AC_DITHER |                                   \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_OPA_TERR(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_TEX_TERR(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |             \
    CVG_X_ALPHA | ALPHA_CVG_SEL | ZMODE_OPA | TEX_EDGE |        \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_ZB_SUB_TERR(clk)                                  \
    AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_FULL |              \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)


#define RM_AA_OPA_SURF(clk)                                     \
    AA_EN | IM_RD | CVG_DST_CLAMP |                             \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_RA_OPA_SURF(clk)                                     \
    AA_EN | CVG_DST_CLAMP |                                     \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_XLU_SURF(clk)                                     \
    AA_EN | IM_RD | CVG_DST_WRAP | CLR_ON_CVG | FORCE_BL |      \
    ZMODE_OPA |                                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_XLU_LINE(clk)                                     \
    AA_EN | IM_RD | CVG_DST_CLAMP | CVG_X_ALPHA |               \
    ALPHA_CVG_SEL | FORCE_BL | ZMODE_OPA |                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_DEC_LINE(clk)                                     \
    AA_EN | IM_RD | CVG_DST_FULL | CVG_X_ALPHA |                \
    ALPHA_CVG_SEL | FORCE_BL | ZMODE_OPA |                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_TEX_EDGE(clk)                                     \
    AA_EN | IM_RD | CVG_DST_CLAMP |                             \
    CVG_X_ALPHA | ALPHA_CVG_SEL | ZMODE_OPA | TEX_EDGE |        \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_SUB_SURF(clk)                                     \
    AA_EN | IM_RD | CVG_DST_FULL |                              \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_AA_PCL_SURF(clk)                                     \
    AA_EN | IM_RD | CVG_DST_CLAMP |                             \
    ZMODE_OPA | G_AC_DITHER |                                   \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_OPA_TERR(clk)                                     \
    AA_EN | IM_RD | CVG_DST_CLAMP |                             \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_TEX_TERR(clk)                                     \
    AA_EN | IM_RD | CVG_DST_CLAMP |                             \
    CVG_X_ALPHA | ALPHA_CVG_SEL | ZMODE_OPA | TEX_EDGE |        \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_AA_SUB_TERR(clk)                                     \
    AA_EN | IM_RD | CVG_DST_FULL |                              \
    ZMODE_OPA | ALPHA_CVG_SEL |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)


#define RM_ZB_OPA_SURF(clk)                                     \
    Z_CMP | Z_UPD | CVG_DST_FULL | ALPHA_CVG_SEL |              \
    ZMODE_OPA |                                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_ZB_XLU_SURF(clk)                                     \
    Z_CMP | IM_RD | CVG_DST_FULL | FORCE_BL | ZMODE_XLU |       \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_ZB_OPA_DECAL(clk)                                    \
    Z_CMP | CVG_DST_FULL | ALPHA_CVG_SEL | ZMODE_DEC |          \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define RM_ZB_XLU_DECAL(clk)                                    \
    Z_CMP | IM_RD | CVG_DST_FULL | FORCE_BL | ZMODE_DEC |       \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_ZB_CLD_SURF(clk)                                     \
    Z_CMP | IM_RD | CVG_DST_SAVE | FORCE_BL | ZMODE_XLU |       \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_ZB_OVL_SURF(clk)                                     \
    Z_CMP | IM_RD | CVG_DST_SAVE | FORCE_BL | ZMODE_DEC |       \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_ZB_PCL_SURF(clk)                                     \
    Z_CMP | Z_UPD | CVG_DST_FULL | ZMODE_OPA |                  \
    G_AC_DITHER |                                               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)


#define RM_OPA_SURF(clk)                                        \
    CVG_DST_CLAMP | FORCE_BL | ZMODE_OPA |                      \
    GBL_c##clk(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)

#define RM_XLU_SURF(clk)                                        \
    IM_RD | CVG_DST_FULL | FORCE_BL | ZMODE_OPA |               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_TEX_EDGE(clk)                                        \
    CVG_DST_CLAMP | CVG_X_ALPHA | ALPHA_CVG_SEL | FORCE_BL |    \
    ZMODE_OPA | TEX_EDGE | AA_EN |                              \
    GBL_c##clk(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)

#define RM_CLD_SURF(clk)                                        \
    IM_RD | CVG_DST_SAVE | FORCE_BL | ZMODE_OPA |               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define RM_PCL_SURF(clk)                                        \
    CVG_DST_FULL | FORCE_BL | ZMODE_OPA |                       \
    G_AC_DITHER |                                               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)

#define RM_ADD(clk)                                             \
    IM_RD | CVG_DST_SAVE | FORCE_BL | ZMODE_OPA |               \
    GBL_c##clk(G_BL_CLR_IN, G_BL_A_FOG, G_BL_CLR_MEM, G_BL_1)

#define RM_NOOP(clk)    \
    GBL_c##clk(0, 0, 0, 0)

#define RM_VISCVG(clk)                                          \
    IM_RD | FORCE_BL |                                          \
    GBL_c##clk(G_BL_CLR_IN, G_BL_0, G_BL_CLR_BL, G_BL_A_MEM)

/* for rendering to an 8-bit framebuffer */
#define RM_OPA_CI(clk)                                          \
    CVG_DST_CLAMP | ZMODE_OPA |                                 \
    GBL_c##clk(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)



#define G_RM_AA_ZB_OPA_SURF     RM_AA_ZB_OPA_SURF(1)
#define G_RM_AA_ZB_OPA_SURF2    RM_AA_ZB_OPA_SURF(2)
#define G_RM_AA_ZB_XLU_SURF     RM_AA_ZB_XLU_SURF(1)
#define G_RM_AA_ZB_XLU_SURF2    RM_AA_ZB_XLU_SURF(2)
#define G_RM_AA_ZB_OPA_DECAL    RM_AA_ZB_OPA_DECAL(1)
#define G_RM_AA_ZB_OPA_DECAL2   RM_AA_ZB_OPA_DECAL(2)
#define G_RM_AA_ZB_XLU_DECAL    RM_AA_ZB_XLU_DECAL(1)
#define G_RM_AA_ZB_XLU_DECAL2   RM_AA_ZB_XLU_DECAL(2)
#define G_RM_AA_ZB_OPA_INTER    RM_AA_ZB_OPA_INTER(1)
#define G_RM_AA_ZB_OPA_INTER2   RM_AA_ZB_OPA_INTER(2)
#define G_RM_AA_ZB_XLU_INTER    RM_AA_ZB_XLU_INTER(1)
#define G_RM_AA_ZB_XLU_INTER2   RM_AA_ZB_XLU_INTER(2)
#define G_RM_AA_ZB_XLU_LINE     RM_AA_ZB_XLU_LINE(1)
#define G_RM_AA_ZB_XLU_LINE2    RM_AA_ZB_XLU_LINE(2)
#define G_RM_AA_ZB_DEC_LINE     RM_AA_ZB_DEC_LINE(1)
#define G_RM_AA_ZB_DEC_LINE2    RM_AA_ZB_DEC_LINE(2)
#define G_RM_AA_ZB_TEX_EDGE     RM_AA_ZB_TEX_EDGE(1)
#define G_RM_AA_ZB_TEX_EDGE2    RM_AA_ZB_TEX_EDGE(2)
#define G_RM_AA_ZB_TEX_INTER    RM_AA_ZB_TEX_INTER(1)
#define G_RM_AA_ZB_TEX_INTER2   RM_AA_ZB_TEX_INTER(2)
#define G_RM_AA_ZB_SUB_SURF     RM_AA_ZB_SUB_SURF(1)
#define G_RM_AA_ZB_SUB_SURF2    RM_AA_ZB_SUB_SURF(2)
#define G_RM_AA_ZB_PCL_SURF     RM_AA_ZB_PCL_SURF(1)
#define G_RM_AA_ZB_PCL_SURF2    RM_AA_ZB_PCL_SURF(2)
#define G_RM_AA_ZB_OPA_TERR     RM_AA_ZB_OPA_TERR(1)
#define G_RM_AA_ZB_OPA_TERR2    RM_AA_ZB_OPA_TERR(2)
#define G_RM_AA_ZB_TEX_TERR     RM_AA_ZB_TEX_TERR(1)
#define G_RM_AA_ZB_TEX_TERR2    RM_AA_ZB_TEX_TERR(2)
#define G_RM_AA_ZB_SUB_TERR     RM_AA_ZB_SUB_TERR(1)
#define G_RM_AA_ZB_SUB_TERR2    RM_AA_ZB_SUB_TERR(2)

#define G_RM_RA_ZB_OPA_SURF     RM_RA_ZB_OPA_SURF(1)
#define G_RM_RA_ZB_OPA_SURF2    RM_RA_ZB_OPA_SURF(2)
#define G_RM_RA_ZB_OPA_DECAL    RM_RA_ZB_OPA_DECAL(1)
#define G_RM_RA_ZB_OPA_DECAL2   RM_RA_ZB_OPA_DECAL(2)
#define G_RM_RA_ZB_OPA_INTER    RM_RA_ZB_OPA_INTER(1)
#define G_RM_RA_ZB_OPA_INTER2   RM_RA_ZB_OPA_INTER(2)

#define G_RM_AA_OPA_SURF    RM_AA_OPA_SURF(1)
#define G_RM_AA_OPA_SURF2   RM_AA_OPA_SURF(2)
#define G_RM_AA_XLU_SURF    RM_AA_XLU_SURF(1)
#define G_RM_AA_XLU_SURF2   RM_AA_XLU_SURF(2)
#define G_RM_AA_XLU_LINE    RM_AA_XLU_LINE(1)
#define G_RM_AA_XLU_LINE2   RM_AA_XLU_LINE(2)
#define G_RM_AA_DEC_LINE    RM_AA_DEC_LINE(1)
#define G_RM_AA_DEC_LINE2   RM_AA_DEC_LINE(2)
#define G_RM_AA_TEX_EDGE    RM_AA_TEX_EDGE(1)
#define G_RM_AA_TEX_EDGE2   RM_AA_TEX_EDGE(2)
#define G_RM_AA_SUB_SURF    RM_AA_SUB_SURF(1)
#define G_RM_AA_SUB_SURF2   RM_AA_SUB_SURF(2)
#define G_RM_AA_PCL_SURF    RM_AA_PCL_SURF(1)
#define G_RM_AA_PCL_SURF2   RM_AA_PCL_SURF(2)
#define G_RM_AA_OPA_TERR    RM_AA_OPA_TERR(1)
#define G_RM_AA_OPA_TERR2   RM_AA_OPA_TERR(2)
#define G_RM_AA_TEX_TERR    RM_AA_TEX_TERR(1)
#define G_RM_AA_TEX_TERR2   RM_AA_TEX_TERR(2)
#define G_RM_AA_SUB_TERR    RM_AA_SUB_TERR(1)
#define G_RM_AA_SUB_TERR2   RM_AA_SUB_TERR(2)

#define G_RM_RA_OPA_SURF    RM_RA_OPA_SURF(1)
#define G_RM_RA_OPA_SURF2   RM_RA_OPA_SURF(2)

#define G_RM_ZB_OPA_SURF    RM_ZB_OPA_SURF(1)
#define G_RM_ZB_OPA_SURF2   RM_ZB_OPA_SURF(2)
#define G_RM_ZB_XLU_SURF    RM_ZB_XLU_SURF(1)
#define G_RM_ZB_XLU_SURF2   RM_ZB_XLU_SURF(2)
#define G_RM_ZB_OPA_DECAL   RM_ZB_OPA_DECAL(1)
#define G_RM_ZB_OPA_DECAL2  RM_ZB_OPA_DECAL(2)
#define G_RM_ZB_XLU_DECAL   RM_ZB_XLU_DECAL(1)
#define G_RM_ZB_XLU_DECAL2  RM_ZB_XLU_DECAL(2)
#define G_RM_ZB_CLD_SURF    RM_ZB_CLD_SURF(1)
#define G_RM_ZB_CLD_SURF2   RM_ZB_CLD_SURF(2)
#define G_RM_ZB_OVL_SURF    RM_ZB_OVL_SURF(1)
#define G_RM_ZB_OVL_SURF2   RM_ZB_OVL_SURF(2)
#define G_RM_ZB_PCL_SURF    RM_ZB_PCL_SURF(1)
#define G_RM_ZB_PCL_SURF2   RM_ZB_PCL_SURF(2)

#define G_RM_OPA_SURF       RM_OPA_SURF(1)
#define G_RM_OPA_SURF2      RM_OPA_SURF(2)
#define G_RM_XLU_SURF       RM_XLU_SURF(1)
#define G_RM_XLU_SURF2      RM_XLU_SURF(2)
#define G_RM_CLD_SURF       RM_CLD_SURF(1)
#define G_RM_CLD_SURF2      RM_CLD_SURF(2)
#define G_RM_TEX_EDGE       RM_TEX_EDGE(1)
#define G_RM_TEX_EDGE2      RM_TEX_EDGE(2)
#define G_RM_PCL_SURF       RM_PCL_SURF(1)
#define G_RM_PCL_SURF2      RM_PCL_SURF(2)
#define G_RM_ADD            RM_ADD(1)
#define G_RM_ADD2           RM_ADD(2)
#define G_RM_NOOP           RM_NOOP(1)
#define G_RM_NOOP2          RM_NOOP(2)
#define G_RM_VISCVG         RM_VISCVG(1)
#define G_RM_VISCVG2        RM_VISCVG(2)
#define G_RM_OPA_CI         RM_OPA_CI(1)
#define G_RM_OPA_CI2        RM_OPA_CI(2)


#define G_RM_FOG_SHADE_A    GBL_c1(G_BL_CLR_FOG, G_BL_A_SHADE, G_BL_CLR_IN, G_BL_1MA)
#define G_RM_FOG_PRIM_A     GBL_c1(G_BL_CLR_FOG, G_BL_A_FOG, G_BL_CLR_IN, G_BL_1MA)
#define G_RM_PASS           GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)

/*
 * G_SETCOMBINE: color combine modes
 */
/* Color combiner constants: */
#define G_CCMUX_COMBINED        0
#define G_CCMUX_TEXEL0          1
#define G_CCMUX_TEXEL1          2
#define G_CCMUX_PRIMITIVE       3
#define G_CCMUX_SHADE           4
#define G_CCMUX_ENVIRONMENT     5
#define G_CCMUX_CENTER          6
#define G_CCMUX_SCALE           6
#define G_CCMUX_COMBINED_ALPHA  7
#define G_CCMUX_TEXEL0_ALPHA    8
#define G_CCMUX_TEXEL1_ALPHA    9
#define G_CCMUX_PRIMITIVE_ALPHA 10
#define G_CCMUX_SHADE_ALPHA     11
#define G_CCMUX_ENV_ALPHA       12
#define G_CCMUX_LOD_FRACTION    13
#define G_CCMUX_PRIM_LOD_FRAC   14
#define G_CCMUX_NOISE           7
#define G_CCMUX_K4              7
#define G_CCMUX_K5              15
#define G_CCMUX_1               6
#define G_CCMUX_0               31

/* Alpha combiner constants: */
#define G_ACMUX_COMBINED        0
#define G_ACMUX_TEXEL0          1
#define G_ACMUX_TEXEL1          2
#define G_ACMUX_PRIMITIVE       3
#define G_ACMUX_SHADE           4
#define G_ACMUX_ENVIRONMENT     5
#define G_ACMUX_LOD_FRACTION    0
#define G_ACMUX_PRIM_LOD_FRAC   6
#define G_ACMUX_1               6
#define G_ACMUX_0               7

#endif
