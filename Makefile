# BUILD_TYPE		= Debug / Release
BUILD_TYPE			= Release

# VERBOSE			= ON / OFF : enable verbose makefile
VERBOSE				= OFF

BUILD_TEST			= ON

WORK_PATH			= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_PATH			= ${WORK_PATH}/build
INSTALL_PATH		= ${WORK_PATH}/output

.PHONY: build
build: ${BUILD_PATH}/CMakeCache.txt
	rm -rf ${INSTALL_PATH}; \
	cmake --build ${BUILD_PATH} --target install -- -j$(shell nproc)

${BUILD_PATH}/CMakeCache.txt:
	${MAKE} configure

.PHONY: configure
configure:
	cmake -B${BUILD_PATH}	\
		  -DCMAKE_BUILD_TYPE=${BUILD_TYPE}			\
		  -DCMAKE_VERBOSE_MAKEFILE=${VERBOSE}		\
		  -DCMAKE_INSTALL_PREFIX=$(abspath $(INSTALL_PATH))	\
		  -DBUILD_TEST=${BUILD_TEST}

.PHONY: clean
clean:
	@rm -rf ${BUILD_PATH} ${INSTALL_PATH}
