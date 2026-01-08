using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Elysium
{
    public class Entity
    {
        protected Entity() { ID = 0; }

        public Entity(ulong id)
        {
            ID = id;
        }

        public Entity(string name)
        {
            ID = InternalCalls.Entity_CreateEntity(name);
        }

        //TODO: destroy entity

        private TransformComponent m_TransformComponent;
        //TODO: Add Parent, retriving children, component cache, etc.

        public readonly ulong ID;

        public virtual void OnCreate() { }
        public virtual void OnUpdate(float deltaTime) { }

        public virtual void OnCollisionEnter(Collision2D collision) { }
        public virtual void OnCollisionStay(Collision2D collision) { }
        public virtual void OnCollisionExit(Collision2D collision) { }

        public string Tag => GetComponent<TagComponent>().Tag;
        public TransformComponent Transform
        {
            get
            {
                if (m_TransformComponent == null)
                    m_TransformComponent = GetComponent<TransformComponent>();
                return m_TransformComponent;
            }
        }

        public Vector3 Translation
        {
            get => Transform.Translation;
            set => Transform.Translation = value;
        }

        public Vector3 Rotation
        {
            get => Transform.Rotation;
            set => Transform.Rotation = value;
        }

        public Vector3 Scale
        {
            get => Transform.Scale;
            set => Transform.Scale = value;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;
               
            T component = new T() { Entity = this};
            return component;
        }

        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return GetComponent<T>();
            
            Type componentType = typeof(T);
            InternalCalls.Entity_AddComponent(ID, componentType);
            T component = new T() { Entity = this };
            return component;
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return;
            Type componentType = typeof(T);
            InternalCalls.Entity_RemoveComponent(ID, componentType);
        }

        public Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
            if (entityID == 0)
                return null;
            return new Entity(entityID);
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.GetScriptInstance(ID);
            return instance as T;
        }
    }
}
