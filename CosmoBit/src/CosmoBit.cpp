//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Central module file of CosmoBit.  
///  Calculates cosmology-related observables.
///
///  Additionally, contains main routines for 
///  interfacing to CLASS and MontePython.
///
///  Most of the model- or observable-specific code is
///  stored in separate source files.
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Selim C. Hotinli
///          (selim.hotinli14@pimperial.ac.uk)
///  \date 2017 Jul
///  \date 2018 May
///  \date 2018 Aug - Sep
///
///  \author Patrick Stoecker
///          (stoecker@physik.rwth-aachen.de)
///  \date 2017 Nov
///  \date 2018 Jan - May
///  \date 2019 Jan, Feb, June, Nov
///
///  \author Janina Renk
///          (janina.renk@fysik.su.se)
///  \date 2018 June
///  \date 2019 Mar,June
///
///  \author Sanjay Bloor
///          (sanjay.bloor12@imperial.ac.uk)
///  \date 2019 June, Nov
///
///  \author Sebastian Hoof
///          (hoof@uni-goettingen.de)
///  \date 2020 Mar
///
///  \author Pat Scott
///          (pat.scott@uq.edu.au)
///  \date 2018 Mar
///  \date 2019 Jul
///  \date 2020 Apr
///
///  *********************************************

#include <stdint.h> // save memory addresses as int
#include <boost/algorithm/string/trim.hpp>

#include "gambit/Utils/ascii_table_reader.hpp"
#include "gambit/Utils/ascii_dict_reader.hpp"
#include "gambit/Elements/gambit_module_headers.hpp"
#include "gambit/CosmoBit/CosmoBit_rollcall.hpp"
#include "gambit/CosmoBit/CosmoBit_types.hpp"
#include "gambit/CosmoBit/CosmoBit_utils.hpp"

namespace Gambit
{

  namespace CosmoBit
  {
    using namespace LogTags;

    /***********************************/
    /* General cosmological quantities */
    /***********************************/

    /// Function for setting k_pivot in Mpc^-1 for consistent use within CosmoBit
    /// (i.e. ensuring a consistent value is used by both CLASS and MultiModeCode)
    void set_k_pivot(double &result)
    {
      result = Pipes::set_k_pivot::runOptions->getValueOrDef<double>(0.05, "k_pivot");
    }

    void get_mNu_tot(double& result)
    {
      using namespace Pipes::get_mNu_tot;

      // The units of StandardModel_SLHA2 are GeV; here we are using eV.
      result = 1e9 * ( *Param["mNu1"] + *Param["mNu2"] + *Param["mNu3"] );
    }

