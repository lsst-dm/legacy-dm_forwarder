FROM centos:7 AS forwarder-build

RUN yum update \
    && yum install -y epel-release \
    && yum install -y git gcc-c++ make cmake3 wget rpm-build \
           boost169-devel \
           cfitsio-devel \
           curl-devel \
           hiredis-devel \
           librabbitmq-devel \
           yaml-cpp-devel
RUN ln -s /usr/lib64/libboost_log.so.1.69.0 /usr/lib64/libboost_log.so
    && ln -s /usr/lib64/libboost_log_setup.so.1.69.0 /usr/lib64/libboost_log_setup.so

RUN yum install -y gdb

RUN mkdir -p /tmp/build \
    && cd /tmp/build \
    && wget -O SimpleAmqpClient.tar.gz https://github.com/alanxz/SimpleAmqpClient/archive/v2.4.0.tar.gz \
    && tar xvzf SimpleAmqpClient.tar.gz \
    && cd SimpleAmqpClient-2.4.0 \
    && mkdir build \
    && cd build \
    && cmake3 .. \
    && cmake3 --build . --target install

ARG LSST_DAQ_VERSION=R2-V2.7
COPY ${LSST_DAQ_VERSION}.tgz /tmp/build/
RUN mkdir -p /opt/lsst/daq \
    && cd /opt/lsst/daq \
    && tar xvzf /tmp/build/${LSST_DAQ_VERSION}.tgz

RUN rm -rf /tmp/build

FROM centos:7

RUN yum update \
    && yum install -y epel-release \
    && yum install -y boost169-date-time boost169-filesystem boost169-log \
           cfitsio \
           curl \
           hiredis \
           librabbitmq \
           yaml-cpp

COPY --from=forwarder-build /usr/local/lib /usr/local/lib
COPY --from=forwarder-build /opt/lsst/daq /opt/lsst/daq
