#include "ExRTCWindow.h"

ExRTCWindow::ExRTCWindow(webrtc::DesktopCapturer::SourceId id, const std::string& title, const std::string& image, int width, int height)
	: m_id(id)
	, m_title(title)
	, m_image(image)
	, m_width(width)
	, m_height(height)
{

}

ExRTCWindow::~ExRTCWindow()
{

}
