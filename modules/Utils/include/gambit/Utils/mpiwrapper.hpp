//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  A simple C++ wrapper for the MPI C bindings.
///  It is by no means comprehensive, and is just
///  intended to simplify some MPI commands within
///  Gambit.
///
///  Extend as needed.
///
///  Note: Some C++ bindings already exist, but
///  it appears that they are deprecated in really
///  new versions of the MPI standard, and don't
///  offer functionality over the C bindings
///  anyway. I have nevertheless mirrored the names
///  of some of the deprecated C++ bindings, in
///  case it helps some people who were used to
///  them.
///
///  There is also boost/mpi, but it is a compiled
///  library and we have been avoiding those.  
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///   
///  \author Ben Farmer
///          (benjamin.farmer@fysik.su.se)
///  \date 2015 Apr
///
///  *********************************************

// WITH_MPI macro determines whether MPI is used or not
// Defined at compile time by cmake. (using something like -DWITH_MPI, in gcc case)

#ifdef WITH_MPI // Contents of this file ignored if MPI not enabled
#ifndef __mpiwrapper_hpp__
#define __mpiwrapper_hpp__

#include <mpi.h>

// Can rip out the <sstream> and error handlers if you want these wrappers to be independent of gambit.
#include <sstream>
#include "gambit/Core/error_handlers.hpp"

// Trigger debugging output in various places (specifically, hdf5 printer)
#define MPI_DEBUG

namespace Gambit {
   namespace GMPI {

      /// Mapping from (basic) C++ types to MPI datatypes
      /// Idea based off of "get_hdf5_data_type" in hdf5tools.hpp

      template<typename T>
      struct get_mpi_data_type;
       // Left undefined because I want a compile error if specialisation doesn't exist.

      // Overload to work with arrays
      template<typename T, size_t SIZE> 
      struct get_mpi_data_type<T[SIZE]> { static MPI_Datatype type() { return get_mpi_data_type<T>::type(); } };

      // /// TODO: don't seem to work for arrays, need overloads?
      // template<> inline MPI_Datatype get_mpi_data_type<char>()                { return MPI_CHAR;               };
      // // bunch of types omitted in get_hdf5_data_type for reasons not totally clear to me...
      // template<> inline MPI_Datatype get_mpi_data_type<long long>()           { return MPI_LONG_LONG;          };
      // template<> inline MPI_Datatype get_mpi_data_type<unsigned long long>()  { return MPI_UNSIGNED_LONG_LONG; };
      // template<> inline MPI_Datatype get_mpi_data_type<int8_t>()              { return MPI_INT8_T;             };
      // template<> inline MPI_Datatype get_mpi_data_type<uint8_t>()             { return MPI_UINT8_T;            };
      // template<> inline MPI_Datatype get_mpi_data_type<int16_t>()             { return MPI_INT16_T;            };
      // template<> inline MPI_Datatype get_mpi_data_type<uint16_t>()            { return MPI_UINT16_T;           };
      // template<> inline MPI_Datatype get_mpi_data_type<int32_t>()             { return MPI_INT32_T;            };
      // template<> inline MPI_Datatype get_mpi_data_type<uint32_t>()            { return MPI_UINT32_T;           };
      // template<> inline MPI_Datatype get_mpi_data_type<int64_t>()             { return MPI_INT64_T;            };
      // template<> inline MPI_Datatype get_mpi_data_type<uint64_t>()            { return MPI_UINT64_T;           };
      // template<> inline MPI_Datatype get_mpi_data_type<float>()               { return MPI_FLOAT;              };
      // template<> inline MPI_Datatype get_mpi_data_type<double>()              { return MPI_DOUBLE;             };
      // template<> inline MPI_Datatype get_mpi_data_type<long double>()         { return MPI_LONG_DOUBLE;        };

      template<> struct get_mpi_data_type<char>              { static MPI_Datatype type() { return MPI_CHAR;       } };
      // bunch of types omitted in get_hdf5_data_type for reasons not totally clear to me...
      template<> struct get_mpi_data_type<long long>         { static MPI_Datatype type() { return MPI_LONG_LONG;  } };
      template<> struct get_mpi_data_type<unsigned long long>{ static MPI_Datatype type() { return MPI_UNSIGNED_LONG_LONG; } };
      template<> struct get_mpi_data_type<int8_t>            { static MPI_Datatype type() { return MPI_INT8_T;     } };
      template<> struct get_mpi_data_type<uint8_t>           { static MPI_Datatype type() { return MPI_UINT8_T;    } };
      template<> struct get_mpi_data_type<int16_t>           { static MPI_Datatype type() { return MPI_INT16_T;    } };
      template<> struct get_mpi_data_type<uint16_t>          { static MPI_Datatype type() { return MPI_UINT16_T;   } };
      template<> struct get_mpi_data_type<int32_t>           { static MPI_Datatype type() { return MPI_INT32_T;    } };
      template<> struct get_mpi_data_type<uint32_t>          { static MPI_Datatype type() { return MPI_UINT32_T;   } };
      template<> struct get_mpi_data_type<int64_t>           { static MPI_Datatype type() { return MPI_INT64_T;    } };
      template<> struct get_mpi_data_type<uint64_t>          { static MPI_Datatype type() { return MPI_UINT64_T;   } };
      template<> struct get_mpi_data_type<float>             { static MPI_Datatype type() { return MPI_FLOAT;      } };
      template<> struct get_mpi_data_type<double>            { static MPI_Datatype type() { return MPI_DOUBLE;     } };
      template<> struct get_mpi_data_type<long double>       { static MPI_Datatype type() { return MPI_LONG_DOUBLE;} };

