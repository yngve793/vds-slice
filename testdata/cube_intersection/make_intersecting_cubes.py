import segyio
import numpy as np
import sys
import subprocess


def create_intersecting_data(path, samples, ilines, xlines):

    print()
    print(f"samples {samples}")
    print(f"ilines  {ilines}")
    print(f"xlines  {xlines}")

    spec = segyio.spec()

    spec.sorting = 2
    spec.format = 1
    spec.samples = samples
    spec.ilines = ilines
    spec.xlines = xlines

    # Set in_line vector to (3, 2)
    il_step_x = int(3 * 10)
    il_step_y = int(2 * 10)

    # Set cross_line vector to (-2, 3)
    xl_step_x = int(-2 * 10)
    xl_step_y = int(3 * 10)

    # Set origin to (2,0)
    ori_x = int(2 * 10)
    ori_y = int(0 * 10)

    with segyio.create(path + ".segy", spec) as f:
        tr = 0

        for iline_index, iline_value in enumerate(ilines):
            for xline_index, xline_value in enumerate(xlines):
                offset_value = iline_index*2**8 + xline_index*2**4
                print(f"Iline index: {iline_index}, value {iline_index*2**8}")
                print(f"Xline index: {xline_index}, value {xline_index*2**4}")
                print(f"Combined {offset_value} hex: {hex(offset_value)}")

                f.header[tr] = {
                    segyio.su.iline: iline_value,
                    segyio.su.xline: xline_value,
                    segyio.su.cdpx:
                        ori_x +
                        (iline_value - spec.ilines[0]) * il_step_x +
                        (xline_value - spec.xlines[0]) * xl_step_x,
                    segyio.su.cdpy:
                        ori_y +
                        (iline_value - spec.ilines[0]) * il_step_y +
                        (xline_value - spec.xlines[0]) * xl_step_y,
                    segyio.su.scalco: -10,
                    segyio.su.delrt: samples[0],
                }

                f.trace[tr] = samples + \
                    ((iline_value * 2**(16)) + (xline_value * 2**(8)))
                tr += 1

        f.bin.update(tsort=segyio.TraceSortingFormat.INLINE_SORTING)


if __name__ == "__main__":
    base_range = np.arange(1, 8+1)

    print(base_range)
    print(base_range * 3)
    print(base_range + 3)
    parameters = [
        {"path": "regular_8x3_cube",
         "samples": (0+base_range)*4,
         "ilines": (0+base_range)*3,
         "xlines": (0+base_range)*2},
        {"path": "shift_4_8x3_cube",
         "samples": (4+base_range)*4,
         "ilines": (4+base_range)*3,
         "xlines": (4+base_range)*2},
        {"path": "shift_minus4_8x3_cube",
         "samples": (-4+base_range)*4,
         "ilines": (-4+base_range)*3,
         "xlines": (-4+base_range)*2},
    ]

    for p in parameters:
        create_intersecting_data(**p)
        name = p["path"]
        subprocess.run(["SEGYImport", "--url", "file://.", "--vdsfile",
                       name+".vds", name+".segy", "--crs-wkt=\"utmXX\""])
        subprocess.run(["rm", name+".segy"])
