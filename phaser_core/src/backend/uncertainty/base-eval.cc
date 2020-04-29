#include "phaser/backend/uncertainty/base-eval.h"

namespace uncertainty {

BaseEval::BaseEval(
    const alignment::BaseAligner& aligner,
    const correlation::SphericalCorrelation& sph)
    : aligner_(aligner), sph_(sph) {}

}  // namespace uncertainty
