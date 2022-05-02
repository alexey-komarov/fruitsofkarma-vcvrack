#include "plugin.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	calcsins();
	pluginInstance = p;
	p->addModel(modelQuadro360);
	p->addModel(modelColoredGlass);
}
