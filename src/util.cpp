#include "util.h"
#include <iostream>
#include <sstream>
#include <iomanip>


float constants::x_preview[constants::PREVIEW_RES+1];
float constants::sine_preview[constants::PREVIEW_RES+1];
float constants::square_preview[constants::PREVIEW_RES+1];
float constants::sawtooth_preview[constants::PREVIEW_RES+1];
float constants::triangle_preview[constants::PREVIEW_RES+1];



/// <summary>
/// Initialise global preview arrays for Signal Generator control
/// </summary>
void init_constants()
{
	const int pr = constants::PREVIEW_RES;
	for (int i = 0; i < pr; i++)
	{
		constants::x_preview[i] = i * 1.0f;
		constants::sine_preview[i] = sinf(i * (1.0f / pr) * 2 * M_PI);
		constants::sawtooth_preview[i] = -1.0f + 2.0f / pr * i;
		if (i < pr / 2)
		{
			constants::square_preview[i] = -1.0f;
			constants::triangle_preview[i] = -1.0f + 4.0f / pr * i;
		}
		else
		{
			constants::square_preview[i] = 1.0f;
			constants::triangle_preview[i] = constants::triangle_preview[pr - i - 1];
		}
	}

	// Wraparound for continuous plot
	constants::x_preview[pr] = pr * 1.0f;
	constants::sine_preview[pr] = constants::sine_preview[0];
	constants::square_preview[pr] = constants::square_preview[0];
	constants::triangle_preview[pr] = constants::triangle_preview[0];
	constants::sawtooth_preview[pr] = constants::sawtooth_preview[0];
}

/// <summary>
/// Replaces all instances of a substring with a provided string
/// </summary>
/// <param name="s"></param>
/// <param name="toReplace"></param>
/// <param name="replaceWith"></param>
void replace_all(std::string& s, std::string const& toReplace, std::string const& replaceWith)
{
	std::string buf;
	std::size_t pos = 0;
	std::size_t prevPos;

	// Reserves rough estimate of final size of string.
	buf.reserve(s.size());

	while (true)
	{
		prevPos = pos;
		pos = s.find(toReplace, pos);
		if (pos == std::string::npos)
			break;
		buf.append(s, prevPos, pos - prevPos);
		buf += replaceWith;
		pos += toReplace.size();
	}

	buf.append(s, prevPos, s.size() - prevPos);
	s.swap(buf);
}

/*
STYLES
*/

/// <summary>
/// Global styles set at the start of the application
/// </summary>
void SetGlobalStyle()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.62f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.16f, 0.80f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.78f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.59f);
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.00f);
}

/// <summary>
/// Defines style for the Signal Generator Preview plot
/// </summary>
void PreviewStyle()
{
	ImPlotStyle& style = ImPlot::GetStyle();

	ImVec4* colors = style.Colors;
	colors[ImPlotCol_Line] = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // WHITE
	colors[ImPlotCol_Fill] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f); // TRANSPARENT
	//colors[ImPlotCol_MarkerOutline] = IMPLOT_AUTO_COL;
	//colors[ImPlotCol_MarkerFill] = IMPLOT_AUTO_COL;
	//colors[ImPlotCol_ErrorBar] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImPlotCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImPlotCol_PlotBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f); // TRANSPARENT
	colors[ImPlotCol_PlotBorder] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	//colors[ImPlotCol_LegendBg] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
	//colors[ImPlotCol_LegendBorder] = ImVec4(0.80f, 0.81f, 0.85f, 1.00f);
	//colors[ImPlotCol_LegendText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	//colors[ImPlotCol_TitleText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	//colors[ImPlotCol_InlayText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	//colors[ImPlotCol_AxisText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	//colors[ImPlotCol_AxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	//colors[ImPlotCol_AxisBgHovered] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
	//colors[ImPlotCol_AxisBgActive] = ImVec4(0.92f, 0.92f, 0.95f, 0.75f);
	//colors[ImPlotCol_Selection] = ImVec4(1.00f, 0.65f, 0.00f, 1.00f);
	//colors[ImPlotCol_Crosshairs] = ImVec4(0.23f, 0.10f, 0.64f, 0.50f);

	//style.LineWeight = 1.5;
	// style.Marker = ImPlotMarker_Plus;
	style.MarkerSize = 2;
	//style.MarkerWeight = 1;
	//style.FillAlpha = 1.0f;
	//style.ErrorBarSize = 5;
	//style.ErrorBarWeight = 1.5f;
	//style.DigitalBitHeight = 8;
	//style.DigitalBitGap = 4;
	//style.PlotBorderSize = 0;
	//style.MinorAlpha = 1.0f;
	//style.MajorTickLen = ImVec2(0, 0);
	//style.MinorTickLen = ImVec2(0, 0);
	//style.MajorTickSize = ImVec2(0, 0);
	//style.MinorTickSize = ImVec2(0, 0);
	//style.MajorGridSize = ImVec2(1.2f, 1.2f);
	//style.MinorGridSize = ImVec2(1.2f, 1.2f);
	style.PlotPadding = ImVec2(0, 0);
	//style.LabelPadding = ImVec2(5, 5);
	//style.LegendPadding = ImVec2(5, 5);
	//style.MousePosPadding = ImVec2(5, 5);
	//style.PlotMinSize = ImVec2(300, 225);
}

