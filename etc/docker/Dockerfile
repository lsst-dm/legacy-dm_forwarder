FROM centos:latest
WORKDIR /app
COPY . /app
RUN yum update -y && \
    yum install -y \
        epel-release \
        gcc-c++ \
        make \
        cmake \
        cmake3 \
        boost169 \
        boost169-devel