      // Doing same thing with bools (which C does not have) as in the hdf5 case:
      template<> struct get_mpi_data_type<bool>              { static MPI_Datatype type() { return MPI_UINT8_T;    } };

      /// Main "Communicator" class
      class Comm
      {
         public:
            /// Default Constructor
            Comm();

            /// Duplicate constructor
            Comm(const MPI_Comm& comm);
        
            /// Get total number of MPI tasks in this communicator group
            int Get_size() const;

            /// Get "rank" (ID number) of current task in this communicator group
            int Get_rank() const;

            /// Prevent further executation until all members of the bound communicator group enter the call
            void Barrier()
            {
              int errflag;
              errflag = MPI_Barrier(boundcomm);
            }

            /// Blocking receive
            ///  void*        buf      - memory address in which to store received message
            ///  int          count    - number of elements in message
            ///  MPI_Datatype datatype - datatype of each message element
            ///  int          source   - rank of sending (receiving?) process
            ///  int          tag      - message tag          
            ///  MPI_status*  status   - struct containing data about the received message
            /// Returns:
            ///  MPI_status - struct containing data about the received message
            void Recv(void *buf /*out*/, int count, MPI_Datatype datatype, 
                                  int source, int tag, 
                                  MPI_Status *in_status=NULL /*out*/)
            {
               int errflag;
               MPI_Status* status = MPI_STATUS_IGNORE; 
               if(in_status!=NULL) status=in_status;
               errflag = MPI_Recv(buf, count, datatype, source, tag, boundcomm, status);
            }

            /// Templated blocking receive to automatically determine types
            template<class T>
            void Recv(T *buf /*out*/, int count, 
                      int source, int tag, 
                      MPI_Status *status=NULL /*out*/)
            {
               static const MPI_Datatype datatype = get_mpi_data_type<T>::type();
               Recv(buf, count, datatype, source, tag, status);
            }

            /// Blocking send
            void Send(void *buf, int count, MPI_Datatype datatype, 
                                  int destination, int tag)
            {
               int errflag; 
               errflag = MPI_Send(buf, count, datatype, destination, tag, boundcomm);
            }

            /// Templated blocking send
            template<class T>
            void Send(T *buf, int count, 
                      int destination, int tag)
            {
               static const MPI_Datatype datatype = get_mpi_data_type<T>::type();
               Send(buf, count, datatype, destination, tag);
            }

 
            /// Non-blocking send
            void Isend(void *buf, int count, MPI_Datatype datatype, 
                                  int destination, int tag, 
                                  MPI_Request *request /*out*/)
            {
               int errflag; 
               errflag = MPI_Isend(buf, count, datatype, destination, tag, boundcomm, request);
            }

            /// Templated Non-blocking send
            template<class T>
            void Isend(T *buf, int count, 
                      int destination, int tag, 
                      MPI_Request *request /*out*/)
            {
               static const MPI_Datatype datatype = get_mpi_data_type<T>::type();
               Isend(buf, count, datatype, destination, tag, request);
            }

            /// Blocking wait for e.g. Isend to complete
            //void Wait(MPI_Request *request, MPI_Status *status)
            //{
            //   MPI_Wait(MPI_Request *request, MPI_Status *status)
            // }

            // Probe for messages waiting to be delivered
            bool Iprobe(int source, int tag, MPI_Status* in_status=NULL /*out*/)
            {
               int errflag;
               int you_have_mail; // C does not have a bool type...
               MPI_Status def_status;
               MPI_Status* status;
               if(in_status!=NULL) {
                 status = in_status;
               } else {
                 status = &def_status;
               }
               MPI_Iprobe(source, 1, boundcomm, &you_have_mail, status);
               errflag = MPI_Iprobe(source, tag, boundcomm, &you_have_mail, status);
               if(errflag!=0) {
                 std::ostringstream errmsg;
                 errmsg << "Error performing MPI_Iprobe! Received error flag: "<<errflag; 
                 utils_error().raise(LOCAL_INFO, errmsg.str());
               }
               #ifdef MPI_DEBUG
               if(you_have_mail!=0) {
                     std::cout<<"Iprobe: Message waiting from process "<<status->MPI_SOURCE<<std::endl;
               }
               #endif
               return (you_have_mail != 0);
            }

         private:

            // The MPI communicator to which the current object "talks".
            MPI_Comm boundcomm;
      };

      /// Check if MPI_Init has been called (it is an error to call it twice)
      bool Is_initialized(); 
      
      /// Initialise MPI
      /// The arguments are required in order to "fix up" the command line 
      /// arguments so that they match their positions when the "mpirun -n X" 
      /// wrapper call is absent.
      void Init(int& argc, char**& argv);

      /// Shut down MPI
      void Finalize();


      /// @{ Helpers for registration of compound datatypes
 
      /// Structure to hold an MPI startup function plus metadata
      class MpiIniFunc {
        private:
          std::string location;
          std::string name;
          void (*func)();
        public:
          MpiIniFunc(std::string l, std::string n, void(*f)())
            : location(l)
            , name(n)
            , func(f)
          {}
          void runme()
          {
            (*func)();
          }
          std::string mylocation(){return location;}
          std::string myname    (){return name;}
      };

      /// Struct for adding functions to the 'mpi_ini_functions' map
      ///
      /// This will add functions to the map when it is constructed. Works
      /// on the same idea as the "ini_code" struct, except it doesn't
      /// cause the functions to be run, just "queues them up" so to speak.
      struct AddMpiIniFunc {
        AddMpiIniFunc(std::string local_info, std::string name, void(*func)());
      };

      /// @}

   }
}


#endif // include guard
#endif // MPI

