#!/bin/sh

while [ -n "$1" ];do
	case "$1" in
		play) #setup evb board
			aplay /tmp/audio.wav
			;;
		record) #setupkevin board
			arecord -f cd -d 10 /tmp/audio.wav
			;;
		test) #setup gru board
			aplay /dev/urandom
			;;
		init) #setup gru board
                        amixer cset name="HP Playback Switch" 1
                        amixer cset name="HP Playback Volume" 8
                        amixer cset name="HPO MIX DAC1 Switch" 1
                        amixer cset name="OUT MIXR DAC R1 Switch" 1
                        amixer cset name="OUT MIXL DAC L1 Switch" 1
                        amixer cset name="Stereo DAC MIXR DAC R1 Switch" 1
                        amixer cset name="Stereo DAC MIXL DAC L1 Switch" 1
			amixer cset name="IN2 Boost Volume" 1
                        amixer cset name="ADC Boost Volume" 3 3
                        amixer cset name="DAC MIXR INF1 Switch" 1
                        amixer cset name="DAC MIXL INF1 Switch" 1
                        amixer cset name="Stereo1 ADC MIXR ADC1 Switch" 1
                        amixer cset name="Stereo1 ADC MIXL ADC1 Switch" 1
                        amixer cset name="RECMIXR INR1 Switch" 1
                        amixer cset name="RECMIXR BST1 Switch" 1
                        amixer cset name="RECMIXL INL1 Switch" 1
                        amixer cset name="RECMIXL BST1 Switch" 1
			;;
	esac
	shift
done
