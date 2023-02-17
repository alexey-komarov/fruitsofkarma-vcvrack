#include "plugin.hpp"

#define MAX_PITCHES 12
#define MAX_SCALES 43
#define MAX_CHANS 16

bool scales[MAX_SCALES][MAX_PITCHES] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},
	{1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0},
	{1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0},
	{1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0},
	{1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},
	{1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0},
	{1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1},
	{1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0},
	{1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1},
	{1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1},
	{1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0},
	{1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0},
	{1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1},
	{1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1},
	{1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0},
	{1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
	{1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1},
	{1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0},
	{1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0},
	{1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},
	{1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0},
	{1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
	{1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1},
	{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
	{1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},
	{1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1},
	{1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0},
	{1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0},
	{1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0},
	{1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0},
	{1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0},
	{1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0},
	{1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1}
};

struct ScaleMerger : Module {
	const char* pitchNames[MAX_PITCHES] = {
		"C", "C#", "D", "D#", "E", "F","F#","G","G#","A","A#","B"
	};

	const char* customScaleName = "CUSTOM";

	const char* scaleNames[MAX_SCALES] = {
		"CHROMATIC", "WHOLE TONE", "IONIAN M", "DORIAN", "PHRYGIAN", "LYDIAN",
		"MIXOLYDIAN","AEOLIAN M", "LOCRIAN", "ACOUSTIC", "ALTERED", "AUGMENTED",
		"BEBOP DOMINANT", "BLUES", "ENIGMATIC", "FLAMENCO", "GYPSY",
		"HALF DIMINISHED", "HARMONIC MAJOR", "HARMONIC MINOR","HIRAJOSHI",
		"HUNGARIAN","MIYAKO-BUSHI", "INSEN", "IWATO", "LYDIAN AUGMENTED",
		"BEBOB MAJOR", "LOCRIAN MAJOR", "PENTATONIC MAJOR", "MELODIC MINOR",
		"PENTATONIC MINOR YO", "NEAPOLITAN MAJOR", "NEAPOLITAN MINOR",
		"OCTATONIC 1",  "OCTATONIC 2", "PERSIAN", "PHRYGIAN DOMINANT",
		"PROMETHEUS", "HARMONICS", "TRITONE", "TRITONE 2S", "UKRAINIAN DORIAN"
		"TURKISH"
	};

	enum ScaleId {
		SCALE1,
		SCALE2
	};

	enum InputIds {
		SCALE1_INPUT,
		SCALE2_INPUT,
		PITCH_INPUT,
		TRIGGER_INPUT,
		SCALE1_PROB_INPUT,
		SCALE2_PROB_INPUT,
		KEY1_INPUT,
		KEY2_INPUT,
		ENUMS(SCALE1_PITCH_PROB_INPUT, MAX_PITCHES),
		ENUMS(SCALE2_PITCH_PROB_INPUT, MAX_PITCHES),
		NUM_INPUTS
	};

	enum ParamsIds {
		ENUMS(SCALE1_PITCH_SW_PARAM, MAX_PITCHES),
		ENUMS(SCALE2_PITCH_SW_PARAM, MAX_PITCHES),
		ENUMS(SCALE_RES_PITCH_SW_PARAM, MAX_PITCHES),
		ENUMS(SCALE1_PITCH_PROB_PARAM, MAX_PITCHES),
		ENUMS(SCALE2_PITCH_PROB_PARAM, MAX_PITCHES),
		SCALE1_PROB_PARAM,
		SCALE2_PROB_PARAM,
		SCALE1_PARAM,
		SCALE2_PARAM,
		KEY1_PARAM,
		KEY2_PARAM,
		NUM_PARAMS
	};

	enum OutputIds {
		PITCH_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		ENUMS(SCALE1_PITCH_LIGHT, MAX_PITCHES),
		ENUMS(SCALE2_PITCH_LIGHT, MAX_PITCHES),
		ENUMS(SCALE_RES_PITCH_LIGHT, MAX_PITCHES),
		NUM_LIGHTS
	};

	int currentScale1 = 0;
	int currentScale2 = 0;

	int prevScale1 = -1;
	int prevScale2 = -1;

	int currentKey1 = 0;
	int currentKey2 = 0;

	int prevKey1 = -1;
	int prevKey2 = -1;

	char scaleName1[140];
	char scaleName2[140];

	bool customScale1 = false;
	bool customScale2 = false;

	bool scale1Pitches[MAX_PITCHES] = {};
	bool scale2Pitches[MAX_PITCHES] = {};
	bool scale1PitchesOnly[MAX_PITCHES] = {};
	bool scale2PitchesOnly[MAX_PITCHES] = {};
	bool scaleResPitches[MAX_PITCHES] = {};

	float probs1[MAX_PITCHES] = {};
	float probs2[MAX_PITCHES] = {};
	float probs[MAX_PITCHES] = {};

	float prevPitches[MAX_CHANS] = {0.f};

	bool initialized = false;

	int pitchesNorm[MAX_PITCHES] = {};

	dsp::BooleanTrigger scale1Triggers[MAX_PITCHES];
	dsp::BooleanTrigger scale2Triggers[MAX_PITCHES];
	dsp::BooleanTrigger scaleResTriggers[MAX_PITCHES];
	dsp::BooleanTrigger trigTriggers[MAX_CHANS];

	float randomness[MAX_CHANS][MAX_PITCHES];

	ScaleMerger() {
		random::init();
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SCALE1_PARAM, 0.0, MAX_SCALES - 1, 0.0, "Scale 1");
		configParam(SCALE2_PARAM, 0.0, MAX_SCALES - 1, 0.0, "Scale 2");
		configParam(KEY1_PARAM, 0.0, 11.0, 0.0, "Key 1");
		configParam(KEY2_PARAM, 0.0, 11.0, 0.0, "Key 2");

		configParam(SCALE1_PROB_PARAM, 0.0, 1.0, 1.0,
			"Probability of unique pitches in Scale 1");

		configParam(SCALE2_PROB_PARAM, 0.0, 1.0, 0.0,
			"Probability of unique pitches in Scale 2");

		for (int i = 0; i < MAX_PITCHES; i++) {
			pitchesNorm[i] = normalize(float(i) / MAX_PITCHES);

			configParam(SCALE1_PITCH_PROB_PARAM + i, 0.0, 1.0, 0.0,
				"Probability of " + std::string(pitchNames[i]));

			configParam(SCALE2_PITCH_PROB_PARAM + i, 0.0, 1.0, 0.0,
				"Probability of " + std::string(pitchNames[i]));
		}
	}

	const char *getScaleName(ScaleId scale) {
		if (scale == SCALE1) {
			if (customScale1) {
				return customScaleName;
			}

			sprintf(scaleName1, "%s/%s", scaleNames[currentScale1],
				pitchNames[currentKey1]);

			return scaleName1;
		} else if (scale == SCALE2) {
			if (customScale2) {
				return customScaleName;
			}

			sprintf(scaleName1, "%s/%s", scaleNames[currentScale2],
				pitchNames[currentKey2]);

			return scaleName1;
		}

		assert(false);
	}

	int normalize(float f) {
		f += 10;
		int res = std::abs(std::trunc(10000 * (f - std::trunc(f))));
		return res;
	}

	float min(float f1, float f2) {
		return f1 < f2 ? f1 : f2;
	}

	float max(float f1, float f2) {
		return f1 > f2 ? f1 : f2;
	}

	float getProb(int probParam, int probInput) {
		float v = inputs[probInput].isConnected() ?
			inputs[probInput].getVoltage() : 0;

		return max(min(params[probParam].getValue() + v, 1), 0);
	}

	void process(const ProcessArgs& args) override {
		int trigChans = 0;
		bool triggers[MAX_CHANS] = {false};

		if (inputs[TRIGGER_INPUT].isConnected()) {
			trigChans = inputs[TRIGGER_INPUT].getChannels();

			for (int i = 0; i < trigChans; i++) {
				float val = inputs[TRIGGER_INPUT].getPolyVoltage(i);

				if (trigTriggers[i].process(val) > 0.f) {
					triggers[i] = true;

					for (int j = 0; j < MAX_PITCHES; j++) {
						randomness[i][j] = random::uniform();
					}
				}
			}
		}

		int scale1 = params[SCALE1_PARAM].getValue();
		int scale2 = params[SCALE2_PARAM].getValue();

		if (inputs[SCALE1_INPUT].isConnected()) {
			scale1 += std::trunc(std::abs(
				inputs[SCALE1_INPUT].getVoltage() / 10 * (MAX_SCALES - 1)
			));

			scale1 %= MAX_SCALES;
		}

		if (inputs[SCALE2_INPUT].isConnected()) {
			scale2 += std::trunc(std::abs(
				inputs[SCALE2_INPUT].getVoltage() / 10 * (MAX_SCALES - 1)
			));

			scale2 %= MAX_SCALES;
		}

		currentScale1 = scale1;
		currentScale2 = scale2;

		currentKey1 = params[KEY1_PARAM].getValue();
		currentKey2 = params[KEY2_PARAM].getValue();

		if (inputs[KEY1_INPUT].isConnected()) {
			currentKey1 += std::trunc(std::abs(
				inputs[KEY1_INPUT].getVoltage() / 10 * 11
			));

			currentKey1 %= MAX_PITCHES;
		}

		if (inputs[KEY2_INPUT].isConnected()) {
			currentKey2 += std::trunc(std::abs(
				inputs[KEY2_INPUT].getVoltage() / 10 * 11
			));

			currentKey2 %= MAX_PITCHES;
		}

		if (initialized) {
			prevScale1 = currentScale1;
			prevScale2 = currentScale2;
			prevKey1 = currentKey1;
			prevKey2 = currentKey2;
			initialized = false;
		}

		if (currentScale1 != prevScale1 || currentKey1 != prevKey1) {
			for (int i = 0; i < MAX_PITCHES; i++) {
				scale1Pitches[i] =
					scales[currentScale1][(i + currentKey1) % MAX_PITCHES];

				params[SCALE1_PITCH_PROB_PARAM + i].setValue(scale1Pitches[i]);
			}

			prevScale1 = currentScale1;
			prevKey1 = currentKey1;
			customScale1 = false;
		}

		if (currentScale2 != prevScale2 || currentKey2 != prevKey2) {
			for (int i = 0; i < MAX_PITCHES; i++) {
				scale2Pitches[i] =
					scales[currentScale2][(i + currentKey2) % MAX_PITCHES];

				params[SCALE2_PITCH_PROB_PARAM + i].setValue(scale2Pitches[i]);
			}

			customScale2 = false;
			prevScale2 = currentScale2;
			prevKey2 = currentKey2;
		}

		for (int i = 0; i < MAX_PITCHES; i++) {
			float val = params[SCALE1_PITCH_SW_PARAM + i].getValue();

			if (scale1Triggers[i].process(val) > 0.f) {
				customScale1 = true;

				if (params[SCALE1_PITCH_PROB_PARAM + i].getValue()) {
					params[SCALE1_PITCH_PROB_PARAM + i].setValue(0);
				} else {
					params[SCALE1_PITCH_PROB_PARAM + i].setValue(1);
				}
			}

			val = params[SCALE2_PITCH_SW_PARAM + i].getValue();

			if (scale2Triggers[i].process(val) > 0.f) {
				customScale2 = true;

				if (params[SCALE2_PITCH_PROB_PARAM + i].getValue()) {
					params[SCALE2_PITCH_PROB_PARAM + i].setValue(0);
				} else {
					params[SCALE2_PITCH_PROB_PARAM + i].setValue(1);
				}
			}

			val = params[SCALE_RES_PITCH_SW_PARAM + i].getValue();

			if (scaleResTriggers[i].process(val) > 0.f) {
				if (params[SCALE1_PITCH_PROB_PARAM + i].getValue()
						|| params[SCALE2_PITCH_PROB_PARAM + i].getValue()) {
					params[SCALE1_PITCH_PROB_PARAM + i].setValue(0);
					params[SCALE2_PITCH_PROB_PARAM + i].setValue(0);
				} else {
					params[SCALE1_PITCH_PROB_PARAM + i].setValue(1);
					params[SCALE2_PITCH_PROB_PARAM + i].setValue(1);
				}

				customScale1 = true;
				customScale2 = true;
			}

			scale1Pitches[i] =
				params[SCALE1_PITCH_PROB_PARAM + i].getValue() > 0;

			scale2Pitches[i] =
				params[SCALE2_PITCH_PROB_PARAM + i].getValue() > 0;

			scale1PitchesOnly[i] = scale1Pitches[i] &&
				scale1Pitches[i] ^ scale2Pitches[i];

			scale2PitchesOnly[i] = scale2Pitches[i] &&
				scale2Pitches[i] ^ scale1Pitches[i];

			scaleResPitches[i] = scale1Pitches[i] && scale2Pitches[i];

			probs1[i] = getProb(SCALE1_PITCH_PROB_PARAM + i,
				SCALE1_PITCH_PROB_INPUT + i);

			probs2[i] = getProb(SCALE2_PITCH_PROB_PARAM + i,
				SCALE2_PITCH_PROB_INPUT + i);

			probs2[i] = max(min(
				params[SCALE2_PITCH_PROB_PARAM + i].getValue() +
				inputs[SCALE2_PITCH_PROB_INPUT + i].getVoltage(), 1), 0
			);

			if (scale1PitchesOnly[i]) {
				probs1[i] *= getProb(SCALE1_PROB_PARAM, SCALE1_PROB_INPUT);
			}

			if (scale2PitchesOnly[i]) {
				probs2[i] *= getProb(SCALE2_PROB_PARAM, SCALE2_PROB_INPUT);
			}

			probs[i] = max(probs1[i], probs2[i]);

			lights[SCALE1_PITCH_LIGHT + i].setBrightness(probs1[i]);
			lights[SCALE2_PITCH_LIGHT + i].setBrightness(probs2[i]);
			lights[SCALE_RES_PITCH_LIGHT + i].setBrightness(probs[i]);
		}

		if (inputs[PITCH_INPUT].isConnected()) {
			int pitchChans = std::min(
				trigChans,
				inputs[PITCH_INPUT].getChannels()
			);

			outputs[PITCH_OUTPUT].setChannels(pitchChans);

			for (int i = 0; i < pitchChans; i++) {
				if (! triggers[i]) {
					outputs[PITCH_OUTPUT].setVoltage(prevPitches[i], i);
					return;
				}

				int pitch = normalize(inputs[PITCH_INPUT].getPolyVoltage(i));

				int diff = 10000000;
				int result = -1;
				int delta;
				int quantized;

				for (int j = 0; j < MAX_PITCHES; j++) {
					float r = randomness[i][j];

					if (r < probs[j]) {
						quantized = pitchesNorm[j];
						delta = std::abs(pitch - quantized);

						if (delta < diff) {
							result = quantized;
							diff = delta;
						}
					}
				}

				if (result == -1) {
					outputs[PITCH_OUTPUT].setVoltage(0, i);
					prevPitches[i] = 0;
				}

				float v = inputs[PITCH_INPUT].getPolyVoltage(i);
				float qv = float(result) / 10000;

				if (v < 0) {
					qv = std::trunc(v) - 1 + qv;
				} else {
					qv = std::trunc(v) + qv;
				}

				outputs[PITCH_OUTPUT].setVoltage(qv, i);
				prevPitches[i] = qv;
			}
		}
	}

	json_t* dataToJson() override {
		json_t* rootJson = json_object();

		json_object_set_new(rootJson, "customScale1",
			json_boolean(customScale1));

		json_object_set_new(rootJson, "customScale2",
			json_boolean(customScale2));

		return rootJson;
	}

	void dataFromJson(json_t* rootJson) override {
		initialized = true;

		customScale1 = json_boolean_value(json_object_get(rootJson,
			"customScale1"));

		customScale2 = json_boolean_value(json_object_get(rootJson,
			"customScale2"));

		currentScale1 = json_integer_value(json_array_get(
			json_object_get(rootJson, "params"), SCALE1_PARAM));

		currentScale2 = json_integer_value(json_array_get(
			json_object_get(rootJson, "params"), SCALE2_PARAM));

		prevScale1 = currentScale1;
		prevScale2 = currentScale2;

		currentKey1 = json_integer_value(json_array_get(
			json_object_get(rootJson, "params"), KEY1_PARAM));

		currentKey2 = json_integer_value(json_array_get(json_object_get(
			rootJson, "params"), KEY2_PARAM));

		prevKey1 = currentKey1;
		prevKey2 = currentKey2;
	}
};

struct ScaleMergerWidget : ModuleWidget {
	void addScaleControls(unsigned int x, unsigned int y,
	                      ScaleMerger::ParamsIds button,
	                      ScaleMerger::ParamsIds probParam,
	                      ScaleMerger::LightIds light,
	                      ScaleMerger::InputIds probInput) {

		for (int i = 0; i < MAX_PITCHES; i++) {
			unsigned int xx = x + i * 30;

			if (button != ScaleMerger::NUM_PARAMS) {
				addParam(createParam<LEDBezel>(
					Vec(xx + 1, y), module, button + i
				));
			}

			if (light != ScaleMerger::NUM_LIGHTS) {
				addChild(createLight<LEDBezelLight<GreenLight>>(
					Vec(xx + 3, y + 2), module, light + i
				));
			}

			if (probParam != ScaleMerger::NUM_PARAMS) {
				addParam(createParam<RoundSmallBlackKnob>(
					Vec(xx, y + 30), module, probParam + i
				));
			}

			if (probInput != ScaleMerger::NUM_INPUTS) {
				addInput(createInput<PJ301MPort>(
					Vec(xx, y + 60), module, probInput + i
				));
			}
		}

		for (int i = 0; i < MAX_SCALES; i++) {
			int res = 0;

			for (int j = 0; j < MAX_PITCHES; j++) {
				res += scales[i][j] ? 1 << j : 0;
			}
		}
	}

	void addScaleControlsMirrored(unsigned int x, unsigned int y,
	                              ScaleMerger::ParamsIds button,
	                              ScaleMerger::ParamsIds probParam,
	                              ScaleMerger::LightIds light,
	                              ScaleMerger::InputIds probInput) {

		for (int i = 0; i < MAX_PITCHES; i++) {
			unsigned int xx = x + i * 30;

			if (button != ScaleMerger::NUM_PARAMS) {
				addParam(createParam<LEDBezel>(
					Vec(xx + 1, y + 60), module, button + i
				));
			}

			if (light != ScaleMerger::NUM_LIGHTS) {
				addChild(createLight<LEDBezelLight<GreenLight>>(
					Vec(xx + 3, y + 62), module, light + i
				));
			}

			if (probParam != ScaleMerger::NUM_PARAMS) {
				addParam(createParam<RoundSmallBlackKnob>(
					Vec(xx, y + 30), module, probParam + i
				));
			}

			if (probInput != ScaleMerger::NUM_INPUTS) {
				addInput(createInput<PJ301MPort>(
					Vec(xx, y), module, probInput + i
				));
			}
		}
	}

	ScaleMergerWidget(ScaleMerger* module) {
		setModule(module);

		setPanel(APP->window->loadSvg(
			asset::plugin(pluginInstance, "res/ScaleMerger.svg")
		));

		struct ScaleDisplay : TransparentWidget {
			ScaleMerger::ScaleId scaleId;
			ScaleMerger *module;
			std::shared_ptr<Font> font;

			ScaleDisplay(std::shared_ptr<Font> font, ScaleMerger* module,
			             ScaleMerger::ScaleId scaleId) {
				this->font = font;
				this->scaleId = scaleId;
				this->module = module;
			}

			void draw(const DrawArgs &args) override {
				if (this->module == NULL) {
					return;
				}

				nvgFillColor(args.vg, nvgRGBA(0x00, 0x40, 0xa0, 0xff));
				nvgFontSize(args.vg, MAX_PITCHES);
				nvgFontFaceId(args.vg, font->handle);
				nvgTextLetterSpacing(args.vg, 0);
				nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
				nvgText(args.vg, 1, 1, module->getScaleName(scaleId), NULL);
			}
		};

		{
			std::shared_ptr<Font> fnt = APP->window->loadFont(
				asset::plugin(pluginInstance, "res/fonts/DejaVuSansMono.ttf")
			);

			ScaleDisplay *scaleDisplay1 =
				new ScaleDisplay(fnt, module, ScaleMerger::SCALE1);

			scaleDisplay1->box.pos = Vec(73, 42);
			scaleDisplay1->box.size = Vec(120, 10);
			addChild(scaleDisplay1);

			ScaleDisplay *scaleDisplay2 =
				new ScaleDisplay(fnt, module, ScaleMerger::SCALE2);

			scaleDisplay2->box.pos = Vec(73, 230);
			scaleDisplay2->box.size = Vec(120, 10);
			addChild(scaleDisplay2);
		}

		addScaleControls(
			145, 30,
			ScaleMerger::SCALE1_PITCH_SW_PARAM,
			ScaleMerger::SCALE1_PITCH_PROB_PARAM,
			ScaleMerger::SCALE1_PITCH_LIGHT,
			ScaleMerger::SCALE1_PITCH_PROB_INPUT
		);

		addScaleControlsMirrored(
			145, 150,
			ScaleMerger::SCALE2_PITCH_SW_PARAM,
			ScaleMerger::SCALE2_PITCH_PROB_PARAM,
			ScaleMerger::SCALE2_PITCH_LIGHT,
			ScaleMerger::SCALE2_PITCH_PROB_INPUT
		);

		addScaleControls(
			145, 267,
			ScaleMerger::SCALE_RES_PITCH_SW_PARAM,
			ScaleMerger::NUM_PARAMS,
			ScaleMerger::SCALE_RES_PITCH_LIGHT,
			ScaleMerger::NUM_INPUTS
		);

		addParam(createParam<RoundLargeBlackKnob>(
			Vec(26, 81), module, ScaleMerger::SCALE1_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(32, 51), module, ScaleMerger::SCALE1_INPUT
		));

		addParam(createParam<RoundLargeBlackKnob>(
			Vec(84, 81), module, ScaleMerger::KEY1_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(90, 51), module, ScaleMerger::KEY1_INPUT
		));

		addParam(createParam<RoundLargeBlackKnob>(
			Vec(26, 175), module, ScaleMerger::SCALE2_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(32, 145), module, ScaleMerger::SCALE2_INPUT
		));

		addParam(createParam<RoundLargeBlackKnob>(
			Vec(84, 175), module, ScaleMerger::KEY2_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(90, 145), module, ScaleMerger::KEY2_INPUT
		));

		addParam(createParam<RoundLargeBlackKnob>(
			Vec(531, 77), module, ScaleMerger::SCALE1_PROB_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(537, 47), module, ScaleMerger::SCALE1_PROB_INPUT
		));

		addParam(createParam<RoundLargeBlackKnob>(
			Vec(531, 148), module, ScaleMerger::SCALE2_PROB_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(537, 191), module, ScaleMerger::SCALE2_PROB_INPUT
		));

		addInput(createInput<PJ301MPort>(
			Vec(60, 265), module, ScaleMerger::PITCH_INPUT
		));

		addInput(createInput<PJ301MPort>(
			Vec(60, 317), module, ScaleMerger::TRIGGER_INPUT
		));

		addOutput(createOutput<PJ301MPort>(
			Vec(537, 265), module, ScaleMerger::PITCH_OUTPUT
		));
	}
};

Model* modelScaleMerger =
	createModel<ScaleMerger, ScaleMergerWidget>("ScaleMerger");
