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
}
