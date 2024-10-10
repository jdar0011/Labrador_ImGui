#include "ControlWidget.hpp"
#include "librador.h"
#include "util.h"

/// <summary>Power Suppy Unit Widget
/// </summary>
class PSUControl : public ControlWidget
{
public:
	float voltage;

	PSUControl(std::string label, ImVec2 size, const float* borderColor)
	    : ControlWidget(label, size, borderColor)
	    , voltage(8.0f) // Default voltage
	{}

	/// <summary> 
	/// Render UI elements for power supply unit
	/// </summary>
	void renderControl() override
	{
		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		ImGui::Text("Voltage");
		ImGui::SameLine();
		ImGui::SliderFloat("##voltage", &voltage, 4.5f, 12.0f, "%.1f V");	
	}

	/// <summary>
	/// Set the Power Supply Voltage on the labrador board.
	/// </summary>
	bool controlLab() override
	{
		int error = librador_set_power_supply_voltage(voltage);
		if (error)
		{
			printf("librador_set_power_supply_voltage FAILED with error code "
				    "%d\tExiting...",
				error);
			// Board not connected (continue to run)
			if (error == -1101)
			{
				return false;
			}
			std::exit(error);
		}
		return true;
		// printf("Successfully set power supply voltage to %.2fV\n", voltage);
	}
};
