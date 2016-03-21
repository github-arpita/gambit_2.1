#ifndef __abstract_ResonanceGmZ_Pythia_8_212_h__
#define __abstract_ResonanceGmZ_Pythia_8_212_h__

#include "gambit/Backends/abstractbase.hpp"
#include "forward_decls_abstract_classes.h"
#include "forward_decls_wrapper_classes.h"
#include <cstddef>

#include "identification.hpp"

// Forward declaration needed by the destructor pattern.
void set_delete_BEptr(CAT_3(BACKENDNAME,_,SAFE_VERSION)::Pythia8::ResonanceGmZ*, bool);


// Forward declaration needed by the destructor pattern.
void wrapper_deleter(CAT_3(BACKENDNAME,_,SAFE_VERSION)::Pythia8::ResonanceGmZ*);


// Forward declaration for wrapper_creator.
void wrapper_creator(CAT_3(BACKENDNAME,_,SAFE_VERSION)::Pythia8::Abstract_ResonanceGmZ*);


namespace CAT_3(BACKENDNAME,_,SAFE_VERSION)
{
    
    
    namespace Pythia8
    {
        class Abstract_ResonanceGmZ : public virtual AbstractBase
        {
            public:
    
            public:
                virtual void pointer_assign__BOSS(Abstract_ResonanceGmZ*) =0;
                virtual Abstract_ResonanceGmZ* pointer_copy__BOSS() =0;
    
            private:
                ResonanceGmZ* wptr;
                bool delete_wrapper;
            public:
                ResonanceGmZ* get_wptr() { return wptr; }
                void set_wptr(ResonanceGmZ* wptr_in) { wptr = wptr_in; }
                bool get_delete_wrapper() { return delete_wrapper; }
                void set_delete_wrapper(bool del_wrp_in) { delete_wrapper = del_wrp_in; }
    
            public:
                Abstract_ResonanceGmZ()
                {
                    wptr = 0;
                    delete_wrapper = false;
                }
    
                Abstract_ResonanceGmZ(const Abstract_ResonanceGmZ&)
                {
                    wptr = 0;
                    delete_wrapper = false;
                }
    
                Abstract_ResonanceGmZ& operator=(const Abstract_ResonanceGmZ&) { return *this; }
    
                virtual void init_wrapper()
                {
                    if (wptr == 0)
                    {
                        wrapper_creator(this);
                        delete_wrapper = true;
                    }
                }
    
                ResonanceGmZ* get_init_wptr()
                {
                    init_wrapper();
                    return wptr;
                }
    
                ResonanceGmZ& get_init_wref()
                {
                    init_wrapper();
                    return *wptr;
                }
    
                virtual ~Abstract_ResonanceGmZ()
                {
                    if (wptr != 0)
                    {
                        set_delete_BEptr(wptr, false);
                        if (delete_wrapper == true)
                        {
                            wrapper_deleter(wptr);
                            wptr = 0;
                            delete_wrapper = false;
                        }
                    }
                }
        };
    }
    
}


#include "gambit/Backends/backend_undefs.hpp"


#endif /* __abstract_ResonanceGmZ_Pythia_8_212_h__ */
