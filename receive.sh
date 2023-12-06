#!/bin/bash
echo "start talking"

#####################################
# this is for mumble gui client
# NOTE: the mumble client has a vox function too

# if we call starttalking multiple times, which we will while there is audio samples on the radio,
# the mumble client will need exactly the same count of stoptalking calls, to stop transmitting
# which is stupid...
# hence we will play with trasnmitting mode (0=continius, 1=vox, 2=ptt)

#mumble rpc starttalking # do not use this, use the qdbus method
qdbus net.sourceforge.mumble.mumble / net.sourceforge.mumble.Mumble.setTransmitMode 0

#####################################
# this is for barnard client
barnard_fifo="/tmp/barnard.fifo"
#echo "micup" > $barnard_fifo

