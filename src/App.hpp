// Source: https://github.com/ZenSepiol/Dear-ImGui-App-Framework/blob/main/src/app/sample_app/app.hpp
#pragma once
#include "AppBase.hpp"
#include "implot_internal.h"
#include <iostream>
#include <string>
#include "librador.h"
#include "PSUControl.hpp"
#include "OSCControl.hpp"
#include "SGControl.hpp"
#include "PlotWidget.hpp"
#include "PlotControl.hpp"
#include "util.h"
#include <filesystem>
#include <stdlib.h> 
#include <chrono>
#include <thread>
#ifdef NDEBUG
#include <Windows.h>
int windows_system(const char* cmd) {
	PROCESS_INFORMATION p_info;
	STARTUPINFO s_info;
	DWORD ReturnValue = NULL;

	memset(&s_info, 0, sizeof(s_info));
	memset(&p_info, 0, sizeof(p_info));
	s_info.cb = sizeof(s_info);

	if (CreateProcess(NULL,(LPSTR)cmd, NULL, NULL, 0, CREATE_NO_WINDOW, NULL, NULL, &s_info, &p_info)) {
		WaitForSingleObject(p_info.hProcess, INFINITE);
		GetExitCodeProcess(p_info.hProcess, &ReturnValue);
		CloseHandle(p_info.hProcess);
		CloseHandle(p_info.hThread);
	}
	return ReturnValue;
}
#endif

// #define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

// Global textures to be loaded once during start-up
int constants::pinout_width;
int constants::pinout_height;
intptr_t constants::psu_pinout_texture = 0;
intptr_t constants::sg_pinout_texture;
intptr_t constants::osc_pinout_texture;

class App : public AppBase<App>
{
  public:
    App(){};
    virtual ~App() = default;

	void connectToLabrador(bool * flash_firmware_popup)
	{
		// Initialise the USB
		int error = librador_setup_usb();
		if (error)
		{
#ifndef NDEBUG
			printf("librador_setup_usb FAILED with error code %d\t\n", error);
#endif
			// std::exit(error);
			connected = false;
		}
		else
			connected = true;
		if (connected)
		{
#ifndef NDEBUG
			printf("Device Connected Successfully!\n");
#endif

			// Print firmware info
			uint16_t deviceVersion = librador_get_device_firmware_version();
			uint8_t deviceVariant = librador_get_device_firmware_variant();

			*flash_firmware_popup = deviceVariant != 2;
#ifndef NDEBUG
			printf("deviceVersion=%hu, deviceVariant=%hhu\n", deviceVersion, deviceVariant);
#endif

			// Reset Signal Generators
			SG1Widget.reset();
			SG2Widget.reset();
		}
		librador_set_device_mode(2);

		// cannot run too often as this causes the usb sampling bug
		librador_set_oscilloscope_gain(16);
	}
	
    // Anything that needs to be called once OUTSIDE of the main application loop
    void StartUp()
    {
		int error = librador_init();
		if (error)
		{
#ifndef NDEBUG
			printf("librador_init FAILED with error code %d\t\n", error);
#endif
			// std::exit(error);
			connected = false;
		}

		// Load pinout images
		GLuint psu_tmp_texture = 0;
		GLuint sg_tmp_texture = 0;
		GLuint osc_tmp_texture = 0;
		int w, h;

		// All textures have the same width and height (for now)
		bool psu_ret = LoadTextureFromFile(getResourcePath("media/psu-pinout.png").c_str(),
			&psu_tmp_texture, &w, &h);

		bool sg_ret = LoadTextureFromFile(getResourcePath("media/sg-pinout.png").c_str(),
			&sg_tmp_texture, &w, &h);

		bool osc_ret = LoadTextureFromFile(getResourcePath("media/osc-pinout.png").c_str(),
			&osc_tmp_texture, &w, &h);

		PSUWidget.setPinoutImg((intptr_t)psu_tmp_texture, w, h);
		SG1Widget.setPinoutImg((intptr_t)sg_tmp_texture, w, h);
		SG2Widget.setPinoutImg((intptr_t)sg_tmp_texture, w, h);
		OSCWidget.setPinoutImg((intptr_t)osc_tmp_texture, w, h);

		IM_ASSERT(psu_ret);
		IM_ASSERT(sg_ret);
		IM_ASSERT(osc_ret);

		init_constants();


		// Loads README.md contents for in-app documentation
		loadREADME();
		
		SetGlobalStyle();
    }

