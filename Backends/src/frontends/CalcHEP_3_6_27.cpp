//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Frontend for CalcHEP Backend
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Sanjay Bloor
///          (sanjay.bloor12@imperial.ac.uk)
///  \date 2017 May, Oct
///        2018 Sep
///
///  *****************************************

#include <fstream>

#include "gambit/Backends/frontend_macros.hpp"
#include "gambit/Models/partmap.hpp"
#include "gambit/Backends/frontends/CalcHEP_3_6_27.hpp"
#include "gambit/Models/SpectrumContents/RegisteredSpectra.hpp"
#include "gambit/Elements/decay_table.hpp"
#include <unistd.h>

BE_INI_FUNCTION
{

  // Scan-level.
  static bool scan_level = true;

  if (scan_level)
  {
    // Declare backend path variables
    str BEpath;
    const char *path;
    char *modeltoset;

    if (ModelInUse("SingletDM"))
    {
      // Set model within CalcHEP
      BEpath = backendDir + "/../models/SingletDM"; // This may, in the future, change to the GAMBIT model directory.
      path = BEpath.c_str();
      modeltoset = (char*)malloc(strlen(path)+11);
      sprintf(modeltoset, "%s", path);
    }

    int error = setModel(modeltoset, 1);
    if (error != 0) backend_error().raise(LOCAL_INFO, "Unable to set model" + std::string(modeltoset) +
          " in CalcHEP. CalcHEP error code: " + std::to_string(error) + ". Please check your model files.\n");
  }

  // Point-level.
  scan_level = false;

  if (ModelInUse("SingletDM"))
  {
    // Obtain model contents
    static const SpectrumContents::SingletDM SingletDM_contents;

    // Obtain list of all parameters within model
    static const std::vector<SpectrumParameter> SingletDM_params = SingletDM_contents.all_parameters();

    // Obtain spectrum information to pass to CalcHEP
    const Spectrum& spec = *Dep::SingletDM_spectrum;

    Assign_All_Values(spec, SingletDM_params);
  }

}
END_BE_INI_FUNCTION

