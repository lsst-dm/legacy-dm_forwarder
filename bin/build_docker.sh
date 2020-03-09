GIT_TAG=`git describe --tags | sed -z -e s/-/_/g | tr -d '\n'`

docker build \
    --network host \
    --build-arg cmakeURL=https://github.com/Kitware/CMake/releases/download/v3.15.4/cmake-3.15.4-Linux-x86_64.sh \
    --build-arg bbcpURL=https://www.slac.stanford.edu/~abh/bbcp/bbcp.git/ \
    --build-arg rabbitmqcURL=http://lsst-web.ncsa.illinois.edu/~hwin16/packages/librabbitmq-c/v0.10.0.tar.gz \
    --build-arg SimpleAmqpClientURL=http://lsst-web.ncsa.illinois.edu/~hwin16/packages/SimpleAmqpClient/SimpleAmqpClient-2.4.0.tar.gz \
    --build-arg yamlcppURL=http://lsst-web.ncsa.illinois.edu/~hwin16/packages/yaml-cpp/yaml-cpp-0.6.3.tar.gz \
    --build-arg cfitsioURL=http://lsst-web.ncsa.illinois.edu/~hwin16/packages/cfitsio/cfitsio-3.450.tar.gz \
    --build-arg daqURL=http://lsst-web.ncsa.illinois.edu/~hwin16/packages/daq/R2-V2.7.tgz \
    -t lsst-dm/dm_forwarder:v${GIT_TAG} ../
