#include "plugin.hpp"

const int PARTICLES_MAX = 5000;
const int RADIUS_MAX = 200;
const int halfsize = 198;

#define	PARAM_MAP(X)  \
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
		Settings.angle       = inputs[ANGLE_INPUT].getVoltage()        / 10   + params[ANGLE_PARAM].getValue();
		Settings.rotateAll   = inputs[ROTATE_ALL_INPUT].getVoltage()   * 18   + params[ROTATE_ALL_PARAM].getValue();
		Settings.rotate      = inputs[ROTATE_INPUT].getVoltage()       * 18   + params[ROTATE_PARAM].getValue();
		Settings.rotateRand  = inputs[ROTATE_RAND_INPUT].getVoltage()  / 10   + params[ROTATE_RAND_PARAM].getValue();
		Settings.amount      = inputs[AMOUNT_INPUT].getVoltage()       + params[AMOUNT_PARAM].getValue();
		Settings.distort     = inputs[DISTORT_INPUT].getVoltage()      / 10   + params[DISTORT_PARAM].getValue();
		Settings.distortMore  = inputs[DISTORT_MORE_INPUT].getVoltage() / 2    + params[DISTORT_MORE_PARAM].getValue();
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

const double PIX2 = M_PI * 2;
int colorshift = 0;

void setColors(int shift) {
	for (int i = 0; i < Settings.amount; i++) {
		TParticle &p = Particles[i];
		double angle = p.vector * 5 - ((shift % 628) / 100);

		p.red = abs(round(getCos(angle / 3) * 128 + 128));
		p.green = round(abs(round(getSin(angle / 4) * 128 + 128)));
		p.blue = round(abs(round(getSin(angle / 12) * 128 + 128)));
	}
}

void initParticles(void) {
	for (int i = 0; i < PARTICLES_MAX; i++) {
		Particles[i].x = 0;
		Particles[i].y = 0;
		Particles[i].inverted = 1;
		Particles[i].vector = (double(rand() % 1000) / 1000) * PIX2;
		Particles[i].radius = 1 + (rand() % 50);
		Particles[i].width = rand() % 4;
		Particles[i].angle = 0;
		Particles[i].alpha = rand() % 256;
		Particles[i].edges = 1 + rand() % 5;
		Particles[i].angle = rand() % 360;
	}

	setColors(0);
}

double getRadius(TParticle p) {
	return std::min(int(Settings.radius + p.radius * Settings.radiusRand), RADIUS_MAX);
}

void tick(void) {
	setColors(colorshift++);

	for (int i = 0; i < Settings.amount; i++) {
		double r = sqrt(pow(Particles[i].x, 2) + pow(Particles[i].y, 2)) +
			Particles[i].inverted * Settings.speed;

		Particles[i].x = getCos(Particles[i].vector + Settings.rotateAll / 180 * NVG_PI) * r;
		Particles[i].y = getSin(Particles[i].vector + Settings.rotateAll / 180 * NVG_PI) * r;

		if (r < 0) {
			Particles[i].inverted *= -1;
		}

		Particles[i].vector += Settings.angle;

		int x = Particles[i].x + Settings.centerX;
		int y = Particles[i].y + Settings.centerY;

		if (x < halfsize * -1 || x > halfsize || y > halfsize || y < halfsize * -1) {
			Particles[i].inverted *= -1;
			Particles[i].vector += M_PI_2 / 10;
		}
	}
}

struct ColoredGlassGlWidget : ModuleLightWidget {
	ColoredGlass *module;

	int fixCoord(int c) {
		int width = (Settings.stroke + Settings.strokeRand * 3) / 2;
		return std::min(std::max(c, width), (halfsize << 1) - 20);
	}

	void drawPoly(const DrawArgs &args, int x, int y, int radius, int edges, double angle) {
		angle = angle / 180 * NVG_PI;

		double dm = Settings.distortMore;
		double d = Settings.distort + 1;

		int xx = fixCoord(x + radius * getCos(angle * (d / (dm + (1 / (dm + 1))))));
		int yy = fixCoord(y + radius * getSin(angle * d));

		nvgMoveTo(args.vg, xx, yy);

		for (int i = 1; i < edges; i++) {
			int nx = fixCoord(x + radius * getCos((i * PIX2 / edges + angle) * (d / (dm + (1 / (dm + 1))))));
			int ny = fixCoord(y + radius * getSin((i * PIX2 / edges + angle) * d));
			nvgLineTo(args.vg, nx, ny);
		}

		nvgLineTo(args.vg, xx, yy);
	}

	void draw(const DrawArgs &args) override {
		if (module == NULL) {
			return;
		}

		nvgMiterLimit(args.vg, 0);

		for (int i = 0; i < Settings.amount; i++) {
			nvgBeginPath(args.vg);
			TParticle p = Particles[i];

			drawPoly(
				args,
				p.x + halfsize + Settings.centerX,
				p.y + halfsize + Settings.centerY,
				std::min(int(Settings.radius + p.radius * Settings.radiusRand), RADIUS_MAX),
				Settings.edges + p.edges * Settings.edgesRand,
				Settings.rotate + p.angle * Settings.rotateRand
			);

			int red = int(p.red * Settings.red/5) % 256;
			int green = int(p.green * Settings.green/5) % 256;
			int blue = int(p.blue * Settings.blue/5) % 256;

			nvgFillColor(args.vg, nvgRGBA(red, green, blue,
				int(Settings.alpha + p.alpha * Settings.alphaRand)));

			nvgFill(args.vg);

			nvgStrokeColor(args.vg, nvgRGBA(red >> 1, green >> 1, blue >> 1,
				int(Settings.alpha + p.alpha * Settings.alphaRand)));

			nvgStrokeWidth(args.vg, Settings.stroke + p.width * Settings.strokeRand);
			nvgStroke(args.vg);
		}

		tick();
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

		for (int i = 0; i < ColoredGlass::NUM_PARAMS; i++) {
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

		initParticles();

		{
			ColoredGlassGlWidget *display = new ColoredGlassGlWidget();
			display->module = module;
			display->setSize(Vec(400, 400));
			display->setPosition(Vec(434, 1));
			addChild(display);
		}
	}
};

Model* modelColoredGlass = createModel<ColoredGlass, ColoredGlassWidget>("ColoredGlass");
