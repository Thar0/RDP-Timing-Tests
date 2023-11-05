# RDP Timing Tests

This repository contains a series of tests for timing the RDP under various conditions.

Every test renders a 320x240 fill rectangle to an rgba16 framebuffer 1000 times and measures how long each individual fillrect takes to execute with the RDP PIPEBUSY and BUFBUSY counter registers. These registers start and stop counting with the RDP producing precise timing results. From these timing results the minimum, average and maximum times over the 1000 runs are extracted.

There are 100 tests, each timing various combinations of:
- RDP cycle type (1-cycle or 2-cycle per pixel)
- Framebuffer read on/off (image_read/IM_RD in othermodes)
- Z-Buffer read on/off (z_compare/Z_CMP in othermodes)
- Z-Buffer write on/off (z_update/Z_UPD in othermodes)
- Z-Buffer test pass/fail (only meaningful with z_compare on)
- Alpha compare on/off (when on, every pixel is set to fail)
- VI on/off
- Framebuffer/Z-Buffer/VI in various RDRAM banks

A sample of results obtained during a run can be found in [sample_results.txt](sample_results.txt).

Some comments on the various tests:
- Framebuffer read and Z-Buffer read/write ON increases how often the RDP has to reach into RDRAM. The more of these that are switched on, the longer the RDP tends to take as it stalls more often waiting for RDRAM contents to arrive. The relative impact of these modes are seen in the test results.
- Z-Buffer fail allows the RDP to skip writing color/depth back to RDRAM, the tests aimed to show the difference between these cases. Z-Buffer Pass/fail can have a noticeable performance impact, applications should strive to render near-to-far when Z-Buffering is enabled.
- Alpha compare ON (and failing) also allows the RDP to skip writing color/depth, and can do so more efficiently as it does not need to enable depth reading. The fastest possible timing is with alpha compare failing all pixels, where framebuffer read and z-buffer read are both disabled. The RDP pipeline is allowed to run freely without ever stalling for RDRAM.
- VI on/off tests are intended to illustrate the effect of memory bandwidth. RDP performance drops when the VI is on as it must share RDRAM access with other components. The VI is a particularly good test as it uses a lot of memory bandwidth at a higher priority than the RDP (that is, if the RDP and VI both ask for memory at the same time, RDRAM will service the VI request first).
- Varying RDRAM bank configurations illustrates the importance of making the most of RDRAM row caching. Every 1MB RDRAM bank is made up of 0x800 byte rows, one of these rows is designated as the currently open row and accesses to this row are fast. Whenever data from a different row is needed the currently open row is closed and the new row is opened, resulting in delays. Putting each buffer in their own RDRAM bank can cause significant speedups as the frequency at which rows have to be reopened decreases. The RDP memory transactions are in order of color read, depth read, color write, depth write; if the framebuffer and z-buffer were in the same bank, this causes rows to be closed and reopened several times per ~8 pixels (the RDP buffers about 8 rgba16 pixels in a span buffer and performs memory transactions in bulk for all of these pixels at once). The test results confirm that it is important to consider the effects of RDRAM banking for these buffers.
- The 1/2 cycle mode tests show a number of interesting features. Tests show that without any RDRAM accesses 2-cycle mode is almost exactly twice as slow as 1-cycle mode. However when RDRAM access is considered, 2-cycle mode rapidly catches up to 1-cycle mode and in some cases even appears to perform faster. 2-cycle and 1-cycle modes are essentially equal in speed when Z-buffering is enabled and more pixels are passing the Z-Buffer test than not. The reason why 2-cycle appears to be faster than 1-cycle in some cases is not yet fully understood.
- The effect of the force_blend (FORCE_BL) othermode bit is NOT tested here. Preliminary tests showed that this mode bit does not affect timings. That is, when force_blend is disabled the RDP will still perform framebuffer reads even when there are no edge pixels (that is pixels with partial coverage) in the current span segment.

## Building

To build, install [libdragon](https://github.com/dragonminded/libdragon) (if not done already) and run `make`.

## Running

Run `make run` with an N64 console powered on and ready to receive a ROM over USB. This will produce a file `results.txt` in the root of the project. Then run `analyze.py` to collect the min/average/max timings for each test.

Currently `client.py` only supports Everdrive X7, as this is the only flashcart owned by the author; however it is hoped that it is not too difficult to add support for other flashcarts if desired, provided it has support in the UNFLoader USB library used by libdragon.
