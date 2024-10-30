/* Copyright (c) 2019-2024, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License"; you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sandbox.h"
#include "common/vk_common.h"
#include "gltf_loader.h"
#include "gui.h"
#include "filesystem/legacy.h"
#include "platform/platform.h"
#include "rendering/subpasses/forward_subpass.h"
#include "stats/stats.h"
#include "core/util/logging.hpp"

sandbox::sandbox()
{
	LOGI("Something");
	LOGI("Something else");
}

bool sandbox::prepare(const vkb::ApplicationOptions &options)
{
	if (!VulkanSample::prepare(options))
	{
		return false;
	}

	// Load a scene from the assets folder
	// load_scene("scenes/sponza/Sponza01.gltf");
	load_scene("scenes/Sandbox/sandbox.gltf");

	// Attach a move script to the camera component in the scene
	// auto &camera_node = vkb::add_free_camera(get_scene(), "main_camera", get_render_context().get_surface_extent());
	// auto camera       = &camera_node.get_component<vkb::sg::Camera>();
	vkb::sg::Camera* camera = nullptr;

	// Attach a move script to a player character with attached camera
	auto &player_node = vkb::add_player_controller(get_scene(), "player", get_render_context().get_surface_extent());
	auto camera_pivot = vkb::find_child_by_name(&player_node, "camera_pivot");
	auto player_mesh = vkb::find_child_by_name(&player_node, "player_mesh");
	camera = &vkb::find_child_by_name(camera_pivot, "player_camera")->get_component<vkb::sg::Camera>();

	if (!camera) {
		LOGE("No camera found for `%`", player_node.get_name());
		throw std::runtime_error("No camera found for PlayerController: " + player_node.get_name());
	}

	// Example Scene Render Pipeline
	vkb::ShaderSource vert_shader("base.vert");
	vkb::ShaderSource frag_shader("base.frag");
	auto              scene_subpass   = std::make_unique<vkb::ForwardSubpass>(get_render_context(), std::move(vert_shader), std::move(frag_shader), get_scene(), *camera);
	auto              render_pipeline = std::make_unique<vkb::RenderPipeline>();
	render_pipeline->add_subpass(std::move(scene_subpass));
	set_render_pipeline(std::move(render_pipeline));

	// Add a GUI with the stats you want to monitor
	get_stats().request_stats({/*stats you require*/});
	create_gui(*window, &get_stats());

	return true;
}

std::unique_ptr<vkb::VulkanSampleC> create_sandbox()
{
	return std::make_unique<sandbox>();
}
