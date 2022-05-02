#include "plugin.hpp"

struct Quadro360 : Module {
	enum ParamIds {
		ANGLE_PARAM,
		RADIUS_IN_PARAM,
		RADIUS_OUT_PARAM,  
		NUM_PARAMS
	};

	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		IN5_INPUT,
		IN6_INPUT,
		IN7_INPUT,
		IN8_INPUT,
		ANGLE_INPUT,
		RADIUS_IN_INPUT,
		RADIUS_OUT_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT00_OUTPUT,
		OUT10_OUTPUT,
		OUT11_OUTPUT,
		OUT01_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	static const int SQRTS_SIZE = 32769;
	float sqrts[SQRTS_SIZE];

	Quadro360() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ANGLE_PARAM, -360.f, 360.f, 0.f, "Angle of rotation");
		configParam(RADIUS_IN_PARAM, 0.f, 100.f, 100.f, "Input radius");
		configParam(RADIUS_OUT_PARAM, 0.f, 100.f, 100.f, "Output radius");

		for (int i = 0; i < 128; i++) {
			for (int j = 0; j < 128; j++) {
				printf("%d, %f\n", i * i + j * j, sqrt(i * i + j * j));
				sqrts[0] = sqrt(i * i + j * j);
			}
		}
	}

	float measureDistance(float x1, float y1, float x2, float y2) {
		float w = (x1 - x2);
		float h = (y1 - y2);;
		float res = sqrt(h * h + w * w);

		if (res == 0) {
			res = 0.01;
		}

		return res;
	}

	int colorshift = 0;

	void process(const ProcessArgs& args) override {
		float shift = (inputs[ANGLE_INPUT].getVoltage() * 18 + params[ANGLE_PARAM].getValue()) / 180 * M_PI;
		float radiusIn = std::fmax(((inputs[RADIUS_IN_INPUT].getVoltage()) / 0.1 + params[RADIUS_IN_PARAM].getValue()) / 100, 0.f);
		float radiusOut = std::fmax(((inputs[RADIUS_OUT_INPUT].getVoltage()) / 0.1 + params[RADIUS_OUT_PARAM].getValue()) / 100, 0.f);

		float out00 = 0;
		float out10 = 0;
		float out01 = 0;
		float out11 = 0;

		InputIds inps[] = {
			IN1_INPUT, IN2_INPUT, IN3_INPUT, IN4_INPUT,
			IN5_INPUT, IN6_INPUT, IN7_INPUT, IN8_INPUT
		};

		for (int i = 0; i < 8; i++) {
			float i2 = i;
			float x = getSin(i2 * 0.785 + shift) * 64 * radiusIn;
			float y = getCos(i2 * 0.785 + shift) * 64 * radiusIn;

			float v = inputs[inps[i]].getVoltage();

			float d00 = v * (128 - measureDistance(-64 * radiusOut, -64 * radiusOut, x, y)) / 128;
			float d10 = v * (128 - measureDistance( 64 * radiusOut, -64 * radiusOut, x, y)) / 128;
			float d01 = v * (128 - measureDistance(-64 * radiusOut,  64 * radiusOut, x, y)) / 128;
			float d11 = v * (128 - measureDistance( 64 * radiusOut,  64 * radiusOut, x, y)) / 128;

			out00 += d00;
			out10 += d10;
			out01 += d01;
			out11 += d11;
		}

		outputs[OUT00_OUTPUT].setVoltage(out00);
		outputs[OUT10_OUTPUT].setVoltage(out10);
		outputs[OUT01_OUTPUT].setVoltage(out01);
		outputs[OUT11_OUTPUT].setVoltage(out11);
	} 
};

struct Quadro360Widget : ModuleWidget {
	Quadro360Widget(Quadro360* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Quadro360.svg")));

		addParam(createParam<RoundHugeBlackKnob>(Vec(13, 35), module, Quadro360::ANGLE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 20)), module, Quadro360::ANGLE_INPUT));

		addParam(createParam<RoundBlackKnob>(Vec(17, 315), module, Quadro360::RADIUS_IN_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22, 112)), module, Quadro360::RADIUS_IN_INPUT));

		addParam(createParam<RoundBlackKnob>(Vec(181, 315), module, Quadro360::RADIUS_OUT_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.5, 112)), module, Quadro360::RADIUS_OUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.7, 16)), module, Quadro360::OUT00_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.5, 27.5)), module, Quadro360::OUT10_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(67.5, 16)), module, Quadro360::OUT01_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(67.5, 27.5)), module, Quadro360::OUT11_OUTPUT));

		for (int i = 0; i < 8; i ++) {
			double x = double(8 - i - 6) / 4 * M_PI;
			double y = cos(x) * 27 + 75;
			x = getSin(x) * 27 + 38.1;
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x, y)), module, i));
		}
	}
};

Model* modelQuadro360 = createModel<Quadro360, Quadro360Widget>("Quadro360");
