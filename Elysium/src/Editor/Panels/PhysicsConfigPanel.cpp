#include "PhysicsConfigPanel.h"

#include "ImGui/ImGuiHelper.h"

PhysicsConfigPanel::PhysicsConfigPanel(const std::shared_ptr<Scene>& Scene)
{
	SetScene(Scene);
}

void PhysicsConfigPanel::SetScene(const std::shared_ptr<Scene>& Scene)
{
	m_Scene = Scene;
}

void PhysicsConfigPanel::OnImGuiRender()
{
	if (m_Scene)
	{
		ImGui::Begin("Physics Settings");
		DrawVec2Control("Gravity", m_Scene->m_gravity, 0.0f, 80.0f);
		DrawVec2Control("Force", m_Scene->m_externalForce, 0.0f, 80.0f);
		DrawIntControl("Velocity Iterations", m_Scene->m_velocityIterations, 1, 120, 150.0f);
		DrawIntControl("Position Iterations", m_Scene->m_positionIterations, 1, 120, 150.0f);
		ImGui::Checkbox("Draw Colliders", &m_Scene->m_drawPhysicsColliders);
		ImGui::Checkbox("Broadphase Collision: KD Trees", &m_Scene->m_KDTreeBroadPhaseCollision);
		ImGui::End();
	}
}
