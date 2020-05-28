#include "phaser/backend/registration/mock/sph-registration-mock-translated.h"
#include "phaser/backend/registration/sph-registration.h"
#include "phaser/common/data/datasource-ply.h"
#include "phaser/common/metric-utils.h"
#include "phaser/common/test/testing-entrypoint.h"
#include "phaser/common/test/testing-predicates.h"

#include <chrono>
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <random>

namespace phaser_core {

class TranslationAlignmentTest : public ::testing::Test {
 public:
  TranslationAlignmentTest()
      : generator_(std::chrono::system_clock::now().time_since_epoch().count()),
        distribution_(0, 100) {}

 protected:
  virtual void SetUp() {
    ds_ = std::make_unique<data::DatasourcePly>();
    ds_->setDatasetFolder("./test_clouds/translation_only/");
  }

  BaseRegistration* initializeRegistration(bool mocked) {
    if (mocked)
      registrator_ = std::make_unique<SphRegistrationMockTranslated>();
    else
      registrator_ = std::make_unique<SphRegistration>();
    return registrator_.get();
  }

  float getRandomTranslation() {
    return distribution_(generator_);
  }

  data::DatasourcePlyPtr ds_;
  BaseRegistrationPtr registrator_;
  std::default_random_engine generator_;
  std::uniform_real_distribution<float> distribution_;
};

TEST_F(TranslationAlignmentTest, TranslationSelfSingle) {
  CHECK(ds_);
  SphRegistrationMockTranslated* reg =
      dynamic_cast<SphRegistrationMockTranslated*>(
          initializeRegistration(true));
  // Define a random translation.
  Eigen::Vector3d trans_xyz(12.9f, 33.1f, 21.5f);
  reg->setRandomTranslation(trans_xyz(0), trans_xyz(1), trans_xyz(2));

  model::RegistrationResult result;
  ds_->subscribeToPointClouds([&](const model::PointCloudPtr& cloud) {
    CHECK(cloud);
    // Estimate the translation.
    result = reg->registerPointCloud(cloud, cloud);
    // ASSERT_TRUE(result.foundSolutionForTranslation());

    // Get the result and compare it.
    EXPECT_NEAR_EIGEN(-trans_xyz, result.getTranslation(), 4.0);
    ASSERT_LE(
        common::MetricUtils::HausdorffDistance(
            cloud, result.getRegisteredCloud()),
        5.0);
  });
  ds_->startStreaming(1);
}

/*
TEST_F(TranslationAlignmentTest, TranslationSelfAll) {
  CHECK(ds_);
  SphRegistrationMockTranslated* reg =
      dynamic_cast<SphRegistrationMockTranslated*>(
          initializeRegistration(true));

  model::RegistrationResult result;
  ds_->subscribeToPointClouds([&] (const model::PointCloudPtr& cloud) {
    CHECK(cloud);
    // Define a new random translation for each cloud.
    Eigen::Vector3d trans_xyz(
        getRandomTranslation(), getRandomTranslation(), getRandomTranslation());
    reg->setRandomTranslation(trans_xyz(0), trans_xyz(1), trans_xyz(2));

    // Estimate the translation.
    result = reg->registerPointCloud(cloud, cloud);
    ASSERT_TRUE(result.foundSolutionForTranslation());

    // Get the result and compare it.
    EXPECT_NEAR_EIGEN(-trans_xyz, result.getTranslation(), 4.0);
    ASSERT_LE(common::MetricUtils::HausdorffDistance(cloud,
          result.getRegisteredCloud()), 5.0);
  });
  ds_->startStreaming();
}
*/

/*
TEST_F(TranslationAlignmentTest, TranslationEasy) {
  CHECK(ds_);
  SphRegistration* reg = dynamic_cast<
    SphRegistration*>(initializeRegistration(false));

  model::RegistrationResult result;
  model::PointCloudPtr prev_cloud = nullptr;
  ds_->subscribeToPointClouds([&] (const model::PointCloudPtr& cloud) {
    CHECK(cloud);
    if (prev_cloud == nullptr) {
      prev_cloud = cloud;
      return;
    }

    // Register the point clouds.
    const float initHausdorff =
        common::MetricUtils::HausdorffDistance(prev_cloud, cloud);
    result = reg->estimateTranslation(prev_cloud, cloud);
    ASSERT_TRUE(result.foundSolutionForTranslation());

    // Check that the Hausdorff distance decreased after the registration.
    ASSERT_LT(
        common::MetricUtils::HausdorffDistance(
            prev_cloud, result.getRegisteredCloud()),
        initHausdorff);
  });
  ds_->startStreaming(2);
}
*/

}  // namespace phaser_core

MAPLAB_UNITTEST_ENTRYPOINT