    // Anything that needs to be called cyclically INSIDE of the main application loop
    void Update()
    {
		
		{
			// Main window
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			ImGui::Begin("Main Window", NULL,
			    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
			        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			

			// Menu Bar
			static bool showDemoWindows = false;
			static bool showHelpWindow = false;
			static bool flash_firmware_popup = false;
			if (frames == 0) connectToLabrador(&flash_firmware_popup);
			if (flash_firmware_next_frame)
			{
				flashFirmware();
				connectToLabrador(&flash_firmware_popup);
				flash_firmware_popup = true; // keep the window open to display success
				flash_firmware_next_frame = false;
			}
			if (ImGui::BeginMenuBar())
			{

				// Device
				if (ImGui::BeginMenu("Device"))
				{
					if (connected)
					{
						// Print firmware info
						uint16_t deviceVersion = librador_get_device_firmware_version();
						uint8_t deviceVariant = librador_get_device_firmware_variant();
						ImGui::TextColored(constants::GRAY_TEXT, "Version: %hu", deviceVersion);
						ImGui::TextColored(constants::GRAY_TEXT, "Firmware: %hhu", deviceVariant);
					}
					else
					{
						ImGui::TextColored(constants::GRAY_TEXT, "No Labrador board detected");
					}
					ImGui::MenuItem("Check firmware", NULL, &flash_firmware_popup);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Help"))
				{
					ImGui::MenuItem("User Guide", NULL, &showHelpWindow);
					ImGui::EndMenu();
				}

				if (connected)
				{
					TextRight("Labrador Connected     ");
				}
				else
				{

					TextRight("No Labrador Found     ");
					if (frames % labRefreshRate == 0)
					{
#ifndef NDEBUG
						std::cout << "Attempting to connect to Labrador\n";
#endif					
						bool tmp = flash_firmware_popup;
						connectToLabrador(&flash_firmware_popup);
						if (tmp) flash_firmware_popup = true; // persistent display if it was open
					}
				}

				const ImU32 status_colour = ImGui::GetColorU32(
				connected ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1));
				const float radius = ImGui::GetTextLineHeight() * 0.4;
				const ImVec2 p1 = ImGui::GetCursorScreenPos();
				draw_list->AddCircleFilled(
					ImVec2(p1.x - 10, p1.y + ImGui::GetTextLineHeight() - radius), radius, status_colour);
				 
				ImGui::EndMenuBar();
				
				
			}

			// Apply custom padding
			const int padding = 6;
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowPadding = ImVec2(padding, padding); // Custom padding within window
			
			// Get window dimensions
			ImVec2 window_size = ImGui::GetWindowSize();
			float plot_width = (window_size.x - 2*style.WindowPadding.x) * 0.60f - padding;
			float plot_height = (window_size.y - 2*style.WindowPadding.y) * 1.00f - padding;
			
			// Left Column Widgets
			style.ItemSpacing = ImVec2(0, 0); // No padding for left and right columns
			int menu_height = ImGui::GetFrameHeight();
			ImGui::BeginChild("Left Column",
			    ImVec2(plot_width, window_size.y - 2*style.WindowPadding.y - menu_height),
			    false);
			style.ItemSpacing = ImVec2(padding, padding);
			
			// Render scope
			PlotWidgetObj.setSize(ImVec2(plot_width, plot_height));
			PlotWidgetObj.Render();
			
			ImGui::EndChild(); // End left column

			// Right column control widgets
			float control_widget_height = (window_size.y - 2*style.WindowPadding.y - 3*style.ItemSpacing.y) * 0.25f;
			
			style.ItemSpacing = ImVec2(padding, 0);
			ImGui::SameLine(); 
			ImGui::BeginChild("Right Column",ImVec2(
				window_size.x - plot_width - 2 * padding - 2 * style.WindowPadding.x,
			    window_size.y - 2 * style.WindowPadding.y - menu_height),
			    false);
			style.ItemSpacing = ImVec2(padding, padding);

			// ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(10, 10, 10, 255));
			
			// Render PSU Control
			PSUWidget.setSize(ImVec2(0, control_widget_height));
			PSUWidget.Render();

			// Render Signal Gen 1
			SG1Widget.setSize(ImVec2(0, control_widget_height));
			SG1Widget.Render();

			// Render Signal Gen 2
			SG2Widget.setSize(ImVec2(0, control_widget_height));
			SG2Widget.Render();

			// Render Oscilloscope Widget
			OSCWidget.Render();
			

			ImGui::EndChild(); // End right column
			ImGui::End();

			// Updates state of labrador to match widgets
			if (connected)
			{
				// Check connection status
				const uint8_t firmware_variant = librador_get_device_firmware_variant();
				connected = !(firmware_variant == 179 || firmware_variant == 176); // disconnected status
				if (!connected) librador_reset_usb(); // not sure if this is neccessary
				if (connected)
				{
					// Call controlLab functions for each widget
					if (frames % labRefreshRate == 0)
					{
						PSUWidget.controlLab();
					}

					// Signal generators update on change
					SG1Widget.controlLab();
					SG2Widget.controlLab();
				}
				
			}

			if (showDemoWindows)
			{
				// Show ImGui and ImPlot demo windows
				ImGui::ShowDemoWindow();
				ImPlot::ShowDemoWindow();
			}
			else SetGlobalStyle();

			if (showHelpWindow) renderHelpWindow(&showHelpWindow);
			for (ControlWidget* w : widgets)
			{
				if (w->show_help) w->renderHelp();
			}

			if (flash_firmware_popup)
			{
				// Show firmware check window
				uint8_t deviceVariant = librador_get_device_firmware_variant();
				float row_height = ImGui::GetFrameHeight();
				ImGui::SetNextWindowSize(ImVec2(450, row_height * 7));
				ImGui::PushStyleColor(ImGuiCol_TitleBgActive, (connected && deviceVariant == 2) ? ImVec4(0, 0.55, 0, 1) : ImVec4(0.8, 0, 0, 1));
				ImGui::Begin("Flash Firmware");
				if (!ImGui::IsWindowFocused())
				{
					flash_firmware_popup = false;
				}
				else
				{
					if (connected && deviceVariant != 2)
					{
						ImGui::TextWrapped("Device detected with invalid firmware variant!");
						ImGui::TextWrapped("Would you like to flash required firmware variant 2.0?");
						if (ImGui::Button(" Yes "))
						{
							ImGui::TextColored(ImVec4(1, 0, 0, 1), "Flashing variant 2.0... This takes 5 seconds.");
							flash_firmware_next_frame = true;
						}
						ImGui::SameLine();
						if (ImGui::Button(" Cancel "))
						{
							flash_firmware_popup = false;
						}

					}
					else if (connected && deviceVariant == 2)
					{
						
						ImGui::TextWrapped("Device detected with valid firmware variant (2.0)");
						if (ImGui::Button(" OK "))
						{
							flash_firmware_popup = false;
						}
					}
					else if (!connected)
					{
						ImGui::TextWrapped("No device detected.");
						if (ImGui::Button(" OK "))
						{
							flash_firmware_popup = false;
						}
					}
				}
				ImGui::PopStyleColor();
				ImGui::End();
			}
		}
		// Output framerate for performance metric
//		if (frames % 30 == 0)
//		{
//#ifndef NDEBUG
//			std::cout << ImGui::GetIO().Framerate << ',';
//#endif	
//		}
		frames++;
    }

	void ShutDown()
	{
#ifndef NDEBUG
		printf("Shutting Down\n");
#endif
		// Turn off Signal Generators
		SG1Widget.reset();
		SG2Widget.reset();
	}

	void loadREADME()
	{
		// Load documentation
		std::string filename = getResourcePath("README.md");
		std::ifstream file(filename);

		if (!file.is_open())
		{
			throw std::runtime_error("Unable to open file: " + filename);
		}

		std::stringstream buffer;
		std::string line;
		std::string curr_header = "";

		// Read until User Documentation section
		while (std::getline(file, line))
		{
			if (line == "# User Documentation") break;
		}

		while (std::getline(file, line))
		{
			
			if (line.compare(0, 4, "### ") == 0)
			{
				line.erase(0, 4);
				// Write the current buffer to the correct widget
				// Widget Label must be contained within README header
				for (ControlWidget* w : widgets)
				{
					if ((w->getLabel()).find(curr_header) != std::string::npos)
					{
						w->setHelpText(buffer.str());
					}
				}
				buffer.str("");
				buffer.clear();
				curr_header = line;
			}
			else if (line != "" && curr_header != "")
			{
				replace_all(line, "**", "");
				buffer << line << '\n';
			}
			
		}
	}

	void renderHelpWindow(bool* p_open)
	{
		
		if (!ImGui::Begin("Labrador User Guide", p_open))
		{
			ImGui::End();
			return;
		}
		if (!ImGui::IsWindowFocused())
		{
			*p_open = false;
			ImGui::End();
			return;
		}

		static char search[50] = "";
		ImGui::InputTextWithHint("##help_search", "Search...", search, 50);
		ImGui::SameLine();
		bool expand = ImGui::Button("Expand all");
		ImGui::SameLine();
		bool collapse = ImGui::Button("Collapse all");
		std::string search_str = std::string(search);

		for (ControlWidget* w : widgets)
		{
			ImGui::SeparatorText(w->getLabel().c_str());
			w->renderHelpText(expand, collapse, search_str.length() > 2 ? search_str : "");
		}
		ImGui::End();
	}

    // The callbacks are updated and called BEFORE the Update loop is entered
    // It can be assumed that inside the Update loop all callbacks have already been processed
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        // For Dear ImGui to work it is necessary to queue if the mouse signal is already processed by Dear ImGui
        // Only if the mouse is not already captured it should be used here.
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse)
        {
        }
    }

    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        // For Dear ImGui to work it is necessary to queue if the mouse signal is already processed by Dear ImGui
        // Only if the mouse is not already captured it should be used here.
        ImGuiIO& ioa = ImGui::GetIO();
        if (!ioa.WantCaptureMouse)
        {
        }
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int actions, int mods)
    {
        // For Dear ImGui to work it is necessary to queue if the keyboard signal is already processed by Dear ImGui
        // Only if the keyboard is not already captured it should be used here.
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard)
        {
        }
    }

	void flashFirmware()
	{
		// Flash Firmware Variant 2 if it is not currently flashed
		uint8_t deviceVariant = librador_get_device_firmware_variant();
		if (deviceVariant != 2 && connected)
		{
			librador_jump_to_bootloader();
#ifdef NDEBUG
			printf("deviceVariant: %hhu", librador_get_device_firmware_variant());
			std::filesystem::current_path("./firmware");
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			windows_system("dfu-programmer atxmega32a4u erase --force");
			windows_system("dfu-programmer atxmega32a4u flash labrafirm_0006_02.hex");
			windows_system("dfu-programmer atxmega32a4u launch");
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			windows_system("dfu-programmer atxmega32a4u launch");
			std::filesystem::current_path("..");
#else
			std::filesystem::current_path("./firmware");
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			system("dfu-programmer atxmega32a4u erase --force");
			system("dfu-programmer atxmega32a4u flash labrafirm_0006_02.hex");
			system("dfu-programmer atxmega32a4u launch");
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			system("dfu-programmer atxmega32a4u launch");
			std::filesystem::current_path("..");
#endif
			librador_reset_usb();
		}
	}

  private:
	int frames = 0;
	const int labRefreshRate = 60; // send controls to labrador every this many frames
	bool connected = false; // state of labrador connection
	bool flash_firmware_next_frame = false;
	
	// Define default configurations for widgets here
	PSUControl PSUWidget = PSUControl("Power Supply Unit (PSU)", ImVec2(0,0), constants::PSU_ACCENT);
	SGControl SG1Widget = SGControl("Signal Generator 1 (SG1)", ImVec2(0, 0),
	    constants::SG1_ACCENT, 1, &PSUWidget.voltage);
	SGControl SG2Widget = SGControl("Signal Generator 2 (SG2)", ImVec2(0, 0),
	    constants::SG2_ACCENT, 2, &PSUWidget.voltage);
	OSCControl OSCWidget
	    = OSCControl("Plot Settings", ImVec2(0, 0), constants::OSC_ACCENT);
	PlotWidget PlotWidgetObj 
		= PlotWidget("Plot Window",ImVec2(0, 0),&OSCWidget);
	HelpWidget TroubleShoot = HelpWidget("Troubleshooting"); // Purely for universal help
	ControlWidget* widgets[6]
	    = { &TroubleShoot, &PSUWidget, &SG1Widget, &SG2Widget, &OSCWidget, &PlotWidgetObj };
};

