/*
 * Copyright (c) 2011-2015, Georgia Tech Research Corporation
 * All rights reserved.
 *
 * Author(s): Sehoon Ha <sehoon.ha@gmail.com>,
 *            Jeongseok Lee <jslee02@gmail.com>
 *
 * Georgia Tech Graphics Lab and Humanoid Robotics Lab
 *
 * Directed by Prof. C. Karen Liu and Prof. Mike Stilman
 * <karenliu@cc.gatech.edu> <mstilman@cc.gatech.edu>
 *
 * This file is provided under the following "BSD-style" License:
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

#include "dart/optimizer/Problem.h"

#include <algorithm>
#include <iostream>
#include <limits>

#include "dart/math/Helpers.h"
#include "dart/optimizer/Function.h"

namespace dart {
namespace optimizer {

//==============================================================================
template<typename T>
static T getVectorObjectIfAvailable(size_t _idx, const std::vector<T>& _vec)
{
  // TODO: Should we have an out-of-bounds assertion or throw here?
  if (_idx < _vec.size())
    return _vec[_idx];

  return nullptr;
}

//==============================================================================
Problem::Problem(size_t _dim)
  : mDimension(_dim)
{
  mInitialGuess = Eigen::VectorXd::Zero(mDimension);
  mLowerBounds = Eigen::VectorXd::Constant(mDimension, -HUGE_VAL);
  mUpperBounds = Eigen::VectorXd::Constant(mDimension,  HUGE_VAL);
  mOptimalSolution = Eigen::VectorXd::Zero(mDimension);
}

//==============================================================================
Problem::~Problem()
{
}

//==============================================================================
size_t Problem::getDimension() const
{
  return mDimension;
}

//==============================================================================
void Problem::setInitialGuess(const Eigen::VectorXd& _initGuess)
{
  assert(static_cast<size_t>(_initGuess.size()) == mDimension
         && "Invalid size.");
  mInitialGuess = _initGuess;
}

//==============================================================================
const Eigen::VectorXd& Problem::getInitialGuess() const
{
  return mInitialGuess;
}

//==============================================================================
void Problem::addSeed(const Eigen::VectorXd& _seed)
{
  mSeeds.push_back(_seed);
}

//==============================================================================
Eigen::VectorXd& Problem::getSeed(size_t _index)
{
  if(_index < mSeeds.size())
    return mSeeds[_index];

  if(mSeeds.size() == 0)
    dtwarn << "[Problem::getSeed] Requested seed at index [" << _index << "], "
           << "but there are currently no seeds. Returning the problem's "
           << "initial guess instead.\n";
  else
    dtwarn << "[Problem::getSeed] Requested seed at index [" << _index << "], "
           << "but the current max index is [" << mSeeds.size()-1 << "]. "
           << "Returning the Problem's initial guess instead.\n";

  return mInitialGuess;
}

//==============================================================================
const Eigen::VectorXd& Problem::getSeed(size_t _index) const
{
  return const_cast<Problem*>(this)->getSeed(_index);
}

//==============================================================================
std::vector<Eigen::VectorXd>& Problem::getAllSeeds()
{
  return mSeeds;
}

//==============================================================================
const std::vector<Eigen::VectorXd>& Problem::getAllSeeds() const
{
  return mSeeds;
}

//==============================================================================
void Problem::clearAllSeeds()
{
  mSeeds.clear();
}

//==============================================================================
void Problem::setLowerBounds(const Eigen::VectorXd& _lb)
{
  assert(static_cast<size_t>(_lb.size()) == mDimension && "Invalid size.");
  mLowerBounds = _lb;
}

//==============================================================================
const Eigen::VectorXd& Problem::getLowerBounds() const
{
  return mLowerBounds;
}

//==============================================================================
void Problem::setUpperBounds(const Eigen::VectorXd& _ub)
{
  assert(static_cast<size_t>(_ub.size()) == mDimension && "Invalid size.");
  mUpperBounds = _ub;
}

//==============================================================================
const Eigen::VectorXd& Problem::getUpperBounds() const
{
  return mUpperBounds;
}

//==============================================================================
void Problem::setObjective(FunctionPtr _obj)
{
  assert(_obj && "NULL pointer is not allowed.");
  mObjective = _obj;
}

//==============================================================================
FunctionPtr Problem::getObjective() const
{
  return mObjective;
}

//==============================================================================
void Problem::addEqConstraint(FunctionPtr _eqConst)
{
  assert(_eqConst);
  mEqConstraints.push_back(_eqConst);
}

//==============================================================================
void Problem::addIneqConstraint(FunctionPtr _ineqConst)
{
  assert(_ineqConst);
  mIneqConstraints.push_back(_ineqConst);
}

//==============================================================================
size_t Problem::getNumEqConstraints()
{
  return mEqConstraints.size();
}

//==============================================================================
size_t Problem::getNumIneqConstraints()
{
  return mIneqConstraints.size();
}

//==============================================================================
FunctionPtr Problem::getEqConstraint(size_t _idx) const
{
  assert(_idx < mEqConstraints.size());
  return getVectorObjectIfAvailable<FunctionPtr>(_idx, mEqConstraints);
}

//==============================================================================
FunctionPtr Problem::getIneqConstraint(size_t _idx) const
{
  assert(_idx < mIneqConstraints.size());
  return getVectorObjectIfAvailable<FunctionPtr>(_idx, mIneqConstraints);
}

//==============================================================================
void Problem::removeEqConstraint(FunctionPtr _eqConst)
{
  // TODO(JS): Need to delete?
  mEqConstraints.erase(
        std::remove(mEqConstraints.begin(), mEqConstraints.end(),
                    _eqConst),
        mEqConstraints.end());
}

//==============================================================================
void Problem::removeIneqConstraint(FunctionPtr _ineqConst)
{
  // TODO(JS): Need to delete?
  mIneqConstraints.erase(
        std::remove(mIneqConstraints.begin(), mIneqConstraints.end(),
                    _ineqConst),
        mIneqConstraints.end());
}

//==============================================================================
void Problem::removeAllEqConstraints()
{
  // TODO(JS): Need to delete?
  mEqConstraints.clear();
}

//==============================================================================
void Problem::removeAllIneqConstraints()
{
  // TODO(JS): Need to delete?
  mIneqConstraints.clear();
}

//==============================================================================
void Problem::setOptimumValue(double _val)
{
  mOptimumValue = _val;
}

//==============================================================================
double Problem::getOptimumValue() const
{
  return mOptimumValue;
}

//==============================================================================
void Problem::setOptimalSolution(const Eigen::VectorXd& _optParam)
{
  assert(static_cast<size_t>(_optParam.size()) == mDimension
         && "Invalid size.");
  mOptimalSolution = _optParam;
}

//==============================================================================
const Eigen::VectorXd& Problem::getOptimalSolution()
{
  return mOptimalSolution;
}

}  // namespace optimizer
}  // namespace dart
