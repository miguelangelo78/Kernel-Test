#!/bin/bash
cd `dirname $0`
./build.sh && ./prepare_hda.sh && ./gkd-qemu-debugger.sh
