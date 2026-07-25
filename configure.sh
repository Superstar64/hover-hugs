#!/bin/sh

# Manual configuration script

prefix=/usr/local
hugspath=".:{Home}/lib/hugs/packages/*:{Hugs}/packages/*"

for flag; do
    case "$flag" in
        --prefix=*) prefix=${flag#*=};;
        *)
            echo "$0: unknown flag: $flag"
            exit 1;;
    esac
done

exec 3> src/options.h

echo "#pragma once" >&3
echo "#define HUGSPATH \"${hugspath}\"" >&3
echo "#define HUGSSUFFIXES \".hs:.lhs\"" >&3
echo "#define HUGSDIR \"${prefix}/lib/hugs\"" >&3

exec 3>&-