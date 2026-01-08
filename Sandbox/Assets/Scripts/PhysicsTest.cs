using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Elysium;

public class PhysicsTest : Entity
{
	private RigidBodyComponent m_Rigidbody;

	public float Speed = 1.0f;

	public override void OnCreate()
	{
		m_Rigidbody = GetComponent<RigidBodyComponent>();
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
