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
    -u `id -u $FORWARDER_USER`:`id -g $FORWARDER_USER` \
    --name dm_forwarder \
    --network host \
    -v /home/$FORWARDER_USER/.ssh:/home/$FORWARDER_USER/.ssh \
    -v /home/$FORWARDER_USER/.lsst:/home/$FORWARDER_USER/.lsst \
    -v /var/tmp/data:/var/tmp/data \
    -v /var/log/iip:/var/log/iip \
    -v /etc/passwd:/etc/passwd \
    -v /etc/group:/etc/group \
    -v /opt/lsst/dm_forwarder/config:/opt/lsst/dm_forwarder/config \
    -v /home/lsst-daq/images:/home/lsst-daq/images \
    lsstts/dm_forwarder:$container_version
