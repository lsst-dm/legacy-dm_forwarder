#!/bin/bash
export FORWARDER_USER=iip

if [ -z $1 ];
then
    echo "$0: missing argument: container_version"
    exit 1
fi

container_version=$1

docker run \
    -d \
    -u `id -u iip`:`id -g iip` \
    --name dm_forwarder \
    --network host \
    --pid host \
    -v /home/iip/.ssh:/home/iip/.ssh \
    -v /home/iip/.lsst:/home/iip/.lsst \
    -v /tmp:/tmp \
    -v /var/log/iip:/var/log/iip \
    -v /etc/passwd:/etc/passwd \
    -v /etc/group:/etc/group \
    -v /opt/lsst/dm_forwarder/config:/opt/lsst/dm_forwarder/config \
    -v /home/lsst-daq/images:/home/lsst-daq/images \
    ts-dockerhub.lsst.org/lsstts/dm_forwarder:$container_version
