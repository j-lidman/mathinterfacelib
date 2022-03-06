#ifndef MATHINTERFACELIB_IPC_HDR
#define MATHINTERFACELIB_IPC_HDR

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#define PROCESS_SHARED_BUFF_SIZE 65536
#define MATHINTERFACELIB_SHARED_MEM_NAME "mathinterfacelibSharedMem"

namespace MathInterfaceLib {
   struct SharedRegion {
      boost::interprocess::interprocess_mutex regionInUse;
      boost::interprocess::interprocess_semaphore reader, writer;
      size_t currRead, currWrite;
      char scratchPad[PROCESS_SHARED_BUFF_SIZE];

      SharedRegion() : regionInUse(), reader(0), writer(0), currRead(0), currWrite(0) {}
      ~SharedRegion() {}
   };
};

#endif
