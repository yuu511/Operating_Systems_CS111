#!/bin/sh
cd /usr/src
make buildkernel -j8 KERNCONF=GENERIC
make installkernel KERNCONF=GENERIC
