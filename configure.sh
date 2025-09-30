#!/bin/sh

# Manual configuration script

prefix=/usr/local
build=$(./config.guess)
host="$build"
# todo, hugs is an interpter, get rid of target eventually
target="$host"

hugspath=".:{Home}/lib/hugs/packages/*:{Hugs}/packages/*"

for flag; do
    case "$flag" in
        --prefix=*) prefix=${flag#*=};;
        --build=*) build=$(./config.sub ${flag#*=});;
        --host=*) host=$(./config.sub ${flag#*=});;
        --target=*) target=$(./config.sub ${flag#*=});;
        *)
            echo "$0: unknown flag: $flag"
            exit 1;;
    esac
done

build_c=$(./fp-platform $build | tr - _ )
host_c=$(./fp-platform $host | tr - _ )
target_c=$(./fp-platform $target | tr - _ )

build_cpu=$(echo $build | cut -d '-' -f 1 -)
host_cpu=$(echo $host | cut -d '-' -f 1 -)
target_cpu=$(echo $target | cut -d '-' -f 1 -)

build_os=$(echo $build | cut -d '-' -f 3 -)
host_os=$(echo $host | cut -d '-' -f 3 -)
target_os=$(echo $target | cut -d '-' -f 3 -)

exec 3> src/platform.h

echo '#pragma once' >&3

echo "#define BuildPlatform  ${build_c}" >&3
echo "#define HostPlatform   ${host_c}"  >&3
echo "#define TargetPlatform ${target_c}" >&3

echo "#define ${build_c}_BUILD 1" >&3
echo "#define ${host_c}_HOST 1" >&3
echo "#define ${target_c}_TARGET 1" >&3

echo "#define ${build_cpu}_BUILD_ARCH 1"  >&3
echo "#define ${host_cpu}_HOST_ARCH 1"  >&3
echo "#define ${target_cpu}_TARGET_ARCH 1"  >&3

echo "#define ${build_os}_BUILD_OS 1" >&3
echo "#define ${host_os}_HOST_OS 1" >&3
echo "#define ${target_os}_TARGET_OS 1" >&3

echo "#define BUILD_ARCH	\"${build_cpu}\" " >&3
echo "#define HOST_ARCH	\"${host_cpu}\" " >&3
echo "#define TARGET_ARCH	\"${target_cpu}\" " >&3

echo "#define BUILD_OS	\"${build_os}\"" >&3
echo "#define HOST_OS		\"${host_os}\"" >&3
echo "#define TARGET_OS	\"${target_os}\"" >&3

exec 3>&-

exec 3> src/options.h

echo "#pragma once" >&3
echo "#define HUGSPATH \"${hugspath}\"" >&3
echo "#define HUGSSUFFIXES \".hs:.lhs\"" >&3
echo "#define HUGSDIR \"${prefix}/lib/hugs\"" >&3

exec 3>&-