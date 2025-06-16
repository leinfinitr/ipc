WORK_PATH			= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_PATH			= ${WORK_PATH}/build

all: clean build

build:
	cmake -B ${BUILD_PATH} && \
	cmake --build ${BUILD_PATH} -- -j$(nproc)

clean:
	rm -rf ${BUILD_PATH}

run_sender:
	${BUILD_PATH}/sender

run_receiver:
	${BUILD_PATH}/receiver