#!bin/bash

export BUILD_PATH=~/src/Vulkan-Samples/build/mac
export SAMPLES_BIN=~/src/Vulkan-Samples/build/mac/app/bin/Release/arm64/vulkan_samples
export VULKAN_ENV_INIT=~/VulkanSDK/1.3.283.0/setup-env.sh

install() {
	source ${VULKAN_ENV_INIT}	
}

buildtree() {
	mkdir -p ${BUILD_PATH}
	rm -rf ${BUILD_PATH}/*
	cmake -Bbuild/mac -DCMAKE_BUILD_TYPE=Release
	cmake --build build/mac --config Release --target vulkan_samples -j4
}

build() {
	cmake --build build/mac --config Release --target vulkan_samples -j4
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
