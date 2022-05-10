#include "plugin.hpp"

const int PARTICLES_MAX = 20000;
const int RADIUS_MAX = 200;
const int halfsize = 198;

#define PARAM_MAP(X)      \
	X(AMOUNT)         \
	X(ROTATE)         \
	X(EDGES)          \
	X(ALPHA)          \
	X(DISTORT)        \
	X(STROKE)         \
	X(RADIUS)         \
	X(ROTATE_ALL)     \
	X(ROTATE_RAND)    \
	X(EDGES_RAND)     \
	X(ALPHA_RAND)     \
	X(DISTORT_MORE)   \
	X(STROKE_RAND)    \
	X(RADIUS_RAND)    \
	X(ANGLE)          \
	X(CENTERX)        \
	X(CENTERY)        \
	X(SPEED)          \
	X(RED)            \
	X(GREEN)          \
	X(BLUE)           \
	X(RESET)          \

typedef struct {
	double x;
	double y;
	double inverted;
	double radius;
	double vector;
	int edges;
	int width;
	double angle;
	int red;
	int green;
	int blue;
	int alpha;
	bool locked;
	double distance;
} TParticle;

TParticle Particles[PARTICLES_MAX];

typedef struct {
	double angle;
	double rotateAll;
	double rotate;
	double rotateRand;
	double amount;
	double distort;
	double distortMore;
	double edges;
	double edgesRand;
	double alpha;
	double alphaRand;
	double stroke;
	double strokeRand;
	double radius;
	double radiusRand;
	double centerX;
	double centerY;
	double speed;
	double red;
	double green;
	double blue;
} TSettings;

TSettings Settings;

const double PIX2 = M_PI * 2;
int colorshift = 0;

void setColors(int shift) {
	for (int i = 0; i < Settings.amount; i++) {
		TParticle &p = Particles[i];
		double angle = p.vector + shift / 100;

		p.red = getSin(angle / 2) * 127 + 128;
		p.green = getCos(angle / 2) * 127 + 128;
		p.blue = getSin(angle / 3) * 127 + 128;
	}
}

void initParticles(void) {
	for (int i = 0; i < PARTICLES_MAX; i++) {
		Particles[i].locked = false;
		Particles[i].x = 0;
		Particles[i].y = 0;
		Particles[i].distance = 0;
		Particles[i].inverted = 1;
		Particles[i].vector = (double(rand() % 1000) / 1000) * PIX2;
		Particles[i].radius = 1 + (rand() % 50);
		Particles[i].width = rand() % 4;
		Particles[i].angle = 0;
		Particles[i].alpha = 1 + rand() % 250;
		Particles[i].edges = 1 + rand() % 5;
		Particles[i].angle = rand() % 360;
	}

	setColors(0);
}

struct ColoredGlass : Module {
#define	PARAM_EXPAND(enum_name) enum_name##_PARAM,
	enum ParamIds {
		PARAM_MAP(PARAM_EXPAND)
		NUM_PARAMS
	};
#undef PARAM_EXPAND

#define	INPUT_EXPAND(enum_name) enum_name##_INPUT,
	enum InputIds {
		PARAM_MAP(INPUT_EXPAND)
		NUM_INPUTS
	};
#undef INPUT_EXPAND

	enum OutputIds {
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	ColoredGlass() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(AMOUNT_PARAM,            0.f,  PARTICLES_MAX, 500.f, "Amount of particles");
		configParam(ROTATE_PARAM,         -180.f,  180.f,         0.f,   "Angle of rotation");
		configParam(EDGES_PARAM,             0.f,   10.f,         3.f,   "Number of edges");
		configParam(ALPHA_PARAM,             0.f,  255.f,         60.f,  "Opacity");
		configParam(DISTORT_PARAM,           0.f,   10.f,         0.45f, "Amount of distortion");
		configParam(STROKE_PARAM,            0.f,   10.f,         1.7f,  "Stroke width");
		configParam(RADIUS_PARAM,            0.f,  RADIUS_MAX,    9.f,   "Radius");
		configParam(ROTATE_ALL_PARAM,     -180.f,  180.f,         0.f,   "Rotate all");
		configParam(ROTATE_RAND_PARAM,       0.f,    1.f,         0.f,   "Angle of rotation randomness");
		configParam(EDGES_RAND_PARAM,        0.f,    1.f,         0.6f,  "Number of edges randomess");
		configParam(ALPHA_RAND_PARAM,        0.f,    1.f,         0.5f,  "Opacity randomness");
		configParam(DISTORT_MORE_PARAM,      0.f,    5.f,         0.f,   "Amount of distortion randomness");
		configParam(STROKE_RAND_PARAM,       0.f,    1.f,         0.f,   "Stroke width randomness");
		configParam(RADIUS_RAND_PARAM,       0.f,    1.f,         1.f,   "Radius randomness");
		configParam(ANGLE_PARAM,            -1.f,    1.f,         0.01f, "Rotation speed");
		configParam(CENTERX_PARAM,        -100.f,  100.f,         0.f,   "Center X");
		configParam(CENTERY_PARAM,        -100.f,  100.f,         0.f,   "Center Y");
		configParam(SPEED_PARAM,           -10.f,   10.f,         2.0f,  "Speed");
		configParam(RED_PARAM,             -10.f,   10.f,         -9.f,  "Red color shift");
		configParam(GREEN_PARAM,           -10.f,   10.f,         1.9f,  "Green color shift");
		configParam(BLUE_PARAM,            -10.f,   10.f,         2.14f, "Blue color shift");
	}

