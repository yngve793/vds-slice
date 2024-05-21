
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

# f.close()
# src.close()


# import segyio
# import numpy as np
# import sys
# import subprocess


# def create_vds_file(path, samples, ilines, xlines, data):

#     spec = segyio.spec()
#     spec.sorting = 2
#     spec.format = 1
#     spec.samples = samples
#     spec.ilines = ilines
#     spec.xlines = xlines

#     iline_stepsize = ilines[1] - ilines[0]
#     xlines_stepsize = xlines[1] - xlines[0]
#     samples_stepsize = samples[1] - samples[0]

#     print("stepsize", iline_stepsize, xlines_stepsize, samples_stepsize)


#     # Set in_line step size for Annotated to CDP
#     annotated_to_cdp_inline_step = np.array([3, 2])

#     # Set cross_line step size for Annotated to CDP
#     annotated_to_cdp_xline_step = np.array([-2, 3])

#     # Set CDP origin index coordinates
#     index_origin_cdp = np.array([2, 0])

#     with segyio.create(path + ".segy", spec) as f:
#         tr = 0

#         for iline_value in ilines:
#             for xline_value in xlines:

#                 il = iline_value- spec.ilines[0]
#                 xl = xline_value- spec.xlines[0]

#                 cdp_coordinate = 10 * (index_origin_cdp +
#                                        il * annotated_to_cdp_inline_step +
#                                        xl * annotated_to_cdp_xline_step)
#                 f.header[tr] = {
#                     segyio.su.iline: iline_value,
#                     segyio.su.xline: xline_value,
#                     segyio.su.cdpx: int(cdp_coordinate[0]),
#                     segyio.su.cdpy: int(cdp_coordinate[1]),
#                     segyio.su.scalco: -10,
#                     segyio.su.delrt: 4,
#                 }

#                 if xl == 0:
#                     f.trace[tr] = data[il,:].tolist()
#                 else:
#                     f.trace[tr] = (data[il,:] + 0.1).tolist()
#                 if xl == 0:
#                     print(il, xl, f.trace[tr][600:610], data[il,:].shape)
#                 tr += 1

#         f.bin.update(tsort=segyio.TraceSortingFormat.INLINE_SORTING)


# if __name__ == "__main__":

#     with segyio.open('penobscot_xl1155.sgy') as f:

#         xlines = f.xlines.astype(np.int32)
#         ilines = f.ilines.astype(np.int32)
#         samples = f.samples.astype(np.int32)
#         data = f.xline[f.xlines[0]].astype(np.float32)

#     # Append extra xline
#     xlines = np.append(xlines, xlines[0] +1)

#     print(xlines, xlines.shape)
#     print(ilines, ilines.shape)
#     print(samples, samples.shape)
#     print(data.shape)
#     print(data[300:310,700:710])

#     parameters = [
#         {"path": "penobscot_xl1155",
#          "samples": samples,
#          "ilines": ilines,
#          "xlines": xlines,
#          "data" : data}
#     ]

#     for p in parameters:
#         create_vds_file(**p)
#         name = p["path"]
#         subprocess.run(["SEGYImport", "--url", "file://.", "--vdsfile",
#                        name+".vds", name+".segy", "--crs-wkt=utmXX"])
#         subprocess.run(["rm", name+".segy"])

# # Copy to cloud require a fresh sas token.
# # VDSCopy "regular_8x3_cube.vds"  "azureSAS://onevdstest.blob.core.windows.net/testdata/cube_intersection/regular_8x3_cube"  --compression-method=None -d "Suffix=?$SAS"
# # VDSCopy "shift_4_8x3_cube.vds"  "azureSAS://onevdstest.blob.core.windows.net/testdata/cube_intersection/shift_4_8x3_cube"  --compression-method=None -d "Suffix=?$SAS"
