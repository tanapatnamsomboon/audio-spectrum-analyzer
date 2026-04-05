#pragma once

#include <vector>

namespace audio {

bool init();
void cleanup();

std::vector<float> get_latest_samples();

} // namespace audio
