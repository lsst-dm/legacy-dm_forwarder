FROM centos:7 as builder

ARG cmakeURL
ARG rabbitmqcURL
ARG SimpleAmqpClientURL
ARG yamlcppURL
ARG cfitsioURL
ARG daqURL
ARG bbcpURL

RUN yum install -y epel-release
RUN yum install -y \
        git-1.8.3.1 \
        make-3.82 \
        gcc-c++-4.8.5 \
        boost169-devel-1.69.0 \
        libcurl-devel-7.29.0 \
        hiredis-devel-0.12.1 \
        zlib-devel-1.2.7 \
        openssl-devel-1.0.2k

# cmake
RUN curl -L -o cmake.sh ${cmakeURL} && \
    chmod +x cmake.sh && \
    /bin/sh cmake.sh --prefix=/usr --skip-license

# bbcp
RUN git clone ${bbcpURL} bbcp && \
    cd bbcp/src && \
    make && \
    cp /bbcp/bin/amd64_linux31/bbcp /usr/bin

# rabbitmq-c
RUN mkdir rabbitmqc && cd rabbitmqc && \
    curl -L -o rabbitmqc.tar.gz ${rabbitmqcURL}  && \
    tar zxvf rabbitmqc.tar.gz && \
    cd * && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && \
    cmake --build . --target install

# SimpleAmqpClient
RUN mkdir SimpleAmqpClient && cd SimpleAmqpClient && \
    curl -L -o SimpleAmqpClient.tar.gz ${SimpleAmqpClientURL}  && \
    tar zxvf SimpleAmqpClient.tar.gz && \
    cd * && \
    mkdir build && cd build && \
    cmake .. \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DBOOST_INCLUDEDIR=/usr/include/boost169 \
        -DBOOST_LIBRARYDIR=/usr/lib64/boost169 .. && \
    cmake --build . --target install

# yaml-cpp
RUN mkdir yamlcpp && cd yamlcpp && \
    curl -L -o yamlcpp.tar.gz ${yamlcppURL}  && \
    tar zxvf yamlcpp.tar.gz && \
    cd * && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DYAML_BUILD_SHARED_LIBS=ON .. && \
    cmake --build . --target install

# cfitsio
RUN mkdir cfitsio && cd cfitsio && \
    curl -L -o cfitsio.tar.gz ${cfitsioURL}  && \
    tar zxvf cfitsio.tar.gz && \
    cd * && \
    mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && \
    cmake --build . --target install

# daq
RUN mkdir daq && cd daq && \
    curl -L -o daq.tar.gz ${daqURL} && \
    tar zxvf daq.tar.gz && \
    cd * && \
    mkdir -p /opt/lsst/daq && \
    cp -r ./x86 /opt/lsst/daq/x86 && \
    cp -r ./include /opt/lsst/daq/include

# build dm_forwarder
WORKDIR /app
COPY . /app
RUN rm -rf /app/build 2> /dev/null && \
    mkdir /app/build && cd build && \
    cmake .. && \
    cmake --build .

# dm_forwarder
FROM centos:7
WORKDIR /app

ENV IIP_CONFIG_DIR=/opt/lsst/dm_forwarder/config
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/lsst/daq/x86/lib:/usr/lib:/usr/lib64

COPY --from=builder /usr/lib64/librabbitmq.so /usr/lib/
COPY --from=builder /usr/lib/libyaml-cpp.so /usr/lib/
COPY --from=builder /usr/lib/libSimpleAmqpClient.so /usr/lib/
COPY --from=builder /usr/lib/libcfitsio.so /usr/lib/
COPY --from=builder /opt/lsst/daq/x86 /opt/lsst/daq/x86
COPY --from=builder /app/build/src/forwarder/dm_forwarder /app
COPY --from=builder /usr/bin/bbcp /usr/bin/bbcp

RUN yum install -y epel-release
RUN yum install -y \
        boost169-1.69.0 \
        libcurl-7.29.0 \
        hiredis-0.12.1 \
        openssh-clients-7.4p1 \
        openssl-1.0.2k \
        zlib-1.2.7

RUN mkdir /var/tmp/data && \
    mkdir /var/log/iip && \
    chmod 777 /var/tmp/data && \
    chmod 777 /var/log/iip

CMD ["/app/dm_forwarder"]
