#include "Physics.h"
#include <cmath>

Vec2 Physics::GetOverlap(Entity a, Entity b)
{
    Vec2 halfSizeA = a.getComponent<CBoundingBox>().halfSize;
    Vec2 halfSizeB = b.getComponent<CBoundingBox>().halfSize;
    Vec2 offsetA = a.getComponent<CBoundingBox>().offset;
    Vec2 offsetB = b.getComponent<CBoundingBox>().offset;

    Vec2 delta = a.getComponent<CTransform>().pos - b.getComponent<CTransform>().pos + (offsetA - offsetB);
    float ox = halfSizeA.x + halfSizeB.x - abs(delta.x);
    float oy = halfSizeA.y + halfSizeB.y - abs(delta.y);

    return Vec2(ox, oy);
}

Vec2 Physics::GetPreviousOverlap(Entity a, Entity b)
{
    Vec2 halfSizeA = a.getComponent<CBoundingBox>().halfSize;
    Vec2 halfSizeB = b.getComponent<CBoundingBox>().halfSize;
    Vec2 offsetA = a.getComponent<CBoundingBox>().offset;
    Vec2 offsetB = b.getComponent<CBoundingBox>().offset;

    Vec2 delta = a.getComponent<CTransform>().prevPos - b.getComponent<CTransform>().prevPos + (offsetA - offsetB);
    float ox = halfSizeA.x + halfSizeB.x - abs(delta.x);
    float oy = halfSizeA.y + halfSizeB.y - abs(delta.y);

    return Vec2(ox, oy);
}
