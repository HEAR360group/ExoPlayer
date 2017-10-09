
########################################################################################################################

# © 2016 Hear360

########################################################################################################################

hear360_algr_hrir_objects = \
hear360/algr/Base/DSPUtils$(o)\
hear360/algr/Equalizer/HPSEqualizerBand$(o)\
hear360/algr/Equalizer/HPSEqualizer4Band$(o)\
hear360/algr/Delay/HPSStaticDelay$(o)\
hear360/algr/Convolution/CRFilter$(o)

########################################################################################################################

hear360_algr_clean:
	cd hear360/algr/Base && $(rmo)
	cd hear360/algr/Convolution && $(rmo)
	cd hear360/algr/Delay && $(rmo)
	cd hear360/algr/Equalizer && $(rmo)

hear360_algr_build: $(hear360_algr_hrir_objects)

########################################################################################################################
