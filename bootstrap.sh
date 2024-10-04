#!bin/bash

export REPO_ROOT_DIR=~/src/Vulkan-Samples
export BUILD_PATH=${REPO_ROOT_DIR}/build/mac
export SAMPLES_BIN=${REPO_ROOT_DIR}/build/mac/app/bin/Release/arm64/vulkan_samples
export VULKAN_ENV_INIT=~/VulkanSDK/1.3.283.0/setup-env.sh
export PROJECT_SRC_DIR=${REPO_ROOT_DIR}/samples/general/sandbox/
export CMAKE_EXPORT_COMPILE_COMMANDS=1
export VKB_BUILD_SAMPLES=OFF
export VKB_SANDBOX=ON

install() {
	source ${VULKAN_ENV_INIT}	
}

buildtree() {
	mkdir -p ${BUILD_PATH}
	rm -rf ${BUILD_PATH}/*
	cmake -B${BUILD_PATH} -DCMAKE_BUILD_TYPE=Release -G Ninja
	cmake --build ${BUILD_PATH} --config Release --target vulkan_samples -j4
	compdb -p ${BUILD_PATH} list > ${REPO_ROOT_DIR}/build/compile_commands.json
}

build() {
	cmake --build ${BUILD_PATH} --config Release --target vulkan_samples -j4
}

samples() {
	${SAMPLES_BIN} $1
}

start() {
	${SAMPLES_BIN} sample $1
}

test() {
	if [ "$1" = "rebuild" ]; then
		echo "j"
	else
		echo "n"
	fi
}

in() {
	cd ${PROJECT_SRC_DIR}
}

out() {
	cd ${REPO_ROOT_DIR}
}