/// <summary>
/// Defines the style of the general control widget
/// </summary>
/// <param name="ac">Accent colour (RGB 0..1) </param>
void SetControlWidgetStyle(const float ac[3])
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_FrameBg] = ImVec4(ac[0], ac[1], ac[2], 0.5f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(ac[0], ac[1], ac[2], 0.65f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(ac[0], ac[1], ac[2], 1.0f);
	colors[ImGuiCol_Button] = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_HeaderHovered] = ImVec4(ac[0], ac[1], ac[2], 0.65f);
	colors[ImGuiCol_HeaderActive] = ImVec4(ac[0], ac[1], ac[2], 1.0f);
}

ImU32 colourConvert(const float c[3], float alpha)
{
	return ImGui::ColorConvertFloat4ToU32(ImVec4(c[0], c[1], c[2], alpha));
}

void MultiplyButtonColour(ImU32* ButtonColour, float multiplier)
{
	ImColor im_ButtonColour = ImColor(*ButtonColour);

	im_ButtonColour.Value.x = multiplier * im_ButtonColour.Value.x > 1 ? 1 : multiplier * im_ButtonColour.Value.x;
	im_ButtonColour.Value.y = multiplier * im_ButtonColour.Value.y > 1 ? 1 : multiplier * im_ButtonColour.Value.y;
	im_ButtonColour.Value.z = multiplier * im_ButtonColour.Value.z > 1 ? 1 : multiplier * im_ButtonColour.Value.z;


	*ButtonColour = ImU32(im_ButtonColour);
}

std::string NumToString(double num, int precision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << num;
	std::string str = stream.str();
	return str;
}
int MetricFormatter(double value, char* buff, int size, void* data) {
	const char* unit = (const char*)data;
	static double v[] = { 1000000000,1000000,1000,1,0.001,0.000001,0.000000001 };
	static const char* p[] = { "G","M","k","","m",u8"\u03bc","n" };
	if (value == 0) {
		return snprintf(buff, size, "0 %s", unit);
	}
	for (int i = 0; i < 7; ++i) {
		if (fabs(value) >= v[i]) {
			return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
		}
	}
	return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
}
void ToggleTriggerTypeComboChannel(int* ComboCurrentItem)
{
	switch (*ComboCurrentItem)
	{
		case 0:
			*ComboCurrentItem = 2;
			break;
		case 1:
			*ComboCurrentItem = 3;
			break;
		case 2:
			*ComboCurrentItem = 0;
			break;
		case 3:
			*ComboCurrentItem = 1;
	}
}
void ToggleTriggerTypeComboType(int* ComboCurrentItem)
{
	switch (*ComboCurrentItem)
	{
	case 0:
		*ComboCurrentItem = 1;
		break;
	case 1:
		*ComboCurrentItem = 0;
		break;
	case 2:
		*ComboCurrentItem = 3;
		break;
	case 3:
		*ComboCurrentItem = 2;
	}
}



