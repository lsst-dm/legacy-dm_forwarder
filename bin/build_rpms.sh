ARG=$1

cd ../
mkdir -p rpms && 

if [ "${ARG}" = "rmq" ]; then
    mkdir -p build/${ARG}
    cd build/${ARG}
    cmake -DRMQC=ON ../..
elif [ "${ARG}" = "lib" ]; then
    mkdir -p build/${ARG}
    cd build/${ARG}
    cmake -DRPMS=ON ../..
elif [ "${ARG}" = "fwd" ]; then
    mkdir -p build/${ARG}
    cd build/${ARG}
    cmake ../..
else
    echo "usage: $ ./build_lib_rpm.sh [rmq|lib|fwd] [-d]"
    exit
fi

cmake --build . &&
cpack -G RPM &&
mv *.rpm ../../rpms
