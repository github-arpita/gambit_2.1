// ====================================================================
// This file is part of FlexibleSUSY.
//
// FlexibleSUSY is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// FlexibleSUSY is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FlexibleSUSY.  If not, see
// <http://www.gnu.org/licenses/>.
// ====================================================================

// File generated at Sun 24 Sep 2017 15:54:43

/**
 * @file MSSMEFTHiggs_two_scale_model.hpp
 * @brief contains class for model with routines needed to solve boundary
 *        value problem using the two_scale solver by solving EWSB
 *        and determine the pole masses and mixings
 *
 * This file was generated at Sun 24 Sep 2017 15:54:43 with FlexibleSUSY
 * 2.0.0-dev (git commit: 4d4c39a2702e9a6604f84813ccb0b85d40987f3b) and SARAH 4.11.0 .
 */

#ifndef MSSMEFTHiggs_TWO_SCALE_H
#define MSSMEFTHiggs_TWO_SCALE_H

#include "MSSMEFTHiggs_model.hpp"
#include "MSSMEFTHiggs_mass_eigenstates.hpp"

#include "model.hpp"

namespace flexiblesusy {

class Two_scale;
/**
 * @class MSSMEFTHiggs<Two_scale>
 * @brief model class with routines for determining masses and mixings and EWSB
 */
template<>
class MSSMEFTHiggs<Two_scale> : public Model, public MSSMEFTHiggs_mass_eigenstates {
public:
   explicit MSSMEFTHiggs(const MSSMEFTHiggs_input_parameters& input_ = MSSMEFTHiggs_input_parameters());
   MSSMEFTHiggs(const MSSMEFTHiggs&) = default;
   MSSMEFTHiggs(MSSMEFTHiggs&&) = default;
   virtual ~MSSMEFTHiggs() = default;
   MSSMEFTHiggs& operator=(const MSSMEFTHiggs&) = default;
   MSSMEFTHiggs& operator=(MSSMEFTHiggs&&) = default;

   // interface functions
   virtual void calculate_spectrum() override;
   virtual void clear_problems() override;
   virtual std::string name() const override;
   virtual void run_to(double scale, double eps = -1.0) override;
   virtual void print(std::ostream& out = std::cerr) const override;
   virtual void set_precision(double) override;
};

std::ostream& operator<<(std::ostream&, const MSSMEFTHiggs<Two_scale>&);

} // namespace flexiblesusy

#endif
