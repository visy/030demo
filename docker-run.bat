docker rm amiga-gcc-build-process > /dev/null && docker run --name=amiga-gcc-build-process -v .:/build --env BUILD_RESULT=demo.exe phobosys/amiga-gcc-builder:latest
