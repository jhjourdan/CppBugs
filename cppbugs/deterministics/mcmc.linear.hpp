///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011 Whit Armstrong                                     //
//                                                                       //
// This program is free software: you can redistribute it and/or modify  //
// it under the terms of the GNU General Public License as published by  //
// the Free Software Foundation, either version 3 of the License, or     //
// (at your option) any later version.                                   //
//                                                                       //
// This program is distributed in the hope that it will be useful,       //
// but WITHOUT ANY WARRANTY; without even the implied warranty of        //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
// GNU General Public License for more details.                          //
//                                                                       //
// You should have received a copy of the GNU General Public License     //
// along with this program.  If not, see <http://www.gnu.org/licenses/>. //
///////////////////////////////////////////////////////////////////////////

#ifndef MCMC_LINEAR_HPP
#define MCMC_LINEAR_HPP

#include <cppbugs/mcmc.dynamic.hpp>

namespace cppbugs {

  template<typename T, typename U, typename V>
  class Linear : public Deterministic<T> {
    const U& X_;
    const V& b_;
  public:
    Linear(T& x, const U& X, const V& b): Deterministic<T>(x), X_(X), b_(b) {
      Deterministic<T>::value = X_ * b_;
    }
    void jump(RngBase& rng) {
      Deterministic<T>::value = X_ * b_;
    }
  };
} // namespace cppbugs
#endif //MCMC_LINEAR_HPP
