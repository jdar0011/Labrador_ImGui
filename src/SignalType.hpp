#include "librador.h"
#include "implot.h"
#include "util.h"
#include "UIComponents.hpp"
#include <string>

/// <summary>
/// Abstract class representing signal from signal generator
/// </summary>
class GenericSignal
{
public:
	GenericSignal(const std::string& label, float* preview)
	    : label(label)
	    , preview(preview)
	    , amplitude("##" + label + "amp", "Vpeak-peak", 1.0f, 0.15f, 9.0f, "V",
	          constants::volt_prefs, constants::volt_formats)
	    , frequency("##" + label + "freq", "Frequency", 100.f, 1.0f, 1e6f, "Hz",
	          constants::freq_prefs, constants::freq_formats)
	    , offset("##" + label + "os", "Vbase", 0.0f, -9.0f, 9.0f, "V",
	          constants::volt_prefs, constants::volt_formats)
	{
	}

	std::string getLabel() const
	{
		return label;
	}

	/// <summary>
	/// Generic UI elements for Signal Control
	/// </summary>
	bool renderControl()
	{
		const float width = ImGui::GetContentRegionAvail().x;
		const float height = ImGui::GetFrameHeightWithSpacing() * (label=="Square" ? 4.0f : 3.0f);

		// Controls
		ImGui::BeginChild((label + "_control").c_str(), ImVec2(width * 0.6, height));

		bool changed = renderProperties();

		ImGui::EndChild();

		// Preview
		ImGui::SameLine();
		ImPlotStyle backup = ImPlot::GetStyle();
		PreviewStyle();
		if (ImPlot::BeginPlot((label + "_preview").c_str(), ImVec2(width * 0.35, height),
		        ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs))
		{
			renderPreview();
		}

		ImPlot::GetStyle() = backup;
		return changed;
	}

	/// <summary>
	/// Render preview of signal
	/// </summary>
	void renderPreview()
	{
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations,
		    ImPlotAxisFlags_NoDecorations);
		int period = constants::PREVIEW_RES;
		int padding = period / 4;
		float plt_amp = 1.0f;
		ImPlot::SetupAxesLimits(-5 - padding, period + padding + 5, 1.2, -1.2, ImGuiCond_Always);

		// Plot half a waveform before and after preview 
		ImPlot::PlotLine(("##" + label + "_plot_preview_pre").c_str(),&preview[period-padding], padding+1, 1.0, (double) - padding);
		ImPlot::PlotLine(("##" + label + "_plot_preview").c_str(), constants::x_preview,
		    preview, period + 1);
		ImPlot::PlotLine(("##" + label + "_plot_preview_post").c_str(),
			preview, padding, 1.0, period);

		// Render annotations
		float amp_label_x[2] = { 0, 0 };
		float amp_label_y[2] = { plt_amp, -plt_amp };

		ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1, 1, 1, 1));
		ImPlot::PlotLine(("##" + label + "_amp").c_str(), amp_label_x, amp_label_y, 2);
		ImPlot::PlotScatter(("##" + label + "_amp_pnt").c_str(), amp_label_x, amp_label_y, 2);
		// Annotate amplitude
		ImPlot::Annotation(period*0.5, 0, ImVec4(0, 0, 0, 0), ImVec2(0, -5), true,
			"Vpp = %.2f V", amplitude.getValue());

		float per_label_x[2] = { 0, period };
		float per_label_y[2] = { 0.0f, 0.0f };

		ImPlot::PlotLine(("##" + label + "_per").c_str(), per_label_x, per_label_y, 2);
		ImPlot::PlotScatter(("##" + label + "_per_pnt").c_str(), per_label_x, per_label_y, 2);
		// Annotate frequency
		ImPlot::Annotation(period*0.5, 0, ImVec4(0, 0, 0, 0), ImVec2(0, 5), true,
			"T = %.2E s", 1 / frequency.getValue());
		ImPlot::PopStyleColor();

		ImPlot::EndPlot();
	}

	/// <summary>
	/// Render Control Widgets
	/// </summary>
	/// <returns></returns>
	virtual bool renderProperties()
	{
		bool changed = false;

		const float width = ImGui::GetContentRegionAvail().x * 0.9;
		const float labWidth = 80.0f;
		const float unitWidth = 50.0f;
		const float inpWidth = width - labWidth - unitWidth;

		if (ImGui::BeginTable((label + "_prop_table").c_str(), 2))
		{
			ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_WidthFixed, labWidth);
			ImGui::TableSetupColumn("Two", ImGuiTableColumnFlags_WidthFixed, inpWidth + unitWidth);

			changed |= amplitude.renderInTable(inpWidth);
			changed |= frequency.renderInTable(inpWidth);
			changed |= offset.renderInTable(inpWidth);

			if (label == "Square")
			{
				ImGui::TableNextColumn();
				ImGui::Text("Duty Cycle");
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(inpWidth);
				if (ImGui::DragInt(("##dc_control_" + label).c_str(), &dutycycle, 1, 1,
					100, "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					changed = true;
					int period = constants::PREVIEW_RES;
					for (int i = 0; i < constants::PREVIEW_RES; i++)
					{
						preview[i] = (float)i / period < dutycycle * 0.01 ? 1.0 : -1.0;
					}
					preview[period] = preview[0];
				}
				ImGui::SameLine();
				ImGui::Text("%%");
				
			}

			ImGui::EndTable();
		}
		return changed;
	}

	/// <summary>
	/// Set the Signal Generator on the labrador board.
	/// </summary>
	virtual void controlLab(int channel) = 0;

	/// <summary>
	/// Set signal generator amplitude to zero
	/// </summary>
	/// <param name="channel"></param>
	virtual void turnOff(int channel)
	{
		librador_send_sin_wave(channel, 100, 0.0, 0.0);
	}

	float getSignalMax()
	{
		return amplitude.getValue() + offset.getValue();
	}

