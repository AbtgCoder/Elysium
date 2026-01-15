using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Elysium;

public class DemoScene : Entity
{
	private Entity m_PlayerBall;
	private float m_Time;


	public int m_PlankCount = 8;
	public float m_PlankWidth = 1.0f;
	public float m_PlankHeight = 0.25f;
	public float m_StartX = -4.0f;
	public float m_BridgeY = 1.0f;
	public float m_Gap = 0.2f;

	public override void OnCreate()
	{
		Debug.Log("Initializing Demo Scene...");

		//CreateGround();
		//CreatePlatforms();
		CreatePendulum();
		//CreateBridge();
		CreateSuspensionBridge(plankCount: 10, plankWidth: m_PlankWidth, plankHeight: m_PlankHeight, gap: 0.15f, startPosition: new Vector2(-10.0f, 0));
        //CreateSuspensionBridgeSafe(plankCount: 10, plankWidth: m_PlankWidth, plankHeight: m_PlankHeight, gap: 0.15f, startPosition: new Vector2(-5.0f, 0));

    }

    public override void OnUpdate(float deltaTime)
	{
		m_Time += deltaTime;
	}

	// Scene Construction
	private void CreateGround()
	{
		Entity ground = new Entity("Ground");

		ground.Translation = new Vector3(0, -5, 0);

		var rect = ground.AddComponent<RectangleComponent>();
		rect.Size = new Vector2(20, 1);
		rect.Color = new Vector4(0.3f, 0.3f, 0.3f, 1);

		ground.AddComponent<BoundingBoxComponent>().Size = rect.Size;

		var rb = ground.AddComponent<RigidBodyComponent>();
	}

	private void CreatePlatforms()
	{
		for (int i=0; i < 5; i++)
		{
			Entity platform = new Entity("Platform_" + i);

			platform.Translation = new Vector3(-1 + i * 1.5f, -2.5f, 0);

			var rect = platform.AddComponent<RectangleComponent>();
			rect.Size = new Vector2(0.5f, 4.0f);
			rect.Color = new Vector4(0.2f, 0.6f, 0.8f, 1);

			platform.AddComponent<BoundingBoxComponent>().Size = rect.Size;

			var rb = platform.AddComponent<RigidBodyComponent>();
			rb.Type = RigidBodyComponent.BodyType.Dynamic;
		}
	}

	private void CreatePendulum()
	{
		Entity anchor = new Entity("PendulumAnchor");
		anchor.Translation = new Vector3(9, 4, 0);
		var rect = anchor.AddComponent<RectangleComponent>();
		rect.Size = new Vector2(4, 0.4f);
		rect.Color = new Vector4(1.0f, 0.3f, 0.3f, 1.0f);
		anchor.AddComponent<BoundingBoxComponent>().Size = rect.Size;
		anchor.AddComponent<RigidBodyComponent>();

		Entity bob = new Entity("PendulumBob");
		bob.Translation = new Vector3(13, -1, 0);

		var circle = bob.AddComponent<CircleComponent>();
		circle.Radius = 0.4f;
		circle.Color = new Vector4(1, 0.3f, 0.3f, 1);

		bob.AddComponent<CircleColliderComponent>().Radius = circle.Radius;

		var rb = bob.AddComponent<RigidBodyComponent>();
		rb.Type = RigidBodyComponent.BodyType.Dynamic;

		var joint = anchor.AddComponent<JointComponent>();
		joint.ConnectedEntity = bob;
		joint.Anchor = new Vector2(0, 0);
		joint.Softness = 0.02f;
		joint.Bias = 0.2f;
	}

