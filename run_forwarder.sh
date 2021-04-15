#!/bin/bash

export IIP_UID=$(id -u iip)
export IIP_GID=$(id -g iip)
export KRB5CCNAME_FILE=/tmp/krb5cc_$(id -u iip)

error_msg="$0: missing argument: -d [latiss|comcam] -s [summit|nts]"

export IIP_CONFIG_DIR=
export IIP_DEVICE=
export IIP_SITE=
export IIP_CONTAINER=

while getopts d:s: option
do
case "${option}"
in
d) 
    if [ "$OPTARG" = "latiss" ] || [ "$OPTARG" = "comcam" ]; then
        IIP_DEVICE=${OPTARG}
    else
        echo "-d argument must be latiss or comcam"
        exit 1
    fi;;
s) 
    if [ "$OPTARG" = "summit" ] || [ "$OPTARG" = "nts" ]; then
        IIP_SITE=${OPTARG}
    else
        echo "-s argument must be summit or nts"
        exit 1
    fi;;
esac
done

if [[ -z $IIP_DEVICE ]] || [[ -z $IIP_SITE ]]; then
    echo $error_msg
    exit 1
fi

if [ "$IIP_SITE" = "summit" ]; then
    IIP_CONTAINER=fwd_summit
else 
    # IIP_CONTAINER=fwd_${IIP_SITE}_${IIP_DEVICE}
    IIP_CONTAINER=fwd_nts
fi;

IIP_CONFIG_DIR=/opt/lsst/dm_forwarder/config/$IIP_SITE/$IIP_DEVICE

echo "IIP_CONFIG_DIR="$IIP_CONFIG_DIR
echo "IIP_DEVICE = "$IIP_DEVICE
echo "IIP_SITE = "$IIP_SITE
echo "KRB5CCNAME_FILE= "$KRB5CCNAME_FILE
echo "IIP_CONTAINER= "$IIP_CONTAINER

docker-compose -f docker-compose.yml up -d $IIP_CONTAINER
