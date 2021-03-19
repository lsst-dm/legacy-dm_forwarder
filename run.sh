#!/bin/bash

export IIP_UID=$(id -u iip)
export IIP_GID=$(id -g iip)
export KRB5CCNAME_FILE=/tmp/krb5cc_$(id -u iip)

error_msg="$0: missing argument: -d [latiss|comcam] -s [summit|ncsa] -c container_version"

export IIP_CONFIG_DIR=
export IIP_DEVICE=
export IIP_SITE=
export IIP_CONTAINER_VERSION=
while getopts d:s:c: option
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
    if [ "$OPTARG" = "summit" ] || [ "$OPTARG" = "ncsa" ]; then
        IIP_SITE=${OPTARG}
    else
        echo "-s argument must be summit or ncsa"
        exit 1
    fi;;
c) IIP_CONTAINER_VERSION=${OPTARG};;
esac
done

if [[ -z $IIP_DEVICE ]] || [[ -z $IIP_SITE ]] || [[ -z $IIP_CONTAINER_VERSION ]]; then
    echo $error_msg
    exit 1
fi

IIP_CONFIG_DIR=/opt/lsst/dm_forwarder/config/$IIP_SITE/$IIP_DEVICE

echo "IIP_CONFIG_DIR="$IIP_CONFIG_DIR
echo "IIP_DEVICE = "$IIP_DEVICE
echo "IIP_SITE = "$IIP_SITE
echo "IIP_CONTAINER_VERSION = "$IIP_CONTAINER_VERSION
echo "KRB5CCNAME_FILE= "$KRB5CCNAME_FILE

docker-compose -f docker-compose.yml up -d dm_forwarder
