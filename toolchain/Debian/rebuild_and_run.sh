#!/bin/bash
cd `dirname $0`
./rebuild.sh && ./prepare_hda.sh && ./launch_x86_64.sh
