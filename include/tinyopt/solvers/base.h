// Copyright (C) 2025 Julien Michot. All Rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <algorithm>
#include <cassert>
#include <type_traits>

#include <tinyopt/cost.h>
#include <tinyopt/log.h>
#include <tinyopt/math.h>
#include <tinyopt/output.h>

#include <tinyopt/solvers/options.h>

namespace tinyopt::solvers {

template <typename _Scalar, int _Dims>
class SolverBase {
 public:
  using Scalar = _Scalar;
  static constexpr Index Dims = _Dims;

  SolverBase(const solvers::Options1 &options = {}) : options_{options} {}

  /// @brief Clamp the gradient 'g' to within [-minmax, minmax], if minmax is not 0.
  /// Returns true if 'g' was clamped.
  template <typename Grad_t>
  bool Clamp(Grad_t &g, Scalar minmax) const {
    if (minmax == 0) return false;
    if constexpr (std::is_scalar_v<Grad_t>) {
      g = std::clamp(g, -minmax, minmax);
    } else {
      g = g.cwiseMax(-minmax).cwiseMin(minmax);
    }
    return true;
  }

  /// Eventually normalize the cost
  void NormalizeCost(Cost &cost) {
    if (!options_.cost.use_squared_norm) cost.cost = std::sqrt(cost.cost);
    if (options_.cost.downscale_by_2) cost.cost *= 0.5f;
    if (options_.cost.normalize && cost.num_resisuals > 0) cost.cost /= cost.num_resisuals;
  }


 public:
  /// Solve the linear system dx = -H^-1 * grad, returns nullopt on failure
  virtual std::optional<Vector<Scalar, Dims>> Solve() const = 0;

  virtual void GoodStep(Scalar /*quality*/ = 0.0f) {}
  virtual void BadStep(Scalar /*quality*/ = 0.0f) {}
  virtual void FailedStep() {}

  virtual void Rebuild(bool) {}

  virtual std::string stateAsString() const { return ""; }
  virtual Index dims() const = 0;

  const Cost &cost() const { return cost_; }

 protected:
  const solvers::Options1 options_;
  Cost cost_;  // Last cost
};

}  // namespace tinyopt::solvers
