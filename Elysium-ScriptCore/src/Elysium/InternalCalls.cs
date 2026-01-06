using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Elysium
{
    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_CreateEntity(string name);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_AddComponent(ulong entityID, Type componentType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_RemoveComponent(ulong entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TagComponent_GetTag(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TagComponent_SetTag(ulong entityID, ref string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectangleComponent_GetSize(ulong entityID, out Vector2 size);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectangleComponent_SetSize(ulong entityID, ref Vector2 size);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectangleComponent_GetColor(ulong entityID, out Vector4 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectangleComponent_SetColor(ulong entityID, ref Vector4 color);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 scale);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern int Texture2D_GetWidth(ulong handle);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern int Texture2D_GetHeight(ulong handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong SpriteRendererComponent_GetTexture(ulong entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetTexture(ulong entityID, ulong textureID);

        #region RigiBodyComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBodyComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBodyComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impuls);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBodyComponent_GetLinearVelocities(ulong entityID, out Vector2 linearVelocity);
        #endregion


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode key);
    }
}
