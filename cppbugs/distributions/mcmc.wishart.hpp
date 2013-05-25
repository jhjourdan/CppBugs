///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013 Whit Armstrong                                     //
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

#ifndef MCMC_WISHART_HPP
#define MCMC_WISHART_HPP

#include <armadillo>
#include <cppbugs/mcmc.dynamic.stochastic.hpp>
#include <cppbugs/mcmc.observed.hpp>
#include <cppbugs/mcmc.gcc.version.hpp>

namespace cppbugs {

  template<typename T, typename U, typename V>
  class Wishart : public DynamicStochastic<T> {
  private:
    const U& tau_;
    const V& n_;
    const bool destory_tau_, destory_n_;

    // element indices for lower diagonal
    const arma::uvec ld_elems_;

    // scratch space for cholesky and aux vecs
    arma::mat LL;
    arma::vec R_log_diag;
    arma::vec R_offdiag;

    const arma::uvec lower_diag(const size_t n) {
      arma::uvec ans(n*(n-1)/2);
      size_t idx(0);

      for(size_t i = 1; i < n; ++i) {
        for(size_t j = 0; j < i; ++j) {
          ans[idx++] = i + j * n;
        }
      }
      return ans;
    }

  public:
    Wishart(T& value, const U& tau, const V& n): DynamicStochastic<T>(value), tau_(tau), n_(n), destory_tau_(false), destory_n_(false), ld_elems_(lower_diag(value.n_cols)), LL(arma::zeros<arma::mat>(value.n_rows, value.n_cols)) {
      if(value.n_rows != tau_.n_rows || value.n_cols != tau_.n_cols) {
        throw std::logic_error("ERROR: dimensions of initial value do not match tau");
      }

      if(n <= 0) {
        throw std::logic_error("ERROR: N <= 0");
      }

      // if (not positive definite value)
      arma::mat R;
      if(chol(R,value) == false) {
        throw std::logic_error("ERROR: value is not positive definite.");
      }

      // if (not positive definite tau)
      arma::mat R_tau;
      if(chol(R_tau,tau) == false) {
        throw std::logic_error("ERROR: tau is not positive definite.");
      }
      
      arma::mat R_value = R.t();
      R_log_diag = log(diagvec(R));
      R_offdiag = R.elem(ld_elems_);

      // recover X to test
      LL.diag() = exp(R_log_diag);
      LL.elem(ld_elems_) = R_offdiag;
      arma::mat value_recovered(LL * LL.t());
      arma::umat ne(find(value !=value_recovered));
      if(ne.n_elem) { throw std::logic_error("did not recover original value."); }
    }

    // modified jumper to preserve symetric positive definite
    void jump(RngBase& rng) {
      //positive_jump_impl(rng, DynamicStochastic<T>::value, DynamicStochastic<T>::scale_);
      for(size_t i = 0; i < R_log_diag.n_elem; ++i) {
        R_log_diag[i] += rng.normal() * DynamicStochastic<T>::scale_;
      }
      for(size_t i = 0; i < R_offdiag.n_elem; ++i) {
        R_offdiag[i] += rng.normal() * DynamicStochastic<T>::scale_;
      }
      LL.diag() = exp(R_log_diag);
      LL.elem(ld_elems_) = R_offdiag;
      DynamicStochastic<T>::value = LL * LL.t();
    }

    ~Wishart() {
      if(destory_tau_) { delete &tau_; }
      if(destory_n_) { delete &n_; }
    }
    const double loglik() const { return wishart_logp(DynamicStochastic<T>::value,tau_,n_); }
  };

} // namespace cppbugs
#endif // MCMC_WISHART_HPP