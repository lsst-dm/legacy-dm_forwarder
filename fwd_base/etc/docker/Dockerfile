FROM centos:7 as builder
WORKDIR /app
RUN curl -L -O https://github.com/Kitware/CMake/releases/download/v3.15.4/cmake-3.15.4-Linux-x86_64.sh && \
    chmod +x cmake-3.15.4-Linux-x86_64.sh && \
    /bin/sh cmake-3.15.4-Linux-x86_64.sh --skip-license

FROM centos:7
WORKDIR /app
RUN yum install -y epel-release && \
    yum update -y && \
    yum install -y \
        gcc-c++ \
        make \
        rpm-build \
        librabbitmq-devel \
        boost169-devel \
        libcurl-devel \
        hiredis-devel
COPY --from=builder /app/bin/* /usr/local/bin/
COPY --from=builder /app/share/cmake-3.15 /usr/local/share/cmake-3.15/
CMD ["cpack", "--version"]
