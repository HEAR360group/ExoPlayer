
//######################################################################################################################

// © 2016 Hear360

//######################################################################################################################

#ifndef hear360_dsp_private_H
#define hear360_dsp_private_H

//######################################################################################################################

#if (defined (_MSC_VER) || defined (__XTENSA__))

  // msvc and xtensa don't support the "= delete" keyword, so we just omit it.

  #define hear360_dsp_private_DELETEFUNCTION

#else

  #define hear360_dsp_private_DELETEFUNCTION = delete

#endif

//######################################################################################################################
/*

This macro creates a PRIVATE struct for your class, includes a private pointer to it, and deletes
 the copy constructor and copy assignment operator.

Remember that you still need to create a constructor and destructor that dynamically allocates
 memory for the private pointer, if you wish to use it.

*/

#define hear360_dsp_private_SIMPLE(classname)                                          \
                                                                                        \
public:                                                                                 \
                                                                                        \
  struct PRIVATE;                                                                       \
                                                                                        \
private:                                                                                \
                                                                                        \
  PRIVATE *pprivate;                                                                    \
                                                                                        \
  classname (const classname &) hear360_dsp_private_DELETEFUNCTION;                    \
                                                                                        \
  classname & operator = (const classname &) hear360_dsp_private_DELETEFUNCTION;

//######################################################################################################################
/*

This is similar to the SIMPLE version above, but written to avoid dynamic memory allocations and c++ exceptions.

You may still create a constructor, but it should be limited to initialization lists and operations that cannot fail.

You will need to declare the PRIVATE type in a private header file.
You can make it public or private depending on whether or not there are functions outside the class
  that will need to access it.

*/

#define hear360_dsp_private_LOCAL(classname)                                           \
                                                                                        \
private:                                                                                \
                                                                                        \
  PRIVATE privatedata;                                                                  \
                                                                                        \
  classname (const classname &) hear360_dsp_private_DELETEFUNCTION;                    \
                                                                                        \
  classname & operator = (const classname &) hear360_dsp_private_DELETEFUNCTION;

//######################################################################################################################

#endif // include guard

//######################################################################################################################
