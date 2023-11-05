#!/usr/bin/env python3
#
#   Reads results.txt containing debug prints emitted by rdp_fill_timing and
#   collects results
#

import os
import numpy as np
import matplotlib.pyplot as plt

# Whether to plot the result data
DO_PLOTS = False
FILENAME = "results.txt"

def rdp_clk_to_ms(clk):
    return clk / 62500

contents = None
with open(FILENAME, "r") as infile:
    contents = infile.read()

contents = contents.split("!!BEGIN!!")[1].split("!!DONE!!")[0].strip()

data_segments = []
for i,line in enumerate(contents.split("\n")):
    if i % 7 == 0:
        data_segments.append([])
    data_segments[-1].append(line)

for i,seg in enumerate(data_segments):
    desc = seg[0]
    assert seg[1] == "BUF = ["
    buf_data = seg[2].strip()
    assert seg[3] == "]"
    assert seg[4] == "PIPE = ["
    pipe_data = seg[5].strip()
    assert seg[6] == "]"
    assert len(seg) == 7

    assert buf_data[-1] == ","
    assert pipe_data[-1] == ","
    buf_data = buf_data[:-1]
    pipe_data = pipe_data[:-1]

    buf_data = [int(buf) for buf in buf_data.split(", ")]
    pipe_data = [int(pipe) for pipe in pipe_data.split(", ")]

    assert len(buf_data) == len(pipe_data)

    # Plot results including outliers
    if DO_PLOTS:
        os.makedirs("figures/outliers", exist_ok=True)
        plt.title(desc)
        plt.plot(range(len(buf_data)), buf_data)
        plt.savefig(f"figures/outliers/fig{i}_outliers.png")
        plt.clf()
        plt.cla()

    orig_num = len(buf_data)

    # Identify outliers (<1% or >99%)
    bmin = np.quantile(buf_data, 0.01)
    bmax = np.quantile(buf_data, 0.99)
    pmin = np.quantile(pipe_data, 0.01)
    pmax = np.quantile(pipe_data, 0.99)

    # Prune outliers
    buf_data = [buf for buf in buf_data if bmin <= buf <= bmax]
    pipe_data = [pipe for pipe in pipe_data if pmin <= pipe <= pmax]

    # Plot results without outliers
    if DO_PLOTS:
        plt.title(desc)
        plt.plot(range(len(buf_data)), buf_data)
        plt.savefig(f"figures/fig{i}.png")
        plt.clf()
        plt.cla()

    # Collect results
    min_buf = rdp_clk_to_ms(min(buf_data))
    avg_buf = rdp_clk_to_ms(sum(buf_data) / len(buf_data))
    max_buf = rdp_clk_to_ms(max(buf_data))
    min_pipe = rdp_clk_to_ms(min(pipe_data))
    avg_pipe = rdp_clk_to_ms(sum(pipe_data) / len(pipe_data))
    max_pipe = rdp_clk_to_ms(max(pipe_data))

    # Print aggregate statistics
    print(desc)
    print(f"    Buf:  pruned {orig_num - len(buf_data)} outliers")
    print(f"    Pipe: pruned {orig_num - len(pipe_data)} outliers")
    print(f"    Buf result:  {min_buf:.07f}, {avg_buf:.07f}, {max_buf:.07f}")
    print(f"    Pipe result: {min_pipe:.07f}, {avg_pipe:.07f}, {max_pipe:.07f}")
