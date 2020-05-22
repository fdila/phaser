#ifndef PHASER_BACKEND_REGISTRATION_SPH_OPT_REGISTRATION_H_
#define PHASER_BACKEND_REGISTRATION_SPH_OPT_REGISTRATION_H_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "phaser/backend/alignment/base-aligner.h"
#include "phaser/backend/correlation/spherical-correlation.h"
#include "phaser/backend/registration/base-registration.h"
#include "phaser/backend/uncertainty/base-eval.h"
#include "phaser/backend/uncertainty/phase-correlation-eval.h"
#include "phaser/common/spherical-sampler.h"
#include "phaser/common/thread-pool.h"

namespace registration {

class SphOptRegistration : public BaseRegistration {
 public:
  SphOptRegistration();
  virtual ~SphOptRegistration() = default;

  model::RegistrationResult registerPointCloud(
      model::PointCloudPtr cloud_prev, model::PointCloudPtr cloud_cur) override;

  void getStatistics(common::StatisticsManager* manager) const
      noexcept override;

  model::RegistrationResult estimateRotation(
      model::PointCloudPtr cloud_prev, model::PointCloudPtr cloud_cur);

  void estimateTranslation(
      model::PointCloudPtr cloud_prev, model::RegistrationResult* result);

  void setBandwith(const int bandwith);

  uncertainty::BaseEval& getRotEvaluation();
  uncertainty::BaseEval& getPosEvaluation();

 protected:
  std::vector<correlation::SphericalCorrelation> correlatePointcloud(
      model::PointCloudPtr target, model::PointCloudPtr source);

  const uint32_t bandwidth_;
  common::SphericalSampler sampler_;
  std::vector<model::FunctionValue> f_values_;
  std::vector<model::FunctionValue> h_values_;
  alignment::PhaseAligner aligner_;
  uncertainty::PhaseCorrelationEvalPtr correlation_eval_;
  common::ThreadPool th_pool_;
};

using SphOptRegistrationPtr = std::unique_ptr<SphOptRegistration>;

}  // namespace registration

#endif  // PHASER_BACKEND_REGISTRATION_SPH_OPT_REGISTRATION_H_
