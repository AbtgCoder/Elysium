#include "Entity.h"

Entity::Entity(ECS::Entity handle, Scene* scene)
	: m_EntityHandle(handle), m_Scene(scene)
{
}