    // Returns the effective number of ultrarelativistic species today
    void get_N_ur(double& result)
    {
      using namespace Pipes::get_N_ur;

      // The units of StandardModel_SLHA2 are GeV; here we are using eV.
      std::vector<double> nuMasses{
        1e9*(*Param["mNu1"]), 1e9*(*Param["mNu2"]), 1e9*(*Param["mNu3"])
      };

      // Count the nonzero entries
      auto isNonZero = [](double i) {return i > 0.;};
      int N_ncdm = std::count_if(nuMasses.begin(), nuMasses.end(), isNonZero);

      // Assing the result to the standard value of N_ur depending on the number of massive neutrinos.
      switch (N_ncdm)
      {
        case 1:
          result = 2.0328;  // N_ur (today) = 2.0328 for 1 massive neutrino at CMB release
          break;
        case 2:
          result = 1.0196;  // N_ur (today) = 1.0196 for 2 massive neutrino at CMB release
          break;
        case 3:
          result = 0.00641;  // N_ur (today) = 0.00641 for 3 massive neutrinos at CMB release
          break;
        case 0:
          result = 3.046;
          break;
        default:
          {
            std::ostringstream err;
            err << "You are asking for more than three massive neutrino species.\n";
            err << "Such a case is not implemented in CosmoBit. ";
            err << "If you want to consider this you can add it to the function ";
            err << "'get_N_ur' of the capability 'N_ur'.";
            CosmoBit_error().raise(LOCAL_INFO, err.str());
          }
      }

      // If "etaBBN_rBBN_rCMB_dNurBBN_dNurCMB" is in use, the result will
      // be scaled and gets extra contributions
      if (ModelInUse("etaBBN_rBBN_rCMB_dNurBBN_dNurCMB"))
      {
        // Check if the input for dNeff is negative (unphysical)
        // NOTE: CosmoBit performs no sanity checks if you allow negative dNEff; you're on your own.
        static bool allow_negative_delta_N_ur = runOptions->getValueOrDef<bool>(false,"allow_negative_delta_N_ur");

        // Get values of the temperature ratio and any ultrarelativistic contribution.
        const ModelParameters& NP_params = *Dep::etaBBN_rBBN_rCMB_dNurBBN_dNurCMB_parameters;
        double dNurCMB =  NP_params.at("dNur_CMB");
        double rCMB =  NP_params.at("r_CMB");

        // Only let the user have negative contributions to dNEff if they've signed off on it.
        if ( (!allow_negative_delta_N_ur) && (dNurCMB < 0.0) )
        {
          std::ostringstream err;
          err << "A negative value for \"dNur_CMB\" is unphysical and is not allowed in CosmoBit by default!\n\n";
          err << "If you want to proceed with negative values, please add\n\n";
          err << "  - module: CosmoBit\n";
          err << "    options:\n";
          err << "      allow_negative_delta_N_ur: true\n\n";
          err << "to the Rules section of the YAML file.";
          CosmoBit_error().raise(LOCAL_INFO,err.str());
        }

        // If the check is passed, set the result.
        result = pow(rCMB,4)*(result) + dNurCMB;
      }
      logger() << "N_ur calculated to be " << result << EOM;
    }

    /// Temperature of non-CDM in the (cosmological) SM.
    void T_ncdm_SM(double &result)
    {
      using namespace Pipes::T_ncdm_SM;

      // Set to 0.71611 in units of photon temperature, above the instantaneous decoupling value (4/11)^(1/3)
      // to recover Sum_i mNu_i/omega = 93.14 eV resulting from studies of active neutrino decoupling (arXiv:hep-ph/0506164)
      result = 0.71611;
      // This standard value enters in many assumptions entering CLASS. Therefore changing this value in
      // the YAML file is disabled at the moment. If you still want to modify it, uncomment the line below and
      // you can set is as a runOption of this capability.
      // result = runOptions->getValueOrDef<double>(0.71611,"T_ncdm");
    }

    /// Temperature of non-CDM in non-standard theories.
    void T_ncdm(double &result)
    {
      using namespace Pipes::T_ncdm;

      double rCMB = 1.0; // Default value if no energy injection is assumed.

      // If the "etaBBN_rBBN_rCMB_dNurBBN_dNurCMB" model is included in the scan,
      // we use rCMB of this model.
      if (ModelInUse("etaBBN_rBBN_rCMB_dNurBBN_dNurCMB"))
      {
        const ModelParameters& NP_params = *Dep::etaBBN_rBBN_rCMB_dNurBBN_dNurCMB_parameters;
        rCMB = NP_params.at("r_CMB");
      }

      // Take the SM value of T_ncdm (T_nu) and multiply it with the value of rCMB
      result = rCMB*(*Dep::T_ncdm_SM);
    }

    /// Baryon-to-photon ratio in LCDM
    void eta0_LCDM(double &result)
    {
      using namespace Pipes::eta0_LCDM;

      double ngamma, nb;
      ngamma = 16*pi*zeta3*pow(*Param["T_cmb"]*_kB_eV_over_K_/_hc_eVcm_,3); // photon number density today
      nb = *Param["omega_b"]*3*100*1e3*100*1e3/_Mpc_SI_/_Mpc_SI_/(8*pi*_GN_cgs_* m_proton*1e9*eV2g); // baryon number density today

      result =  nb/ngamma;
      logger() << "Baryon to photon ratio (eta) today computed to be " << result << EOM;
    }