	void process(const ProcessArgs& args) override {
		if (params[RESET_PARAM].getValue() + inputs[RESET_INPUT].getVoltage() > 0.f) {
			initParticles();
		}

		Settings.angle       = inputs[ANGLE_INPUT].getVoltage()        / 10   + params[ANGLE_PARAM].getValue();
		Settings.rotateAll   = inputs[ROTATE_ALL_INPUT].getVoltage()   * 18   + params[ROTATE_ALL_PARAM].getValue();
		Settings.rotate      = inputs[ROTATE_INPUT].getVoltage()       * 18   + params[ROTATE_PARAM].getValue();
		Settings.rotateRand  = inputs[ROTATE_RAND_INPUT].getVoltage()  / 10   + params[ROTATE_RAND_PARAM].getValue();
		Settings.amount      = inputs[AMOUNT_INPUT].getVoltage()       + params[AMOUNT_PARAM].getValue();
		Settings.distort     = inputs[DISTORT_INPUT].getVoltage()      / 10   + params[DISTORT_PARAM].getValue();
		Settings.distortMore = inputs[DISTORT_MORE_INPUT].getVoltage() / 2    + params[DISTORT_MORE_PARAM].getValue();
		Settings.edges       = inputs[EDGES_INPUT].getVoltage()        + params[EDGES_PARAM].getValue();
		Settings.edgesRand   = inputs[EDGES_RAND_INPUT].getVoltage()   + params[EDGES_RAND_PARAM].getValue();
		Settings.alpha       = inputs[ALPHA_INPUT].getVoltage()        * 25.5 + params[ALPHA_PARAM].getValue();
		Settings.alphaRand   = inputs[ALPHA_RAND_INPUT].getVoltage()   / 10   + params[ALPHA_RAND_PARAM].getValue();
		Settings.stroke      = inputs[STROKE_INPUT].getVoltage()       + params[STROKE_PARAM].getValue();
		Settings.strokeRand  = inputs[STROKE_RAND_INPUT].getVoltage()  + params[STROKE_RAND_PARAM].getValue();
		Settings.radius      = inputs[RADIUS_INPUT].getVoltage()       * 50   + params[RADIUS_PARAM].getValue();
		Settings.radiusRand  = inputs[RADIUS_RAND_INPUT].getVoltage()  / 10   + params[RADIUS_RAND_PARAM].getValue();
		Settings.centerX     = inputs[CENTERX_INPUT].getVoltage()      * 10   + params[CENTERX_PARAM].getValue();
		Settings.centerY     = inputs[CENTERY_INPUT].getVoltage()      * 10   + params[CENTERY_PARAM].getValue();
		Settings.speed       = inputs[SPEED_INPUT].getVoltage()        + params[SPEED_PARAM].getValue();
		Settings.red         = inputs[RED_INPUT].getVoltage()          + params[RED_PARAM].getValue();
		Settings.green       = inputs[GREEN_INPUT].getVoltage()        + params[GREEN_PARAM].getValue();
		Settings.blue        = inputs[BLUE_INPUT].getVoltage()         + params[BLUE_PARAM].getValue();
	}
};

double getRadius(TParticle p) {
	return std::min(int(Settings.radius + p.radius * Settings.radiusRand), RADIUS_MAX);
}

