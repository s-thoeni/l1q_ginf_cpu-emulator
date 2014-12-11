#include <stdint.h>
#include "src/sid.h"
#include "src/sys.h"
#include "src/types.h"



extern "C" {
    //	InitAll(argc, argv);
    //	int32 speed = PrefsFindInt32("speed");
    // Init everything
    //extern void InitAll(int &argc, char **&argv);

    // Exit everything
    //    extern void ExitAll();

    void cSIDInit(){
	SIDInit();
    }
    // Exit SID emulation
    void cSIDExit(){
	SIDExit();
    }

// Reset SID emulation
    void cSIDReset(uint32  now){
	SIDReset((cycle_t) now);
    }

    void csid_write(uint32 adr, uint32 byte, uint32 now, int rmw){
	sid_write(adr, byte, (cycle_t) now, (bool) rmw);
    }
}
