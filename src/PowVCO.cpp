#include "plugin.hpp"

struct PowVCO : Module {
	enum ParamIds {
		FREQ_PARAM,
		POW1_PARAM,
		POW2_PARAM,
		POW3_PARAM,
		POW4_PARAM,
		FREQDIV_PARAM,
		ENV_PARAM,
		CLIP_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		PITCH_INPUT,
		POW1_INPUT,
		POW2_INPUT,
		POW3_INPUT,
		POW4_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		WAVE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	int numChannels = 0;
	float phase = 0.f;
	float phases[PORT_MAX_CHANNELS] = {};

	const float range = 10;
	const float range2x = range * 2;
	const float range3x = range2x + range;
	const float range4x = range2x * 2;

	PowVCO() {
		for (int i = 0; i < PORT_MAX_CHANNELS; i++) {
			phases[i] = 0;
		}

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ENV_PARAM, 0.f, 1.f, 0.f, "Envelope mode");
		configParam(CLIP_PARAM, 0.f, 1.f, 1.f, "Clip");
		configParam(FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(POW1_PARAM, -10.f, 10.f, 2.f, "Pow1");
		configParam(POW2_PARAM, -10.f, 10.f, 2.f, "Pow2");
		configParam(POW3_PARAM, -10.f, 10.f, 2.f, "Pow3");
		configParam(POW4_PARAM, -10.f, 10.f, 2.f, "Pow4");
		configParam(FREQDIV_PARAM, 1.f, 128.f, 1.f, "Frequency divider");
	}

	float pow1 = 0.f;
	float pow2 = 0.f;
	float pow3 = 0.f;
	float pow4 = 0.f;
	bool prevEnvMode = false;

	float calc(float x) {
		float y;
		bool envMode = params[ENV_PARAM].getValue() == 1;


		if (envMode) {
			if (x <= range) {
				y = (std::pow(x / range, pow1) * range - range) * -1;
			} else if (x < range2x) {
				y = std::pow((range - (x - range)) / range, pow2) * range - range;
			} else if (x < range2x + range) {
				y = std::pow((x - range2x) / range, pow3) * range - range;
			} else {
				y = (std::pow((range - x + range3x) / range, pow4) * range - range) * -1;
			}
		} else {
			if (x <= range) {
				y = std::pow((range - x) / range, pow1) * range - range;
			} else if (x < range2x) {
				y = std::pow((x - range) / range, pow2) * range - range;
			} else if (x < range2x + range) {
				y = (std::pow((range - x + range2x) / range, pow3) * range - range) * -1;
			} else {
				y = (std::pow((x - range3x) / range, pow4) * range - range) * -1;
			}
		}

		if (params[CLIP_PARAM].getValue() == 1) {
			y = std::min(std::max(-10.0f, y), 10.0f);
		}

		return y;
	}

	void process(const ProcessArgs& args) override {
		pow1 = inputs[POW1_INPUT].getVoltage() + params[POW1_PARAM].getValue();
		pow2 = inputs[POW2_INPUT].getVoltage() + params[POW2_PARAM].getValue();
		pow3 = inputs[POW3_INPUT].getVoltage() + params[POW3_PARAM].getValue();
		pow4 = inputs[POW4_INPUT].getVoltage() + params[POW4_PARAM].getValue();

		float y;
		float freq;
		float pitch;
		float newPhase;
		float deltaPhase;

		if (inputs[RESET_INPUT].isConnected() && inputs[RESET_INPUT].getVoltage() >= 1) {
			phase = 0;

			for (int i = 0; i < PORT_MAX_CHANNELS; i++) {
				phases[i] = 0;
			}
		}

		bool envMode = params[ENV_PARAM].getValue() == 1;

		if (inputs[PITCH_INPUT].isConnected()) {
			numChannels = inputs[PITCH_INPUT].getChannels();
			outputs[WAVE_OUTPUT].setChannels(numChannels);

			for (int c = 0; c < numChannels; c++) {
				pitch = params[FREQ_PARAM].getValue() / 12.f;
				pitch += inputs[PITCH_INPUT].getPolyVoltage(c);
				freq = dsp::FREQ_C4 * std::pow(2.f, pitch);
				freq /= params[FREQDIV_PARAM].getValue();

				deltaPhase = freq * args.sampleTime;
				newPhase = phases[c] + deltaPhase * 40;

				if (params[ENV_PARAM].getValue() == 1 && newPhase > range4x) {
					outputs[WAVE_OUTPUT].setVoltage(0, c);
				} else {
					if (newPhase > range4x) {
						phases[c] = newPhase - range4x;
					} else {
						phases[c] = newPhase;
					}

					y = calc(phases[c]);

					if (envMode) {
						y = y * -1 + 10;
					}

					outputs[WAVE_OUTPUT].setVoltage(y / 2, c);
				}
			}
		} else {
			pitch = params[FREQ_PARAM].getValue() / 12.f;
			pitch += inputs[PITCH_INPUT].getVoltage();
			freq = dsp::FREQ_C4 * std::pow(2.f, pitch);
			freq /= params[FREQDIV_PARAM].getValue();

			deltaPhase = freq * args.sampleTime;
			newPhase = phase + deltaPhase * 40;

			if (params[ENV_PARAM].getValue() == 1 && newPhase > range4x) {
				outputs[WAVE_OUTPUT].setVoltage(0);
			} else {
				if (newPhase > range4x) {
					phase = newPhase - range4x;
				} else {
					phase = newPhase;
				}
			}

			y = calc(phase);

			if (envMode) {
				y = y * -1 + 10;
			}

			outputs[WAVE_OUTPUT].setVoltage(y / 2);
		}
	}
};

struct PowVCODisplay : ModuleLightWidget {
	PowVCO *module;

