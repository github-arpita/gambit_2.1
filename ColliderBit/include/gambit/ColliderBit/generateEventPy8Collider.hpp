//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  ColliderBit event loop functions returning
///  collider Monte Carlo events.
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Abram Krislock
///          (a.m.b.krislock@fys.uio.no)
///
///  \author Aldo Saavedra
///
///  \author Andy Buckley
///
///  \author Chris Rogan
///          (crogan@cern.ch)
///  \date 2014 Aug
///  \date 2015 May
///
///  \author Pat Scott
///          (p.scott@imperial.ac.uk)
///  \date 2015 Jul
///  \date 2018 Jan
///  \date 2019 Jan
///  \date 2019 May
///
///  \author Anders Kvellestad
///          (anders.kvellestad@fys.uio.no)
///  \date   2017 March
///  \date   2018 Jan
///  \date   2018 May
///
///  *********************************************

#include "gambit/ColliderBit/ColliderBit_eventloop.hpp"
#include "gambit/ColliderBit/colliders/Pythia8/Py8EventConversions.hpp"

// #define COLLIDERBIT_DEBUG
#define DEBUG_PREFIX "DEBUG: OMP thread " << omp_get_thread_num() << ":  "

namespace Gambit
{

  namespace ColliderBit
  {

    /// Generate a hard scattering event with Pythia
    template<typename PythiaT, typename EventT>
    void generateEventPy8Collider(HEPUtils::Event& event,
                                  const MCLoopInfo& RunMC,
                                  const Py8Collider<PythiaT,EventT>& HardScatteringSim,
                                  const map_int_ProcessXsecInfo & ProcessCrossSections,
                                  const int iteration,
                                  void(*wrapup)())
    {
      static int nFailedEvents;
      thread_local EventT pythia_event;

      // If the event loop has not been entered yet, reset the counter for the number of failed events
      if (iteration == BASE_INIT)
      {
        // Although BASE_INIT should never be called in parallel, we do this in omp_critical just in case.
        #pragma omp critical (pythia_event_failure)
        {
          nFailedEvents = 0;
        }
        return;
      }

      // If in any other special iteration, do nothing
      if (iteration < BASE_INIT) return;

      // Reset the Pythia and HEPUtils events
      pythia_event.clear();
      event.clear();

      // Attempt (possibly repeatedly) to generate an event
      while(nFailedEvents <= RunMC.current_maxFailedEvents())
      {
        try
        {
          HardScatteringSim.nextEvent(pythia_event);
          break;
        }
        catch (typename Py8Collider<PythiaT,EventT>::EventGenerationError& e)
        {
          #ifdef COLLIDERBIT_DEBUG
          cout << DEBUG_PREFIX << "Py8Collider::EventGenerationError caught in generateEventPy8Collider. Check the ColliderBit log for event details." << endl;
          #endif
          #pragma omp critical (pythia_event_failure)
          {
            // Update global counter
            nFailedEvents += 1;
            // Store Pythia event record in the logs
            std::stringstream ss;
            pythia_event.list(ss, 1);
            logger() << LogTags::debug << "Py8Collider::EventGenerationError error caught in generateEventPy8Collider. Pythia record for event that failed:\n" << ss.str() << EOM;
          }
        }
      }

      // Wrap up event loop if too many events fail.
      if(nFailedEvents > RunMC.current_maxFailedEvents())
      {
        if(RunMC.current_invalidate_failed_points())
        {
          piped_invalid_point.request("exceeded maxFailedEvents");
        }
        else
        {
          piped_warnings.request(LOCAL_INFO,"exceeded maxFailedEvents");
        }
        wrapup();
        return;
      }

      // Attempt to convert the Pythia event to a HEPUtils event
      try
      {
        if (HardScatteringSim.partonOnly)
          convertPartonEvent(pythia_event, event, HardScatteringSim.antiktR);
        else
          convertParticleEvent(pythia_event, event, HardScatteringSim.antiktR);
      }
      // No good.
      catch (Gambit::exception& e)
      {
        #ifdef COLLIDERBIT_DEBUG
          cout << DEBUG_PREFIX << "Gambit::exception caught during event conversion in generateEventPy8Collider. Check the ColliderBit log for details." << endl;
        #endif

        #pragma omp critical (event_conversion_error)
        {
          // Store Pythia event record in the logs
          std::stringstream ss;
          pythia_event.list(ss, 1);
          logger() << LogTags::debug << "Gambit::exception caught in generateEventPy8Collider. Pythia record for event that failed:\n" << ss.str() << EOM;
        }

        str errmsg = "Bad point: generateEventPy8Collider caught the following runtime error: ";
        errmsg    += e.what();
        piped_invalid_point.request(errmsg);
        wrapup();
        return;
      }

      // Assign weight to event
      // _Anders: work in progress
      double weight = 1.0;
      int process_code = HardScatteringSim.pythia()->info.code();

      ProcessXsecInfo xs_info = ProcessCrossSections.at(process_code);

      cout << DEBUG_PREFIX << "process_code: " << process_code << ",  process_xsec: " << xs_info.process_xsec() << ",  pid_pairs.size(): " << xs_info.pid_pairs.size() << ",  processes_sharing_xs.size(): " << xs_info.processes_sharing_xsec.size() << endl;


      event.set_weight(weight);

      // //  Fra Are

      // // Separate into two cases:
      // // i)  Single set of PIDs for process, possible multiple processes with same PID (initial states)
      // // ii) Multiple PIDs for same process (ccs), usually only one process with those PIDs
      // //     (The exception is squark--anti-squark production, so we need also to loop over processes with same PID)
      // int nPIDs = proc2PID.count(process);
      // double PYxsec = 0;
      // // Case i)
      // if(nPIDs == 1){
      //   // Find Pythia LO cross section from all processes with same PIDs
      //   PIDs current = proc2PID.find(process)->second;
      //   for(multimap<int,PIDs>::iterator it = proc2PID.begin(); it != proc2PID.end(); ++it){
      //     if(it->second == current) PYxsec += info.sigmaGen(it->first);
      //   }
      //   weight = xsec[process]*1E-9/PYxsec;
      // }
      // // Case ii)
      // else if(nPIDs > 1){
      //   // The NLO cross section from all PID combinations is already stored in the xsec map
      //   // Find Pythia LO cross section from all processes with same PIDs
      //   PIDs current = proc2PID.find(process)->second; // Now this is not unique for the process code, but this does not matter
      //   for(multimap<int,PIDs>::iterator it = proc2PID.begin(); it != proc2PID.end(); ++it){
      //     if(it->second == current) PYxsec += info.sigmaGen(it->first);
      //   }
      //   weight = xsec[process]*1E-9/PYxsec;
      // }
      // else{
      //   cout << "WARNING! Could not find the generated process " << process << " in NLO lookup! Using identity weight." << endl;
      //   weight = 1.0;
      // }
      // //cout << "Weight: " << weight << endl;

    }

    /// Generate a hard scattering event with a specific Pythia
    #define GET_PYTHIA_EVENT(NAME)                               \
    void NAME(HEPUtils::Event& result)                           \
    {                                                            \
      using namespace Pipes::NAME;                               \
      generateEventPy8Collider(result, *Dep::RunMC,              \
       *Dep::HardScatteringSim, *Dep::ProcessCrossSections,      \
       *Loop::iteration, Loop::wrapup);                          \
    }

  }

}
