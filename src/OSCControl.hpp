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
#include "implot.h"
#include "NetworkAnalyser.hpp"
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
		std::array<const char*, 55> SampleRatesComboList;
		// OWN the storage here so c_str() stays valid
		std::array<std::string, 55> SampleRatesLabels;
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
	struct NetworkAcquireState {
		bool acquiring = false;          // currently running
		double t_start_s = 0.0;          // time when started
		double elapsed_last_s = 0.0;     // total elapsed
		double success_until_time = 0.0; // time to stop showing "?"
	};
	struct
	{
		bool Display = false;
		bool Acquire = false;
		const char* SignalGenComboList[2] = { "SG1", "SG2" };
		const char* OscComboList[2] = { "OSC1", "OSC2" };
		int StimulusComboCurrentItem = 0;
		int InputComboCurrentItem = 0;
		int ResponseComboCurrentItem = 1;
		float ComboBoxWidth = 100.0f;
		bool PhaseOn = false;
		float ItemWidth = 100.0f;
		double f_start = 10;
		double f_stop = 1000;
		const char* UnitsComboList[2] = { "dbV", "V RMS" };
		int UnitsComboCurrentItem = 0;
		bool Autofit = false;
		bool AcquisitionExists = false;
		int PointSpacingComboCurrentItem = 0;
		const char* PointSpacingComboList[2] = { "Logarithmic", "Linear" };
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

	void SetNetworkAnalyser(NetworkAnalyser* na, NetworkAnalyser::Config* na_cfg)
	{
		if (na != this->na)
		{
			this->na = na;
		}
		if (na_cfg != this->na_cfg)
		{
			this->na_cfg = na_cfg;
		}
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
			AutofitY = WhiteOutlineButton(u8"Auto Fit  \u2195##Vertical", ImVec2(button_width, 30));
			ImGui::TableNextColumn();
			AutofitX = WhiteOutlineButton(u8"Auto Fit  \u2194##Horizontal", ImVec2(button_width, 30));
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
		SpectrumAnalyserControls.Autofit = WhiteOutlineButton("Auto Fit##SpectrumAnalyser", ImVec2(100, 30));
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
						IM_ARRAYSIZE(SpectrumAnalyserControls.UnitsComboList));
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
		ImGui::SameLine();
		ImGui::Text("Phase On");
		ImGui::SameLine();
		ToggleSwitch("##NetworkAnalyserPhaseOn", &NetworkAnalyserControls.PhaseOn, ImU32(NetworkAnalyserColour));
		ImGui::Text("Stimulus Generator");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(NetworkAnalyserControls.ComboBoxWidth);
		ImGui::Combo("##NetworkAnalyserGenCombo", &NetworkAnalyserControls.StimulusComboCurrentItem,
			NetworkAnalyserControls.SignalGenComboList, IM_ARRAYSIZE(NetworkAnalyserControls.SignalGenComboList));
		ImGui::Text("Reference (Input) Channel");
		ImGui::SameLine();
		// Assume OscComboList == {"Channel 1", "Channel 2"} (indices 0 and 1)
		int& in = NetworkAnalyserControls.InputComboCurrentItem;
		int& out = NetworkAnalyserControls.ResponseComboCurrentItem;

		// Clamp to {0,1} and ensure they start mirrored
		in = (in != 0); // 0 or 1
		out = (out != 0);
		if (in == out) out = 1 - in;

		// Draw + mirror logic: the one you edit wins this frame
		ImGui::SetNextItemWidth(NetworkAnalyserControls.ComboBoxWidth);
		bool changed_in = ImGui::Combo("##NetworkAnalyserInputCombo",
			&in, NetworkAnalyserControls.OscComboList,
			IM_ARRAYSIZE(NetworkAnalyserControls.OscComboList));
		if (changed_in) out = 1 - in;

		ImGui::Text("Response (Output) Channel");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(NetworkAnalyserControls.ComboBoxWidth);
		bool changed_out = ImGui::Combo("##NetworkAnalyserResponseCombo",
			&out, NetworkAnalyserControls.OscComboList,
			IM_ARRAYSIZE(NetworkAnalyserControls.OscComboList));
		if (changed_out) in = 1 - out;
		// Channel Logic
		na_cfg->gen_channel = NetworkAnalyserControls.StimulusComboCurrentItem + 1; // SG1=1, SG2=2
		na_cfg->ch_input = (in == 0 ? 1 : 2);
		na_cfg->ch_output = (out == 0 ? 1 : 2);

		RangeSliderDoubleLog("Frequency Range", &NetworkAnalyserControls.f_start, &NetworkAnalyserControls.f_stop,
			1, 5000,
			0.1,           // min span in Hz
			true,          // auto-swap
			true,          // snap to 1-2-5 on release
			0.0f,          // width: use remaining content width
			5.0f,          // bar thickness px
			8.0f);         // knob radius px
		// Update config with new frequency range
		na_cfg->f_start = NetworkAnalyserControls.f_start;
		na_cfg->f_stop = NetworkAnalyserControls.f_stop;
		// If you want tau terms counted, set:
		na_cfg->use_ifbw_limits = false; // or true if you DO want 3–4tau waits

		// Build label from UI config so it updates while dragging
		static NetworkAcquireState net_acq_state;

		// Replace the simple button with the dynamic acquire overlay
		if (DrawNetworkAcquireButton(net_acq_state, na, na_cfg, ImVec2(200, 30))) {
			// Optional: add any logging or event trigger on start
			printf("Network Analyser acquisition started\n");
		}
		ImGui::SameLine();
		NetworkAnalyserControls.Autofit = WhiteOutlineButton("Auto Fit##NetworkAnalyser", ImVec2(100, 30));

		ImGui::SetNextItemOpen(false, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("Advanced Options##NetworkOptions", ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			// Table: [ Label | Control ]
			if (ImGui::BeginTable("NetworkAdvTbl", 2, ImGuiTableFlags_SizingStretchProp))
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
					if (NetworkAnalyserControls.ItemWidth > 0)
						ImGui::SetNextItemWidth(NetworkAnalyserControls.ItemWidth);
					else
						ImGui::SetNextItemWidth(-FLT_MIN); // fill remaining width

					widget();
				};

				// Rows
				


				row("Vertical Units", [&] {
					ImGui::Combo("##Vertical Units",
						&NetworkAnalyserControls.UnitsComboCurrentItem,
						NetworkAnalyserControls.UnitsComboList,
						IM_ARRAYSIZE(NetworkAnalyserControls.UnitsComboList));
				});

				row("Number of Data Points", [&] {
					ImGui::SliderInt("##Points", &na_cfg->points, 2, 501, "%d",
						ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
				});

				row("Point Spacing", [&] {
					ImGui::Combo("##PointSpacing", &NetworkAnalyserControls.PointSpacingComboCurrentItem,
						NetworkAnalyserControls.PointSpacingComboList,
						IM_ARRAYSIZE(NetworkAnalyserControls.PointSpacingComboList));
				});
				switch (NetworkAnalyserControls.PointSpacingComboCurrentItem)
				{
				case 0:
						na_cfg->log_spacing = true;
						break;
				case 1:
					na_cfg->log_spacing = false;
						break;
				}

				ImGui::EndTable();
			}
		}
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
		for (int i = 0; i < 55; ++i) {
			// Format however you want (Hz/kHz etc.)
			SpectrumAnalyserControls.SampleRatesLabels[i] =
				fmt_hz_khz(constants::DIVISORS_375000[55 - i]);

			SpectrumAnalyserControls.SampleRatesComboList[i] =
				SpectrumAnalyserControls.SampleRatesLabels[i].c_str();

			SpectrumAnalyserControls.SampleRatesValues[i] = constants::DIVISORS_375000[55 - i];
		}
	}
	

	// Network Analyser Stuff
	NetworkAnalyser* na;
	NetworkAnalyser::Config* na_cfg;
	// Log-scale two-handle range slider (pure ImGui) with:
	// - reserved top space for labels
	// - explicit sizing
	// - left/right padding so right-side label/knob don't clip when at max
	//
	// Usage example (stretch + padding):
	//   RangeSliderDoubleLog("Frequency", &f_lo, &f_hi,
	//                        0.1, 5e6, 0.1, true, true,
	//                        0.0f, 5.0f, 8.0f, 4.0f, 0.0f, 8.0f);

	inline bool RangeSliderDoubleLog(const char* label,
		double* v_lower, double* v_upper,
		double v_min, double v_max,
		double min_span_hz = 0.0,
		bool   auto_swap = true,
		bool   snap_125_on_rel = false,
		float  widget_width = 0.0f,  // <=0: use remaining width
		float  bar_thickness_px = 4.0f,
		float  knob_radius_px = 7.0f,
		float  top_label_margin_px = 4.0f,
		float  left_pad_px = 0.0f,
		float  right_pad_px = 16.0f)
	{
		IM_ASSERT(v_lower && v_upper);
		if (!(v_min > 0.0 && v_min < v_max))
			return false;

		// ---- Local helpers ----
		auto clampd = [](double x, double a, double b) -> double {
			return x < a ? a : (x > b ? b : x);
		};
		auto snap125 = [](double x) -> double {
			if (x <= 0) return x;
			double e = std::floor(std::log10(x));
			double m = x / std::pow(10.0, e);
			double t = (m < 1.5) ? 1.0 : (m < 3.5 ? 2.0 : (m < 7.5 ? 5.0 : 10.0));
			return t * std::pow(10.0, e);
		};
		auto format_hz_si = [](char* buf, size_t n, double hz) {
			const char* unit = "Hz";
			double v = hz;
			if (hz >= 1e9) { v = hz / 1e9; unit = "GHz"; }
			else if (hz >= 1e6) { v = hz / 1e6; unit = "MHz"; }
			else if (hz >= 1e3) { v = hz / 1e3; unit = "kHz"; }
			std::snprintf(buf, n, "%.4g %s", v, unit); // 4 significant figures
		};
		struct LogMap {
			double lmin, lmax, inv_span;
			LogMap(double vmin, double vmax) {
				lmin = std::log10(vmin);
				lmax = std::log10(vmax);
				inv_span = 1.0 / (lmax - lmin);
			}
			double v_to_t(double v) const { return (std::log10(v) - lmin) * inv_span; } // [0,1]
			double t_to_v(double t) const { return std::pow(10.0, lmin + (lmax - lmin) * t); }
		};

		// ---- Sanitize initial values ----
		double lo = clampd(*v_lower, v_min, v_max);
		double hi = clampd(*v_upper, v_min, v_max);
		auto apply_constraints = [&](double& a, double& b) {
			a = clampd(a, v_min, v_max);
			b = clampd(b, v_min, v_max);
			if (auto_swap) {
				if (a > b) std::swap(a, b);
				if (min_span_hz > 0 && b - a < min_span_hz)
					b = clampd(a + min_span_hz, v_min, v_max);
			}
			else {
				if (min_span_hz > 0) {
					if (a > b - min_span_hz) a = b - min_span_hz;
					if (b < a + min_span_hz) b = a + min_span_hz;
				}
				a = clampd(a, v_min, v_max);
				b = clampd(b, v_min, v_max);
			}
		};
		apply_constraints(lo, hi);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) { *v_lower = lo; *v_upper = hi; return false; }

		const ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiID id = ImGui::GetID(label);

		// Layout: "Label  [slider widget]"
		ImGui::BeginGroup();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);

		// Pre-format labels to reserve vertical space above the bar
		char lbl_lo[32], lbl_hi[32];
		format_hz_si(lbl_lo, sizeof(lbl_lo), lo);
		format_hz_si(lbl_hi, sizeof(lbl_hi), hi);
		ImVec2 sz_lo = ImGui::CalcTextSize(lbl_lo);
		ImVec2 sz_hi = ImGui::CalcTextSize(lbl_hi);
		const float labels_h = (sz_lo.y > sz_hi.y ? sz_lo.y : sz_hi.y);

		// Compute widget area width with paddings
		float avail = ImGui::GetContentRegionAvail().x;
		float inner_w = (widget_width > 0.0f ? widget_width : avail);
		// Shrink by left/right padding so drawing/interaction stays in-bounds
		inner_w = ImMax(1.0f, inner_w - (left_pad_px + right_pad_px));

		const float grab_w = knob_radius_px * 2.0f; // click zone width per knob
		const float base_h = ImMax(knob_radius_px * 2.0f + 4.0f, bar_thickness_px + 8.0f);
		const float widget_h = labels_h + top_label_margin_px + base_h;

		// Position: shift start by left_pad_px so the bar/knobs are inset
		ImVec2 pos = ImGui::GetCursorScreenPos();
		pos.x += left_pad_px;

		ImVec2 size(inner_w, widget_h);
		ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id)) { *v_lower = lo; *v_upper = hi; ImGui::EndGroup(); return false; }

		// Geometry of bar (placed at bottom; labels area at top)
		const float bar_y = bb.Max.y - (base_h * 0.5f);
		const float bar_x0 = bb.Min.x + grab_w * 0.5f;
		const float bar_x1 = bb.Max.x - grab_w * 0.5f;
		const float bar_w = ImMax(1.0f, bar_x1 - bar_x0);

		LogMap lm(v_min, v_max);
		auto v_to_x = [&](double v) -> float { return (float)(bar_x0 + lm.v_to_t(v) * bar_w); };
		auto x_to_v = [&](float  x) -> double { return lm.t_to_v(clampd((x - bar_x0) / bar_w, 0.0, 1.0)); };

		float x_lo = v_to_x(lo);
		float x_hi = v_to_x(hi);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 col_bar_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
		const ImU32 col_bar_sel = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
		const ImU32 col_grab = ImGui::GetColorU32(ImGuiCol_SliderGrab);
		const ImU32 col_grab_act = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
		const ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);

		// Track
		dl->AddRectFilled(ImVec2(bar_x0, bar_y - bar_thickness_px * 0.5f),
			ImVec2(bar_x1, bar_y + bar_thickness_px * 0.5f),
			col_bar_bg, bar_thickness_px * 0.5f);

		// Selected region
		float sel_x0 = (x_lo < x_hi ? x_lo : x_hi);
		float sel_x1 = (x_lo < x_hi ? x_hi : x_lo);
		dl->AddRectFilled(ImVec2(sel_x0, bar_y - bar_thickness_px * 0.5f),
			ImVec2(sel_x1, bar_y + bar_thickness_px * 0.5f),
			col_bar_sel, bar_thickness_px * 0.5f);

		// Two invisible buttons for handles
		ImGui::PushID(id);
		ImRect lo_rect(ImVec2(x_lo - grab_w * 0.5f, bb.Min.y), ImVec2(x_lo + grab_w * 0.5f, bb.Max.y));
		ImGui::SetCursorScreenPos(lo_rect.Min);
		ImGui::InvisibleButton("lo", lo_rect.GetSize());
		bool lo_active = ImGui::IsItemActive();
		bool lo_hover = ImGui::IsItemHovered();

		ImRect hi_rect(ImVec2(x_hi - grab_w * 0.5f, bb.Min.y), ImVec2(x_hi + grab_w * 0.5f, bb.Max.y));
		ImGui::SetCursorScreenPos(hi_rect.Min);
		ImGui::InvisibleButton("hi", hi_rect.GetSize());
		bool hi_active = ImGui::IsItemActive();
		bool hi_hover = ImGui::IsItemHovered();

		// Click on track: activate nearest handle
		if (!lo_active && !hi_active && ImGui::IsMouseClicked(0) &&
			ImGui::IsMouseHoveringRect(bb.Min, bb.Max)) {
			float mx = ImGui::GetIO().MousePos.x;
			if (fabsf(mx - x_lo) <= fabsf(mx - x_hi)) {
				ImGui::SetActiveID(ImGui::GetID("lo"), window);
				ImGui::SetFocusID(ImGui::GetID("lo"), window);
				lo_active = true;
			}
			else {
				ImGui::SetActiveID(ImGui::GetID("hi"), window);
				ImGui::SetFocusID(ImGui::GetID("hi"), window);
				hi_active = true;
			}
		}

		bool changed_now = false;

		// Drag logic (log mapped)
		if (lo_active) {
			float mx = ImGui::GetIO().MousePos.x;
			lo = x_to_v(mx);
			apply_constraints(lo, hi);
			changed_now = true;
		}
		if (hi_active) {
			float mx = ImGui::GetIO().MousePos.x;
			hi = x_to_v(mx);
			apply_constraints(lo, hi);
			changed_now = true;
		}

		// Knobs
		x_lo = v_to_x(lo);
		x_hi = v_to_x(hi);
		const ImU32 lo_col = (lo_active || lo_hover) ? col_grab_act : col_grab;
		const ImU32 hi_col = (hi_active || hi_hover) ? col_grab_act : col_grab;
		dl->AddCircleFilled(ImVec2(x_lo, bar_y), knob_radius_px, lo_col, 20);
		dl->AddCircleFilled(ImVec2(x_hi, bar_y), knob_radius_px, hi_col, 20);

		// Labels ABOVE knobs (inside reserved top area)
		format_hz_si(lbl_lo, sizeof(lbl_lo), lo);
		format_hz_si(lbl_hi, sizeof(lbl_hi), hi);
		sz_lo = ImGui::CalcTextSize(lbl_lo);
		sz_hi = ImGui::CalcTextSize(lbl_hi);
		const float text_baseline = bb.Min.y + labels_h; // end of label block
		dl->AddText(ImVec2(x_lo - sz_lo.x * 0.5f, text_baseline - sz_lo.y), col_text, lbl_lo);
		dl->AddText(ImVec2(x_hi - sz_hi.x * 0.5f, text_baseline - sz_hi.y), col_text, lbl_hi);

		// Snap to 1-2-5 on release
		static bool lo_was_active = false, hi_was_active = false;
		if (lo_was_active && !lo_active && snap_125_on_rel) { lo = snap125(lo); apply_constraints(lo, hi); changed_now = true; }
		if (hi_was_active && !hi_active && snap_125_on_rel) { hi = snap125(hi); apply_constraints(lo, hi); changed_now = true; }
		lo_was_active = lo_active;
		hi_was_active = hi_active;

		ImGui::PopID();
		ImGui::EndGroup();

		bool any_change = changed_now || (lo != *v_lower) || (hi != *v_upper);
		*v_lower = lo; *v_upper = hi;
		return any_change;
	}

	static bool DrawNetworkAcquireButton(
		NetworkAcquireState& st,
		NetworkAnalyser* na,
		NetworkAnalyser::Config* cfg,
		ImVec2 size = ImVec2(200, 30))
	{
		const double now = ImGui::GetTime();
		bool clicked = false;

		// --- Update acquisition state ---
		if (st.acquiring && na) {
			st.elapsed_last_s = now - st.t_start_s;
			if (na->done()) {
				st.acquiring = false;
				st.success_until_time = now + 1.2; // flash for 1.2s
			}
		}

		// --- Compute label text ---
		char label[128];
		if (st.acquiring && na && na->CurrentConfig().points > 0) {
			const int total = na->CurrentConfig().points;
			const int done = std::clamp(na->current_index(), 0, total);
			const double frac = (double)done / (double)total;

			std::snprintf(label, sizeof(label),
				"Acquiring: %d / %d (%.1f s)", done, total, st.elapsed_last_s);

			// Draw progress overlay (behind text, inside button bounds)
			ImVec2 p_min = ImGui::GetCursorScreenPos();
			ImVec2 p_max = ImVec2(p_min.x + size.x, p_min.y + size.y);
			float w = (float)((p_max.x - p_min.x) * frac);
			ImGui::GetWindowDrawList()->AddRectFilled(
				p_min, ImVec2(p_min.x + w, p_max.y),
				IM_COL32(80, 255, 100, 120), ImGui::GetStyle().FrameRounding);
		}
		else if (now < st.success_until_time) {
			std::snprintf(label, sizeof(label),
				u8"Completed in %.1f s  \u2713", st.elapsed_last_s); // ?
		}
		else {
			const double est = na ? na->EstimateSweepSeconds_UI(*cfg) : 0.0;
			std::snprintf(label, sizeof(label),
				"Acquire (est. %.0f s)", est);
		}

		// --- Draw button with white outline ---
		// (assuming you already have this function defined elsewhere)
		if (WhiteOutlineButton(label, size)) {
			if (!st.acquiring && na) {
				na->StartSweep(*cfg);
				st.acquiring = true;
				st.t_start_s = now;
				st.elapsed_last_s = 0.0;
				st.success_until_time = 0.0;
				clicked = true;
			}
		}

		// --- Re-draw overlay inside button after WhiteOutlineButton ---
		if (st.acquiring && na && na->CurrentConfig().points > 0) {
			const int total = na->CurrentConfig().points;
			const int done = std::clamp(na->current_index(), 0, total);
			const double frac = (double)done / (double)total;

			ImVec2 r_min = ImGui::GetItemRectMin();
			ImVec2 r_max = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddRectFilled(
				r_min, ImVec2(r_min.x + (r_max.x - r_min.x) * frac, r_max.y),
				IM_COL32(80, 255, 100, 120), ImGui::GetStyle().FrameRounding);
		}

		return clicked;
	}




};