BE_NAMESPACE
{
  /// Assigns gambit value to parameter, with error-checking.
  void Assign_Value(char *parameter, double value)
  {
    int error;
    error = assignVal(parameter, value);
    if (error != 0) backend_error().raise(LOCAL_INFO, "Unable to set " + std::string(parameter) +
          " in CalcHEP. CalcHEP error code: " + std::to_string(error) + ". Please check your model files.\n");
  }

  /// Takes all parameters in a model, and assigns them by value to the appropriate CalcHEP parameter names.
  void Assign_All_Values(const Spectrum& spec, std::vector<SpectrumParameter> params)
  {

    for (auto it = params.begin(); it != params.end(); ++it)
    {

      // If the parameter is a Pole Mass, pass the mass by PDG code
      if (it->tag() == Par::Pole_Mass)
      {
        // Scalar case
        if (it->shape()[0] == 1)
        {
          std::pair<int,int> PDG_code = Models::ParticleDB().partmap::pdg_pair(it->name());
          Assign_Value(pdg2mass(PDG_code.first), spec.get(Par::Pole_Mass, PDG_code.first, PDG_code.second));
        }
        // Vector case
        else
        {
          for (int i=0; i<it->shape()[0]; ++i)
          {
            str long_name = it->name() + "_" + std::to_string(i+1);
            std::pair<int,int> PDG_code = Models::ParticleDB().partmap::pdg_pair(long_name);
            Assign_Value(pdg2mass(PDG_code.first), spec.get(Par::Pole_Mass, PDG_code.first, PDG_code.second));
          }
        }
        // Ignore any matrix cases, as Par::Pole_Mass should only be scalars or vectors
      }

      // If it is dimensionless, handle on case-by-case basis
      else if (it->tag() == Par::dimensionless)
      {
        // TODO - it->name() to be replaced by it->externalName()
        Assign_Value(const_cast<char*>(it->name().c_str()), spec.get(Par::dimensionless, it->name()));
      }

      // N.B. neutrinos can be easily added to this when they are implemented in GAMBIT.
    }
    double theta_W = asin(sqrt(spec.get(Par::dimensionless, "sinW2")));

    // Create SMInputs struct from spectrum
    const SMInputs& sminputs = spec.get_SMInputs();

    // First, some constants. These should always have the correct names if generated by SARAH/FeynRules...
    Assign_Value((char*)"Gf", sminputs.GF);                      // Fermi
    Assign_Value((char*)"aS", sminputs.alphaS);                  // Strong coupling (mZ)
    Assign_Value((char*)"aEWinv", sminputs.alphainv);            // Inverse EM coupling

    // Then, particle masses by PDG -- more secure than particle names.
    Assign_Value(pdg2mass(1), sminputs.mD);                        // Down
    Assign_Value(pdg2mass(2), sminputs.mU);                        // Up
    Assign_Value(pdg2mass(3), sminputs.mS);                        // Strange
    Assign_Value(pdg2mass(4), sminputs.mCmC);                      // Charm (mC) MSbar
    Assign_Value(pdg2mass(11), spec.get(Par::Pole_Mass, "e-"));    // Electron
    Assign_Value(pdg2mass(13), spec.get(Par::Pole_Mass, "mu-"));   // Muon
    Assign_Value(pdg2mass(15), spec.get(Par::Pole_Mass, "tau-"));  // Tau
    Assign_Value(pdg2mass(23), spec.get(Par::Pole_Mass, "Z0"));    // Z
    Assign_Value(pdg2mass(5), spec.get(Par::Pole_Mass, "d_3"));    // mB(mB) MSbar
    Assign_Value(pdg2mass(6), spec.get(Par::Pole_Mass, "u_3"));    // mT(mT) MSbar
    Assign_Value((char*)"g3", spec.get(Par::dimensionless, "g3")); // Strong
    Assign_Value((char*)"TW", theta_W);                            // Weinberg angle
  }

  /// Passes the width of each BSM particle in the model, from DecayTable to CalcHEP. For every 2->2 process -- for internal lines.
  void Assign_Widths(const DecayTable& tbl)
  {
    // Obtain all SM pdg codes. We don't want to set these widths at every point.
    const std::vector<std::pair<int, int>> SM_particles = Models::ParticleDB().partmap::get_SM_particles();

    // Iterate through DecayTable. If it is not in the SM particles (i.e. changes at every point), then assign the width in CalcHEP.
    for (auto it = tbl.particles.begin(); it != tbl.particles.end(); ++it)
    {
      Assign_Value(pdg2width(it->first.first), tbl.at(it->first).width_in_GeV);
    }
  }

  /// Provides spin-averaged decay width for 2 body decay process in CM frame at tree-level.
  // TODO: remove dependence on g3 (for alphaS(mZ)).
  double CH_Decay_Width(str& model, str& in, std::vector<str>& out, double& QCD_coupling)
  {
    // Check size of in and out states;
    if (out.size() != 2) backend_error().raise(LOCAL_INFO, "Output vector"
                        " must have only 2 entries for a 1 to 2 process.");

    // Calculates and updates all PUBLIC (model-dependent) parameters. These come from $CALCHEP/aux/VandP.c, generated by setModel() in the INI_FUNCTION.
    int err = calcMainFunc();

    if(err != 0) backend_error().raise(LOCAL_INFO, "Unable to calculate parameter " + std::string(varNames[err]) +
          " in CalcHEP. Please check your model files.\n");

    // Check if channel is kinematically open before doing anything. No need to compile processes that are not relevant.
    char *inbound = new char[(in).length() + 1];
    strcpy(inbound, (in).c_str());

    char *outbound_1 = new char[(out[0]).length() + 1];
    strcpy(outbound_1, (out[0]).c_str());

    char *outbound_2 = new char[(out[1]).length() + 1];
    strcpy(outbound_2, (out[1]).c_str());

    // Obtain mass of decaying particle
    double M =  *pMass(inbound);
    double m1 = *pMass(outbound_1);
    double m2 = *pMass(outbound_2);

    // If channel is kinematically closed, return 0.
    if (m1 + m2 > M) { return 0; }

    // Generate process from in and out states
    char *process = new char[(in + " -> " + out[0] + "," + out[1]).length() + 1];
    strcpy(process, (in + " -> " + out[0] + "," + out[1]).c_str());

    // Remove all non-alpha numeric characters from the library names
    in.resize(std::remove_if(in.begin(), in.end(), [](char x) {return !isalnum(x) && !isspace(x);})-in.begin());
    out[0].resize(std::remove_if(out[0].begin(), out[0].end(), [](char x) {return !isalnum(x) && !isspace(x);})-out[0].begin());
    out[1].resize(std::remove_if(out[1].begin(), out[1].end(), [](char x) {return !isalnum(x) && !isspace(x);})-out[1].begin());

    // Generate libname from model and process name
    char *libname = new char[(model + "_" + in + "_to_" + out[0] + out[1]).length() + 1];
    strcpy(libname, (model + "_" + in + "_to_" + out[0] + out[1]).c_str());

    char *excludeVirtual = NULL; // Exclude any internal particles
    char *excludeOut = NULL;     // Exclude any products
    int twidth = 0;              // T-channel propagator width
    int UG = 0;                  // Unitary gauge

    // Generates shared object file based on libName - unless it already exists.
    numout *cc = getMEcode(twidth, UG, process, excludeVirtual, excludeOut, libname);

    // Export numerical values of parameters to link to dynamical code
    err=passParameters(cc);

    if(err != 0) backend_error().raise(LOCAL_INFO, "Unable to calculate parameter " + std::string(varNames[err]) +
          " in CalcHEP. Please check your model files.\n");

    // Kinematic factors.
    double m_plus = m1+m2;
    double m_minus = m1-m2;
    double Msquared = M*M;
    double p = sqrt((Msquared - m_plus*m_plus)*(Msquared - m_minus*m_minus))/(2*M); // Magnitude of momentum in CM frame
    double E_1 = (Msquared + m1*m1 - m2*m2)/(2*M);                                  // Energy of first particle

    // Momentum vector for decay. This is 3 4-vectors, for the decaying particle and the products, respectively. 1 <--- M ---> 2
    double pvect[12] = {M, 0, 0, 0, E_1, 0, 0, p, (M-E_1), 0, 0, -p};

    // Compute squared matrix element
    double matElement = cc -> interface -> sqme(1, QCD_coupling, pvect, NULL, &err);

    // Compute kinematic prefactor for X -> Y, Z decay
    double prefactor = p/(8*pi*Msquared);

    // Return partial width
    return prefactor*matElement;
  }

  /// Computes annihilation cross-section for 2->2 process, DM+DMbar -> X + Y at tree level.
  /// Coannihilations not currently supported; we require the mass of both in states are equal.
  double CH_Sigma_V(str& model, std::vector<str>& in, std::vector<str>& out, double& QCD_coupling, double& v_rel, const DecayTable& decays)
  {
    // Check size of in and out states;
    if (in.size() != 2 or out.size() != 2) backend_error().raise(LOCAL_INFO, "Input and output vectors"
                        " must have only 2 entries for a 2 to 2 process.");

    // Calculates and updates all PUBLIC (model-dependent) parameters. These come from $CALCHEP/aux/VandP.c, generated by setModel() in the INI_FUNCTION.
    int err = calcMainFunc();

    if(err != 0) backend_error().raise(LOCAL_INFO, "Unable to calculate parameter " + std::string(varNames[err]) +
          " in CalcHEP. Please check your model files.\n");

    // Check if channel is kinematically open before doing anything. No need to compile processes that are not relevant.
    char *inbound_1 = new char[(in[0]).length() + 1];
    strcpy(inbound_1, (in[0]).c_str());

    char *inbound_2 = new char[(in[1]).length() + 1];
    strcpy(inbound_2, (in[1]).c_str());

    char *outbound_1 = new char[(out[0]).length() + 1];
    strcpy(outbound_1, (out[0]).c_str());

    char *outbound_2 = new char[(out[1]).length() + 1];
    strcpy(outbound_2, (out[1]).c_str());

    // Obtain mass of in & out states
    double m_DM = *pMass(inbound_1);
    if (*pMass(inbound_2) != m_DM) backend_error().raise(LOCAL_INFO, "Mass for both in states must be identical for CH_Sigma_V. "
                                  "Coannihilations not currently supported.");
    double m3 = *pMass(outbound_1);
    double m4 = *pMass(outbound_2);

    // Kinematics (in states)
    double s = 16*m_DM*m_DM/(4-v_rel*v_rel);
    double E_cm = sqrt(s);
    double E_1 = E_cm/2;
    double E_2 = E_1;
    double p_in = m_DM*v_rel/(sqrt(4-v_rel*v_rel));

    // Not enough energy for the process. Closed.
    if (m3+m4 > E_cm) { return 0; }

    // Pass particle widths - for propagators.
    Assign_Widths(decays);

    // Generate process from in and out states
    char *process = new char[(in[0]+ "," + in[1] + " -> " + out[0] + "," + out[1]).length() + 1];
    strcpy(process, (in[0] + "," + in[1] + " -> " + out[0] + "," + out[1]).c_str());

    str DM = in[0]; // Create copy of the string, as we are about to manipulate.
    str DMbar = in[1];

    // Remove all non-alpha numeric characters from the library names
    DM.resize(std::remove_if(DM.begin(), DM.end(), [](char x) {return !isalnum(x) && !isspace(x);})-DM.begin());
    DMbar.resize(std::remove_if(DMbar.begin(), DMbar.end(), [](char x) {return !isalnum(x) && !isspace(x);})-DMbar.begin());
    out[0].resize(std::remove_if(out[0].begin(), out[0].end(), [](char x) {return !isalnum(x) && !isspace(x);})-out[0].begin());
    out[1].resize(std::remove_if(out[1].begin(), out[1].end(), [](char x) {return !isalnum(x) && !isspace(x);})-out[1].begin());

    // Generate libname from model and process name
    char *libname = new char[(model + "_" + DM + DMbar + "_to_" + out[0] + out[1]).length() + 1];
    strcpy(libname, (model + "_" + DM + DMbar + "_to_" + out[0] + out[1]).c_str());

    /// TODO: are these options for the function..?
    char *excludeVirtual = NULL; // Exclude any internal particles
    char *excludeOut = NULL;     // Exclude any products
    int twidth = 0;              // T-channel propagator width
    int UG = 0;                  // Unitary gauge

    // Generates shared object file based on libName - unless it already exists.
    numout *cc = getMEcode(twidth, UG, process, excludeVirtual, excludeOut, libname);

    // Export numerical values of parameters to link to dynamical code
    passParameters(cc);

    if(err != 0) backend_error().raise(LOCAL_INFO, "Unable to calculate parameter " + std::string(varNames[err]) +
          " in CalcHEP. Please check your model files.\n");

    // Kinematic factors (out states)
    double E_3 = (s - (m4*m4) + (m3*m3))/(2*E_cm);
    double E_4 = E_cm - E_3;
    double m_out_plus = m3+m4;
    double m_out_minus = m3-m4;
    double p_out = sqrt((s - m_out_plus*m_out_plus)*(s - m_out_minus*m_out_minus))/(2*E_cm); // Magnitude of outbound momentum in CM frame

    // Momentum vector for 2->2. This is 4 4-vectors, for particles 1->4 respectively.
    double pvect[16] = {E_1, 0, 0, p_in, E_2, 0, 0, -p_in, E_3, 0, 0, p_out, E_4, 0, 0, -p_out};

    // Kinematic prefactor
    double prefactor = p_out/(32*pi*m_DM*s/sqrt(4-v_rel*v_rel));

    // Squared matrix element - f(p(theta)) - to be integrated over.
    double M_squared = 0.0;

    int numsteps = 200;
    // Integrate between -1 < cos(theta) < 1
    for (int i=0; i < numsteps; i++)
    {
      double dcos = 2. / numsteps;
      double cosT = -1 + dcos*i;
      double sinT = sqrt(1-cosT*cosT);
      pvect[9] = p_out*sinT;
      pvect[11] = p_out*cosT;
      pvect[13] = -pvect[9];
      pvect[15] = -pvect[11];
      M_squared += dcos*(cc -> interface -> sqme(1, QCD_coupling, pvect, NULL, &err)); // dcos * dM_squared/dcos
    }

    // Total sigma_v
    return prefactor*M_squared;
  }
}
END_BE_NAMESPACE
