using Elysium;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PhysicsTest : Entity
{
	private RigidBodyComponent m_Rigidbody;

	public float Speed = 1.0f;

	public Vector3 rectPos;

	public override void OnCreate()
	{
		m_Rigidbody = GetComponent<RigidBodyComponent>();

		Entity e = new Entity("script_rectangle");
        e.AddComponent<TransformComponent>().Position = rectPos;
		e.AddComponent<RectangleComponent>();
		RectangleComponent rect = e.GetComponent<RectangleComponent>();
		rect.Size = new Vector2(1.5f, 1.5f);
		rect.Color = new Vector4(1, 0, 0, 1);
		e.AddComponent<BoundingBoxComponent>().Size = rect.Size;
		e.AddComponent<RigidBodyComponent>();

		Entity e2 = new Entity("script_circle");
		e2.GetComponent<TransformComponent>().Position = new Vector3(3, 2, 0);
		e2.AddComponent<CircleComponent>().Radius = 0.5f;
		e2.AddComponent<CircleColliderComponent>().Radius = e2.GetComponent<CircleComponent>().Radius;
		e2.AddComponent<RigidBodyComponent>().Type = RigidBodyComponent.BodyType.Dynamic;

		JointComponent joint = e.AddComponent<JointComponent>();
		joint.ConnectedEntity = e2;
		joint.Anchor = new Vector2(0, 0);
		joint.Softness = 0.001f;
		joint.Bias = 0.15f;

    }

	public override void OnUpdate(float deltaTime)
	{
		Vector2 vel = Vector2.Zero;

		if (Input.IsKeyDown(KeyCode.W))
			vel.Y = 1.0f;
		else if (Input.IsKeyDown(KeyCode.S))
			vel.Y = -1.0f;

		if (Input.IsKeyDown(KeyCode.A))
			vel.X = -1.0f;
		else if (Input.IsKeyDown(KeyCode.D))
			vel.X = 1.0f;

		vel *= Speed; // * deltaTime;

		m_Rigidbody.ApplyLinearImpulse(vel);
	}

    public override void OnCollisionEnter(Collision2D collision)
    {
		Debug.Log($"{Tag} just started collision with {collision.OtherEntity.Tag} !!");

		if (collision.OtherEntity.Tag == "script_circle")
		{
			collision.OtherEntity.RemoveComponent<RigidBodyComponent>();
		}
    }

    public override void OnCollisionStay(Collision2D collision)
    {
        Debug.Log($"{Tag} is still in contact with {collision.OtherEntity.Tag} !!");
    }
    public override void OnCollisionExit(Collision2D collision)
    {
        Debug.Log($"{Tag} just finished colliding with {collision.OtherEntity.Tag} !!");
    }
}
