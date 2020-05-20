#include <chrono>
#include <cmath>
#include <memory>
#include <random>

#include <gtest/gtest.h>

#include "phaser/backend/fusion/laplace-pyramid.h"
#include "phaser/common/test/testing-entrypoint.h"
#include "phaser/common/test/testing-predicates.h"

namespace fusion {

class LaplacePyramidTest : public ::testing::Test {
 public:
  LaplacePyramidTest()
      : generator_(std::chrono::system_clock::now().time_since_epoch().count()),
        distribution_(0, 100) {}
  fftw_complex* createRandomCoefficients(const uint32_t n_coeffs) {
    fftw_complex* coeffs = new fftw_complex[n_coeffs];
    for (uint32_t i = 0; i < n_coeffs; ++i) {
      coeffs[i][0] = getRandomFloat();
      coeffs[i][1] = getRandomFloat();
    }
    return coeffs;
  }

  fftw_complex* createFixedCoefficients(
      const double val, const uint32_t n_coeffs) {
    fftw_complex* coeffs = new fftw_complex[n_coeffs];
    for (uint32_t i = 0; i < n_coeffs; ++i) {
      coeffs[i][0] = val;
      coeffs[i][1] = val;
    }
    return coeffs;
  }

  uint32_t nnz(const std::vector<complex_t>& coeffs) const {
    uint32_t non_zeros = 0u;
    for (const complex_t& coeff : coeffs) {
      if (coeff[0] != 0.0 || coeff[1] != 0.0) {
        ++non_zeros;
      }
    }
    return non_zeros;
  }

  float getRandomFloat() {
    return distribution_(generator_);
  }

 private:
  std::default_random_engine generator_;
  std::uniform_real_distribution<float> distribution_;
};

TEST_F(LaplacePyramidTest, SimpleReduceTest) {
  fusion::LaplacePyramid laplace(4.0);
  const uint32_t n_coeffs = 8u;
  fftw_complex* coeffs = createRandomCoefficients(n_coeffs);
  fusion::PyramidLevel first_level = laplace.reduce(coeffs, n_coeffs);

  const std::vector<fusion::complex_t>& low_pass = first_level.first;
  EXPECT_EQ(low_pass.size(), 4u);
  for (uint32_t i = 0u; i < 4u; ++i) {
    EXPECT_GT(low_pass[i][0], 0.0);
    EXPECT_GT(low_pass[i][1], 0.0);
  }
  const std::vector<fusion::complex_t>& coeff_laplace = first_level.second;
  const float tol = 1e-3;
  uint32_t k = 0u;
  for (uint32_t i = 2u; i < 6u; ++i) {
    const double real = coeff_laplace[i][0] + low_pass[k][0];
    const double imag = coeff_laplace[i][1] + low_pass[k][1];
    EXPECT_NEAR(real, coeffs[i][0], tol);
    EXPECT_NEAR(imag, coeffs[i][1], tol);
    ++k;
  }
  delete[] coeffs;
}

TEST_F(LaplacePyramidTest, SimpleExpandTest) {
  fusion::LaplacePyramid laplace(4.0);
  const uint32_t n_coeffs = 8u;
  fftw_complex* coeffs = createRandomCoefficients(n_coeffs);
  fusion::PyramidLevel first_level = laplace.reduce(coeffs, n_coeffs);

  const std::vector<fusion::complex_t>& low_pass = first_level.first;
  std::vector<fusion::complex_t>* lapl = &first_level.second;
  EXPECT_LT(nnz(*lapl), n_coeffs);
  laplace.expand(low_pass, lapl);
  EXPECT_EQ(nnz(*lapl), n_coeffs);

  delete[] coeffs;
}

TEST_F(LaplacePyramidTest, MaxCoeffTest) {
  fusion::LaplacePyramid laplace(4.0);
  const uint32_t n_coeffs = 8u;
  fftw_complex* coeffs = createFixedCoefficients(1, n_coeffs);
  fftw_complex* coeffs_2 = createFixedCoefficients(15, n_coeffs);
  std::vector<fusion::PyramidLevel> levels;
  levels.emplace_back(laplace.reduce(coeffs, n_coeffs));
  levels.emplace_back(laplace.reduce(coeffs_2, n_coeffs));

  std::vector<fusion::complex_t> fused =
      laplace.fuseLevelByMaxCoeff(levels, n_coeffs);
  const std::vector<fusion::complex_t>& lapl_1 = levels[0].second;
  const std::vector<fusion::complex_t>& lapl_2 = levels[1].second;
  const float tol = 1e-3;
  for (uint32_t i = 0u; i < 2u; ++i) {
    EXPECT_GT(fused[i][0], lapl_1[i][0]);
    EXPECT_GT(fused[i][1], lapl_1[i][1]);
    EXPECT_NEAR(fused[i][0], lapl_2[i][0], tol);
    EXPECT_NEAR(fused[i][1], lapl_2[i][1], tol);
  }
  for (uint32_t i = 6u; i < 8u; ++i) {
    EXPECT_GT(fused[i][0], lapl_1[i][0]);
    EXPECT_GT(fused[i][1], lapl_1[i][1]);
    EXPECT_NEAR(fused[i][0], lapl_2[i][0], tol);
    EXPECT_NEAR(fused[i][1], lapl_2[i][1], tol);
  }

  delete[] coeffs;
  delete[] coeffs_2;
}

TEST_F(LaplacePyramidTest, LowPassAverageTest) {
  fusion::LaplacePyramid laplace(4.0);
  const uint32_t n_coeffs = 8u;
  fftw_complex* coeffs = createFixedCoefficients(5, n_coeffs);
  fftw_complex* coeffs_2 = createFixedCoefficients(15, n_coeffs);
  std::vector<fusion::PyramidLevel> levels;
  levels.emplace_back(laplace.reduce(coeffs, n_coeffs));
  levels.emplace_back(laplace.reduce(coeffs_2, n_coeffs));

  std::vector<fusion::complex_t> fused = laplace.fuseLastLowPassLayer(levels);
  const float tol = 1e-3;
  for (uint32_t i = 0u; i < 4u; ++i) {
    EXPECT_NEAR(fused[i][0], 10.0, tol);
    EXPECT_NEAR(fused[i][1], 10.0, tol);
  }

  delete[] coeffs;
  delete[] coeffs_2;
}

TEST_F(LaplacePyramidTest, FuseChannelsUsing2LevelsTest) {
  fusion::LaplacePyramid laplace(4.0);
  const uint32_t n_coeffs = 8u;
  fftw_complex* coeffs = createFixedCoefficients(5, n_coeffs);
  fftw_complex* coeffs_2 = createFixedCoefficients(15, n_coeffs);
  std::vector<complex_t> fused =
      laplace.fuseChannels({coeffs, coeffs_2}, n_coeffs, 2);

  for (uint32_t i = 0u; i < n_coeffs; ++i) {
    EXPECT_GT(fused[i][0], 0.0);
    EXPECT_GT(fused[i][1], 0.0);
  }

  delete[] coeffs;
  delete[] coeffs_2;
}

}  // namespace fusion

MAPLAB_UNITTEST_ENTRYPOINT
