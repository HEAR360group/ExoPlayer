
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef __XTENSA__

//######################################################################################################################

#include <cstddef>
#include <cstdlib>
#include <stdint.h>
#include <new>

#include <hear360/dsp/os/memory.h>

//######################################################################################################################

namespace hear360_dsp_os_memory
{

#define MIN_ALIGNMENT (16)

//######################################################################################################################

// I haven't tested this.  For now, I'm just removing the whole module with the #ifndef above.

#ifdef __XTENSA__
  #define ENABLECLIBRARYMEMORY (0)
#else
  #define ENABLECLIBRARYMEMORY (1)
#endif

//######################################################################################################################

MANAGER::MANAGER ()
:
  pGrab (&GrabDefault),
  pFree (&FreeDefault),

  pmanagerdata (NULL)
{}

//######################################################################################################################

STEREO::STEREO ()
{
  for (int cx = 0 ; cx < 2 ; cx ++)
  {
    ppaudio[cx] = ppaudioasarray[cx];
  }

  return;
}

//######################################################################################################################

#if (ENABLECLIBRARYMEMORY)
//16 bytes alligned (ready for SIMD)
bool GrabDefault (void *pvoid, void **ppmemory, std::size_t totalbytes)
{
  (void)pvoid;

  void *p0, *p;
  if (!(p0 = malloc(totalbytes + MIN_ALIGNMENT))) return true;
  p = (void *) (((uintptr_t) p0 + MIN_ALIGNMENT) & (~((uintptr_t) (MIN_ALIGNMENT - 1))));
  *((void **) p - 1) = p0;
  *ppmemory = p;

  return false;
  /*
  unsigned char *mem = (unsigned char*)std::malloc (totalbytes + 16);   // allocate extra
  if (mem != NULL)
  {
    unsigned char *ptr = (unsigned char*)(((uintptr_t)mem+16) & ~ (uintptr_t)0x0F);
    *(ptr-1) = (unsigned char)(ptr - mem);                         // store padding size
    *ppmemory = ptr;
    return false;
  }
  else
  {
    return true;
  }
  */
}
/*
bool GrabDefault (void *pvoid, void **ppmemory, std::size_t totalbytes)
{
  (void)pvoid;

  *ppmemory = std::malloc (totalbytes);

  if (!*ppmemory) return (true);

  return (false);
}
*/
#else

bool GrabDefault (void *pvoid, void **ppmemory, std::size_t totalbytes)
{
  (void)pvoid, (void)totalbytes;

  *ppmemory = NULL;

  return (true);
}

#endif

//######################################################################################################################

#if (ENABLECLIBRARYMEMORY)
//16 bytes alligned (ready for SIMD)
void FreeDefault (void *pvoid, void *pmemory)
{
  (void)pvoid;

  if (pmemory) free(*((void **) pmemory - 1));
  /*
  unsigned char *ptr = (unsigned char*)pmemory;                   // work out original
  ptr = ptr - *(ptr-1);                 // by subtracting padding
  free (ptr);
  */
  return;
}
/*
void FreeDefault (void *pvoid, void *pmemory)
{
  (void)pvoid;

  std::free (pmemory);

  return;
}
*/
#else

void FreeDefault (void *pvoid, void *pmemory)
{
  (void)pvoid, (void)pmemory;

  return;
}

#endif

//######################################################################################################################

} // namespace

//######################################################################################################################

#endif // Removing everything for ACP

//######################################################################################################################
