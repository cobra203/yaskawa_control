export NDK_TOOLCHAIN=/home/cobra/projects/android/toolchains/android-22/bin
export SDKTARGETSYSROOT=/home/cobra/projects/android/toolchains/android-22/sysroot
export PATH=${NDK_TOOLCHAIN}:${PATH}
export CC="arm-linux-androideabi-gcc --sysroot=$SDKTARGETSYSROOT"
#export CXX="arm-linux-androideabi-g++ $CFLAGS"
export CPP="arm-linux-androideabi-gcc"
export AS="arm-linux-androideabi-as"
export LD="arm-linux-androideabi-ld --sysroot=$SDKTARGETSYSROOT"
export AR="arm-linux-androideabi-ar"
export nm="arm-linux-androideabi-nm"
export CFLAGS="-O2 -pipe -g  -pie -fPIE -D__ANDROID_API__=22"
export LDFLAGS="-L$SDKTARGETSYSROOT/usr/lib -Wl,-O1 -Wl,--hash-style=sysv -Wl,--as-needed"
