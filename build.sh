#!/bin/bash

error_msg="$0: missing argument: -c container_version"

export IIP_CONTAINER_VERSION=
while getopts c: option
do
case "${option}"
in
c) CONTAINER_VERSION=${OPTARG};;
esac
done

if [ -z $CONTAINER_VERSION ]; then
    echo $error_msg
    exit 1
fi

IIP_CONTAINER_VERSION=${CONTAINER_VERSION}

echo "IIP_CONTAINER_VERSION = "$IIP_CONTAINER_VERSION

docker-compose build dm_forwarder
