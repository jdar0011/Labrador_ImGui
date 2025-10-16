#pragma once
#include "ControlWidget.hpp"
#include "librador.h"
#include "UIComponents.hpp"
#include "util.h"
#include "nfd.h"
#include <cstring>
#include <cstddef>
#include "imgui_stdlib.h"
#include "MiniHLInput.h"
#include <string>
#include <array>
#include <chrono>
#include <algorithm>
using Clock = std::chrono::steady_clock;

/// <summary>Oscilloscope Control Widget
/// </summary>
class OSCControl : public ControlWidget
{
public:
	// ImGui State variables
	bool DisplayCheckOSC1 = true;
	bool DisplayCheckOSC2 = true;
	/*int KSComboCurrentItem = 0;
	int AttenComboCurrentItem = 0;*/
	// Export stuff
	nfdchar_t* OSC1WritePath = NULL;
	nfdchar_t* OSC2WritePath = NULL;
	int OSC1WritePathComboCurrentItem = 0;
	int OSC2WritePathComboCurrentItem = 0;
	bool OSC1ClipboardCopyNext = false;
	bool OSC2ClipboardCopyNext = false;
	bool OSC1ClipboardCopied = false;
	bool OSC2ClipboardCopied = false;
	const nfdchar_t* FileExtension = "csv";
	//
	float OffsetVal = 0.0f;
	bool ACCoupledCheck = false;
	bool Paused = false;
	bool AutofitY = false;
	bool AutofitX = false;
	int TriggerTypeComboCurrentItem = 0;
	bool Trigger = true;
	SIValue TriggerLevel = SIValue("##trigger1_level", "Level", 3.3 / 2, -20.0, 20.0, "V", constants::volt_prefs, constants::volt_formats);
	bool AutoTriggerLevel = true;
	float TriggerHysteresis = 0.25;
	float osc1_time_scale = 5;
	float osc1_voltage_scale = 1;
	float osc1_offset = 0;
	float osc2_time_scale = 5;
	float osc2_voltage_scale = 1;
	float osc2_offset = 0;
	int osc1_ts_unit_idx = 1;
	int osc1_vs_unit_idx = 2;
	int osc1_os_unit_idx = 1;
	int osc2_ts_unit_idx = 1;
	int osc2_vs_unit_idx = 2;
	int osc2_os_unit_idx = 1;
	int tl_unit_idx = 1;
	int th_unit_idx = 1;
	bool ts_equalise = false;
	bool vs_equalise = false;
	bool os_equalise = false;
	bool Cursor1toggle = false;
	bool Cursor2toggle = false;
	bool SignalPropertiesToggle = false;
	bool AutoTriggerHysteresisToggle = true;
	bool HysteresisDisplayOptionEnabled = false; 
	//Spectrum Analyser stuff
	struct SpectrumAcquireState {
		bool acquiring = false;
		float max_time_s = 1.0;
		double elapsed_last_s = 0.0;
		Clock::time_point t0{};
		double success_until_time = 0.0;   // ImGui::GetTime() deadline for success overlay
	};
	struct 
	{
		bool On = false;
		bool AcquisitionExists = false;
		int SignalComboCurrentItem = 0;
		bool Acquire = false;
		int SampleRatesComboCurrentItem = 0;
		int WindowComboCurrentItem = 0;
		float TimeWindow = 1.f;
		float LookbackTimeWindow = 1.f;
		const char* SignalComboList[2] = { "OSC1", "OSC2" }; // Add Math Later
		// Pointers ImGui::Combo will read
		std::array<const char*, 56> SampleRatesComboList;
		// OWN the storage here so c_str() stays valid
		std::array<std::string, 56> SampleRatesLabels;
		// Int values that I can use directly
		std::array<int, 56> SampleRatesValues;
		const char* WindowComboList[2] = { "Hann", "Rectangular" };
		float ItemWidth = 100.0f;
		const char* UnitsComboList[2] = { "dbV", "V RMS" };
		int UnitsComboCurrentItem = 0;
		bool Autofit = false;
		const char* AcquisitionModeComboList[2] = { "Gated (Start-Stop)", "Lookback" };
		int AcquisitionModeComboCurrentItem = 0;
		int LastAcquiredMode = 0;
		SpectrumAcquireState AcquireState;
		double OSC1_last_captured = -1;
		double OSC2_last_captured = -1;
	} SpectrumAnalyserControls;
	// Network Analyser stuff
	struct
	{
		bool Display = false;
	} NetworkAnalyserControls;
	// Math stuff
	struct MathControls
	{
		std::string Text{};
		bool On{ true };
		bool Visible{ true };
		bool Parsable{ false };
	};
	MathControls MathControls1, MathControls2, MathControls3, MathControls4;
	


