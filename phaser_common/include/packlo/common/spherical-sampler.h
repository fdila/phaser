#ifndef INCLUDE_PACKLO_COMMON_SPHERICAL_SAMPLER_H_
#define INCLUDE_PACKLO_COMMON_SPHERICAL_SAMPLER_H_

#include <vector>
#include "packlo/common/spherical-projection.h"
#include "packlo/model/point-cloud.h"

namespace common {

class SphericalSampler {
 public:
  explicit SphericalSampler(const int bandwith);
  void sampleUniformly(
      const model::PointCloud& cloud, std::vector<model::FunctionValue>* grid);

  void initialize(const int bandwith);
  int getInitializedBandwith() const noexcept;

 private:
  std::vector<common::Point_t> create2BwGrid(const std::size_t bw);
  std::vector<common::Point_t> convertCartesian(
      const std::vector<common::Point_t>& grid);

  std::vector<common::Point_t> cartesian_grid_;
  SphericalProjection projection_;
  bool is_initialized_;
  int bandwith_;
};

}  // namespace common

#endif  // INCLUDE_PACKLO_COMMON_SPHERICAL_SAMPLER_H_