	private void CreateBridge()
	{
		// left support
		Entity leftSupport = new Entity("BridgeSupport_Left");
		leftSupport.Translation = new Vector3(m_StartX - m_PlankWidth * 0.5f, m_BridgeY, 0);
		var rectLeft = leftSupport.AddComponent<RectangleComponent>();
		rectLeft.Size = new Vector2(m_PlankWidth * 2.0f, 3);
		rectLeft.Color = new Vector4(0.6f, 0.4f, 0.2f, 1);
		leftSupport.AddComponent<BoundingBoxComponent>().Size = rectLeft.Size;
		leftSupport.AddComponent<RigidBodyComponent>();

		// right support
		Entity rightSupport = new Entity("BridgeSupport_Right");
		rightSupport.Translation = new Vector3(m_StartX + m_PlankCount * m_PlankWidth + m_PlankWidth * 0.5f, m_BridgeY, 0);
        var rectRight = rightSupport.AddComponent<RectangleComponent>();
        rectRight.Size = new Vector2(m_PlankWidth * 2.0f, 3);
        rectRight.Color = new Vector4(0.6f, 0.4f, 0.2f, 1);
        rightSupport.AddComponent<BoundingBoxComponent>().Size = rectRight.Size;
		rightSupport.AddComponent<RigidBodyComponent>();


        // bridge planks
        Entity previousEntity = leftSupport;
		for (int i=0; i <m_PlankCount; i++)
		{
			Entity plank = new Entity("BridgePlank_" + i);
			plank.Translation = new Vector3(m_StartX + i*m_PlankWidth + m_PlankWidth*0.5f, m_BridgeY, 0);
			
			var rect = plank.AddComponent<RectangleComponent>();
			rect.Size = new Vector2(m_PlankWidth, m_PlankHeight);
			rect.Color = new Vector4(0.6f, 0.4f, 0.2f, 1);

			plank.AddComponent<BoundingBoxComponent>().Size = rect.Size;

			var rb = plank.AddComponent<RigidBodyComponent>();
			rb.Type = RigidBodyComponent.BodyType.Dynamic;

			// joint: previous -> think plank
			var joint = plank.AddComponent<JointComponent>();
			joint.ConnectedEntity = previousEntity;
			// anchor at start of current plank
			joint.Anchor = new Vector2(-m_PlankWidth * 0.5f, 0);
			joint.Softness = 0.1f;
			joint.Bias = 0.3f;

			previousEntity = plank;
		}

		// final joint: last plank -> right support
		{
			var joint = previousEntity.AddComponent<JointComponent>();
			joint.ConnectedEntity = rightSupport;
			joint.Anchor = new Vector2(m_PlankWidth * 0.5f, 0);
			joint.Softness = 0.1f;
			joint.Bias = 0.3f;
		}
    }

    private void CreateSuspensionBridge(
    int plankCount,
    float plankWidth,
    float plankHeight,
    float gap,
    Vector2 startPosition)
    {
        float step = plankWidth + gap;

        // -----------------------------
        // Left Support
        // -----------------------------
        Entity leftSupport = new Entity("BridgeSupport_Left");
        leftSupport.Translation = new Vector3(startPosition.X - step, startPosition.Y, 0);
		leftSupport.AddComponent<RectangleComponent>().Size = new Vector2(1, 3);
		leftSupport.AddComponent<BoundingBoxComponent>().Size = new Vector2(1, 3);
        leftSupport.AddComponent<RigidBodyComponent>();

        // -----------------------------
        // Right Support
        // -----------------------------
        Entity rightSupport = new Entity("BridgeSupport_Right");
        rightSupport.Translation = new Vector3(startPosition.X + plankCount * step, startPosition.Y, 0);
		rightSupport.AddComponent<RectangleComponent>().Size = new Vector2(1, 3);
		rightSupport.AddComponent<BoundingBoxComponent>().Size = new Vector2(1, 3);
        rightSupport.AddComponent<RigidBodyComponent>();

        Entity previousEntity = leftSupport;

        // -----------------------------
        // Planks
        // -----------------------------
        for (int i = 0; i < plankCount; i++)
        {
            Entity plank = new Entity("BridgePlank_" + i);

            float x = startPosition.X + i * step;

            plank.Translation = new Vector3(x, startPosition.Y + 1.5f - plankHeight * 0.5f, 0);

            var rect = plank.AddComponent<RectangleComponent>();
            rect.Size = new Vector2(plankWidth, plankHeight);
            rect.Color = new Vector4(0.6f, 0.4f, 0.25f, 1.0f);

            plank.AddComponent<BoundingBoxComponent>().Size = rect.Size;

            var rb = plank.AddComponent<RigidBodyComponent>();
            rb.Type = RigidBodyComponent.BodyType.Dynamic;

            // -----------------------------
            // Joint: plank → previous entity
            // -----------------------------
            var joint = previousEntity.AddComponent<JointComponent>();
            joint.ConnectedEntity = plank;

            // LOCAL anchor on this plank: left edge
			if (i == 0)
			{
                joint.Anchor = new Vector2(plankWidth * 0.5f, 1.5f);
            }
			else
			{
                joint.Anchor = new Vector2(plankWidth * 0.5f, 0.0f);
            }

            // Suspension feel
            //joint.Softness = 0.25f;
            //joint.Bias = 0.15f;

            previousEntity = plank;
        }

        // -----------------------------
        // Final joint: last plank → right support
        // -----------------------------
        {
            var joint = previousEntity.AddComponent<JointComponent>();
            joint.ConnectedEntity = rightSupport;

            // LOCAL anchor on last plank: right edge
            joint.Anchor = new Vector2(plankWidth * 0.5f, 0.0f);

            //joint.Softness = 0.25f;
            //joint.Bias = 0.15f;
        }
    }