	// Public consts
	ImColor OSC1Colour = colourConvert(constants::OSC1_ACCENT);
	ImColor OSC2Colour = colourConvert(constants::OSC2_ACCENT);
	ImColor GenColour = colourConvert(constants::GEN_ACCENT);
	ImColor MathColour = colourConvert(constants::MATH_ACCENT);
	ImColor Green = ImColor(float(20./255), float(143./255), float(0), float(1));
	ImColor Red = ImColor(float(143./255), float(0. / 255), float(0), float(1));
	ImColor SpectrumAnalyserColour = colourConvert(constants::SPECTRUM_ANALYSER_ACCENT);
	ImColor NetworkAnalyserColour = colourConvert(constants::NETWORK_ANALYSER_ACCENT);


	OSCControl(const char* label, ImVec2 size, const float* borderColor)
	    : ControlWidget(label, size, borderColor)
	{
		BuildSampleRatesCombo();
	}
	/// <summary>
	/// Render UI elements for oscilloscope control
	/// </summary>
	void renderControl() override
	{
		if (ImGui::BeginTable("Buttons", 3))
		{
			float button_width = 100;
			ImGui::TableNextColumn();
			ToggleButton("Run/Stop", ImVec2(button_width,30), & Paused, Red, Green);
			ImGui::TableNextColumn();
			//// TODO: Implement Single Capture (Stop after one trigger event is found)
			//ImGui::Button("Single", ImVec2(120, 30));
			//ImGui::TableNextColumn();
			AutofitY = WhiteOutlineButton(u8"Auto Fit  \u2195", ImVec2(button_width, 30));
			ImGui::TableNextColumn();
			AutofitX = WhiteOutlineButton(u8"Auto Fit  \u2194", ImVec2(button_width, 30));
			ImGui::EndTable();
		}

		const float width = ImGui::GetContentRegionAvail().x * 0.95;
		float labWidth = 120.0f;
		float controlWidth = (width - 2*labWidth) / 2;

		ImGui::SeparatorText("Display");
		if (ImGui::BeginTable("ChannelsTable", 4))
		{
			ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_WidthFixed, labWidth);
			ImGui::TableSetupColumn("Two", ImGuiTableColumnFlags_WidthFixed, controlWidth);
			ImGui::TableSetupColumn("Three", ImGuiTableColumnFlags_WidthFixed, labWidth);
			ImGui::TableSetupColumn("Four", ImGuiTableColumnFlags_WidthFixed, controlWidth);

			// Channel 1 Toggle
			ImGui::TableNextColumn();
			ImGui::Text("Channel 1 (OSC1)");
			ImGui::TableNextColumn();
			ToggleSwitch((label + "Display1_toggle").c_str(), &DisplayCheckOSC1,
				ImU32(OSC1Colour));

			// Channel 2 Toggle
			ImGui::TableNextColumn();
			ImGui::Text("Channel 2 (OSC2)");
			ImGui::TableNextColumn();
			ToggleSwitch((label + "Display2_toggle").c_str(), &DisplayCheckOSC2,
				ImU32(OSC2Colour));

			// Cursor 1 Toggle
			ImGui::TableNextColumn();
			ImGui::Text("Cursor 1");
			ImGui::TableNextColumn();
			ToggleSwitch((label + "Cursor1_toggle").c_str(), &Cursor1toggle, GenColour);

			// Cursor 2 Toggle
			ImGui::TableNextColumn();
			ImGui::Text("Cursor 2");
			ImGui::TableNextColumn();
			ToggleSwitch((label + "Cursor2_toggle").c_str(), &Cursor2toggle, GenColour);

			// Signal Properties Toggle
			ImGui::TableNextColumn();
			ImGui::Text("Signal Properties");
			ImGui::TableNextColumn();
			ToggleSwitch(
			    (label + "sig_prop_toggle").c_str(), &SignalPropertiesToggle, GenColour);

			ImGui::EndTable();
		}

