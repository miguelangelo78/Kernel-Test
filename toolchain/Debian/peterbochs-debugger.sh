#!/bin/bash
cd `dirname $0`/Debugger/peterbochs-debugger

java -jar peter-bochs-debugger20130922.jar bochs -f ../../../VMachs/Bochs/kerneltest.bxrc