private:
	

protected:
	std::string label;
	float* preview;
	SIValue amplitude;
	SIValue frequency;
	SIValue offset;
	int dutycycle = 50;
};


/// <summary>
/// Sine Signal Generator Widget
/// </summary>
class SineSignal : public GenericSignal
{
public:
	
	SineSignal(const std::string& label)
	    : GenericSignal(label, constants::sine_preview)
	{}

	/// <summary>
	/// Set the Signal Generator on the labrador board.
	/// </summary>
	void controlLab(int channel) override
	{
		librador_send_sin_wave(channel, frequency.getValue(), amplitude.getValue(), offset.getValue());
	}
	
};

/// <summary>
/// Square Signal Generator Widget
/// </summary>
class SquareSignal : public GenericSignal
{
public:
	SquareSignal(const std::string& label)
	    : GenericSignal(label, constants::square_preview)
	{
	}

	/// <summary>
	/// Set the Signal Generator on the labrador board.
	/// </summary>
	void controlLab(int channel) override
	{
		librador_imgui_send_square_wave(
		    channel, frequency.getValue(), amplitude.getValue(), offset.getValue(), dutycycle / 100.0);
		// librador_send_square_wave(channel, getSIFrequency(), getSIAmp(), getSIOffset());
	}


	// Could be integrated with librador
	int librador_imgui_send_square_wave(int channel, double frequency_Hz, double amplitude_v, double offset_v, double duty_cycle = 0.5)
	{
		if ((amplitude_v + offset_v) > 9.6)
		{
			return -1;
			// Voltage range too high
		}
		if ((amplitude_v < 0) || (offset_v < 0))
		{
			return -2;
			// Negative voltage
		}

		if ((channel != 1) && (channel != 2))
		{
			return -3;
			// Invalid channel
		}
		int num_samples = fmin(1000000.0 / frequency_Hz, 512);
		// The maximum number of samples that Labrador's buffer holds is 512.
		// The minimum time between samples is 1us.  Using T=1/f, this gives a maximum
		// sample number of 10^6/f.
		num_samples = 2 * (num_samples / 2);
		// Square waves need an even number.  Others don't care.
		double usecs_between_samples = 1000000.0 / ((double)num_samples * frequency_Hz);
		// Again, from T=1/f.
		unsigned char* sampleBuffer = (unsigned char*)malloc(num_samples);

		int i;
		double x_temp;
		for (i = 0; i < num_samples; i++)
		{
			x_temp = (double)i * (2.0 * M_PI / (double)num_samples);
			// Generate points at interval 2*pi/num_samples.
			sampleBuffer[i] = sample_generator(x_temp, duty_cycle);
		}

		librador_update_signal_gen_settings(channel, sampleBuffer, num_samples,
			usecs_between_samples, amplitude_v, offset_v);

		free(sampleBuffer);
		return 0;
	}

	unsigned char sample_generator(double x, double duty_cycle = 0.5)
	{
		return (x < 2*M_PI*duty_cycle) ? 255 : 0;
	}
};

/// <summary>
/// Sawtooth Signal Generator Widget
/// </summary>
class SawtoothSignal : public GenericSignal
{
public:
	SawtoothSignal(const std::string& label)
	    : GenericSignal(label, constants::sawtooth_preview)
	{
	}

	/// <summary>
	/// Set the Signal Generator on the labrador board.
	/// </summary>
	void controlLab(int channel) override
	{
		librador_send_sawtooth_wave(
		    channel, frequency.getValue(), amplitude.getValue(), offset.getValue());
	}
};

/// <summary>Triangle Signal Generator Widget
/// </summary>
class TriangleSignal : public GenericSignal
{
public:
	TriangleSignal(const std::string& label)
	    : GenericSignal(label, constants::triangle_preview)
	{
	}

	/// <summary>
	/// Set the Signal Generator on the labrador board.
	/// </summary>
	void controlLab(int channel) override
	{
		librador_send_triangle_wave(channel, frequency.getValue(), amplitude.getValue(), offset.getValue());
	}
};