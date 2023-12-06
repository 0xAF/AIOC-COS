# AIOC-COS
Listen for AIOC (All-In-One-Cable) COS events via hidraw interface and start scripts when the radio is receiving or in silence.

```
AIOC_COS - Listen for COS (reception) events on AIOC (with the correct firmware).
All-In-One-Cable: https://github.com/skuep/AIOC

Usage:
  ./build/aioc_cos [-h] [-q] [-v]
        <-H /dev/hidraw?>
        <-R cos_on_script.sh>
        <-S cos_off_script.sh> [-t 2000]

  -h            Print this help.
  -q            Be quiet
  -v            Be verbose
  -H file       hiddev device file
  -R file       script to run when radio starts Receiving
  -S file       script to run when radio become Silent
  -t msec       timeout in milliseconds after the last audio
                and before calling the Silent script (default 1s)

This tool will watch the All-In-One-Cable (AIOC) hidraw device
for events. If the radio starts receiving, it will call the
cos_on_script. Respectively, when the radio is not receiving
anymore it will call cos_off_script.

At the time of writing this tool, the AIOC stable firmware is not
sending events to the hidraw interface.
But you can compile the 'autoptt' branch of AIOC or use the .bin
from this repo, to enable this function.
The 'autoptt' branch uses 10msec timeout on audio samples and then
sends Silent event, even if the radio is still receiving.
Because of this, we need our timeout in this tool, before calling
the Silent script. Hopefully this will change in the final release
of this feature in AIOC.
autoptt branch of AIOC: https://github.com/skuep/AIOC/tree/autoptt

I'm using this tool to send the audio from my radio to my Mumble
server via mumble client. The other direction, when the mumble client
receives audio and plays it on AIOC audio interface, will automatically
enable the PTT on the radio with the 'autoptt' firmware.

```
