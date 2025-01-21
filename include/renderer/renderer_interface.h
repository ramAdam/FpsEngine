#pragma once
#include <vector>

class RendererInterface
{
public:
	virtual ~RendererInterface() = default;
	virtual void init(int width, int height, const char *title) = 0;
	virtual void begin_frame() = 0;
	virtual void end_frame() = 0;
	virtual void cleanup() = 0;
	virtual void render_mesh() = 0;
};