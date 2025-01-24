#pragma once
#include <vector>

class RendererInterface
{
public:
	virtual ~RendererInterface() = default;

	virtual void init(int width, int height, const char *title) = 0;
	virtual void processFrame() = 0;
	virtual void cleanup() = 0;
	virtual void render_mesh() = 0;
};