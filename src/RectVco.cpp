#include "plugin.hpp"

struct RectVCO : Module
{
	template <class TParamQuantity = ParamQuantity>
	TParamQuantity* configQuant(int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f) {
		assert(paramId < (int) params.size() && paramId < (int) paramQuantities.size());
		if (paramQuantities[paramId])
			delete paramQuantities[paramId];

		TParamQuantity* q = new TParamQuantity;
		q->ParamQuantity::module = this;
		q->ParamQuantity::paramId = paramId;
		q->ParamQuantity::minValue = minValue;
		q->ParamQuantity::maxValue = maxValue;
		q->ParamQuantity::defaultValue = defaultValue;
		q->ParamQuantity::name = name;
		q->ParamQuantity::unit = unit;
		q->ParamQuantity::displayBase = displayBase;
		q->ParamQuantity::displayMultiplier = displayMultiplier;
		q->ParamQuantity::displayOffset = displayOffset;
		q->ParamQuantity::snapEnabled = true;
		paramQuantities[paramId] = q;

		Param* p = &params[paramId];
		p->value = q->getDefaultValue();
		return q;
	}

	float phase = 0.0f;

	enum ParamId
	{
		PW_PARAM,
		INTERVAL_PARAM,
		MIX_PARAM,
		PW_CV_PARAM,
		MIX_CV_PARAM,
		PARAMS_LEN
	};
	enum InputId
	{
		PITCH_INPUT,
		PW_CV_INPUT,
		MIX_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId
	{
		RECT_OUTPUT,
		SAW_OUTPUT,
		MIX_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId
	{
		LIGHTS_LEN
	};

	RectVCO()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PW_PARAM, 0.01f, 0.99f, 0.5f, "Pulse width", " %", 0.f, 100.0f);
		configInput(PW_CV_INPUT, "Pulse width CV");
		configParam(PW_CV_PARAM, -0.5f, 0.5f, 0.0f, "Pulse width CV amount", " %", 0.f, 200.0f);

		configParam(MIX_PARAM, 0.0f, 1.0f, 0.5f, "OSC Mix", " %", 0.f, 100.0f);
		configInput(MIX_CV_INPUT, "Mix CV");
		configParam(MIX_CV_PARAM, -1.0f, 1.0f, 0.0f, "Mix CV amount", " %", 0.f, 100.0f);

		configQuant(INTERVAL_PARAM, -12.0f, 12.0f, 0.0f, "Interval", " HT", 0.0f, 1.0f);
		configInput(PITCH_INPUT, "");
		configOutput(RECT_OUTPUT, "");
	}

	float saw(float phase, float pw = 0.0f) {
		if (phase >= pw) {
			return (phase - pw) / (0.5f - pw) - 1.0f;
		} else {
			return (phase + 0.5f) / (pw + 0.5f);
		}
	}

	float rect(float phase, float pw = 0.0f) {
		return (phase < pw) * 2.0f + -1.0f;
	}

	float mixLin(float a, float b, float mix = 0.5f) {
		return a * (1.0f - mix) + b * mix;
	}

	void process(const ProcessArgs &args) override
	{
		// PITCH
		float pitch = inputs[PITCH_INPUT].getVoltage();
		float freq = dsp::FREQ_C4 * std::pow(2.0f, pitch) * std::pow(dsp::FREQ_SEMITONE, params[INTERVAL_PARAM].getValue());
		phase += freq * args.sampleTime;
		if (phase >= 0.5f)
			phase -= 1.0f;

		// PULSE WIDTH
		float pw = params[PW_PARAM].getValue() - 0.5f;
		float pwCV = inputs[PW_CV_INPUT].getVoltage() / 5.0f;
		pwCV *= params[PW_CV_PARAM].getValue();
		pw = clamp(pw + pwCV, -1.0f, 1.0f);

		// OSC MIX
		float mix = params[MIX_PARAM].getValue();

		mix = clamp(mix, 0.0f, 1.0f);

		float sampRect = rect(phase, pw);
		float sampSaw = saw(phase, pw);

		outputs[RECT_OUTPUT].setVoltage(5.0f * sampRect);
		outputs[SAW_OUTPUT].setVoltage(5.0f * sampSaw);
		outputs[MIX_OUTPUT].setVoltage(5.0f * mixLin(sampRect, sampSaw, mix));
	}
};

struct RectVCOWidget : ModuleWidget
{
	RectVCOWidget(RectVCO *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RectVCO.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.0, 20.0)), module, RectVCO::MIX_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.0, 15.0)), module, RectVCO::MIX_CV_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 25.0)), module, RectVCO::MIX_CV_INPUT));


		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(19.0, 45.0)), module, RectVCO::PW_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.0, 40.0)), module, RectVCO::PW_CV_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 50.0)), module, RectVCO::PW_CV_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.7, 70.0)), module, RectVCO::INTERVAL_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.7, 80.0)), module, RectVCO::PITCH_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 100.0)), module, RectVCO::RECT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 110.0)), module, RectVCO::SAW_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(12.7, 120.0)), module, RectVCO::MIX_OUTPUT));
	}
};

Model *modelRectVCO = createModel<RectVCO, RectVCOWidget>("RectVCO");