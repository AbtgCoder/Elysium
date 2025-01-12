#pragma once

#include "core/Layer.h"

class ImGuiLayer : public Layer
{
public:
	ImGuiLayer();
	~ImGuiLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent() override;

	void Begin();
	void End();

	void BlockEvents(bool block) { m_BlockEvents = block; }

	void SetImGuiStyle();

	uint32_t GetActiveWidgetID() const;
private:
	bool m_BlockEvents = true;
};
