#include "phaser/backend/correlation/spatial-correlation-low-pass.h"

#include <algorithm>

#include <glog/logging.h>

#include "phaser/common/signal-utils.h"

DEFINE_int32(
    phaser_core_spatial_low_pass_lower_bound, 0,
    "Defines the lower frequency bound of the spatial low pass filtering.");

DEFINE_int32(
    phaser_core_spatial_low_pass_upper_bound, 1000,
    "Defines the lower frequency bound of the spatial low pass filtering.");

namespace correlation {

SpatialCorrelationLowPass::SpatialCorrelationLowPass(const uint32_t n_voxels)
    : SpatialCorrelation(n_voxels),
      low_pass_lower_bound_(FLAGS_phaser_core_spatial_low_pass_lower_bound),
      low_pass_upper_bound_(std::min(
          static_cast<uint32_t>(FLAGS_phaser_core_spatial_low_pass_upper_bound),
          n_voxels)) {}

void SpatialCorrelationLowPass::complexMulSeq(
    fftw_complex* F, fftw_complex* G, fftw_complex* C) {
  for (uint32_t i = 0u; i < total_n_voxels_; ++i) {
    C[i][0] = F[i][0] * G[i][0] - F[i][1] * (-G[i][1]);
    C[i][1] = F[i][0] * (-G[i][1]) + F[i][1] * G[i][0];
  }
}

void SpatialCorrelationLowPass::shiftSignals(fftw_complex* F, fftw_complex* G) {
  common::SignalUtils::FFTShift(F, total_n_voxels_);
  common::SignalUtils::FFTShift(G, total_n_voxels_);
}

void SpatialCorrelationLowPass::inverseShiftSignals(fftw_complex* C) {
  common::SignalUtils::IFFTShift(C, total_n_voxels_);
}

double* SpatialCorrelationLowPass::correlateSignals(
    double* const f, double* const g) {
  const uint32_t function_size = total_n_voxels_ * sizeof(double);
  memcpy(f_, f, function_size);
  memcpy(g_, g, function_size);

  // Perform the two FFTs on the discretized signals.
  VLOG(1) << "Performing FFT on the first point cloud.";
  fftw_execute(f_plan_);
  VLOG(1) << "Performing FFT on the second point cloud.";
  fftw_execute(g_plan_);

  // Low pass filtering of the signals.
  VLOG(1)
      << "Shifting the low frequency components to the center of the spectrum.";
  shiftSignals(F_, G_);

  // Correlate the signals in the frequency domain.
  complexMulSeq(F_, G_, C_);

  // Perform the IFFT on the correlation tensor.
  VLOG(1) << "Shifting back the signals. Performing IFFT on low passed filtered"
             "correlation.";
  inverseShiftSignals(C_);
  fftw_execute(c_plan_);
  return c_;
}

}  // namespace correlation
