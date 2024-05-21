import segyio
import numpy as np
import codecs
import json
from scipy.signal import hilbert as hilbert_transform

# Top surface at sample 2000 with stepsize 4 (Zero index)
top_surface_index = 500-1

# Primary surface at sample 2400 with stepsize 4 relative to top surface
primary_relative_index = 100

# Bottom surface at sample 3000 with stepsize 4
bottom_surface_index = 750


with segyio.open('penobscot_xl1155.sgy') as f:

    # 1155 Is the only crossLine in the original file
    line = f.xline[1155]

    seismic = line[:, top_surface_index:bottom_surface_index]

phase = np.zeros(seismic.shape)
envelope = np.zeros(seismic.shape)
hilbert = np.zeros(seismic.shape)

for i in range(seismic.shape[0]):
    phase[i, :] = np.angle(hilbert_transform(seismic[i, :]))
    envelope[i, :] = np.abs(hilbert_transform(seismic[i, :]))
    hilbert[i, :] = np.imag(hilbert_transform(seismic[i, :]))

expected = {}
expected["value"] = seismic[:, primary_relative_index].tolist()
expected["phase"] = phase[:, primary_relative_index].tolist()
expected["envelope"] = envelope[:, primary_relative_index].tolist()
expected["hilbert"] = hilbert[:, primary_relative_index].tolist()

json.dump(expected, codecs.open("penobscot_expected.json", 'w', encoding='utf-8'),
          separators=(',', ':'),
          sort_keys=True,
          indent=4)


# primary_relative_index = primary_surface_idex - top_surface_index
# json.dump(hilbert[:, primary_relative_index].tolist(), codecs.open("hilbert.json", 'w', encoding='utf-8'),
#           separators=(',', ':'),
#           sort_keys=True,
#           indent=4)

# json.dump(envelope[:, primary_relative_index].tolist(), codecs.open("envelope.json", 'w', encoding='utf-8'),
#           separators=(',', ':'),
#           sort_keys=True,
#           indent=4)

# json.dump(phase[:, primary_relative_index].tolist(), codecs.open("phase.json", 'w', encoding='utf-8'),
#           separators=(',', ':'),
#           sort_keys=True,
#           indent=4)
