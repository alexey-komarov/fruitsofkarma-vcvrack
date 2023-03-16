#include "plugin.hpp"

#define NEW_STATES 8
#define CUR_STATES NEW_STATES * 3
#define MAX_WIDTH 64
#define MAX_INIT 63
#define MAX_HEIGHT 256
#define MAX_CHANS 16

struct CellularAuto : Module {
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		FLIPH_INPUT,
		FLIPV_INPUT,
		ROTATE_INPUT,
		X_INPUT,
		Y_INPUT,
		W_INPUT,
		H_INPUT,
		INIT_INPUT,
		RULE_INPUT,
		ITER_INPUT,
		NUM_INPUTS
	};

	enum ParamsIds {
		X_PARAM,
		Y_PARAM,
		W_PARAM,
		H_PARAM,
		INIT_PARAM,
		RULE_PARAM,
		RESET_PARAM,
		ITER_PARAM,
		FLIPH_PARAM,
		FLIPV_PARAM,
		ROTATE_PARAM,
		NUM_PARAMS
	};

	enum OutputIds {
		EOS_OUTPUT,
		TRIGGER_OUTPUT,
		GATE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		INIT_LIGHT,
		FLIPV_LIGHT,
		FLIPH_LIGHT,
		RESET_LIGHT,
		NUM_LIGHTS
	};

	dsp::BooleanTrigger inStateTrigger[CUR_STATES];
	dsp::BooleanTrigger outStateTrigger[NEW_STATES];

	CellularAuto() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(X_PARAM, 0, MAX_WIDTH, 0, "X");
		configParam(Y_PARAM, 0, MAX_HEIGHT, 0, "Y");
		configParam(W_PARAM, 0, MAX_WIDTH, MAX_WIDTH, "Width");
		configParam(H_PARAM, 0, MAX_HEIGHT, MAX_HEIGHT, "Height");
		configParam(RULE_PARAM, 0, 255, 30, "Rule");
		configParam(ITER_PARAM, 0, 1000, 0, "Iterations");
		configParam(INIT_PARAM, 0, 1, 0, "Initialize");
		configParam(FLIPV_PARAM, 0, 1, 0, "Flip vertically");
		configParam(FLIPH_PARAM, 0, 1, 0, "Flip horizontally");
		configParam(RESET_PARAM, 0, 1, 0, "Reset");
	}

	int InitState[MAX_WIDTH] = {0};
	bool Reset = false;
	int Rule = 0;
	bool Field[MAX_HEIGHT][MAX_WIDTH] = {0};

	void generateCA() {
		int iters = params[ITER_PARAM].getValue();

		if (inputs[ITER_INPUT].isConnected()) {
			iters += inputs[ITER_INPUT].getVoltage() * 100;
		}

		iters = std::max(0, iters);

		bool curRow[MAX_WIDTH] = {0};
		bool newRow[MAX_WIDTH] = {0};

		for (int i = 0; i < MAX_WIDTH; i++) {
			curRow[i] = InitState[i] == 1;
		}

		int max_width = InitState[MAX_WIDTH - 1] == 2 ?
			MAX_WIDTH - 1 : MAX_WIDTH;

		bool buffer[MAX_HEIGHT][MAX_WIDTH] = {0};

		for (int i = 0; i <= iters; i++) {
			for (int j = 0; j < MAX_HEIGHT; j++) {
				for (int k = 0; k < max_width; k++) {
					const int left = curRow[(k - 1 + max_width) % max_width];
					const int center = curRow[k];
					const int right = curRow[(k + 1) % max_width];
					const int ruleIndex = (left << 2) + (center << 1) + right;
					const unsigned char byte = 1 << ruleIndex;
					newRow[k] = ((int)Rule & byte) == byte;
				}

				memcpy(buffer[j], newRow, sizeof(newRow));
				memcpy(curRow, newRow, sizeof(newRow));
			}
		}

		memcpy(Field, buffer, sizeof(buffer));
	}

	int FrameX = 0;
	int FrameY = 0;
	int FrameW = MAX_WIDTH;
	int FrameH = MAX_HEIGHT;
	int Positions[MAX_CHANS] = {0};

	dsp::BooleanTrigger tickTriggers[MAX_CHANS];
	dsp::SchmittTrigger initTrigger;
	dsp::SchmittTrigger flipVTrigger;
	dsp::SchmittTrigger flipHTrigger;
	dsp::SchmittTrigger resetTrigger;

	void onRandomize() override {
		for (int i = 0; i < MAX_WIDTH; i++) {
			InitState[i] = (random::uniform() > 0.5f);
		}
	}

	int ClockChans() {
		if (inputs[CLOCK_INPUT].isConnected()) {
			return inputs[CLOCK_INPUT].getChannels();
		}

		return 0;
	}

	bool sizeSet = false;
	int prevChannels = 0;

	void setSize() {
		int x = params[X_PARAM].getValue();
		int y = params[Y_PARAM].getValue();
		int w = params[W_PARAM].getValue();
		int h = params[H_PARAM].getValue();

		if (inputs[X_INPUT].isConnected()) {
			x += inputs[X_INPUT].getVoltage() * MAX_WIDTH / 10;
		}

		if (inputs[Y_INPUT].isConnected()) {
			y += inputs[Y_INPUT].getVoltage() * MAX_HEIGHT/ 10;
		}

		if (inputs[W_INPUT].isConnected()) {
			w += inputs[W_INPUT].getVoltage() * MAX_WIDTH / 10;
		}

		if (inputs[H_INPUT].isConnected()) {
			h += inputs[H_INPUT].getVoltage() * MAX_HEIGHT/ 10;
		}

		x = std::max(x, 0);
		x = std::min(x, MAX_WIDTH);

		y = std::max(y, 0);
		y = std::min(y, MAX_HEIGHT);

		w = std::max(0, w);
		h = std::max(0, h);

		if (w + x > MAX_WIDTH) {
			w = MAX_WIDTH - x;
		}

		if (h + y > MAX_HEIGHT) {
			h = MAX_HEIGHT - y;
		}

		FrameX = x;
		FrameY = y;
		FrameW = w;
		FrameH = h;

		sizeSet = true;
	}

	void resetSequencer() {
		if (!sizeSet) {
			setSize();
		}

		int chans = ClockChans();

		if (chans == 0) {
			return;
		}

		// Process channels
		int cells = (FrameW * FrameH) / chans;

		for (int c = 0; c < chans; c++) {
			outputs[GATE_OUTPUT].setVoltage(0.f, c);
			Positions[c] = c * cells;
		}
	}

	void onReset() override {
		params[RESET_PARAM].setValue(0.f);
		params[FLIPV_PARAM].setValue(0.f);
		params[FLIPH_PARAM].setValue(0.f);
		params[INIT_PARAM].setValue(0.f);
		resetSequencer();
	}

	bool initialized = false;

	void process(const ProcessArgs& args) override {
		if (! initialized && inputs[CLOCK_INPUT].isConnected()) {
			prevChannels = inputs[CLOCK_INPUT].getChannels();
			onReset();
			initialized = true;
		}

		int clockChans = ClockChans();

		if (clockChans != prevChannels) {
			resetSequencer();
			prevChannels = clockChans;
		}

		// Process rule param
		int rule = params[RULE_PARAM].getValue();

		if (inputs[RULE_INPUT].isConnected()) {
			rule += inputs[RULE_INPUT].getVoltage() * 25.6;
		}

		rule = std::max(0, rule);
		Rule = rule;

		// Process init param
		float initTrigVal = params[INIT_PARAM].getValue() +
			inputs[INIT_INPUT].getVoltage();

		if (initTrigger.process(initTrigVal) > 0.f) {
			generateCA();
		}

		lights[INIT_LIGHT].setSmoothBrightness(
			initTrigger.isHigh(), args.sampleTime
		);

		// Process flip params
		float flipHTrigVal = params[FLIPH_PARAM].getValue() +
			inputs[FLIPH_INPUT].getVoltage();

		if (flipHTrigger.process(flipHTrigVal) > 0.f) {
			bool buffer[MAX_HEIGHT][MAX_WIDTH] = {0};

			for (int y = 0; y < MAX_HEIGHT; y++) {
				for (int x = 0; x < MAX_WIDTH; x++) {
					buffer[y][MAX_WIDTH - x - 1] = Field[y][x];
				}
			}

			memcpy(Field, buffer, sizeof(buffer));
		}

		lights[FLIPH_LIGHT].setSmoothBrightness(
			flipHTrigger.isHigh(), args.sampleTime
		);

		float flipVTrigVal = params[FLIPV_PARAM].getValue() +
			inputs[FLIPV_INPUT].getVoltage();

		if (flipVTrigger.process(flipVTrigVal) > 0.f) {
			bool buffer[MAX_HEIGHT][MAX_WIDTH] = {0};

			for (int y = 0; y < MAX_HEIGHT; y++) {
				for (int x = 0; x < MAX_WIDTH; x++) {
					buffer[MAX_HEIGHT - y - 1][x] = Field[y][x];
				}
			}

			memcpy(Field, buffer, sizeof(buffer));
		}

		lights[FLIPV_LIGHT].setSmoothBrightness(
			flipVTrigger.isHigh(), args.sampleTime
		);

		setSize();

		if (FrameW == 0) {
			return;
		}

		int chans = ClockChans();
		outputs[TRIGGER_OUTPUT].setChannels(chans);
		outputs[GATE_OUTPUT].setChannels(chans);
		outputs[EOS_OUTPUT].setChannels(chans);

		if (chans == 0) {
			return;
		}

		// Process channels
		int cells = (FrameW * FrameH) / chans;

		// Process Reset
		float resetTrigVal = params[RESET_PARAM].getValue() +
			inputs[RESET_INPUT].getVoltage();

		if (resetTrigger.process(resetTrigVal) > 0.f) {
			onReset();
		}

		lights[RESET_LIGHT].setSmoothBrightness(
			resetTrigger.isHigh(), args.sampleTime
		);

		for (int c = 0; c < chans; c++) {
			float clockVal = inputs[CLOCK_INPUT].getPolyVoltage(c);

			if (tickTriggers[c].process(clockVal) > 0.f) {
				Positions[c]++;
				int start = c * cells;

				if (Positions[c] - start >= cells) {
					Positions[c] = start;
				}
			}

			int posY = Positions[c] / FrameW;
			int posX = Positions[c] % FrameW;

			// Gate
			float outVal = Field[posY + FrameY][posX + FrameX] ? 5.f : 0.0f;
			outputs[GATE_OUTPUT].setVoltage(outVal, c);

			// Trigger
			outVal = clockVal > 0.f && outVal > 0.f ? 5.f : 0.0f;

			outputs[TRIGGER_OUTPUT].setVoltage(outVal, c);

			outputs[EOS_OUTPUT].setVoltage(
				(Positions[c] == 0 && clockVal > 0.f) ? 5.f : 0.f, c
			);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJson = json_object();
		json_t* initState = json_array();
		json_t* state = json_array();

		for (int i = 0; i < MAX_WIDTH; i++) {
			json_array_append(initState, json_integer(InitState[i]));
		}

		for (int y = 0; y < MAX_HEIGHT; y++) {
			json_t* row = json_array();

			for (int x = 0; x < MAX_WIDTH; x++) {
				json_array_append(row, json_boolean(Field[y][x]));
			}

			json_array_append(state, row);
		}

		json_object_set_new(rootJson, "initialState", initState);
		json_object_set_new(rootJson, "state", state);

		return rootJson;
	}

	void dataFromJson(json_t* rootJson) override {
		json_t* initState = json_object_get(rootJson, "initialState");
		json_t* state = json_object_get(rootJson, "state");

		int size = json_array_size(initState);

		for (int i = 0; i < size && i < MAX_WIDTH; i++) {
			json_t* val = json_array_get(initState, i);
			InitState[i] = json_integer_value(val);
		}

		size = json_array_size(state);

		for (int y = 0; y < size && y < MAX_HEIGHT; y++) {
			json_t* row = json_array_get(state, y);
			int len = json_array_size(row);

			for (int x = 0; x < len && x < MAX_WIDTH; x++) {
				json_t* val = json_array_get(row, x);
				Field[y][x] = json_boolean_value(val);
			}
		}
	}
};

