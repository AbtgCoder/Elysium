#pragma once


class LoggerPanel
{
public:
	LoggerPanel() = default;

	void OnImGuiRender();
private:
	bool m_LogErrors = true;
	bool m_LogWarnings = true;
	bool m_LogDebug = true;
	bool m_AutoScroll = true;
};