    private void CreateSuspensionBridgeSafe(
    int plankCount,
    float plankWidth,
    float plankHeight,
    float gap,
    Vector2 startPosition)
    {
        float step = plankWidth + gap;

        // Lists to store created entities
        var planks = new List<Entity>();

        // Create supports
        Entity leftSupport = new Entity("BridgeSupport_Left");
        leftSupport.Transform.Translation = new Vector3(startPosition.X - step, startPosition.Y, 0);
        leftSupport.AddComponent<RectangleComponent>().Size = new Vector2(1, 3);
        leftSupport.AddComponent<BoundingBoxComponent>().Size = new Vector2(1, 3);
        leftSupport.AddComponent<RigidBodyComponent>().Type = RigidBodyComponent.BodyType.Static;

        Entity rightSupport = new Entity("BridgeSupport_Right");
        rightSupport.Transform.Translation = new Vector3(startPosition.X + plankCount * step, startPosition.Y, 0);
        rightSupport.AddComponent<RectangleComponent>().Size = new Vector2(1, 3);
        rightSupport.AddComponent<BoundingBoxComponent>().Size = new Vector2(1, 3);
        rightSupport.AddComponent<RigidBodyComponent>().Type = RigidBodyComponent.BodyType.Static;

        // --- PASS 1: create all planks + physics bodies ---
        for (int i = 0; i < plankCount; i++)
        {
            float x = startPosition.X + i * step;
            Entity plank = new Entity("BridgePlank_" + i);
            plank.Transform.Translation = new Vector3(x, startPosition.Y, 0);

            var rect = plank.AddComponent<RectangleComponent>();
            rect.Size = new Vector2(plankWidth, plankHeight);
            rect.Color = new Vector4(0.6f, 0.4f, 0.25f, 1.0f);

            plank.AddComponent<BoundingBoxComponent>().Size = rect.Size;

            var rb = plank.AddComponent<RigidBodyComponent>();
            rb.Type = RigidBodyComponent.BodyType.Dynamic;

            planks.Add(plank);
        }

        // --- PASS 2: create joints now that all runtime bodies exist ---
        // connect leftSupport -> first plank (reciprocal not always necessary; editor sometimes only had single joint per connection)
        {
            var j = leftSupport.AddComponent<JointComponent>();
            j.ConnectedEntity = planks[0];
            j.Anchor = new Vector2(0.5f, 1.5f); // local to support (match editor)
            j.Softness = 0.08f;
            j.Bias = 0.2f;

            var j2 = planks[0].AddComponent<JointComponent>();
            j2.ConnectedEntity = leftSupport;
            j2.Anchor = new Vector2(-plankWidth * 0.5f, 0.0f);
            j2.Softness = 0.08f;
            j2.Bias = 0.2f;
        }

        // connect planks in chain (reciprocal joints left/right)
        for (int i = 0; i < planks.Count - 1; ++i)
        {
            var cur = planks[i];
            var next = planks[i + 1];

            var jA = cur.AddComponent<JointComponent>();
            jA.ConnectedEntity = next;
            jA.Anchor = new Vector2(plankWidth * 0.5f, 0.0f); // local right edge
            jA.Softness = 0.20f;
            jA.Bias = 0.12f;

            var jB = next.AddComponent<JointComponent>();
            jB.ConnectedEntity = cur;
            jB.Anchor = new Vector2(-plankWidth * 0.5f, 0.0f); // local left edge
            jB.Softness = 0.20f;
            jB.Bias = 0.12f;
        }

        // connect last plank -> right support (reciprocal)
        {
            var last = planks[planks.Count - 1];
            var j = last.AddComponent<JointComponent>();
            j.ConnectedEntity = rightSupport;
            j.Anchor = new Vector2(plankWidth * 0.5f, 0.0f);
            j.Softness = 0.08f;
            j.Bias = 0.2f;

            var j2 = rightSupport.AddComponent<JointComponent>();
            j2.ConnectedEntity = last;
            j2.Anchor = new Vector2(-0.5f, 1.5f); // local left/top of support (tweak for look)
            j2.Softness = 0.08f;
            j2.Bias = 0.2f;
        }
    }

}