	PowVCODisplay() {
	}

	void draw (const DrawArgs &args) override {
		if(module == NULL) 
			return;

		bool envMode = module->params[PowVCO::ENV_PARAM].getValue() == 1;
		bool clipMode = module->params[PowVCO::CLIP_PARAM].getValue() == 1;

		nvgSave(args.vg);
		Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(args.vg, b.pos.x + 1, b.pos.y + 1, b.size.x - 2, b.size.y - 2);

		nvgBeginPath(args.vg);
		nvgFillColor(args.vg, nvgRGB(50, 50, 50));
		nvgRect(args.vg, 0, 0,  b.size.x,  b.size.y);
		nvgFill(args.vg);
		nvgBeginPath(args.vg);

		nvgStrokeColor(args.vg, nvgRGB(0, 200, 30));
		nvgStrokeWidth(args.vg, 2);

		nvgMoveTo(args.vg, 13.5, module->calc(0) * -1.5 * (envMode ? -1 : 1) + 26);

		for (int i = 1; i < 80; i += 2) {
			float y = module->calc(i / 2);

			if (envMode) {
				y = y * -1;
			}

			if (clipMode) {
				y = std::min(std::max(-10.0f, y), 10.0f);
			}

			nvgLineTo(args.vg, i * 1.5 + 13, y * -1.5 + 26);
		}

		nvgStroke(args.vg);
		nvgResetScissor(args.vg);
		nvgRestore(args.vg);
	}
};

struct PowVCOWidget : ModuleWidget {
	TransparentWidget *display;

	PowVCOWidget(PowVCO* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PowVCO.svg")));

		{
			PowVCODisplay *display = new PowVCODisplay();
			display->module = module;
			display->box.pos = Vec(1, 22);
			display->box.size = Vec(147, 50);
			addChild(display);
			this->display = display;
		}

		addParam(createParam<RoundHugeBlackKnob>(Vec(9, 83), module, PowVCO::FREQ_PARAM));
		addParam(createParam<RoundLargeBlackKnob>(Vec(102, 102), module, PowVCO::FREQDIV_PARAM));

		addParam(createParam<CKSS>(Vec(57, 160), module, PowVCO::ENV_PARAM));
		addParam(createParam<CKSS>(Vec(77, 160), module, PowVCO::CLIP_PARAM));


		addParam(createParam<RoundBlackKnob>(Vec(10, 193), module, PowVCO::POW1_PARAM));
		addParam(createParam<RoundBlackKnob>(Vec(110, 193), module, PowVCO::POW2_PARAM));
		addParam(createParam<RoundBlackKnob>(Vec(10, 260), module, PowVCO::POW3_PARAM));
		addParam(createParam<RoundBlackKnob>(Vec(110, 260), module, PowVCO::POW4_PARAM));

		addInput(createInput<PJ301MPort>(Vec(42, 195), module, PowVCO::POW1_INPUT));
		addInput(createInput<PJ301MPort>(Vec(83, 195), module, PowVCO::POW2_INPUT));
		addInput(createInput<PJ301MPort>(Vec(42, 262), module, PowVCO::POW3_INPUT));
		addInput(createInput<PJ301MPort>(Vec(83, 262), module, PowVCO::POW4_INPUT));

		addInput(createInput<PJ301MPort>(Vec(65, 315), module, PowVCO::RESET_INPUT));
		addInput(createInput<PJ301MPort>(Vec(10, 315), module, PowVCO::PITCH_INPUT));
		addOutput(createOutput<PJ301MPort>(Vec(115, 315), module, PowVCO::WAVE_OUTPUT));
	}
};

Model* modelPowVCO = createModel<PowVCO, PowVCOWidget>("PowVCO");
