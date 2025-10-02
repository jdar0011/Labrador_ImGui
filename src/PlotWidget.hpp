#pragma once
#include "imgui.h"
#include "implot.h"
#include "librador.h"
#include "algorithm"
#include <chrono>
#include "OSCControl.hpp"
#include "OscData.hpp"
#include "fftw3.h"
#include "nfd.h"
#include <fstream>
#include <iostream>
#include <float.h>
#include <math.h>
#include "ControlWidget.hpp"


/// <summary>
/// Renders oscilloscope data
/// </summary>
class PlotWidget : public ControlWidget
{
public:
	int currentLabOscGain = 4;
	double time_of_last_gain_update = 0;

	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="label">Name of controller</param>
	/// <param name="size">Child window size</param>
	PlotWidget(const char* label, ImVec2 size, OSCControl* osc_control)
	    : ControlWidget(label, size, accentColour)
	    , size(size)
	    , osc_control(osc_control)
	{
		librador_set_oscilloscope_gain(1<<currentLabOscGain);
		ImPlot::GetInputMap().Fit = -1; // Disable double-click to fit so that it doesn't conflict with double-click to reset
	}

	/// <summary>
	/// Update size of child window
	/// </summary>
	/// <param name="new_size">New size</param>
	void setSize(ImVec2 new_size)
	{
		size = new_size;
	}