struct CellularAutoClearInitStateItem : MenuItem {
	CellularAuto* module;

	void onAction(const event::Action& e) override {
		if (module == NULL) {
			return;
		}

		for (int i = 0; i < MAX_WIDTH; i++) {
			module->InitState[i] = 0;
		}
	}
};

struct CellularAutoWidget : ModuleWidget {
	CellularAutoWidget(CellularAuto* module) {
		setModule(module);

		setPanel(APP->window->loadSvg(
			asset::plugin(pluginInstance, "res/CellularAuto.svg")
		));

		const int MAX_WIDTH2 = MAX_WIDTH << 1;
		const int displayWidth = 585;

		struct SeqDisplay : TransparentWidget {
			CellularAuto *module;
			const int xShift = 4;
			const int yShift = 2;
			const int initStateSize = 9;
			int sizeX = 0;
			int sizeY = 0;
			int stepX = 0;
			int stepY = 0;
			int offsetX = 0;
			int offsetY = 0;
			std::shared_ptr<Font> font;

			SeqDisplay(CellularAuto *instance) {
				this->module = instance;

				this->font = APP->window->loadFont(
					asset::plugin(pluginInstance,
					"res/fonts/DejaVuSansMono.ttf")
				);
			}

			void onButton(const event::Button &e) override {
				if (e.action == GLFW_PRESS &&
						e.button == GLFW_MOUSE_BUTTON_LEFT) {
					int x = e.pos.x - xShift;

					if (x < 0) {
						return;
					}

					int y = e.pos.y - yShift;

					if (y < 0) {
						return;
					}

					int xc = x / initStateSize;
					int yc = y / initStateSize;

					if (xc > MAX_WIDTH) {
						return;
					}

					if (yc == 0) {
						if (xc == (MAX_WIDTH - 1)) {
							if (module->InitState[xc] == 0) {
								module->InitState[xc] = 1;
							} else if (module->InitState[xc] == 1) {
								module->InitState[xc] = 2;
							} else {
								module->InitState[xc] = 0;
							}
						} else if (module->InitState[xc] == 0) {
							module->InitState[xc] = 1;
						} else {
							module->InitState[xc] = 0;
						}
					} else if (sizeX > 0 && sizeY > 0) {
						int xs = (x - offsetX - MAX_WIDTH2) / stepX;
						int ys = (y - offsetY) / stepY;
						int ix = xs + module->FrameX;
						int iy = ys + module->FrameY;
						 
						module->Field[iy][ix] ^= true;
					}
				}
			}

			void draw(const DrawArgs &args) override {
				if (this->module == NULL) {
					return;
				}

				// Background
				nvgBeginPath(args.vg);
				nvgRect(args.vg, 0, 0, displayWidth, 280);
				nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 255));
				nvgFill(args.vg);

				int rule = module->Rule;

				// Display rule
				for (int s = 0; s <= 7; s++) {
					for (int b = 0; b < 3; b++) {
						nvgBeginPath(args.vg);
						bool bit = (s & (1 << b)) == (1 << b);
						nvgStrokeColor(args.vg, nvgRGBA(64, 200, 255, 128));

						if (bit) {
							nvgFillColor(args.vg, nvgRGBA(64, 200, 255, 128));
						}

						nvgRect(
							args.vg,
							541 + b * 13,
							14 + s * 35,
							11,
							11
						);

						if (bit) {
							nvgFill(args.vg);
						}

						nvgStroke(args.vg);
					}

					nvgBeginPath(args.vg);
					bool bit = (rule & (1 << s)) == (1 << s);

					if (bit) {
						nvgFillColor(args.vg, nvgRGBA(0, 255, 0, 170));
					} else {
						nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 255));
					}

