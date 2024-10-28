#include "physics/physics.hpp"
#include "entt/entt.hpp"
#include<stdio.h>

RigidBodyTransform::RigidBodyTransform() {
	gravity_constant = 9;
}

RigidBodyTransform::~RigidBodyTransform() {
	printf("RigidBodyTransform deleted");
}