	/// <summary>
	/// Generic function to render plot widget with correct style
	/// </summary>
	void Render() override
	{
		// Show cursor deltas if they are both active
		bool show_cursor_props = (osc_control->Cursor1toggle && osc_control->Cursor2toggle);
		// Show text underneath if cursors or signal properties activated
		bool text_window = show_cursor_props || osc_control->SignalPropertiesToggle;
		ImVec2 region_size = ImGui::GetContentRegionAvail();
		float row_height = ImGui::GetFrameHeightWithSpacing();
		ImVec2 plot_size_total
		    = ImVec2(region_size.x, region_size.y - (text_window ? 3 * row_height : row_height));
		ImVec2 plot_size
			= ImVec2(region_size.x, region_size.y - (text_window ? 3 * row_height : row_height));
		ImVec2 plot_size_spectrum = ImVec2(0, 0);
		ImVec2 plot_size_spectrum_magnitude = ImVec2(0, 0);
		ImVec2 plot_size_spectrum_phase = ImVec2(0, 0);
		if (osc_control->SpectrumAnalyserOn)
		{
			plot_size.y /= 2;
			plot_size_spectrum = plot_size;
			plot_size_spectrum_magnitude = plot_size_spectrum;
			if (osc_control->SpectrumAnalyserPhaseOn)
			{
				plot_size_spectrum_phase = plot_size_spectrum;
				plot_size_spectrum_magnitude.x /= 2;
				plot_size_spectrum_phase.x /= 2;
			}
		}
		
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 255, 255));
		UpdateOscData();
		std::vector<double> analog_data_osc1 = OSC1Data.GetData();
		std::vector<double> analog_data_osc2 = OSC2Data.GetData();
		
		ImPlot::SetNextAxesLimits(init_time_range_lower, init_time_range_upper,
		    init_voltage_range_lower, init_voltage_range_upper, ImPlotCond_Once);
		if (next_resetX)
		{
			ImPlot::SetNextAxisLimits(ImAxis_X1, init_time_range_lower, init_time_range_upper, ImPlotCond_Always);
			next_resetX = false;
		}
		if (next_resetY)
		{
			ImPlot::SetNextAxisLimits(ImAxis_Y1, init_voltage_range_lower, init_voltage_range_upper, ImPlotCond_Always);
			next_resetY = false;
		}
		if (next_autofitX)
		{
			ImPlot::SetNextAxisLimits(ImAxis_X1, next_autofitX_min, next_autofitX_max, ImPlotCond_Always);
			next_autofitX = false;
		}
		if (next_autofitY)
		{
			ImPlot::SetNextAxisLimits(ImAxis_Y1, next_autofitY_min, next_autofitY_max, ImPlotCond_Always);
			next_autofitY = false;
		}

		// --- before BeginPlot ---
		ImPlotFlags base_plot_flags = ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_NoMenus;

		ImPlotFlags plot_flags = base_plot_flags;

		if (trig_dragging) {
			// Option A (strict): block all plot inputs (pan, zoom, box select, etc.)
			plot_flags |= ImPlotFlags_NoInputs;

		}
		if (ImPlot::BeginPlot("##Oscilloscopes", plot_size, plot_flags))
		{
			// No Labels for plot
			ImPlot::SetupAxes("", "", ImPlotAxisFlags_NoLabel, ImPlotAxisFlags_NoLabel);
			// Ensure plot cannot be expanded larger than the size of the buffer
			ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 0, 60);
			ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -20, 20);
			// --- Double-click to reset plot limits ---
			// Setup Axis to show units
			ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void*)"s");
			ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void*)"V");
			if (ImPlot::IsPlotHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
				{
					next_resetX = true;
					next_resetY = true;
				}
				else
				{
					osc_control->AutofitX = true;
					osc_control->AutofitY = true;
				}

			}
			if (ImPlot::IsAxisHovered(ImAxis_X1) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
				{
					next_resetX = true;
				}
				else
				{
					osc_control->AutofitX = true;
				}
			}
			if (ImPlot::IsAxisHovered(ImAxis_Y1) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
				{
					next_resetY = true;
				}
				else
				{
					osc_control->AutofitY = true;
				}
			}
			// Autofit X so that 3 periods occur in a time window
			// If there are 2 oscs visible use the one with the larger period
			// If none are visible, reset to default
			if (osc_control->AutofitX)
			{
				double T_osc1 = OSC1Data.GetPeriod();
				double T_osc2 = OSC2Data.GetPeriod();
				double T = init_time_range_upper;
				if (osc_control->DisplayCheckOSC1 && !osc_control->DisplayCheckOSC2)
				{
					T = T_osc1;
					T *= 3;
				}
				else if (osc_control->DisplayCheckOSC2 && !osc_control->DisplayCheckOSC1)
				{
					T = T_osc2;
					T *= 3;
				}
				else if (osc_control->DisplayCheckOSC2 && osc_control->DisplayCheckOSC1)
				{
					T = T_osc1 > T_osc2 ? T_osc1 : T_osc2;
					T *= 3;
				}
				else if (!osc_control->DisplayCheckOSC2 && !osc_control->DisplayCheckOSC1)
				{
					T = init_time_range_upper;
				}
				// if no data, 0 period, set to default range
				T = T == 0 ? init_time_range_upper : T;
				next_autofitX_min = -T / 2;
				next_autofitX_max = T / 2;
				next_autofitX = true;
			}
			// Autofits Y
			if (osc_control->AutofitY)
			{
				double osc1_max;
				double osc2_max;
				double osc1_min;
				double osc2_min;
				if (osc_control->DisplayCheckOSC1) // set to actual max and min only if the oscilloscope channel is visible
				{
					osc1_max = OSC1Data.GetDataMax();
					osc1_min = OSC1Data.GetDataMin();
				}
				else // else set to opposite extremes so that it always loses on comparison
				{
					osc1_max = DBL_MIN;
					osc1_min = DBL_MAX;
				}
				if (osc_control->DisplayCheckOSC2) // set to actual max and min only if the oscilloscope channel is visible
				{
					osc2_max = OSC2Data.GetDataMax();
					osc2_min = OSC2Data.GetDataMin();
				}
				else // else set to opposite extremes so that it always loses on comparison
				{
					osc2_max = DBL_MIN;
					osc2_min = DBL_MAX;
				}
				double osc_max = osc1_max > osc2_max ? osc1_max : osc2_max;
				double osc_min = osc1_min < osc2_min ? osc1_min : osc2_min;
				double osc_range = osc_max - osc_min;
				double osc_frac = 0.3;
				double pad = 0.5 * osc_range * (1 / osc_frac - 1);
				// set to default
				next_autofitY_min = init_voltage_range_lower;
				next_autofitY_max = init_voltage_range_upper;
				if (osc_control->DisplayCheckOSC1 || osc_control->DisplayCheckOSC2) // only autofit if there is visible data
				{
					next_autofitY_min = osc_min - pad;
					next_autofitY_max = osc_max + pad;
				}
				next_autofitY = true;
			}
			// Plot oscilloscope 1 signal
			std::vector<double> time_osc1 = OSC1Data.GetTime();
			ImPlot::SetNextLineStyle(osc_control->OSC1Colour.Value);
			if (osc_control->DisplayCheckOSC1)
			{
				ImPlot::PlotLine("##Osc 1", time_osc1.data(), analog_data_osc1.data(),
					analog_data_osc1.size());
			}
			// Set OscData Time Vector to match the current X-axis
			OSC1Data.SetTime(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max);
			// Plot Oscilloscope 2 Signal
			std::vector<double> time_osc2 = OSC2Data.GetTime();
			ImPlot::SetNextLineStyle(osc_control->OSC2Colour.Value);
			if (osc_control->DisplayCheckOSC2)
			{
				ImPlot::PlotLine("##Osc 2", time_osc2.data(), analog_data_osc2.data(),
					analog_data_osc2.size());
			}
			// Set OscData Time Vector to match the current X-axis
			OSC2Data.SetTime(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max);
			// Plot Math Signal
			double math_time_step = OSC1Data.GetTimeStep();
			std::string expr = osc_control->MathControls1.Text;
			bool parse_success = false;
			std::vector<double> math_data = EvalUserExpression(expr, analog_data_osc1, analog_data_osc2, parse_success);
			if (parse_success)
			{
				osc_control->MathControls1.Parsable = true;
				if (osc_control->MathControls1.On)
				{
					std::vector<double> time_math = time_osc1.size() >= time_osc2.size() ? time_osc1 : time_osc2;
					MathData.SetTime(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max, time_math);
					MathData.SetData(math_data);
					ImPlot::SetNextLineStyle(osc_control->MathColour.Value);
					ImPlot::PlotLine("##Math", time_osc1.data(), math_data.data(), math_data.size());
				}
			}
			else
			{
				osc_control->MathControls1.Parsable = false;
			}
			// Plot cursors
			if (osc_control->Cursor1toggle)
				drawCursor(1, &cursor1_x, &cursor1_y);
			if (osc_control->Cursor2toggle)
				drawCursor(2, &cursor2_x, &cursor2_y);

			DrawAndDragTriggers();
			

			ImPlot::EndPlot();
		}
		if (osc_control->SpectrumAnalyserOn)
		{
			if(ImPlot::BeginPlot())
		}
		
		// Help Button
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (ImGui::Button(" ? "))
		{
			show_help = true;
		}

		ImGui::PopStyleColor();

		// Signal and Cursor Properties
		ImGui::SameLine();
		if (ImGui::BeginTable("signal_props", 4))
		{
			ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_WidthFixed, 180);
			if (show_cursor_props)
			{
				ImGui::TableNextColumn();
				ImGui::Text("Cursor properties:");
				ImGui::TableNextColumn();
				ImGui::Text(u8"\u2206T = %.2f ms", (cursor2_x - cursor1_x)*1000);
				ImGui::TableNextColumn();
				ImGui::Text(u8"1/\u2206T = %.2f Hz", 1 / (cursor2_x - cursor1_x));
				ImGui::TableNextColumn();
				ImGui::Text(u8"\u2206V = %.2f V", cursor2_y - cursor1_y);
				ImGui::TableNextRow();
			}

			if (osc_control->SignalPropertiesToggle)
			{
				writeSignalProps(OSC1Data, osc_control->OSC1Colour);
				ImGui::TableNextRow();
				writeSignalProps(OSC2Data, osc_control->OSC2Colour);
			}

			ImGui::EndTable();
		}
		
		ImGui::PopStyleColor();
	}

	void writeSignalProps(OscData data, ImVec4 col)
	{
		double T = data.GetPeriod();
		double Vpp = data.GetVpp();
		

		ImGui::TableNextColumn();
		ImGui::TextColored(col, "OSC%d Signal Properties: ", data.GetChannel());

		if (T != (double)-1 && T != 0)
		{
			// Period found
			ImGui::TableNextColumn();
			ImGui::Text(u8"\u2206T = % .2f ms", 1000 * T);
			ImGui::TableNextColumn();
			ImGui::Text(u8"1/\u2206T = %.2f Hz", 1 / T);
		}
		else
		{
			ImGui::TableNextColumn();
			ImGui::Text("[No periodicity detected] ");
		}
		if (Vpp < 100 && Vpp > 0)
		{
			ImGui::TableNextColumn();
			ImGui::Text("Vpp = %.2f V", Vpp);
		}
		else
		{
			ImGui::TableNextColumn();
			ImGui::Text(" [No voltage min/max detected] ");
		}
	}

	void drawCursor(int id, double* cx, double* cy)
	{
		if (*cx == -1000 && *cy == -1000)
		{
			// Initial cursor centered
			ImPlotRect lim = ImPlot::GetPlotLimits();
			ImPlotPoint max = lim.Max();
			ImPlotPoint min = lim.Min();
			*cx = (max.x + min.x) / 2;
			*cy = (max.y + min.y) / 2;
		}

		ImPlot::DragPoint(id, cx, cy, ImVec4(1, 1, 1, 1), 8.0f);
		char cursor_label[20];
		std::string label = std::to_string(id) + ":";
		// Label is in (ms, V)
		sprintf(cursor_label, (label + "(%.2f, %.2f)").c_str(), *cx, *cy);
		ImPlot::PlotText(cursor_label, *cx, *cy, ImVec2(50, -12));
		ImPlot::SetNextLineStyle(ImVec4(1, 1, 1, 0.8));
		ImPlot::PlotInfLines((label + "vert").c_str(), cx, 1, ImPlotInfLinesFlags_None);
		ImPlot::SetNextLineStyle(ImVec4(1, 1, 1, 0.8));
		ImPlot::PlotInfLines((label + "vert").c_str(), cy, 1, ImPlotInfLinesFlags_Horizontal);
	}

	void UpdateOscData()
	{
		// sets whether the osc is paused or not (if paused, data will not update)
		OSC1Data.SetPaused(osc_control->Paused);
		OSC2Data.SetPaused(osc_control->Paused);
		// sets the time that the trigger on the plot will trigger (basically the time where the trigger marker on the plot is; defaults at 0)
		OSC1Data.SetTriggerTimePlot(trigger_time_plot);
		OSC2Data.SetTriggerTimePlot(trigger_time_plot);
		// sets the entire vector which will be used to plot (including part cut off due to trigger)
		OSC1Data.SetExtendedData();
		OSC2Data.SetExtendedData();
		constants::Channel trigger_channel = maps::ComboItemToChannelTriggerPair.at(osc_control->TriggerTypeComboCurrentItem).channel;
		constants::TriggerType trigger_type = maps::ComboItemToChannelTriggerPair.at(osc_control->TriggerTypeComboCurrentItem).trigger_type;
		// calculates the time that the trigger occurs in the extended_data vector depending on which channel is triggering
		double trigger_time = 0;
		// sets trigger variable for each osc (this is a bit hacky but whatever)
		OSC1Data.SetTriggerOn(osc_control->Trigger);
		OSC2Data.SetTriggerOn(osc_control->Trigger);
		if (trigger_channel == constants::Channel::OSC1)
		{
			trigger_time = OSC1Data.GetTriggerTime(osc_control->Trigger, trigger_type,
				osc_control->TriggerLevel.getValue(), osc_control->TriggerHysteresis);
		}
		if (trigger_channel == constants::Channel::OSC2)
		{
			trigger_time = OSC2Data.GetTriggerTime(osc_control->Trigger, trigger_type,
				osc_control->TriggerLevel.getValue(), osc_control->TriggerHysteresis);
		}
		// sets the trigger time for both oscs
		OSC1Data.SetTriggerTime(trigger_time);
		OSC2Data.SetTriggerTime(trigger_time);
		// sets the data vector that will be plotted
		OSC1Data.SetData();
		OSC2Data.SetData();
		// sets the data vector used for analysis (FFT, period, Vpp etc; quite large)
		OSC1Data.SetRawData();
		OSC2Data.SetRawData();
		// sets another data vector used for analysis (contains a whole number of periods)
		OSC1Data.SetPeriodicData();
		OSC2Data.SetPeriodicData();
		// applies FFT to the raw_data vector
		OSC1Data.ApplyFFT();
		OSC2Data.ApplyFFT();
		// auto sets gain to maximise resolution without clipping
		AutoSetOscGain();
		// auto sets trigger level
		if (osc_control->AutoTriggerLevel)
		{
			AutoSetTriggerLevel(trigger_channel, trigger_type, &osc_control->TriggerLevel, &osc_control->TriggerHysteresis);
		}
		// handles writing to clipboard and csv files (copying data)
		HandleWrites();
	}
	void HandleWrites()
	{
		bool* OSCClipboardCopyNext_p = NULL; // pointer to the bool for OSC which needs copying to clipboard (only one wll need copying at a time, prevents rewriting of code)
		bool* OSCClipboardCopied_p = NULL; // pointer to a status for whether copying was successful
		nfdchar_t** OSCWritePath_p = NULL; // pointer to the OSCWritePath which needs writing to file (one at a time as well)
		OscData* OSCData_p = NULL;

		// set up pointers based what we are doing to which osc
		if (osc_control->OSC1ClipboardCopyNext) // if we are copying osc1 to clipboard
		{
			OSCClipboardCopyNext_p = &osc_control->OSC1ClipboardCopyNext; // sets the pointer to the clipboard bool
			OSCClipboardCopied_p = &osc_control->OSC1ClipboardCopied; // sets the pointer to the clipboard success
			OSCData_p = &OSC1Data; // sets the pointer to the OSC we are copying
		}
		if (osc_control->OSC2ClipboardCopyNext) // if we are copying osc2 to clipboard
		{
			OSCClipboardCopyNext_p = &osc_control->OSC2ClipboardCopyNext; // sets the pointer to the clipboard bool
			OSCClipboardCopied_p = &osc_control->OSC2ClipboardCopied; // sets the pointer to the clipboard success
			OSCData_p = &OSC1Data; // sets the pointer to the OSC we are copying
		}
		if (osc_control->OSC1WritePath != NULL) // if we are writing osc1 to csv file
		{
			OSCWritePath_p = &osc_control->OSC1WritePath; // sets the pointer to the write path var
			OSCData_p = &OSC1Data; // sets the pointer to the OSC we are copying
		}
		if (osc_control->OSC2WritePath != NULL) // if we are writing osc2 to csv file
		{
			OSCWritePath_p = &osc_control->OSC2WritePath; // sets the pointer to the write path var
			OSCData_p = &OSC1Data; // sets the pointer to the OSC we are copying
		}

		// once pointers are set up, perform desired task on desired osc
		if (OSCClipboardCopyNext_p != NULL) // if we are copying to clipboard
		{
			// get OSCData signal and time vectors
			std::vector<double> data = OSCData_p->GetData();
			std::vector<double> time = OSCData_p->GetTime();
			// write osc to string
			std::string clipboard_str = ""; // define empty string which we will add to
			clipboard_str += "Time\tVoltage\n"; // header
			if (data.size() != 0)
			{
				for (int i = 0; i < data.size(); i++)
				{
					std::string time_str = std::to_string(time[i]); // convert time point to string
					std::string data_str = std::to_string(data[i]); // convert data point to string
					clipboard_str += time_str + "\t" + data_str + "\n"; // write to the clipboard string
				}
				ImGui::SetClipboardText(clipboard_str.c_str());
				*OSCClipboardCopied_p = true;
			}
			*OSCClipboardCopyNext_p = false; // reset state so we don't repeat on next render
		}
		if (OSCWritePath_p != NULL) // if we are writing to a csv file
		{
			// get OSCData signal and time vectors
			std::vector<double> data = OSCData_p->GetData();
			std::vector<double> time = OSCData_p->GetTime();
			// write osc to string
			std::string file_str = ""; // define empty string which we will add to
			file_str += "Time,Voltage\n"; // header
			if (data.size() != 0)
			{
				for (int i = 0; i < data.size(); i++)
				{
					std::string time_str = std::to_string(time[i]); // convert time point to string
					std::string data_str = std::to_string(data[i]); // convert data point to string
					file_str += time_str + "," + data_str + "\n"; // write to the clipboard string
				}
				std::ofstream file;
				// Automatically add the file extension to the end of the directory string if it isn't already there
				// NOTE: This is not an ideal implementation. It doesn't behave entirely how file saving should work in windows
				// For example, usually if you overwriting a file, it will warn you that you are overwriting a file
				// But because we add the extension afterwards, it doesn't do this
				// There may be a workaround, perhaps by rewriting some of nfd source code to fit our needs
				// or perhaps we may just need to use a windows library and then create separate code for handling file saving 
				// on macOS and linux.
				size_t path_length = strlen(*OSCWritePath_p);
				std::string extension_str = "." + std::string(osc_control->FileExtension);
				if (path_length > extension_str.length())
				{
					// get last n characters (where n is the length of the extension)
					int extension_start_index = path_length - extension_str.length();
					if (std::strncmp(*OSCWritePath_p + extension_start_index, extension_str.c_str(), extension_str.length()) != 0) // checks if file path string does not end in the file extension
					{
						// append the file extension since it is not there already
						std::string file_path_str = std::string(*OSCWritePath_p); // convert file path to std::string
						file_path_str += extension_str; // append the extension
						delete[] *OSCWritePath_p; // deallocate the old OSCWritePath memory as we are about to reallocate it
						*OSCWritePath_p = new char[file_path_str.length() + 1]; // allocates the memory for OSCWritePath
						std::strcpy(*OSCWritePath_p, file_path_str.c_str()); // copies the string to the OSCWritePath
					}
				}
				file.open(*OSCWritePath_p);
				file << file_str.c_str();
				file.close();
			}
			*OSCWritePath_p = NULL; // reset state so we don't repeat on next render
		}
	}
	void AutoSetTriggerLevel(constants::Channel trigger_channel,
	    constants::TriggerType trigger_type, SIValue* TriggerLevel, float* TriggerHysteresis)
	{
		double hysteresis_factor = 0.4;
		OscData* OSCData_ptr;
		if (trigger_channel == constants::Channel::OSC1)
		{
			OSCData_ptr = &OSC1Data;
		}
		if (trigger_channel == constants::Channel::OSC2)
		{
			OSCData_ptr = &OSC2Data;
		}
		TriggerLevel->setLevel(OSCData_ptr->GetAverage());
		if (trigger_type == constants::TriggerType::RISING_EDGE)
		{
			*TriggerHysteresis
			    = hysteresis_factor * std::abs((TriggerLevel->getValue() - OSCData_ptr->GetDataMin()));
		}
		if (trigger_type == constants::TriggerType::FALLING_EDGE)
		{
			*TriggerHysteresis
			    = hysteresis_factor * std::abs((OSCData_ptr->GetDataMax() - TriggerLevel->getValue()));
		}
	}
	void DrawAndDragTriggers() {
		ImDrawList* draw_list = ImPlot::GetPlotDrawList();
		double trig_x = trigger_time_plot;
		double trig_y = osc_control->TriggerLevel.getValue();

		ImVec2 plot_min = ImPlot::GetPlotPos();
		ImVec2 plot_size = ImPlot::GetPlotSize();
		ImVec2 plot_max(plot_min.x + plot_size.x, plot_min.y + plot_size.y);
		ImPlotRect lim = ImPlot::GetPlotLimits();

		ImVec2 px = ImPlot::PlotToPixels(trig_x, trig_y);
		ImGuiIO& io = ImGui::GetIO();

		// trigger information
		constants::Channel trigger_channel = maps::ComboItemToChannelTriggerPair.at(osc_control->TriggerTypeComboCurrentItem).channel;
		constants::TriggerType trigger_type = maps::ComboItemToChannelTriggerPair.at(osc_control->TriggerTypeComboCurrentItem).trigger_type;

		ImColor col_trig_col = trigger_channel == constants::Channel::OSC1 ? osc_control->OSC1Colour : osc_control->OSC2Colour;
		if (!osc_control->Trigger)
		{
			col_trig_col.Value.w = 0.5f; // make transparent
		}
		ImU32 col_trig = ImGui::GetColorU32(col_trig_col.Value);

		// --- persistent per-frame drag state ---
		struct { bool x = false, y = false; } static drag;

		bool lmb_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool lmb_dragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left,1.0f);
		bool lmb_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		bool double_clicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		bool ctrl_down = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
		bool alt_down = ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt);
		bool shift_down = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);

		auto clampf = [](float v, float a, float b) { return v < a ? a : (v > b ? b : v); };

		// ---------------- TOP (X trigger) ----------------
		{
			float s = 10.0f;
			float x, y;
			bool on_screen = (px.x >= plot_min.x && px.x <= plot_max.x);
			bool scaled_font = false;

			if (on_screen) { x = px.x; y = plot_min.y; }
			else if (px.x < plot_min.x) { // left indicator
				float ss = 3.f * expf(100.f * float(trig_x - lim.X.Min)) + 5.f;
				s = ss; x = plot_min.x - s * 0.5f; y = plot_min.y;
				ImGui::SetWindowFontScale(s / 10.0f); scaled_font = true;
			}
			else {                      // right indicator
				float ss = 3.f * expf(100.f * float(lim.X.Max - trig_x)) + 5.f;
				s = ss; x = plot_max.x + s * 0.5f; y = plot_min.y;
				ImGui::SetWindowFontScale(s / 10.0f); scaled_font = true;
			}

			// Triangle + "T"
			ImVec2 p1(x, y + s), p2(x - 0.5f * s, y), p3(x + 0.5f * s, y);
			draw_list->AddTriangleFilled(p1, p2, p3, col_trig);
			ImVec2 ts = ImGui::CalcTextSize("T");
			draw_list->AddText(ImVec2(x - ts.x * 0.5f, y - ts.y + 4.f), col_trig, "T");

			


			// Hit box (larger than triangle)
			ImRect hit(ImVec2(x - 0.6f * s - 4.f, y - s - 4.f),
				ImVec2(x + 0.6f * s + 4.f, y + s + 4.f));
			bool hovered = hit.Contains(io.MousePos) && ImPlot::IsPlotHovered();
			if (hovered && !drag.y)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}
			if (ctrl_down) DrawEdgeIndicatorX(draw_list, x, y, ts, trigger_type, col_trig);
			if (hovered && shift_down)
			{
				osc_control->Trigger = !osc_control->Trigger; // if ctrl + double clicked, toggle trigger
			}
			else if (hovered && double_clicked) // if x trigger left double clicked, set to 0s
			{
				trig_x = 0.0;
				next_resetX = false;
				next_resetY = false;
				next_autofitX = false;
				next_autofitY = false;
			}
			else if (hovered && lmb_clicked && ctrl_down)
			{
				ToggleTriggerTypeComboType(&osc_control->TriggerTypeComboCurrentItem);
			}
			else if (hovered && lmb_clicked && alt_down)
			{
				ToggleTriggerTypeComboChannel(&osc_control->TriggerTypeComboCurrentItem);
			}
			else if (hovered && lmb_clicked && !drag.y) drag.x = true;

			// Dragging
			if (drag.x && lmb_dragging) {
				float mx = clampf(io.MousePos.x, plot_min.x, plot_max.x);
				ImPlotPoint plot_xy = ImPlot::PixelsToPlot(ImVec2(mx, plot_min.y));
				trig_x = ImClamp(double(plot_xy.x), lim.X.Min, lim.X.Max);
			}
			if (drag.x && lmb_released) drag.x = false;

			if (scaled_font) ImGui::SetWindowFontScale(1.0f);
		}

		// ---------------- RIGHT (Y trigger) ----------------
		{
			float s = 10.0f;
			float x, y;
			bool on_screen = (px.y >= plot_min.y && px.y <= plot_max.y);
			bool scaled_font = false;

			if (on_screen) { x = plot_max.x; y = px.y; }
			else if (px.y > plot_max.y) { // bottom indicator
				float ss = 3.f * expf(1.f * float(trig_y - lim.Y.Min)) + 5.f;
				s = ss; x = plot_max.x; y = plot_max.y + s * 0.5f;
				ImGui::SetWindowFontScale(s / 10.0f); scaled_font = true;
			}
			else {                      // top indicator
				float ss = 3.f * expf(1.f * float(lim.Y.Max - trig_y)) + 5.f;
				s = ss; x = plot_max.x; y = plot_min.y - s * 0.5f;
				ImGui::SetWindowFontScale(s / 10.0f); scaled_font = true;
			}

			ImVec2 p1(x - s, y), p2(x, y - 0.5f * s), p3(x, y + 0.5f * s);
			draw_list->AddTriangleFilled(p1, p2, p3, col_trig);
			ImVec2 ts = ImGui::CalcTextSize("T");
			draw_list->AddText(ImVec2(x, y - ts.y * 0.5f), col_trig, "T");

			// Draw Automatic indicator
			if (osc_control->AutoTriggerLevel)
			{
				ImGui::SetWindowFontScale(s / 17.f); scaled_font = true;
				ImVec2 at_ts = ImGui::CalcTextSize("A");
				ImVec2 at_text_pos(x + 2.f, y - ts.y * 0.5f - at_ts.y+1.f);
				ImVec2 at_circle_pos(at_text_pos.x + 0.5 * at_ts.x - 0.5f, at_text_pos.y + 0.5 * at_ts.y + 0.5f);
				draw_list->AddText(ImVec2(at_text_pos.x, at_text_pos.y), col_trig, "A");
				draw_list->AddCircle(ImVec2(at_circle_pos.x, at_circle_pos.y), 0.4f * at_ts.y, col_trig);
			}

			ImRect hit(ImVec2(x - s - 4.f, y - s - 4.f),
				ImVec2(x + ts.x + 4.f, y + s + 4.f));
			bool hovered = hit.Contains(io.MousePos) && ImPlot::IsPlotHovered();
			if (hovered && !drag.x)
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
			}
			if (ctrl_down) DrawEdgeIndicatorY(draw_list, x, y, ts, trigger_type, col_trig);
			if (hovered && shift_down && lmb_clicked)
			{
				osc_control->Trigger = !osc_control->Trigger; // if ctrl + double clicked, toggle trigger
			}
			else if (hovered && double_clicked) // if y trigger left double clicked, toggel automode
			{
				osc_control->AutoTriggerLevel = !osc_control->AutoTriggerLevel;
				next_resetX = false;
				next_resetY = false;
				next_autofitX = false;
				next_autofitY = false;
			}
			else if (hovered && lmb_clicked && ctrl_down)
			{
				ToggleTriggerTypeComboType(&osc_control->TriggerTypeComboCurrentItem);
			}
			else if (hovered && lmb_clicked && alt_down)
			{
				ToggleTriggerTypeComboChannel(&osc_control->TriggerTypeComboCurrentItem);
			}
			else if (hovered && lmb_clicked && !drag.x) drag.y = true;
			if (drag.y && lmb_dragging) {
				osc_control->AutoTriggerLevel = false;
				float my = clampf(io.MousePos.y, plot_min.y, plot_max.y);
				ImPlotPoint plot_xy = ImPlot::PixelsToPlot(ImVec2(plot_min.x, my));
				trig_y = ImClamp(double(plot_xy.y), lim.Y.Min, lim.Y.Max);
			}
			if (drag.y && lmb_released) drag.y = false;

			if (scaled_font) ImGui::SetWindowFontScale(1.0f);
		}

		// Write back
		trigger_time_plot = trig_x;
		osc_control->TriggerLevel.setLevel(trig_y);

		// Publish current drag state so BeginPlot can apply flags next frame
		trig_dragging = drag.x || drag.y;
	}
	void DrawEdgeIndicatorX(ImDrawList* draw_list, float x, float y, ImVec2 ts, constants::TriggerType trigger_type, ImU32 col_trig)
	{
		// Draw Trigger Type indicator
		if (trigger_type == constants::TriggerType::RISING_EDGE)
		{
			ImVec2 tt_indicator_size(ts.x * 0.8, ts.y - 10.f);
			ImVec2 tt_indicator_pos(x - ts.x * 0.5f - tt_indicator_size.x + -4.0f, y - ts.y + 8.f);
			ImVec2 p1_line(tt_indicator_pos.x, tt_indicator_pos.y + tt_indicator_size.y),
				p2_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y + tt_indicator_size.y),
				p3_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y),
				p4_line(tt_indicator_pos.x + tt_indicator_size.x, tt_indicator_pos.y);
			draw_list->AddLine(p1_line, p2_line, col_trig, 1.0f);
			draw_list->AddLine(p2_line, p3_line, col_trig, 1.0f);
			draw_list->AddLine(p3_line, p4_line, col_trig, 1.0f);
		}
		else if (trigger_type == constants::TriggerType::FALLING_EDGE)
		{
			ImVec2 tt_indicator_size(ts.x * 0.8, ts.y - 10.f);
			ImVec2 tt_indicator_pos(x - ts.x * 0.5f - tt_indicator_size.x + -4.0f, y - ts.y + 8.f);
			ImVec2 p1_line(tt_indicator_pos.x, tt_indicator_pos.y),
				p2_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y),
				p3_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y + tt_indicator_size.y),
				p4_line(tt_indicator_pos.x + tt_indicator_size.x, tt_indicator_pos.y + tt_indicator_size.y);
			draw_list->AddLine(p1_line, p2_line, col_trig, 1.0f);
			draw_list->AddLine(p2_line, p3_line, col_trig, 1.0f);
			draw_list->AddLine(p3_line, p4_line, col_trig, 1.0f);
		}
	}
	void DrawEdgeIndicatorY(ImDrawList* draw_list, float x, float y, ImVec2 ts, constants::TriggerType trigger_type, ImU32 col_trig)
	{
		// Draw Trigger Type indicator
		ImVec2 tt_indicator_size(ts.x * 0.8, ts.y - 10.f);
		ImVec2 tt_indicator_pos(x + 1.0f, y + ts.y * 0.5f + 1.f);
		if (trigger_type == constants::TriggerType::RISING_EDGE)
		{
			ImVec2 p1_line(tt_indicator_pos.x, tt_indicator_pos.y + tt_indicator_size.y),
				p2_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y + tt_indicator_size.y),
				p3_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y),
				p4_line(tt_indicator_pos.x + tt_indicator_size.x, tt_indicator_pos.y);
			draw_list->AddLine(p1_line, p2_line, col_trig, 1.0f);
			draw_list->AddLine(p2_line, p3_line, col_trig, 1.0f);
			draw_list->AddLine(p3_line, p4_line, col_trig, 1.0f);
		}
		else if (trigger_type == constants::TriggerType::FALLING_EDGE)
		{
			ImVec2 p1_line(tt_indicator_pos.x, tt_indicator_pos.y),
				p2_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y),
				p3_line(tt_indicator_pos.x + tt_indicator_size.x * 0.5f, tt_indicator_pos.y + tt_indicator_size.y),
				p4_line(tt_indicator_pos.x + tt_indicator_size.x, tt_indicator_pos.y + tt_indicator_size.y);
			draw_list->AddLine(p1_line, p2_line, col_trig, 1.0f);
			draw_list->AddLine(p2_line, p3_line, col_trig, 1.0f);
			draw_list->AddLine(p3_line, p4_line, col_trig, 1.0f);
		}
	}

	// Draw a Combo when right clicking on the trigger (if there is time)
	void DrawTriggerCombo()
	{

	}

	

	void AutoSetOscGain()
	{
		int frame = ImGui::GetFrameCount();
		if (frame - last_update_frame < 5)
		{
			return;
		}

		int gainUpdate1 = 1;
		int gainUpdate2 = 1;

		// Check whether gain needs to increase or decrease
		if (osc_control->DisplayCheckOSC1)
		{
			gainUpdate1 = GetChangeToGain(OSC1Data.GetMiniBuffer());
		}
		
		if (osc_control->DisplayCheckOSC2)
		{
			gainUpdate2 = GetChangeToGain(OSC2Data.GetMiniBuffer());
		}
		if (osc_control->DisplayCheckOSC1 == false && osc_control->DisplayCheckOSC2 == false)
		{
			gainUpdate1 = 0;
			gainUpdate2 = 0;
		}

		bool changed = false;
		// Zoom out if either OSC data is clipping
		if (gainUpdate1 == -1 || gainUpdate2 == -1)
		{
			changed = true;
			currentLabOscGain--;
		}
		// Zoom in if both OSC datas are within threshold
		else if (gainUpdate1 == 1 && gainUpdate2 == 1)
		{
			changed = true;
			currentLabOscGain++;
		}

		if (changed)
		{
			librador_set_oscilloscope_gain(1 << currentLabOscGain);
			last_update_frame = frame;
#ifndef NDEBUG
			printf("Frame: %03d, gain: %02d\n", frame, 1 << currentLabOscGain);
#endif
		}
	}

	void renderControl() override
	{
		return; // no control
	}

	bool controlLab() override
	{
		return false; // no control
	}

