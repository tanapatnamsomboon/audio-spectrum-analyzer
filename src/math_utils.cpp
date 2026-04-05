#include "math_utils.h"
#include <complex>
#include <cmath>

namespace math {

std::vector<float> calculate_dft(const std::vector<float>& input) {
    int input_size = input.size();
    std::vector<float> magnitudes(input_size / 2);

    for (int k = 0; k < input_size / 2; ++k) {
        std::complex<float> sum(0, 0);
        for (int n = 0; n < input_size; ++n) {
            float angle = 2.0f * M_PI * k * n / input_size;
            std::complex<float> exponent(cos(angle), -sin(angle));
            sum += std::complex<float>(input[n], 0) * exponent;
        }
        magnitudes[k] = std::abs(sum);
    }
    return magnitudes;
}

} // namespace math
