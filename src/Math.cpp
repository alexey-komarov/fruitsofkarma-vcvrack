#include "plugin.hpp"


struct Math : Module {
	enum ParamIds {
		NUM_PARAMS
	};

	enum InputIds {
		MULA1_INPUT,
		MULA2_INPUT,
		MULA3_INPUT,
		DIVA1_INPUT,
		DIVA2_INPUT,
		DIVA3_INPUT,
		POWA1_INPUT,
		POWA2_INPUT,
		POWA3_INPUT,
		MULB1_INPUT,
		MULB2_INPUT,
		MULB3_INPUT,
		DIVB1_INPUT,
		DIVB2_INPUT,
		DIVB3_INPUT,
		POWB1_INPUT,
		POWB2_INPUT,
		POWB3_INPUT,
		LN1_INPUT,
		LN2_INPUT,
		LOG21_INPUT,
		LOG22_INPUT,
		LOG101_INPUT,
		LOG102_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		MUL1_OUTPUT,
		MUL2_OUTPUT,
		MUL3_OUTPUT,
		DIV1_OUTPUT,
		DIV2_OUTPUT,
		DIV3_OUTPUT,
		POW1_OUTPUT,
		POW2_OUTPUT,
		POW3_OUTPUT,
		LN1_OUTPUT,
		LN2_OUTPUT,
		LOG21_OUTPUT,
		LOG22_OUTPUT,
		LOG101_OUTPUT,
		LOG102_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	Math() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
		int numChannels;

		for (int k = 0; k < 3; k++) {
			for (int j = 0; j < 3; j++) {
				int i = k * 3 + j;

				if (inputs[MULA1_INPUT + i].isConnected() && inputs[MULB1_INPUT + i].isConnected() && outputs[MUL1_OUTPUT + i].isConnected()) {
					numChannels = inputs[MULA1_INPUT + i].getChannels();
					outputs[MUL1_OUTPUT + i].setChannels(numChannels);

					for (int c = 0; c < numChannels; c++) {
						double a = inputs[MULA1_INPUT + i].getVoltage(c);
						double b = inputs[MULB1_INPUT + i].getVoltage(c);
						if (k == 0) {
							outputs[MUL1_OUTPUT + j].setVoltage(a * b, c);
						} else if (k == 1) {
							outputs[DIV1_OUTPUT + j].setVoltage(a / b, c);
						} else if (k == 2) {
							outputs[POW1_OUTPUT + j].setVoltage(std::pow(a, b), c);
						}
					}
				}
			}
		}

		for (int k = 0; k < 3; k++) {
			for (int j = 0; j < 2; j++) {
				int i = k * 2 + j;

				if (inputs[LN1_INPUT + i].isConnected() && outputs[LN1_OUTPUT + i].isConnected()) {
					numChannels = inputs[LN1_INPUT + i].getChannels();
					outputs[LN1_OUTPUT + i].setChannels(numChannels);

					for (int c = 0; c < numChannels; c++) {
						double a = inputs[LN1_INPUT + i].getVoltage(c);

						if (k == 0) {
							outputs[LN1_OUTPUT + j].setVoltage(std::log(a), c);
						} else if (k == 1) {
							outputs[LOG21_OUTPUT + j].setVoltage(std::log2(a), c);
						} else if (k == 2) {
							outputs[LOG101_OUTPUT + j].setVoltage(std::log10(a), c);
						}
					}
				}
			}
		}
	}
};

struct MathWidget : ModuleWidget {
	MathWidget(Math* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Math.svg")));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 19)), module, Math::DIVA1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 19)), module, Math::DIVA2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42, 19)), module, Math::DIVA3_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 30)), module, Math::DIVB1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 30)), module, Math::DIVB2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42, 30)), module, Math::DIVB3_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18, 41)), module, Math::DIV1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 41)), module, Math::DIV2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42, 41)), module, Math::DIV3_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 55)), module, Math::POWA1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 55)), module, Math::POWA2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42, 55)), module, Math::POWA3_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 66)), module, Math::POWB1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 66)), module, Math::POWB2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42, 66)), module, Math::POWB3_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18, 77)), module, Math::POW1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 77)), module, Math::POW2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42, 77)), module, Math::POW3_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 93)), module, Math::MULA1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 93)), module, Math::MULA2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42, 93)), module, Math::MULA3_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 104)), module, Math::MULB1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30, 104)), module, Math::MULB2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42, 104)), module, Math::MULB3_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18, 115)), module, Math::MUL1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30, 115)), module, Math::MUL2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42, 115)), module, Math::MUL3_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(68, 19)), module, Math::LN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78, 19)), module, Math::LN2_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(68, 29)), module, Math::LN1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(78, 29)), module, Math::LN2_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(68, 55)), module, Math::LOG21_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78, 55)), module, Math::LOG22_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(68, 66)), module, Math::LOG21_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(78, 66)), module, Math::LOG22_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(68, 93)), module, Math::LOG101_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78, 93)), module, Math::LOG102_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(68, 104)), module, Math::LOG101_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(78, 104)), module, Math::LOG102_OUTPUT));
	}
};

Model* modelMath = createModel<Math, MathWidget>("Math");
