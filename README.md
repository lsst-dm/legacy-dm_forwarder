# dm_Forwarder
Forwarder application for assembling and transferring fitsfiles

## building dm_forwarder
    . ./versions.sh
    docker-compose build dm_fowarder

## Starting dm_forwarder
run:
    . ./versions.sh
    ./run_forwarder.sh -d [latiss|comcam] -s [summit|nts]
