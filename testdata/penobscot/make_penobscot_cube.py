import segyio
import numpy as np

with segyio.open('penobscot_xl1155.sgy') as src:
    spec = segyio.spec()
    spec.sorting = src.sorting
    spec.format = src.format
    spec.samples = src.samples
    spec.ilines = src.ilines
    spec.xlines = np.append(src.xlines, src.xlines[0] + 1)
    data = src.xline[1155]

# We use scaling constant of -10, meaning that values will be divided by 10
il_step_x = int(3 * 10)
il_step_y = int(2 * 10)
xl_step_x = int(-2 * 10)
xl_step_y = int(3 * 10)
ori_x = int(2 * 10)
ori_y = int(0 * 10)

with segyio.create("penobscot_xl1155_x2.sgy", spec) as f:

    tr = 0
    for il in spec.ilines:
        for xl in spec.xlines:
            f.header[tr] = {
                segyio.su.iline: il,
                segyio.su.xline: xl,
                segyio.su.cdpx:
                    (il - spec.ilines[0]) * il_step_x +
                    (xl - spec.xlines[0]) * xl_step_x +
                    ori_x,
                segyio.su.cdpy:
                    (il - spec.ilines[0]) * il_step_y +
                    (xl - spec.xlines[0]) * xl_step_y +
                    ori_y,
                segyio.su.scalco: -10,
                segyio.su.delrt: 4,
            }
            tr_il = int(tr / 2)
            if tr == tr_il*2:
                f.trace[tr] = data[tr_il]
            else:
                f.trace[tr] = data[tr_il] + 0.0
            tr += 1

    f.bin.update(tsort=segyio.TraceSortingFormat.INLINE_SORTING)
