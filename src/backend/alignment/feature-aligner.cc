#include "packlo/backend/alignment/feature-aligner.h"
#include <pcl/features/normal_3d.h>

#include <glog/logging.h>

namespace alignment {

void FeatureAligner::alignRegistered(
    const model::PointCloud& cloud_prev, 
    const std::vector<model::FunctionValue>&, 
    const model::PointCloud& cloud_reg,
    const std::vector<model::FunctionValue>&, 
    common::Vector_t* xyz) {
  CHECK(xyz);

  pcl::PointCloud<pcl::PFHSignature125>::Ptr pfhs_prev (
      new pcl::PointCloud<pcl::PFHSignature125> ());
  pcl::PointCloud<pcl::PFHSignature125>::Ptr pfhs_reg (
      new pcl::PointCloud<pcl::PFHSignature125> ());

  calculatePFH(cloud_prev, pfhs_prev);
  calculatePFH(cloud_reg, pfhs_reg);
}

void FeatureAligner::calculatePFH(const model::PointCloud& cloud,
    pcl::PointCloud<pcl::PFHSignature125>::Ptr output) {
  const common::PointCloud_tPtr input = cloud.getRawCloud();

  // Compute normals.
  pcl::NormalEstimation<pcl::PointXYZI, pcl::Normal> ne;
  ne.setInputCloud (input);
  pcl::search::KdTree<pcl::PointXYZI>::Ptr tree (
      new pcl::search::KdTree<pcl::PointXYZI> ());
  ne.setSearchMethod (tree);
  pcl::PointCloud<pcl::Normal>::Ptr normals (new pcl::PointCloud<pcl::Normal>);
  ne.setRadiusSearch (0.03);
  ne.compute (*normals);

  // Compute Point Feature Histograms (PFH)
  pcl::PFHEstimation<pcl::PointXYZI, pcl::Normal, pcl::PFHSignature125> pfh;
  pfh.setInputCloud(input);
  pfh.setInputNormals (normals);
  pfh.setSearchMethod (tree);
  pfh.setRadiusSearch (0.05);
  pfh.compute (*output);
}

void FeatureAligner::calculateFPFH(const model::PointCloud&,
    pcl::PointCloud<pcl::FPFHSignature33>::Ptr) {
  // try omp version
}

} // namespace alignment 
