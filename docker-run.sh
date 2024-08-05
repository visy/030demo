#!/bin/bash
SERVICE_NAME=amiga-gcc-build-process
IMAGE=phobosys/amiga-gcc-builder
PROCESS_RUNNING=`docker ps -a --format '{{.Names}}' | grep $SERVICE_NAME`
OUTPUT=rotation-demo.exe

if [ ! -f $OUTPUT ]
then
    echo creating binary file to avoid wrong permissions
    touch $OUTPUT
    chmod +x $OUTPUT
fi
  
if [ -n "$PROCESS_RUNNING" ]
then
    echo terminating old build process
    docker rm $SERVICE_NAME > /dev/null
fi

echo "starting $IMAGE"
echo
docker run --name=$SERVICE_NAME -v $PWD:/build --env BUILD_RESULT=$OUTPUT $IMAGE:latest
