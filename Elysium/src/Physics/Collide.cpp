#include "PhysicsBodyPairArbiter.h"
#include "PhysicsCircleCollision.h"


int Collide(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2)
{

	if (body1->GetShapeType() == PhysicsShape::Type::e_polygon && body2->GetShapeType() == PhysicsShape::Type::e_polygon)
	{
		// polygon, polygon collision
	}
	else if (body1->GetShapeType() == PhysicsShape::Type::e_circle && body2->GetShapeType() == PhysicsShape::Type::e_circle)
	{
		// collidecircles (contacts, circleshape*, transform1, circleshape*, transform2)
		return PhysicsCircleCircleCollision(contacts, body1, body2);
	}

	return 0;
}


