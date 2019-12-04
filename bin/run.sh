docker run \
    --rm \
    -u `id -u`:`id -g` \
    --name dm_forwarder \
    --network host \
    --mount type=volume,src=dm_forwarder,target=/var/tmp/data \
    --mount type=volume,src=dm_forwarder,target=/var/log/iip \
    --mount type=bind,src=/etc/passwd,target=/etc/passwd,readonly \
    --mount type=bind,src=$HOME/.ssh,target=/home/iip/.ssh,readonly \
    --mount type=bind,src=$HOME/.lsst,target=/home/iip/.lsst,readonly \
    --mount type=bind,src=/opt/lsst/dm_forwarder/config,target=/opt/lsst/dm_forwarder/config,readonly \
    --mount type=bind,src=/home/lsst-daq/images,target=/home/lsst-daq/images \
    lsst-dm/dm_forwarder:v1.0.0-rc7
