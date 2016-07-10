#!/bin/bash
cd `dirname $0`
./build.sh && ./prepare_hda.sh && ./launch_x86_64.sh 