		labWidth = 100.0f;
		float labWidth2 = 70.0f;
		controlWidth = (width - labWidth - labWidth2) / 2;

		// Vertical space
		ImGui::Dummy(ImVec2(0, 10.0f));
		ImGui::SeparatorText("General");
		if (ImGui::BeginTable("GeneralTable", 4))
		{
			ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_WidthFixed, labWidth);
			ImGui::TableSetupColumn("Two", ImGuiTableColumnFlags_WidthFixed, controlWidth);
			ImGui::TableSetupColumn("Three", ImGuiTableColumnFlags_WidthFixed, labWidth2);
			ImGui::TableSetupColumn("Four", ImGuiTableColumnFlags_WidthFixed, controlWidth);

			// Trigger Properties
			ImGui::TableNextColumn();
			ImGui::Text("Trigger");
			ImGui::TableNextColumn();
			ToggleSwitch((label + "Trigger1_toggle").c_str(), &Trigger, GenColour);
			ImGui::TableNextColumn();
			ImGui::Text("Type");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(controlWidth);

			ImGui::Combo("##Trigger Type OSC1", &TriggerTypeComboCurrentItem,
				TriggerTypeComboList, IM_ARRAYSIZE(TriggerTypeComboList));

			if (!AutoTriggerLevel) TriggerLevel.renderInTable(100.0f);
			else
			{
				ImGui::TableNextColumn();
				ImGui::Text("Level");
				ImGui::TableNextColumn();
				// TODO: Write display function that formats text in the appropriate unit
				ImGui::Text("%.2f V", TriggerLevel.getValue());
			}
			ImGui::TableNextColumn();
			ImGui::Text("Auto Level");
			ImGui::TableNextColumn();
			ToggleSwitch("##Auto1", &AutoTriggerLevel, GenColour);

			if (HysteresisDisplayOptionEnabled)
			{
				ImGui::Text("Hysteresis Level");
				ImGui::TableNextColumn();
			//	renderSliderwUnits(label + "_trigger_hysteresis", &TriggerHysteresis, 0, 2, "%.2f",
			//	    constants::volt_units, &tl_unit_idx);
				ImGui::Text("Auto");
				ImGui::SameLine();
				ImGui::Text("ON");
				ImGui::SameLine();
				ToggleSwitch((label + "Auto_trigger1_hysteresis_toggle").c_str(),
					&AutoTriggerHysteresisToggle, GenColour);
				ImGui::SameLine();
				ImGui::Text("OFF");
			}

