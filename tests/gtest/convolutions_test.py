import numpy as np
import scipy

def generate_signal(n:int):
    x = np.random.rand(n)
    w = 2*np.pi/n
    for m_i in range(x.shape[0]):
        x[m_i] = np.sin(w*12*m_i) + np.cos(w*19*m_i)
    return x

if __name__ == "__main__":
    
    data = generate_signal(143)
    fft_result = np.fft.fft(data)

    # Produce expected data for convolution_fft_test
    print()
    print("Generated expected output for fft test")
    print("{")
    for i,v in enumerate(fft_result):
        if i < fft_result.shape[0]-1:
            print(f"{{{np.real(v)},{np.imag(v)}}},")
        else:
            print(f"{{{np.real(v)},{np.imag(v)}}}")
    print("}")

    hilbert_result = scipy.signal.hilbert(data)

    # Produce expected data for convolution_hilbert_test
    print()
    print("Generated expected output for hilbert test")
    print("{")
    for i,v in enumerate(hilbert_result):
        if i < hilbert_result.shape[0]-1:
            print(f"{{{np.real(v)},{np.imag(v)}}},")
        else:
            print(f"{{{np.real(v)},{np.imag(v)}}}")
    print("}")


