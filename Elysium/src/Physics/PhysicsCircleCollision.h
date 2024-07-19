#pragma once

#include "PhysicsBodyPairArbiter.h"
#include "PhysicsCircleShape.h"

int PhysicsCircleCircleCollision(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2);
int PhysicsCirclePolygonCollision(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2);