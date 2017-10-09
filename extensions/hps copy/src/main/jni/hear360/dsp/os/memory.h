
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_dsp_os_memory_H
#define hear360_dsp_os_memory_H

//######################################################################################################################

#ifndef __XTENSA__

//######################################################################################################################

#include <cstddef>
#include <new>

//######################################################################################################################

namespace hear360_dsp_os_memory
{

//######################################################################################################################

#define hear360_dsp_os_memory_AUDIOBUFFERSIZE (1024)

//######################################################################################################################

// This memory manager doesn't use C++ exceptions or virtual functions.

struct MANAGER
{
  bool (*pGrab) (void *pmanagerdata, void **ppmemory, std::size_t totalbytes);

  // Returns true on failure.

  void (*pFree) (void *pmanagerdata, void *pmemory);

  // Doesn't report failure

  void *pmanagerdata;

  MANAGER ();
};

//######################################################################################################################

struct MONO
{
  float paudio[hear360_dsp_os_memory_AUDIOBUFFERSIZE];
};

//######################################################################################################################

struct STEREO
{
  float ppaudioasarray[2][hear360_dsp_os_memory_AUDIOBUFFERSIZE];

  float *ppaudio[2];

  STEREO ();
};

//######################################################################################################################

// Default implementation of the memory manager.
// This will use the C library, except on TrueAudio.

bool GrabDefault (void *pvoid, void **ppmemory, std::size_t totalbytes);

void FreeDefault (void *pvoid, void *pmemory);

//######################################################################################################################

// We assume the contructors never throw any C++ exceptions.

template <typename TYPE> bool Grab (MANAGER manager, TYPE **ppmemory)
{
  void *pmemory;

  if ((*(manager.pGrab))(manager.pmanagerdata, &pmemory, sizeof (TYPE)))
  {
    *ppmemory = NULL;

    return (true);
  }

  *ppmemory = new (pmemory) TYPE();

  return (false);
}

//######################################################################################################################

// We assume the destructors never throw any C++ exceptions.

template <typename TYPE> void Free (MANAGER manager, TYPE *pmemory)
{
  pmemory->~TYPE();

  (*(manager.pFree))(manager.pmanagerdata, pmemory);

  return;
}

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // Removing everything for ACP

//######################################################################################################################

#endif // include guard

//######################################################################################################################
