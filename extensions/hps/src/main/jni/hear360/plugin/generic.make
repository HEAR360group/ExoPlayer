
########################################################################################################################

# © 2016 Hear360

########################################################################################################################

hear360_plugin_generic_objects_hrirfolddown =\
hear360/plugin/generic/dsp/interleave$(o)\
hear360/plugin/generic/dsp/hrirfolddown$(o)\
hear360/plugin/generic/dsp/convolutioncore$(o)\
hear360/plugin/generic/dsp/vsheadtracking$(o)\
hear360/plugin/generic/dsp/stereoupmix51$(o)\
hear360/plugin/generic/dsp/stereoeq$(o)

########################################################################################################################

hear360_plugin_generic_dlls =\
hear360/plugin/generic/dll/hps-hrirfolddown.dll

hear360_plugin_generic_libs =\
hear360/plugin/generic/lib/hps-hrirfolddown.lib

hear360_plugin_generic_dylibs =\
hear360/plugin/generic/dll/hps-hrirfolddown.dylib

hear360_plugin_generic_so =\
hear360/plugin/generic/dll/libhps-hrirfolddown.so

hear360_plugin_generic_a =\
hear360/plugin/generic/lib/libhrirfolddown.a

########################################################################################################################

hear360_plugin_generic_clean:
	cd hear360/plugin/generic/dsp && $(rmo)
	cd hear360/plugin/generic/dll && $(rmo)
	cd hear360/plugin/generic/dll && $(rmdll)

hear360_plugin_generic_build: $(hear360_plugin_generic_objects_hrirfolddown) $(hear360_plugin_generic_dlls)

########################################################################################################################
