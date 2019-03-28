#pragma once
#include "Config.h"
#include "Common.h"

#include "webrtc/modules/desktop_capture/desktop_capturer.h"

class ExRTCWindow {
public:
	ExRTCWindow(webrtc::DesktopCapturer::SourceId id, const std::string& title, const std::string& image, int width, int height);
	virtual ~ExRTCWindow();

	_inline webrtc::DesktopCapturer::SourceId id()const { return m_id; }
	_inline const std::string& title()const { return m_title; }
	_inline const std::string& image()const { return m_image; }
	_inline const int width()const { return m_width; }
	_inline const int height()const { return m_height; }

private:
	webrtc::DesktopCapturer::SourceId m_id;
	std::string m_title;
	std::string m_image;
	int m_width;
	int m_height;
};
