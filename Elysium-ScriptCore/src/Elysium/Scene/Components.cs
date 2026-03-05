using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Elysium
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TagComponent : Component
    {
        public string Tag
        {
            get
            {
                return InternalCalls.TagComponent_GetTag(Entity.ID);
            }
            set
            {
                InternalCalls.TagComponent_SetTag(Entity.ID, ref value);
            }
        }
    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
                return scale;
            }
            set
            {
                InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
            }
        }

        public Vector3 Position
        {
            get => Translation;
            set => Translation = value;
        }
    }

    public class RectangleComponent : Component
    {
        public Vector2 Size
        {
            get
            {
                InternalCalls.RectangleComponent_GetSize(Entity.ID, out Vector2 size);
                return size;
            }
            set
            {
                InternalCalls.RectangleComponent_SetSize(Entity.ID, ref value);
            }
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.RectangleComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }
            set
            {
                InternalCalls.RectangleComponent_SetColor(Entity.ID, ref value);
            }
        }
    }

    public class CircleComponent : Component
    {
        public float Radius
        {
            get
            {
                InternalCalls.CircleComponent_GetRadius(Entity.ID, out float radius);
                return radius;
            }
            set
            {
                InternalCalls.CircleComponent_SetRadius(Entity.ID, ref value);
            }
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.CircleComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }
            set
            {
                InternalCalls.CircleComponent_SetColor(Entity.ID, ref value);
            }
        }
    }

    public class SpriteRendererComponent : Component
    {
        public Texture2D Texture
        {
            get
            {
                ulong handle = InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID);
                return handle == 0 ? null : new Texture2D(handle);
            }
            set
            {
                ulong handle = value?.Handle ?? 0;
                InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, handle);
            }
        }
    }

    public class RigidBodyComponent : Component
    {
        public enum BodyType { Static = 0, Dynamic, Kinematic }

        public BodyType Type
        {
            get => InternalCalls.RigidBodyComponent_GetType(Entity.ID);
            set => InternalCalls.RigidBodyComponent_SetType(Entity.ID, value);
        }

        public Vector2 LinearVelocity
        {
            get
            {
                InternalCalls.RigidBodyComponent_GetLinearVelocities(Entity.ID, out Vector2 velocity);
                return velocity;
            }
        }

        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition)
        {
            InternalCalls.RigidBodyComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition);
        }

        public void ApplyLinearImpulse(Vector2 impulse)
        {
            InternalCalls.RigidBodyComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse);
        }
    }

    public class BoundingBoxComponent : Component
    {
        public Vector2 Size
        {
            get
            {
                InternalCalls.BoundingBox_GetSize(Entity.ID, out Vector2 size);
                return size;
            }
            set
            {
                InternalCalls.BoundingBox_SetSize(Entity.ID, ref value);
            }
        }
        public Vector2 Offset
        {
            get
            {
                InternalCalls.BoundingBox_GetOffset(Entity.ID, out Vector2 offset);
                return offset;
            }
            set
            {
                InternalCalls.BoundingBox_SetOffset(Entity.ID, ref value);
            }
        }
    }

    public class CircleColliderComponent : Component
    {
        public float Radius
        {
            get
            {
                InternalCalls.CircleCollider_GetRadius(Entity.ID, out float radius);
                return radius;
            }
            set
            {
                InternalCalls.CircleCollider_SetRadius(Entity.ID, ref value);
            }
        }
    }

    public class JointComponent : Component
    {
        public Entity ConnectedEntity
        {
            set => InternalCalls.JointComponent_SetConnectedEntity(Entity.ID, value.ID);
        }
        
        public Vector2 Anchor
        {
            set => InternalCalls.JointComponent_SetAnchor(Entity.ID, ref value);
        }

        public float Softness
        {
            set => InternalCalls.JointComponent_SetSoftness(Entity.ID, ref value);
        }

        public float Bias
        {
            set => InternalCalls.JointComponent_SetBias(Entity.ID, ref value);
        }
    }

    public class AnimatorComponent : Component
    {
        public void Play(string state)
        {
            InternalCalls.Animator_Play(Entity.ID, state);
        }

        public void SetBool(string name, bool value)
        {
            InternalCalls.Animator_SetBool(Entity.ID, name, value);
        }

        public void SetFloat(string name, float value)
        {
            InternalCalls.Animator_SetFloat(Entity.ID, name , value);
        }

        public void SetTrigger(string name)
        {
            InternalCalls.Animator_SetTrigger(Entity.ID, name);
        }
    }
}
