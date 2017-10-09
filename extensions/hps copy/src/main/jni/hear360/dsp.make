
########################################################################################################################

# © 2016 Hear360

########################################################################################################################

hear360_dsp_os_objects =\
hear360/dsp/os/memory$(o)\
hear360/dsp/os/subnormal$(o)

hear360_dsp_low_objects =\
hear360/dsp/low/equalizerband$(o)\
hear360/dsp/low/stereoequalizer$(o)\
hear360/dsp/low/monoequalizer$(o)

hear360_dsp_high_hrir_objects =\
hear360/dsp/high/hrirfolddown$(o)\
hear360/dsp/high/hrirfolddownsimple$(o)\
hear360/dsp/high/convolutioncore$(o)

########################################################################################################################

hear360_dsp_objects =\
$(hear360_dsp_os_objects)\
$(hear360_dsp_low_objects)\
$(hear360_dsp_high_hrir_objects)

########################################################################################################################

hear360_dsp_clean:
	cd hear360/dsp/os && $(rmo)
	cd hear360/dsp/low && $(rmo)
	cd hear360/dsp/high && $(rmo)

hear360_dsp_build: $(hear360_dsp_objects)

########################################################################################################################