			ImGui::EndTable();
		}

		// Handle Clipboard Copied Feedback
		std::string OSC1ExportButtonText = "Export OSC1";
		if (OSC1ClipboardCopied == true)
		{
			OSC1ClipboardCopiedFrame = ImGui::GetFrameCount();
			OSC1ClipboardCopied = false;
		}
		if ((ImGui::GetFrameCount() - OSC1ClipboardCopiedFrame) < OSCClipboardCopiedLinger)
		{
			OSC1ExportButtonText = "Copied!";
		}

		if (WhiteOutlineButton((OSC1ExportButtonText+ "##OSC1ExportButton").c_str(),ImVec2(100,30)))
		{
			if (OSC1WritePathComboCurrentItem == 0) // Copy to Clipboard
			{
				OSC1ClipboardCopyNext = true; // state variable to tell PlotWidget to Copy
				                              // to clipboard on the next render
			}
			if (OSC1WritePathComboCurrentItem == 1) // Write to csv
			{
				nfdresult_t result = NFD_SaveDialog(FileExtension, NULL, &OSC2WritePath);
				if (result == NFD_OKAY)
				{
#ifndef NDEBUG
					puts("Success!");
					puts(OSC2WritePath);
#endif
				}
				else if (result == NFD_CANCEL)
				{
#ifndef NDEBUG
					puts("User pressed cancel.");
#endif
				}
				else
				{
#ifndef NDEBUG
					printf("Error: %s\n", NFD_GetError());
#endif
				}
			}
		}
		ImGui::SameLine();
		ImGui::Text("to");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(OSCWritePathComboWidth);
		ImGui::Combo("##OSC1WritePathCombo",&OSC1WritePathComboCurrentItem,OSCWritePathComboList,IM_ARRAYSIZE(OSCWritePathComboList));


		std::string OSC2ExportButtonText = "Export OSC2";
		if (OSC2ClipboardCopied == true)
		{
			OSC2ClipboardCopiedFrame = ImGui::GetFrameCount();
			OSC2ClipboardCopied = false;
		}
		if ((ImGui::GetFrameCount() - OSC2ClipboardCopiedFrame) < OSCClipboardCopiedLinger)
		{
			OSC2ExportButtonText = "Copied!";
		}
		if (WhiteOutlineButton((OSC2ExportButtonText + "##OSC2ExportButton").c_str(),ImVec2(100,30)))
		{
			if (OSC2WritePathComboCurrentItem == 0) // Copy to Clipboard
			{
				OSC2ClipboardCopyNext = true; // state variable to tell PlotWidget to Copy to clipboard on the next render
			}
			if (OSC2WritePathComboCurrentItem == 1) // Write to csv
			{
				nfdresult_t result = NFD_SaveDialog(FileExtension, NULL, &OSC2WritePath);
				if (result == NFD_OKAY)
				{
#ifndef NDEBUG
					puts("Success!");
					puts(OSC2WritePath);
#endif
				}
				else if (result == NFD_CANCEL)
				{
#ifndef NDEBUG
					puts("User pressed cancel.");
#endif
				}
				else
				{
#ifndef NDEBUG
					printf("Error: %s\n", NFD_GetError());
#endif
				}
			}
		}
		ImGui::SameLine();
		ImGui::Text("to");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(OSCWritePathComboWidth);
		ImGui::Combo("##OSC2WritePathCombo",&OSC2WritePathComboCurrentItem,OSCWritePathComboList,IM_ARRAYSIZE(OSCWritePathComboList));


		ImGui::SeparatorText("Math Mode");
		ImGui::Text("On");
		ImGui::SameLine();
		ToggleSwitch((label + "Math1_toggle").c_str(), &MathControls1.On, ImU32(MathColour));
		ImGui::TextColored(MathColour, "Math =");
		ImGui::SameLine();
		MiniHLInput("Expression", MathControls1.Text, rules,
			/*default_text_color=*/ImGui::GetColorU32(ImGuiCol_Text),
			/*bg_color=*/ImGui::GetColorU32(ImGuiCol_FrameBg));
		if (!MathControls1.Parsable && MathControls1.Text.length() > 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"\u2717");
			ImVec2 mn = ImGui::GetItemRectMin();
			ImVec2 mx = ImGui::GetItemRectMax();
			float pad = 6.0f;                          // enlarge hit area by 6 px on each side
			mn.x -= pad; mn.y -= pad; mx.x += pad; mx.y += pad;
			if (ImGui::IsMouseHoveringRect(mn, mx))
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted("Text not parsable!");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
		else if (MathControls1.Text.length())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), u8"\u2714");
		}
		
		// Spectrum Analyzer
		ImGui::SeparatorText("Spectrum Analyzer (FFT)");
		ImGui::Text("Display");
		ImGui::SameLine();
		ToggleSwitch("##SpectrumAnalyserOn", &SpectrumAnalyserControls.On, ImU32(SpectrumAnalyserColour));
		ImGui::Text("Signal");
		ImGui::Combo("##SpectrumAnalyserSignalCombo", &SpectrumAnalyserControls.SignalComboCurrentItem,
			SpectrumAnalyserControls.SignalComboList, IM_ARRAYSIZE(SpectrumAnalyserControls.SignalComboList));
		// Acquire Button
		switch (SpectrumAnalyserControls.AcquisitionModeComboCurrentItem)
		{
			case 0: // Gated
				if (DrawAcquireButton(SpectrumAnalyserControls.AcquireState))
				{
					SpectrumAnalyserControls.Acquire = true;
					SpectrumAnalyserControls.TimeWindow = std::clamp(SpectrumAnalyserControls.AcquireState.elapsed_last_s,0.01,(double)SpectrumAnalyserControls.AcquireState.max_time_s);
				}
				else
				{
					SpectrumAnalyserControls.Acquire = false;
				}
				break;
			case 1: // Lookback
				SpectrumAnalyserControls.Acquire = WhiteOutlineButton("Acquire", ImVec2(100, 30));
				if (SpectrumAnalyserControls.Acquire)
				{
					SpectrumAnalyserControls.TimeWindow = SpectrumAnalyserControls.LookbackTimeWindow;
				}
				break;
		}
		ImGui::SameLine();
		SpectrumAnalyserControls.Autofit = WhiteOutlineButton("Auto Fit", ImVec2(100, 30));
		if (SpectrumAnalyserControls.AcquisitionExists)
		{
			switch (SpectrumAnalyserControls.SignalComboCurrentItem)
			{
			case 0:
				ImGui::TextDisabled("Last captured: %.2f s", SpectrumAnalyserControls.OSC1_last_captured);
				break;
			case 1:
				ImGui::TextDisabled("Last captured: %.2f s", SpectrumAnalyserControls.OSC2_last_captured);
				break;
			}
			
		}
		ImGui::SetNextItemOpen(false, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("Advanced Options##SpectrumOptions", ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			// Table: [ Label | Control ]
			if (ImGui::BeginTable("SpectrumAdvTbl", 2, ImGuiTableFlags_SizingStretchProp))
			{
				// Fixed label column, stretchy control column
				ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.0f);
				ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

				// Helper to draw one row
				auto row = [&](const char* label, auto widget)
				{
					ImGui::TableNextRow();

					// Left: label (text)
					ImGui::TableSetColumnIndex(0);
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted(label);

					// Right: control (fills width unless you want a fixed ItemWidth)
					ImGui::TableSetColumnIndex(1);
					if (SpectrumAnalyserControls.ItemWidth > 0)
						ImGui::SetNextItemWidth(SpectrumAnalyserControls.ItemWidth);
					else
						ImGui::SetNextItemWidth(-FLT_MIN); // fill remaining width

					widget();
				};

				// Rows
				row("Sample Rate", [&] {
					ImGui::Combo("##SampleRate",
						&SpectrumAnalyserControls.SampleRatesComboCurrentItem,
						SpectrumAnalyserControls.SampleRatesComboList.data(),
						(int)SpectrumAnalyserControls.SampleRatesComboList.size());
				});
				
				switch (SpectrumAnalyserControls.AcquisitionModeComboCurrentItem)
				{
				case 0: // Gated
					row("Max Time Window", [&] {
						ImGui::SliderFloat("##MaxTimeWindow",
							&SpectrumAnalyserControls.AcquireState.max_time_s, 0.1f, 20.0f, "%.1f s");
					});
					break;
				case 1: // Lookback
					row("Time Window", [&] {
						ImGui::SliderFloat("##TimeWindow",
							&SpectrumAnalyserControls.LookbackTimeWindow, 0.1f, 20.0f, "%.1f s");
					});
					break;
				}


				row("Windowing Function", [&] {
					ImGui::Combo("##Windowing",
						&SpectrumAnalyserControls.WindowComboCurrentItem,
						SpectrumAnalyserControls.WindowComboList,
						IM_ARRAYSIZE(SpectrumAnalyserControls.WindowComboList));
				});

				row("Vertical Units", [&] {
					ImGui::Combo("##Vertical Units",
						&SpectrumAnalyserControls.UnitsComboCurrentItem,
						SpectrumAnalyserControls.UnitsComboList,
						IM_ARRAYSIZE(SpectrumAnalyserControls.WindowComboList));
				});

				row("Acquistion Mode", [&] {
					ImGui::Combo("##AcquisitionType", &SpectrumAnalyserControls.AcquisitionModeComboCurrentItem,
						SpectrumAnalyserControls.AcquisitionModeComboList,
						IM_ARRAYSIZE(SpectrumAnalyserControls.AcquisitionModeComboList));
				});

				ImGui::EndTable();
			}
		}

		// Network Analyzer
		ImGui::SeparatorText("Network Analyzer");
		ImGui::Text("Display");
		ImGui::SameLine();
		ToggleSwitch("##NetworkAnalyserOn", &NetworkAnalyserControls.Display, ImU32(NetworkAnalyserColour));
	}


	/// <summary>
	/// Control sampling settings on labrador board
	/// </summary>
	bool controlLab() override
	{
		return false;
	}


