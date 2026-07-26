#!/bin/sh
unifdef $(find src -name "*.c") $(find src -name "*.h") $(find src -name "*.y") -m $*