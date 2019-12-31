//
// Created by alex on 12/30/19.
//

#ifndef SPHERE_VIS_FFT_H
#define SPHERE_VIS_FFT_H

#include <array>
#include <complex>

template <size_t N>
std::array<std::complex<float>, N> fft(
    std::array<std::complex<float>, N> input
)
{
    {
        int k = N;
        float theta = M_PI / N;
        std::complex<float> phi_t{cosf(theta), -sinf(theta)};
        while(k > 1)
        {
            int n = k;
            k /= 2;
            phi_t *= phi_t;
            std::complex<float> t = 1.f;
            for(int i = 0; i < k; i++)
            {
                for(int j = i; j < N; j += n)
                {
                    int b = j + k;
                    auto x = input[j] - input[b];
                    input[j] += input[b];
                    input[b] = x * t;
                }
                t *= phi_t;
            }
        }
    }

    unsigned int m = -1;
    for(int i = N; i > 0; i /= 2)
        m++;

    for(int i = 0; i < N; i++)
    {
        unsigned int j = i;

        j = (((j & 0xaaaaaaaau) >> 1u) | ((j & 0x55555555u) << 1u));
        j = (((j & 0xccccccccu) >> 2u) | ((j & 0x33333333u) << 2u));
        j = (((j & 0xf0f0f0f0u) >> 4u) | ((j & 0x0f0f0f0fu) << 4u));
        j = (((j & 0xff00ff00u) >> 8u) | ((j & 0x00ff00ffu) << 8u));
        j = ((j >> 16u) | (j << 16u)) >> (32 - m);

        if(j > i)
            std::swap(input[i], input[j]);
    }

    return input;
}

#endif //SPHERE_VIS_FFT_H