private:
	const double adc_center = 3.3 / 2;
	const double gain_scale = 0.35;
	const double threshold_samples_frac = 0.1;
	
	int GetChangeToGain(
	    std::vector<double> osc_data)
	{
		// Check whether gain needs to decrease (zoom out, prevent clipping of larger waveforms)
		if (currentLabOscGain > 1)
		{
			const double offset = (1 << (6 - currentLabOscGain)) * gain_scale;
			const double max_thresh = adc_center + offset;
			const double min_thresh = adc_center - offset;

			// counts number of values which we identify to potentially be clipping
			int max_count = std::count_if(osc_data.begin(), osc_data.end(),
			    [max_thresh](double x) { return x > max_thresh; });
			int min_count = std::count_if(osc_data.begin(), osc_data.end(),
			    [min_thresh](double x) { return x < min_thresh; });
			if (max_count > threshold_samples_frac * osc_data.size()
			    || min_count > threshold_samples_frac * osc_data.size())
			{
				return -1; // decrement gain
			}
		}
		// Check if gain needs to increase (zoom in, for smaller waveforms)
		if (currentLabOscGain < 6)
		{
			const double offset = (1 << (5 - currentLabOscGain)) * gain_scale;
			const double max_thresh = adc_center + offset;
			const double min_thresh = adc_center - offset;
			const double grace = 0.2 * offset; // only apply to min game to prevent repetitive switching

			// counts number of values outside of the thresholds of the smaller gain
			int max_count = std::count_if(osc_data.begin(), osc_data.end(),
			    [max_thresh, grace](double x) { return x > max_thresh - grace; });
			int min_count = std::count_if(osc_data.begin(), osc_data.end(),
			    [min_thresh, grace](double x) { return x < min_thresh + grace; });
			if (max_count < threshold_samples_frac * osc_data.size()
			    && min_count < threshold_samples_frac * osc_data.size())
			{
				return 1; // increment gain
			}
		}
		
		return 0; // no change

	}

protected:
	ImVec2 size;
	bool paused = false;
	bool trig_dragging = false;
	bool trig_double_clicked = false;
	bool next_resetX = false;
	bool next_resetY = false;
	bool next_autofitY = false;
	bool next_autofitX = false;
	double next_autofitX_min = 0;
	double next_autofitX_max = 0;
	double next_autofitY_min = 0;
	double next_autofitY_max = 0;
	double init_time_range_lower = -0.015;
	double init_time_range_upper = 0.015;
	double init_voltage_range_lower = -1.0;
	double init_voltage_range_upper = 5.0;
	OSCControl* osc_control;
	OscData OSC1Data = OscData(1);
	OscData OSC2Data = OscData(2);
	OscData MathData = OscData(0);
	double cursor1_x = -1000;
	double cursor1_y = -1000;
	double cursor2_x = -1000;
	double cursor2_y = -1000;
	double trigger_time_plot = 0;
	int last_update_frame = 10;
};