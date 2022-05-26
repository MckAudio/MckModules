#include "plugin.hpp"


struct RectVCO : Module {
	float phase = 0.0f;

	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		PITCH_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		RECT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	RectVCO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(PITCH_INPUT, "");
		configOutput(RECT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		float pitch = inputs[PITCH_INPUT].getVoltage();
		float freq = dsp::FREQ_C4 * std::pow(2.0f, pitch);

		phase += freq * args.sampleTime;
		if (phase >= 0.5f) {
			phase -= 1.0f;
		}

		float rect = phase >= 0.0f ? 1.0f : -1.0f;

		outputs[RECT_OUTPUT].setVoltage(5.0f * rect);
	}
};


struct RectVCOWidget : ModuleWidget {
	RectVCOWidget(RectVCO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RectVCO.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 77.5)), module, RectVCO::PITCH_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 102.5)), module, RectVCO::RECT_OUTPUT));
	}
};


Model* modelRectVCO = createModel<RectVCO, RectVCOWidget>("RectVCO");