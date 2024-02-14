#include "Physics.h"
#include <cmath>

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    Vec2 halfSizeA = a->getComponent<CBoundingBox>().halfSize;
    Vec2 halfSizeB = b->getComponent<CBoundingBox>().halfSize;
    float alpha = a->getComponent<CBoundingBox>().angle;
    float beta = b->getComponent<CBoundingBox>().angle;
    
    Vec2 delta = a->getComponent<CTransform>().pos - b->getComponent<CTransform>().pos;
    float ox = halfSizeA.x*(std::cos(alpha * 3.14159 / 180)) + halfSizeB.x*(std::cos(beta * 3.14159 / 180)) - abs(delta.x);
    float oy = halfSizeA.y*(std::sin(alpha * 3.14159 / 180)) + halfSizeB.y*(std::sin(beta * 3.14159 / 180)) - abs(delta.y);

    return Vec2(ox, oy);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    Vec2 halfSizeA = a->getComponent<CBoundingBox>().halfSize;
    Vec2 halfSizeB = b->getComponent<CBoundingBox>().halfSize;

    Vec2 delta = a->getComponent<CTransform>().prevPos - b->getComponent<CTransform>().prevPos;
    float ox = halfSizeA.x + halfSizeB.x - abs(delta.x);
    float oy = halfSizeA.y + halfSizeB.y - abs(delta.y);

    return Vec2(ox, oy);
}
