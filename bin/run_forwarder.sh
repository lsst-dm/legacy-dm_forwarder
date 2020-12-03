#!/bin/bash
export FORWARDER_USER=iip
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
    -v /opt/lsst/dm_forwarder/config:/opt/lsst/dm_forwarder/config \
    -v /home/lsst-daq/images:/home/lsst-daq/images \
    lsstts/dm_forwarder:1.0.0
