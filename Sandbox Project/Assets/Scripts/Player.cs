using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Elysium;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;

        public float Speed;

        public Entity tile;

        public Texture2D pipeTexture;

        private Entity newRectangle;

        private RectangleComponent rectangleComponent;

        private SpriteRendererComponent sr;

        void OnCreate()
        {
            Console.WriteLine($"Player.OnCreate - {ID}");
            Console.WriteLine($"Tile ID: {tile.ID}");

            m_Transform = GetComponent<TransformComponent>();

            newRectangle = new Entity("New Rectangle");
            Vector3 newEntityTranslation = new Vector3(5.0f, 3.0f, 0.0f);
            newRectangle.Translation = newEntityTranslation;

            sr = newRectangle.AddComponent<SpriteRendererComponent>();
            //ulong textureHandle = 12063736264547966939;
            sr.Texture = pipeTexture; // new Texture2D(textureHandle);

            //rectangleComponent = newRectangle.AddComponent<RectangleComponent>();
            //rectangleComponent.Color = new Vector4(0.0f, 1.0f, 0.6f, 1.0f);
        }

        void OnUpdate(float deltaTime)
        {
           //rectangleComponent.Size += new Vector2(deltaTime, deltaTime);

            float speed = Speed;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
            {
                velocity.Y = 1.0f;
            }
            else if (Input.IsKeyDown(KeyCode.S))
            {
                velocity.Y = -1.0f;
            }

            if (Input.IsKeyDown(KeyCode.A))
            {
                velocity.X = -1.0f;
            }
            else if (Input.IsKeyDown(KeyCode.D))
            {
                velocity.X = 1.0f;
            }

            Entity cameraEntity = FindEntityByName("Main Camera");
            if (cameraEntity != null)
            {
                Camera camera = cameraEntity.As<Camera>();

                if (Input.IsKeyDown(KeyCode.Q))
                    camera.DistanceFromPlayer += speed * 2.0f * deltaTime;
                else if (Input.IsKeyDown(KeyCode.E))
                    camera.DistanceFromPlayer -= speed * 2.0f * deltaTime;
            }

            Vector3 tileTranslation = tile.Translation;
            tileTranslation.X -= deltaTime;
            tile.Translation = tileTranslation;

            velocity *= speed;

            Vector3 translation = m_Transform.Translation;
            translation += velocity * deltaTime;
            m_Transform.Translation = translation;
        }
    }
}
