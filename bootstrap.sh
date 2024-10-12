#!bin/bash

export REPO_ROOT_DIR=~/src/Vulkan-Samples
export BUILD_PATH=${REPO_ROOT_DIR}/build/mac
export SAMPLES_BIN=${REPO_ROOT_DIR}/build/mac/app/bin/Release/arm64/vulkan_samples
export VULKAN_ENV_INIT=~/VulkanSDK/1.3.283.0/setup-env.sh
export PROJECT_SRC_DIR=${REPO_ROOT_DIR}/samples/general/sandbox/
export CMAKE_EXPORT_COMPILE_COMMANDS=1

install() {
	source ${VULKAN_ENV_INIT}	
}

buildtree() {
	mkdir -p ${BUILD_PATH}
	rm -rf ${BUILD_PATH}/*
	cmake -B${BUILD_PATH} -DCMAKE_BUILD_TYPE=Debug -G Ninja -DVKB_BUILD_TESTS=ON
}

start() {
	TMPDIR="$(pwd)"
	cd ${REPO_ROOT_DIR}
	${SAMPLES_BIN} sample $1
	cd ${TMPDIR}
}

build() {
	if [ "$2" = "rebuild" ]; then
		buildtree
	fi

	if [ "$1" = "samples" ]; then
		cmake --build ${BUILD_PATH} --config Debug --target vulkan_samples -j4
	elif [ "$1" = "tests" ]; then
		cmake --build ${BUILD_PATH} --config Debug --target vkb__tests -j4
	elif [ "$1" = "components" ]; then
		cmake --build ${BUILD_PATH} --config Debug --target vkb__components -j4
	fi

	compdb -p ${BUILD_PATH} list > ${REPO_ROOT_DIR}/build/compile_commands.json
}

in() {
	cd ${PROJECT_SRC_DIR}
}

out() {
	cd ${REPO_ROOT_DIR}
}
