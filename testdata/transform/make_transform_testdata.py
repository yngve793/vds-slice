import numpy as np
import scipy
import codecs
import json
import matplotlib.pyplot as plt


def generate_signal(n: int, f1: float, f2: float):
    x = np.random.rand(n)
    w = 2*np.pi/n
    for m_i in range(x.shape[0]):
        x[m_i] = np.sin(w * f1 * m_i) + np.cos(w * f2 * m_i)
    return x


if __name__ == "__main__":

    # Setup test data basic
    f1 = 12
    f2 = 19
    expected = {}
    expected["f1"] = f1
    expected["f2"] = f2

    # Generate odd length signal
    n = 143
    data = generate_signal(n, f1, f2)
    fft_result = np.fft.fft(data)
    hilbert_result = scipy.signal.hilbert(data)
    expected["input_data_143"] = data.tolist()
    expected["fft_result_real_143"] = np.real(fft_result).tolist()
    expected["fft_result_imag_143"] = np.imag(fft_result).tolist()
    expected["hilbert_result_real_143"] = np.real(hilbert_result).tolist()
    expected["hilbert_result_imag_143"] = np.imag(hilbert_result).tolist()

    # Generate even length signal
    n = 144
    data = generate_signal(n, f1, f2)
    fft_result = np.fft.fft(data)
    hilbert_result = scipy.signal.hilbert(data)
    expected["input_data_144"] = data.tolist()
    expected["fft_result_real_144"] = np.real(fft_result).tolist()
    expected["fft_result_imag_144"] = np.imag(fft_result).tolist()
    expected["hilbert_result_real_144"] = np.real(hilbert_result).tolist()
    expected["hilbert_result_imag_144"] = np.imag(hilbert_result).tolist()

    json.dump(
        expected,
        codecs.open("transform_expected.json", 'w', encoding='utf-8'),
        separators=(',', ':'),
        sort_keys=True,
        indent=4)

    # Create a signal figure
    fig, axs = plt.subplots(3)
    fig.suptitle("Signal = 12Hz(sin) + 19Hz(cos)")

    axs[0].plot(20*np.log10(np.abs(fft_result)))
    axs[0].set_ylabel("Magnitude [dB]")

    axs[1].plot(np.real(fft_result), "blue")
    axs[1].plot(np.imag(fft_result), "red")
    axs[1].set_ylabel("Magnitude")
    axs[1].legend(["Real", "Imag"])
    axs[1].set_xlabel("Frequency [Positive + Negative]")

    axs[2].plot(np.real(hilbert_result), "blue")
    axs[2].plot(np.imag(hilbert_result), "red")
    axs[2].set_xlabel("Hilbert transform")

    plt.savefig("transform_test_signal.png")
