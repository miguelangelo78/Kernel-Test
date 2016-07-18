#!/bin/bash
cd `dirname $0`
./rebuild.sh && ./prepare_hda.sh && ./gkd-qemu-debugger.sh
