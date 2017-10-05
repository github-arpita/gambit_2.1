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

// File generated at Sat 27 Aug 2016 12:49:12

/**
 * @file MSSMatMGUT_two_scale_model.cpp
 * @brief implementation of the MSSMatMGUT model class
 *
 * Contains the definition of the MSSMatMGUT model class methods
 * which solve EWSB and calculate pole masses and mixings from DRbar
 * parameters.
 *
 * This file was generated at Sat 27 Aug 2016 12:49:12 with FlexibleSUSY
 * 1.5.1 (git commit: 8356bacd26e8aecc6635607a32835d534ea3cf01) and SARAH 4.9.0 .
 */

#include "MSSMatMGUT_two_scale_model.hpp"

namespace flexiblesusy {

using namespace MSSMatMGUT_info;

#define CLASSNAME MSSMatMGUT<Two_scale>

CLASSNAME::MSSMatMGUT(const MSSMatMGUT_input_parameters& input_)
   : Two_scale_model()
   , MSSMatMGUT_mass_eigenstates(input_)
{
}

CLASSNAME::~MSSMatMGUT()
{
}

void CLASSNAME::calculate_spectrum()
{
   MSSMatMGUT_mass_eigenstates::calculate_spectrum();
}

void CLASSNAME::clear_problems()
{
   MSSMatMGUT_mass_eigenstates::clear_problems();
}

std::string CLASSNAME::name() const
{
   return MSSMatMGUT_mass_eigenstates::name();
}

void CLASSNAME::run_to(double scale, double eps)
{
   MSSMatMGUT_mass_eigenstates::run_to(scale, eps);
}

void CLASSNAME::print(std::ostream& out) const
{
   MSSMatMGUT_mass_eigenstates::print(out);
}

void CLASSNAME::set_precision(double p)
{
   MSSMatMGUT_mass_eigenstates::set_precision(p);
}

std::ostream& operator<<(std::ostream& ostr, const MSSMatMGUT<Two_scale>& model)
{
   model.print(ostr);
   return ostr;
}

} // namespace flexiblesusy
