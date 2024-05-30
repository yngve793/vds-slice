import segyio
import numpy as np
import codecs
import json

# See scipy.signal.hilbert for referenced


def hilbert_np_fft(x, N=None, axis=-1):

    x = np.asarray(x)
    if np.iscomplexobj(x):
        raise ValueError("x must be real.")
    if N is None:
        N = x.shape[axis]
    if N <= 0:
        raise ValueError("N must be positive.")

    Xf = np.fft.fft(x, N, axis=axis)
    h = np.zeros(N, dtype=Xf.dtype)
    if N % 2 == 0:
        h[0] = h[N // 2] = 1
        h[1:N // 2] = 2
    else:
        h[0] = 1
        h[1:(N + 1) // 2] = 2

    if x.ndim > 1:
        ind = [np.newaxis] * x.ndim
        ind[axis] = slice(None)
        h = h[tuple(ind)]
    x = np.fft.ifft(Xf * h, axis=axis)
    return x


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
    phase[i, :] = np.angle(hilbert_np_fft(seismic[i, :]))
    envelope[i, :] = np.abs(hilbert_np_fft(seismic[i, :]))
    hilbert[i, :] = np.imag(hilbert_np_fft(seismic[i, :]))

expected = {}
expected["value"] = seismic[:, primary_relative_index].tolist()
expected["value_row0"] = seismic[0, :].tolist()
expected["hilbert_row0"] = hilbert[0, :].tolist()
expected["phase"] = phase[:, primary_relative_index].tolist()
expected["envelope"] = envelope[:, primary_relative_index].tolist()
expected["hilbert"] = hilbert[:, primary_relative_index].tolist()

json.dump(
    expected,
    codecs.open("penobscot_expected.json", 'w', encoding='utf-8'),
    separators=(',', ':'),
    sort_keys=True,
    indent=4)