					nvgRect(
						args.vg,
						541,
						27 + s * 35,
						37,
						5
					);

					nvgFill(args.vg);
				}

				// Initial State
				for (int i = 0; i < MAX_WIDTH; i++) {
					nvgBeginPath(args.vg);
					nvgRect(args.vg, 4 + (i * 9), 2, 7, 7);

					if (module->InitState[i] == 1) {
						nvgFillColor(args.vg, nvgRGBA(255, 255, 0, 120));
						nvgFill(args.vg);
					} else if (module->InitState[i] == 2) {
						nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));
						nvgFill(args.vg);
					}

					nvgStrokeColor(args.vg, nvgRGBA(255, 255, 0, 150));
					nvgStrokeWidth(args.vg, 1);
					nvgStroke(args.vg);
				}

				nvgBeginPath(args.vg);
				nvgStrokeColor(args.vg, nvgRGBA(0, 255, 0, 150));
				nvgStrokeWidth(args.vg, 1);

				nvgRect(
					args.vg,
					4,
					12 + yShift,
					MAX_WIDTH2 + 7,
					MAX_HEIGHT + 6
				);

				nvgRect(
					args.vg,
					13 + MAX_WIDTH2,
					12 + yShift,
					MAX_WIDTH * 6 + 7,
					MAX_HEIGHT + 6
				);

				nvgStroke(args.vg);

				// Overview
				for (int x = 0; x < MAX_WIDTH; x++) {
					for (int y = 0; y < MAX_HEIGHT; y++) {
						nvgBeginPath(args.vg);

						if (module->Field[y][x]) {
							nvgFillColor(args.vg, nvgRGBA(128, 255, 64, 255));
						} else {
							nvgFillColor(args.vg, nvgRGBA(0, 0, 128, 255));
						}

						nvgRect(
							args.vg,
							4 + xShift + x * 2,
							15 + yShift + y,
							1,
							1
						);

						nvgFill(args.vg);
					}
				}

				int fw = module->FrameW;
				int fh = module->FrameH;
				int fx = module->FrameX;
				int fy = module->FrameY;

				if (fw == 0 || fh == 0) {
					return;
				}

				// Zoom frame borders
				nvgBeginPath(args.vg);
				nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 200));

				nvgRect(
					args.vg,
					4 + xShift + (fx << 1),
					15 + yShift + fy,
					fw << 1,
					fh
				);

				nvgStroke(args.vg);

				sizeX = std::floor(MAX_WIDTH / fw) * 5;
				sizeY = std::floor(MAX_HEIGHT / fh);

				stepX = sizeX == 1 ? 1 : sizeX + 1;
				stepY = sizeY;

				if (sizeY > 3) {
					sizeY--;
				}

				offsetX = (((MAX_WIDTH * 6) - (fw * stepX)) >> 1) + 14;
				offsetY = ((MAX_HEIGHT - (fh * stepY)) >> 1) + 15;

				// Zoom
				for (int x = 0; x < fw; x++) {
					for (int y = 0; y < fh; y++) {
						nvgBeginPath(args.vg);

						if (module->Field[y + fy][x + fx]) {
							nvgFillColor(args.vg, nvgRGBA(64, 192, 64, 255));
						} else {
							nvgFillColor(args.vg, nvgRGBA(0, 0, 128, 128));
						}

						nvgRect(
							args.vg,
							offsetX + xShift + MAX_WIDTH2 + x * stepX,
							offsetY + yShift + y * stepY,
							sizeX,
							sizeY
						);

						nvgFill(args.vg);
					}
				}

				// Cursors
				int clockChans = module->ClockChans();

				char buf[3] = {0};

				for (int c = 0; c < clockChans; c++) {
					int x = module->Positions[c] % fw;
					int y = module->Positions[c] / fw;

					nvgBeginPath(args.vg);
					nvgFillColor(args.vg, nvgRGBA(255, 128, 64, 255));

					int cx = offsetX + xShift + MAX_WIDTH2 + x * stepX;
					int cy = offsetY + yShift + y * stepY;

					nvgRect(args.vg, cx, cy, sizeX, sizeY);

					nvgFill(args.vg);
					nvgStrokeWidth(args.vg, 0.5f);
					nvgStroke(args.vg);

					if (clockChans > 1) {
						sprintf(buf, "%d", c);
						int fontSize = std::min(sizeY < 7 ? 7 : sizeY, 20);

						nvgFillColor(args.vg, nvgRGBA(255, 255, 255, 255));
						nvgFontSize(args.vg, fontSize);
						nvgFontFaceId(args.vg, font->handle);
						nvgTextLetterSpacing(args.vg, 0);
						nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
						nvgText(args.vg, cx + 1, cy + stepY - 2, buf, NULL);
					}
				}
			}
		};

		{
			SeqDisplay *seqDisplay = new SeqDisplay(module);

			seqDisplay->box.pos = Vec(7, 5);
			seqDisplay->box.size = Vec(displayWidth, MAX_HEIGHT + 30);
			addChild(seqDisplay);
		}

		addInput(createInput<PJ301MPort>(
			Vec(20, 328), module, CellularAuto::CLOCK_INPUT
		));

		addParam(createParam<LEDBezel>(
			Vec(61, 300), module, CellularAuto::RESET_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(60, 328), module, CellularAuto::RESET_INPUT
		));

		addChild(createLight<LargeLight<GreenLight>>(
			Vec(64.3, 303.5), module, CellularAuto::RESET_LIGHT)
		);

		addParam(createParam<LEDBezel>(
			Vec(101, 300), module, CellularAuto::FLIPH_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(100, 328), module, CellularAuto::FLIPH_INPUT
		));

		addChild(createLight<LargeLight<BlueLight>>(
			Vec(104.3, 303.5), module, CellularAuto::FLIPH_LIGHT)
		);

		addParam(createParam<LEDBezel>(
			Vec(141, 300), module, CellularAuto::FLIPV_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(140, 328), module, CellularAuto::FLIPV_INPUT
		));

		addChild(createLight<LargeLight<BlueLight>>(
			Vec(144.3, 303.5), module, CellularAuto::FLIPV_LIGHT)
		);

		addInput(createInput<PJ301MPort>(
			Vec(180, 328), module, CellularAuto::INIT_INPUT
		));

		addParam(createParam<LEDBezel>(
			Vec(181, 300), module, CellularAuto::INIT_PARAM
		));

		addChild(createLight<LargeLight<BlueLight>>(
			Vec(184.3, 303.5), module, CellularAuto::INIT_LIGHT)
		);

		addParam(createParam<RoundSmallBlackKnob>(
			Vec(220, 300), module, CellularAuto::RULE_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(220, 328), module, CellularAuto::RULE_INPUT
		));

		addParam(createParam<RoundSmallBlackKnob>(
			Vec(260, 300), module, CellularAuto::ITER_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(260, 328), module, CellularAuto::ITER_INPUT
		));

		addParam(createParam<RoundSmallBlackKnob>(
			Vec(300, 300), module, CellularAuto::X_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(300, 328), module, CellularAuto::X_INPUT
		));

		addParam(createParam<RoundSmallBlackKnob>(
			Vec(340, 300), module, CellularAuto::Y_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(340, 328), module, CellularAuto::Y_INPUT
		));

		addParam(createParam<RoundSmallBlackKnob>(
			Vec(380, 300), module, CellularAuto::W_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(380, 328), module, CellularAuto::W_INPUT
		));

		addParam(createParam<RoundSmallBlackKnob>(
			Vec(420, 300), module, CellularAuto::H_PARAM
		));

		addInput(createInput<PJ301MPort>(
			Vec(420, 328), module, CellularAuto::H_INPUT
		));

		addOutput(createOutput<PJ301MPort>(
			Vec(480, 328), module, CellularAuto::EOS_OUTPUT
		));

		addOutput(createOutput<PJ301MPort>(
			Vec(520, 328), module, CellularAuto::TRIGGER_OUTPUT
		));

		addOutput(createOutput<PJ301MPort>(
			Vec(560, 328), module, CellularAuto::GATE_OUTPUT
		));
	}

	void appendContextMenu(Menu* menu) override {
		CellularAuto* module = dynamic_cast<CellularAuto*>(this->module);

		CellularAutoClearInitStateItem* clearItem  =
			new CellularAutoClearInitStateItem;

		clearItem->text = "Clear Initial State";
		clearItem->module = module;
		menu->addChild(clearItem);
	}
};

Model* modelCellularAuto =
	createModel<CellularAuto, CellularAutoWidget>("CellularAuto");
