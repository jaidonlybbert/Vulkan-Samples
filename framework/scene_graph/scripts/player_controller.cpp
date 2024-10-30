#include "common/utils.h"
#include "free_camera.h"
#include "player_controller.h"

#include "common/error.h"

#include "common/glm_common.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include "scene_graph/components/perspective_camera.h"
#include "scene_graph/components/transform.h"
#include "scene_graph/node.h"

namespace vkb
{
namespace sg
{
const float PlayerController::TOUCH_DOWN_MOVE_FORWARD_WAIT_TIME = 2.0f;

const float PlayerController::ROTATION_MOVE_WEIGHT = 0.1f;

const float PlayerController::KEY_ROTATION_MOVE_WEIGHT = 0.5f;

const float PlayerController::TRANSLATION_MOVE_WEIGHT = 3.0f;

const float PlayerController::TRANSLATION_MOVE_STEP = 50.0f;

const uint32_t PlayerController::TRANSLATION_MOVE_SPEED = 4;

PlayerController::PlayerController(Node &node) :
    NodeScript{node, "PlayerController"}
{}

void PlayerController::update(float delta_time)
{
	glm::vec3 delta_translation(0.0f, 0.0f, 0.0f);
	glm::vec3 delta_rotation(0.0f, 0.0f, 0.0f);

	float mul_translation = speed_multiplier;

	if (key_pressed[KeyCode::W])
	{
		delta_translation.z -= TRANSLATION_MOVE_STEP;
	}
	if (key_pressed[KeyCode::S])
	{
		delta_translation.z += TRANSLATION_MOVE_STEP;
	}
	if (key_pressed[KeyCode::A])
	{
		delta_translation.x -= TRANSLATION_MOVE_STEP;
	}
	if (key_pressed[KeyCode::D])
	{
		delta_translation.x += TRANSLATION_MOVE_STEP;
	}
	if (key_pressed[KeyCode::Q])
	{
		delta_translation.y -= TRANSLATION_MOVE_STEP;
	}
	if (key_pressed[KeyCode::E])
	{
		delta_translation.y += TRANSLATION_MOVE_STEP;
	}
	if (key_pressed[KeyCode::LeftControl])
	{
		mul_translation *= (1.0f * TRANSLATION_MOVE_SPEED);
	}
	if (key_pressed[KeyCode::LeftShift])
	{
		mul_translation *= (1.0f / TRANSLATION_MOVE_SPEED);
	}

	if (key_pressed[KeyCode::I])
	{
		delta_rotation.x += KEY_ROTATION_MOVE_WEIGHT;
	}
	if (key_pressed[KeyCode::K])
	{
		delta_rotation.x -= KEY_ROTATION_MOVE_WEIGHT;
	}
	if (key_pressed[KeyCode::J])
	{
		delta_rotation.y += KEY_ROTATION_MOVE_WEIGHT;
	}
	if (key_pressed[KeyCode::L])
	{
		delta_rotation.y -= KEY_ROTATION_MOVE_WEIGHT;
	}

	if (mouse_button_pressed[MouseButton::Left] && mouse_button_pressed[MouseButton::Right])
	{
		delta_rotation.z += TRANSLATION_MOVE_WEIGHT * mouse_move_delta.x;
	}
	else if (mouse_button_pressed[MouseButton::Right])
	{
		delta_rotation.x -= ROTATION_MOVE_WEIGHT * mouse_move_delta.y;
		delta_rotation.y -= ROTATION_MOVE_WEIGHT * mouse_move_delta.x;
	}
	else if (mouse_button_pressed[MouseButton::Left])
	{
		delta_translation.x += TRANSLATION_MOVE_WEIGHT * mouse_move_delta.x;
		delta_translation.y += TRANSLATION_MOVE_WEIGHT * -mouse_move_delta.y;
	}

	if (touch_pointer_pressed[0])
	{
		delta_rotation.x -= ROTATION_MOVE_WEIGHT * touch_move_delta.y;
		delta_rotation.y -= ROTATION_MOVE_WEIGHT * touch_move_delta.x;

		if (touch_pointer_time > TOUCH_DOWN_MOVE_FORWARD_WAIT_TIME)
		{
			delta_translation.z -= TRANSLATION_MOVE_STEP;
		}
		else
		{
			touch_pointer_time += delta_time;
		}
	}
	delta_rotation.x -= ROTATION_MOVE_WEIGHT * mouse_move_delta.y;
	delta_rotation.y -= ROTATION_MOVE_WEIGHT * mouse_move_delta.x;

	delta_translation *= mul_translation * delta_time;
	delta_rotation *= delta_time;

	// Only re-calculate the transform if it's changed
	if (delta_rotation != glm::vec3(0.0f, 0.0f, 0.0f) || delta_translation != glm::vec3(0.0f, 0.0f, 0.0f))
	{
		glm::quat qx = glm::angleAxis(delta_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat qy = glm::angleAxis(delta_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

		// Get orientation of camera and player root node
		auto camera_pivot = vkb::find_child_by_name(&get_node(), "camera_pivot");
		auto &camera_transform = camera_pivot->get_component<Transform>();
		glm::quat orientation_camera = glm::normalize(qy * camera_transform.get_rotation() * qx);
		auto &transform = get_node().get_component<Transform>();

		// Swing-twist decomposition to isolate camera 'yaw'/'twist' from pitch
		glm::quat camera_twist;
		glm::quat camera_swing;
		glm::vec3 twist_direction = glm::vec3(0., 1., 0);
		swing_twist_decomposition(orientation_camera, twist_direction, camera_swing, camera_twist);
		// player moves in 'forward' direction camera is facing, but not vertically
		auto delta_player_translation = delta_translation * glm::conjugate(camera_twist);
		transform.set_translation(transform.get_translation() + delta_player_translation);

		// Apply transform to camera pivot
		camera_transform.set_rotation(orientation_camera);
		auto camera = vkb::find_child_by_name(camera_pivot, "player_camera");
		// Invalidate child camera matrix to force update from parent pivot
		camera->get_transform().invalidate_world_matrix();

		// only change mesh transform if player moves
		if (delta_player_translation != glm::vec3(0.0f, 0.0f, 0.0f))
		{
			// Rotate player mesh (ball) based on rolling distance
			float radius = 1.;
			// arc length = angle (radians) * radius
			float roll_angle = glm::l2Norm(delta_player_translation) / radius; // distance
			auto roll_direction = glm::normalize(delta_player_translation);
			glm::vec3 rot_axis = -glm::normalize(glm::cross(roll_direction, twist_direction));
			glm::quat q_roll = glm::angleAxis(roll_angle, rot_axis);
			auto player_mesh = vkb::find_child_by_name(&get_node(), "player_mesh");
			auto &player_mesh_transform = player_mesh->get_component<Transform>();
			LOGD("roll_angle {:03.2f}", roll_angle);
			LOGD("x,y,z {:03.2f} {:03.2f} {:03.2f}", rot_axis.x, rot_axis.y, rot_axis.z)
			player_mesh_transform.set_rotation(q_roll * glm::normalize(player_mesh_transform.get_rotation()));
		}
	}

	mouse_move_delta = {};
	touch_move_delta = {};
}

void PlayerController::input_event(const InputEvent &input_event)
{
	if (input_event.get_source() == EventSource::Keyboard)
	{
		const auto &key_event = static_cast<const KeyInputEvent &>(input_event);

		if (key_event.get_action() == KeyAction::Down ||
		    key_event.get_action() == KeyAction::Repeat)
		{
			key_pressed[key_event.get_code()] = true;
		}
		else
		{
			key_pressed[key_event.get_code()] = false;
		}
	}
	else if (input_event.get_source() == EventSource::Mouse)
	{
		const auto &mouse_button = static_cast<const MouseButtonInputEvent &>(input_event);

		glm::vec2 mouse_pos{std::floor(mouse_button.get_pos_x()), std::floor(mouse_button.get_pos_y())};

		if (mouse_button.get_action() == MouseAction::Down)
		{
			mouse_button_pressed[mouse_button.get_button()] = true;
		}

		if (mouse_button.get_action() == MouseAction::Up)
		{
			mouse_button_pressed[mouse_button.get_button()] = false;
		}

		if (mouse_button.get_action() == MouseAction::Move)
		{
			mouse_move_delta = mouse_pos - mouse_last_pos;

			mouse_last_pos = mouse_pos;
		}
	}
	else if (input_event.get_source() == EventSource::Touchscreen)
	{
		const auto &touch_event = static_cast<const TouchInputEvent &>(input_event);

		glm::vec2 touch_pos{std::floor(touch_event.get_pos_x()), std::floor(touch_event.get_pos_y())};

		if (touch_event.get_action() == TouchAction::Down)
		{
			touch_pointer_pressed[touch_event.get_pointer_id()] = true;

			touch_last_pos = touch_pos;
		}

		if (touch_event.get_action() == TouchAction::Up)
		{
			touch_pointer_pressed[touch_event.get_pointer_id()] = false;

			touch_pointer_time = 0.0f;
		}

		if (touch_event.get_action() == TouchAction::Move && touch_event.get_pointer_id() == 0)
		{
			touch_move_delta = touch_pos - touch_last_pos;

			touch_last_pos = touch_pos;
		}
	}
}

void PlayerController::resize(uint32_t width, uint32_t height)
{
	// Resize attached camera
	for (sg::Node* child: get_node().get_children()) {
		if (child->has_component<sg::Camera>())
		{
			if (auto camera = dynamic_cast<PerspectiveCamera *>(&child->get_component<Camera>()))
			{
				camera->set_aspect_ratio(static_cast<float>(width) / height);
			}
		}
	}
}

}        // namespace sg
}        // namespace vkb