private:
	// ImGui const data
	const std::string label;
	int channel;
	int right_column_width = 350;
	float TriggerTypeComboWidth = 100.0;
	float TriggerLevelSliderWidth = 100.0;
	// Drop down content
	const char* TriggerTypeComboList[4]
	    = { "OSC1 Rising Edge", "OSC1 Falling Edge", "OSC2 Rising Edge", "OSC2 Falling Edge"};
	// Export stuff
	float OSCWritePathComboWidth = 100;
	const char* OSCWritePathComboList[2] = { "clipboard", "csv" };
	int OSC1ClipboardCopiedFrame = -1000;
	int OSC2ClipboardCopiedFrame = -1000;
	int OSCClipboardCopiedLinger = 50;
	float OSCExportButtonWidth = 150;
	/*const char* KSComboList[2] = { "375 KSPS", "750 KSPS" };
	const char* AttenComboList[3] = { "1x", "5x", "10x" };*/
	// Math Mode Stuff
	//float MathTextWidth = 250;
	// 1) Define your rules once (colors & tooltips are up to you)
	std::vector<MiniHLRule> rules = {
		// Logicals
		{"and", constants::KeywordColour,  "logical AND",  true, true, false},
		{"or",  constants::KeywordColour,  "logical OR",   true, true, false},
		{"xor", constants::KeywordColour,  "exclusive OR", true, true, false},
		{"not", constants::KeywordColour,  "logical NOT",  true, true, false},

		// Functions
		{"sin", constants::FunctionColour, "sin(x) (sine, in radians)", true, true, true},
		{"cos", constants::FunctionColour, "cos(x) (cosine, in radians)", true, true, true},
		{"tan", constants::FunctionColour, "tan(x) (tangent, in radians)", true, true, true},
		{"exp", constants::FunctionColour, "exp(x) (exponential, e^x)", true, true, true},
		{"log", constants::FunctionColour, "log(x) (natural log, ln(x), loge(x), log base e)", true, true, true},
		{"abs", constants::FunctionColour, "abs(x) (absolute value, |x|)", true, true, true},
		{"sqrt",constants::FunctionColour, "sqrt(x) (square root, x^(1/2))",true, true, true},
		{"sgn", constants::FunctionColour, "sgn(x) (sign, returns +1 if x>0, -1 if x<0, and 0 if x==0)", true, true, true},
		{"floor",constants::FunctionColour,"floor(x) (floor, rounds down to the nearest integer)",true, true, true},
		{"ceil",constants::FunctionColour, "ceil(x) (rounds up to the nearest integer)", true, true, true},
		{"round",constants::FunctionColour,"round(x) (rounds to the nearest integer. halfway cases are rounded away from zero)",true, true, true},
		{"trunc",constants::FunctionColour,"trunc(x) (removes the part right of the decimal place of the decimal number. for x>0, trunc(x)==floor(x). for x<0, trunc(x)==ceil(x)",true, true, true},
		{"frac",constants::FunctionColour, "frac(x) (keeps only the part right of the decimal place of the decimal number", true, true, true},
		{"log10",constants::FunctionColour,"log10(x) (log base 10)",true, true, true},
		{"asin", constants::FunctionColour, "asin(x) (arcsin(x),sin^-1(x), inverse sine)", true, true, true},
		{"acos", constants::FunctionColour, "acos(x) (arccos(x), cos^-1(x), inverse cosine)", true, true, true},
		{"atan", constants::FunctionColour, "atan(x) (arctan(x), tan^-1(x), inverse tangent", true, true, true},
		{"sinh", constants::FunctionColour, "sinh(x) (hyperbolic sine", true, true, true},
		{"cosh", constants::FunctionColour, "cosh(x) (hyperbolic cosine", true, true, true},
		{"tanh", constants::FunctionColour, "tanh(x) (hyperbolic tangent", true, true, true},
		{"pow", constants::FunctionColour,  "pow(x,y) (x^y, x raised to the yth power)	NOTE: does not work for x number, y signal. Instead try exp(log(x)y)", true, true, true},

		// Signals (distinct colors)
		{"osc1", ImU32(OSC1Colour), "oscilloscope signal 1", true, true, true},
		{"osc2", ImU32(OSC2Colour), "oscilloscope signal 2", true, true, true},

		// Constants
		{"pi", constants::NumberColour, "3.14159...", true, true, true},
		{"e",  constants::NumberColour, "2.71828...", true, true, false},

		// Symbols (not whole words)
		{"==", constants::SymbolColour, "equality",   false, true, false},
		{"!=", constants::SymbolColour, "inequality", false, true, false},
		{"+",  constants::SymbolColour, "plus",       false, true, false},
		{"-",  constants::SymbolColour, "minus",      false, true, false},
		{"*",  constants::SymbolColour, "multiply",   false, true, false},
		{"/",  constants::SymbolColour, "divide",     false, true, false},
		{"%",  constants::SymbolColour, "modulus",    false, true, false},
		{"^",  constants::SymbolColour, "power",      false, true, false},
		{"(",  constants::SymbolColour, "open paren", false, false, false},
		{")",  constants::SymbolColour, "close paren",false, false, false},
		{">",  constants::SymbolColour, "greater than", false, true, false},
		{"<",  constants::SymbolColour, "less than",    false, true, false},
		{">=", constants::SymbolColour, "greater than or equal",            false, true, false},
		{"<=", constants::SymbolColour, "less than or equal",            false, true, false},
		{"!",  constants::SymbolColour, "logical NOT",  false, true, false}
	};
	// Spectrum Analyser stuff
	static bool DrawAcquireButton(SpectrumAcquireState& st, const char* base_label = "Start Acquiring") {
		// 1) Update elapsed if running (and auto-stop if max reached)
		double elapsed = 0.0;
		if (st.acquiring) {
			elapsed = std::chrono::duration<double>(Clock::now() - st.t0).count();
			if (elapsed >= st.max_time_s) {
				st.acquiring = false;
				st.elapsed_last_s = st.max_time_s;
				st.success_until_time = ImGui::GetTime() + 1.2; // show success text for 1.2s
			}
		}

		// 2) One-time (and grow-only) width so button doesn't change size as text changes
		static float kAcquireBtnWidth = 0.0f;
		{
			const ImGuiStyle& s = ImGui::GetStyle();
			float w1 = ImGui::CalcTextSize(base_label).x;
			float w2 = ImGui::CalcTextSize("Acquiring: 9999.9 s").x;   // worst-case acquiring label
			float w3 = ImGui::CalcTextSize(u8"\u2713  9999.99 s").x;   // worst-case success label
			float need = ImMax(ImMax(w1, w2), w3) + s.FramePadding.x * 2.0f + 8.0f;
			if (need > kAcquireBtnWidth) kAcquireBtnWidth = need;
		}

		// 3) Draw the button with a stable ID & fixed width (do NOT change WhiteOutlineButton)
		bool pressed = WhiteOutlineButton("##AcquireBtn", ImVec2(kAcquireBtnWidth, 30));

		// 4) Grab geometry/draw list
		ImVec2 rmin = ImGui::GetItemRectMin();
		ImVec2 rmax = ImGui::GetItemRectMax();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		float rounding = ImGui::GetStyle().FrameRounding;

		// 5) Progress fill (under text), only while acquiring
		if (st.acquiring) {
			float t = (float)std::min(elapsed / st.max_time_s, 1.0);
			ImU32 col = ImGui::GetColorU32(ImVec4(0.30f, 0.75f, 0.40f, 0.35f));
			ImVec2 a = ImVec2(rmin.x + 1.0f, rmin.y + 1.0f); // inset to avoid covering white border
			ImVec2 b = ImVec2(rmin.x + 1.0f + t * (rmax.x - rmin.x - 2.0f), rmax.y - 1.0f);
			dl->AddRectFilled(a, b, col, rounding);
		}

		// 6) Decide which text to show *instead of* overlaying success box
		const double now_ui = ImGui::GetTime();
		const bool show_success = (!st.acquiring && now_ui < st.success_until_time);

		char label_buf[96];
		ImU32 txt_col = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
		if (show_success) {
			std::snprintf(label_buf, sizeof(label_buf), u8"\u2713  %.2f s", st.elapsed_last_s);
			txt_col = ImGui::GetColorU32(ImVec4(0.20f, 0.90f, 0.40f, 1.0f)); // green success text
		}
		else if (st.acquiring) {
			std::snprintf(label_buf, sizeof(label_buf), "Acquiring: %.1f s", elapsed);
		}
		else {
			std::snprintf(label_buf, sizeof(label_buf), "%s", base_label);
		}

		// 7) Draw the chosen text centered on the button
		ImVec2 ts = ImGui::CalcTextSize(label_buf);
		ImVec2 c = ImVec2((rmin.x + rmax.x - ts.x) * 0.5f, (rmin.y + rmax.y - ts.y) * 0.5f);
		dl->AddText(c, txt_col, label_buf);

		// 8) Handle clicks / state transitions
		bool do_capture = false;
		if (pressed) {
			if (!st.acquiring) {
				st.acquiring = true;
				st.t0 = Clock::now();
			}
			else {
				st.acquiring = false;
				st.elapsed_last_s = std::max(0.0, std::chrono::duration<double>(Clock::now() - st.t0).count());
				st.success_until_time = ImGui::GetTime() + 1.2;
				do_capture = true;
			}
		}
		// Auto-trigger capture on timeout (the frame we flip from acquiring->not)
		if (!st.acquiring && elapsed >= st.max_time_s && st.elapsed_last_s > 0.0) {
			do_capture = true;
		}

		return do_capture;
	}

	void BuildSampleRatesCombo()
	{
		// Fill Sample Rates Combo List with Formatted Units
		auto fmt_hz_khz = [](int hz) -> std::string { // Format Hz as Hz or kHz
			if (hz >= 1000) {
				// kHz; show 1 decimal if not an exact kHz
				if (hz % 1000 == 0) return std::to_string(hz / 1000) + " kHz";
				char buf[32];
				std::snprintf(buf, sizeof(buf), "%.1f kHz", hz / 1000.0);
				return std::string(buf);
			}
			else {
				return std::to_string(hz) + " Hz";
			}
		};
		for (int i = 0; i < 56; ++i) {
			// Format however you want (Hz/kHz etc.)
			SpectrumAnalyserControls.SampleRatesLabels[i] =
				fmt_hz_khz(constants::DIVISORS_375000[55 - i]);

			SpectrumAnalyserControls.SampleRatesComboList[i] =
				SpectrumAnalyserControls.SampleRatesLabels[i].c_str();

			SpectrumAnalyserControls.SampleRatesValues[i] = constants::DIVISORS_375000[55 - i];
		}
	}
};