void tick(void) {
	setColors(colorshift++);

	for (int i = 0; i < Settings.amount; i++) {
		TParticle *p = &Particles[i];
		double r = p->distance + p->inverted * Settings.speed;

		if (r < 0) {
			r = 1;
			p->inverted *= -1;
		}

		p->distance = r;
		p->x = getCos(p->vector + Settings.rotateAll / 180 * NVG_PI) * r;
		p->y = getSin(p->vector + Settings.rotateAll / 180 * NVG_PI) * r;
		p->vector += Settings.angle / 5;
	}
}

struct ColoredGlassGlWidget : ModuleLightWidget {
	ColoredGlass *module;

	void drawParticle(const DrawArgs &args, TParticle *p) {
		int x = p->x + halfsize + Settings.centerX - 7;
		int y = p->y + halfsize + Settings.centerY - 10;
		int radius = std::max(0, std::min(int(Settings.radius + p->radius * Settings.radiusRand), RADIUS_MAX));
		int edges = Settings.edges + p->edges * Settings.edgesRand;
		float angle = Settings.rotate + p->angle * Settings.rotateRand;

		angle = angle / 180 * NVG_PI;

		double dm = Settings.distortMore;
		double d = Settings.distort + 1;

		int xs[edges];
		int ys[edges];
		int edge = 0;

		for (int i = 0; i < edges; i++) {
			int xx = x + radius * getCos((i * PIX2 / edges + angle) * (d / (dm + (1 / (dm + 1)))));
			int yy = y + radius * getSin((i * PIX2 / edges + angle) * d);

			if (xx < 6 || xx > 370 || yy < 6 || yy > 370) {
				continue;
			}

			xs[edge] = xx;
			ys[edge++] = yy;
		}

		if ((edges <= 2 && edge < 2) || (edges > 2 && edge < 3)) {
			if (!p->locked) {
				p->inverted *= -1;
				p->locked = true;
			}
		} else {
			p->locked = false;
		}

		nvgMoveTo(args.vg, xs[0], ys[0]);

		for (int i = 1; i < edge; i++) {
			nvgLineTo(args.vg, xs[i], ys[i]);
		}

		nvgLineTo(args.vg, xs[0], ys[0]);
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (!module || layer != 1) {
			return;
		}

		nvgMiterLimit(args.vg, 0);

		for (int i = 0; i < Settings.amount; i++) {
			nvgBeginPath(args.vg);
			drawParticle(args, &Particles[i]);
			TParticle p = Particles[i];

			int red = int(p.red * Settings.red / 10);
			int green = int(p.green * Settings.green / 10);
			int blue = int(p.blue * Settings.blue / 10);

			nvgFillColor(args.vg, nvgRGBA(red, green, blue,
				int(Settings.alpha + p.alpha * Settings.alphaRand)));

			nvgFill(args.vg);

			nvgStrokeColor(args.vg, nvgRGBA(red >> 1, green >> 1, blue >> 1,
				int(Settings.alpha + p.alpha * Settings.alphaRand)));

			nvgStrokeWidth(args.vg, Settings.stroke + p.width * Settings.strokeRand);
			nvgStroke(args.vg);
		}

		tick();
		Widget::drawLayer(args, layer);
	}
};

struct ColoredGlassWidget : ModuleWidget {
	ColoredGlassWidget(ColoredGlass* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ColoredGl.svg")));
		const int xp = 47;
		const int xi = 15;
		const int yp = 32;
		const int yi = 34;
		const int size = 49;

		for (int i = 0; i < ColoredGlass::NUM_PARAMS - 1; i++) {
			int xps = xp;
			int xis = xi;

			if (i >= 7 && i < 14) {
				xps += 130;
				xis += 132;
			} else if (i >= 14) {
				xps += 265;
				xis += 267;
			}

			addParam(createParam<RoundBlackKnob>(Vec(xps, yp + (i % 7) * size), module, i));
			addInput(createInput<PJ301MPort>    (Vec(xis, yi + (i % 7) * size), module, i));
		}

		addParam(createParam<LEDButton>(Vec(377, 353), module, ColoredGlass::RESET_PARAM));
		addInput(createInput<PJ301MPort>(Vec(400, 350), module, ColoredGlass::RESET_INPUT));

		initParticles();

		{
			ColoredGlassGlWidget *display = new ColoredGlassGlWidget();
			display->module = module;
			display->setSize(Vec(398, 398));
			display->setPosition(Vec(434, 1));
			addChild(display);
		}
	}
};

Model* modelColoredGlass = createModel<ColoredGlass, ColoredGlassWidget>("ColoredGlass");