    /// Baryon-to-photon ratio
    void etaBBN_SM(double& result)
    {
      using namespace Pipes::etaBBN_SM;

      result = *Dep::eta0;
    }

    /// The total baryon content today.
    void compute_Omega0_b(double &result)
    {
      using namespace Pipes::compute_Omega0_b;

      double h = *Dep::H0/100.;
      result =*Param["omega_b"]/h/h;
    }

    /// The total cold dark matter content today.
    void compute_Omega0_cdm(double &result)
    {
      using namespace Pipes::compute_Omega0_cdm;

      double h = *Dep::H0/100.;
      result =*Param["omega_cdm"]/h/h;
    }

    /// The total photon content today.
    void compute_Omega0_g(double &result)
    {
      using namespace Pipes::compute_Omega0_g;

      double h = *Dep::H0/100.;
      result = (4.*_sigmaB_SI_/_c_SI_*pow(*Param["T_cmb"],4.)) / (3.*_c_SI_*_c_SI_*1.e10*h*h/_Mpc_SI_/_Mpc_SI_/8./pi/_GN_SI_);
    }

    /// Number density of photons today
    void compute_n0_g(double &result)
    {
      using namespace Pipes::compute_n0_g;

      result = 2./pi/pi*zeta3 *pow(*Param["T_cmb"]*_kB_eV_over_K_,3.)/pow(_hP_eVs_*_c_SI_/2./pi,3)/100/100/100; // result per cm^3
    }

    /// The total ultrarelativistic content today.
    void compute_Omega0_ur(double &result)
    {
      using namespace Pipes::compute_Omega0_ur;

      double N_ur = *Dep::N_ur;
      double Omega0_g = *Dep::Omega0_g;
      result = (N_ur)*7./8.*pow(4./11.,4./3.)* Omega0_g;
    }

    /* Classy getter functions */

    /// Hubble
    void get_H0_classy(double &result)
    {
      using namespace Pipes::get_H0_classy;

      // Rescale by c [km/s]
      result = _c_SI_*BEreq::class_get_H0()/1000;
    }

    /// Energy densities *today* (Omega0)

    /// Matter
    void get_Omega0_m_classy(double& result)
    {
      using namespace Pipes::get_Omega0_m_classy;

      result = BEreq::class_get_Omega0_m();
    }

    /// Radiation
    void get_Omega0_r_classy(double& result)
    {
      using namespace Pipes::get_Omega0_r_classy;

      result = BEreq::class_get_Omega0_r();
    }

    /// Ultra-relativistic
    void get_Omega0_ur_classy(double& result)
    {
      using namespace Pipes::get_Omega0_ur_classy;

      result = BEreq::class_get_Omega0_ur();
    }

    /// Non-cold dark matter
    void get_Omega0_ncdm_classy(double& result)
    {
      using namespace Pipes::get_Omega0_ncdm_classy;

      result = BEreq::class_get_Omega0_ncdm_tot();
    }

    /// Sigma8
    void get_Sigma8_classy(double& result)
    {
      using namespace Pipes::get_Sigma8_classy;

      double sigma8 = BEreq::class_get_sigma8();
      double Omega0_m = *Dep::Omega0_m;

      result = sigma8*pow(Omega0_m/0.3, 0.5);
    }

    /// Effective number of neutrino species
    /// (mostly for cross-checking!)
    void get_Neff_classy(double& result)
    {
      using namespace Pipes::get_Neff_classy;

      result = BEreq::class_get_Neff();
    }

    /// Comoving sound horizon at baryon drag epoch
    void get_rs_drag_classy(double& result)
    {
      using namespace Pipes::get_rs_drag_classy;

      result = BEreq::class_get_rs();
    }

  } // namespace CosmoBit
} // namespace Gambit
