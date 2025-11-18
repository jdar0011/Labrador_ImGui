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

/// <summary>Oscilloscope Control Widget (Plot settings + Math Mode)</summary>
class OSCControl : public ControlWidget
{
public:
    // ===== Display toggles =====
    bool DisplayCheckOSC1 = true;
    bool DisplayCheckOSC2 = true;

    // ===== Export controls =====
    nfdchar_t* OSC1WritePath = NULL;
    nfdchar_t* OSC2WritePath = NULL;
    int  OSC1WritePathComboCurrentItem = 0;
    int  OSC2WritePathComboCurrentItem = 0;
    bool OSC1ClipboardCopyNext = false;  // PlotWidget can poll these each frame
    bool OSC2ClipboardCopyNext = false;
    bool OSC1ClipboardCopied = false;    // request -> �Copied!� flash
    bool OSC2ClipboardCopied = false;
    const nfdchar_t* FileExtension = "csv";

    // ===== General / Trigger =====
    float OffsetVal = 0.0f;
    bool  ACCoupledCheck = false;
    bool  Paused = false;
    bool  AutofitY = false;
    bool  AutofitX = false;
    int   TriggerTypeComboCurrentItem = 0;
    bool  Trigger = true;

    // Trigger level (SIValue control)
    SIValue TriggerLevel = SIValue("##trigger1_level", "Level",
        3.3 / 2, -20.0, 20.0, "V",
        constants::volt_prefs, constants::volt_formats);
    bool  AutoTriggerLevel = true;
    float TriggerHysteresis = 0.25f;

    // Channel UI (kept for compatibility)
    float osc1_time_scale = 5, osc1_voltage_scale = 1, osc1_offset = 0;
    float osc2_time_scale = 5, osc2_voltage_scale = 1, osc2_offset = 0;
    int   osc1_ts_unit_idx = 1, osc1_vs_unit_idx = 2, osc1_os_unit_idx = 1;
    int   osc2_ts_unit_idx = 1, osc2_vs_unit_idx = 2, osc2_os_unit_idx = 1;
    int   tl_unit_idx = 1, th_unit_idx = 1;
    bool  ts_equalise = false, vs_equalise = false, os_equalise = false;

    // UI toggles
    bool Cursor1toggle = false;
    bool Cursor2toggle = false;
    bool SignalPropertiesToggle = false;
    bool AutoTriggerHysteresisToggle = true;
    bool HysteresisDisplayOptionEnabled = false;

    // ===== Math Mode =====
    struct MathControls
    {
        std::string Text{};
        bool On{ true };
        bool Visible{ true };
        bool Parsable{ false };
    };
    MathControls MathControls1, MathControls2, MathControls3, MathControls4;

    // ===== Colours =====
    ImColor OSC1Colour = colourConvert(constants::OSC1_ACCENT);
    ImColor OSC2Colour = colourConvert(constants::OSC2_ACCENT);
    ImColor GenColour = colourConvert(constants::GEN_ACCENT);
    ImColor MathColour = colourConvert(constants::MATH_ACCENT);
    ImColor Green = ImColor(float(20. / 255), float(143. / 255), 0.f, 1.f);
    ImColor Red = ImColor(float(143. / 255), 0.f, 0.f, 1.f);

    OSCControl(const char* label, ImVec2 size, const float* borderColor)
        : ControlWidget(label, size, borderColor),
        label(label)
    {
    }

    /// Render UI elements for oscilloscope control (plot/display/trigger/export + math mode)
    void renderControl() override
    {
        // --- Top buttons ---
        if (ImGui::BeginTable("Buttons", 3))
        {
            float button_width = 100.f;
            ImGui::TableNextColumn();
            ToggleButton("Run/Stop", ImVec2(button_width, 30), &Paused, Red, Green);
            ImGui::TableNextColumn();
            AutofitY = WhiteOutlineButton(u8"Auto Fit  \u2195##Vertical", ImVec2(button_width, 30));
            ImGui::TableNextColumn();
            AutofitX = WhiteOutlineButton(u8"Auto Fit  \u2194##Horizontal", ImVec2(button_width, 30));
            ImGui::EndTable();
        }

        const float width = ImGui::GetContentRegionAvail().x * 0.95f;
        float labWidth = 120.0f;
        float controlWidth = (width - 2 * labWidth) / 2;

        // --- Display toggles ---
        ImGui::SeparatorText("Display");
        if (ImGui::BeginTable("ChannelsTable", 4))
        {
            ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_WidthFixed, labWidth);
            ImGui::TableSetupColumn("Two", ImGuiTableColumnFlags_WidthFixed, controlWidth);
            ImGui::TableSetupColumn("Three", ImGuiTableColumnFlags_WidthFixed, labWidth);
            ImGui::TableSetupColumn("Four", ImGuiTableColumnFlags_WidthFixed, controlWidth);

            ImGui::TableNextColumn(); ImGui::Text("Channel 1 (OSC1)");
            ImGui::TableNextColumn(); ToggleSwitch((label + "Display1_toggle").c_str(), &DisplayCheckOSC1, ImU32(OSC1Colour));

            ImGui::TableNextColumn(); ImGui::Text("Channel 2 (OSC2)");
            ImGui::TableNextColumn(); ToggleSwitch((label + "Display2_toggle").c_str(), &DisplayCheckOSC2, ImU32(OSC2Colour));

            ImGui::TableNextColumn(); ImGui::Text("Cursor 1");
            ImGui::TableNextColumn(); ToggleSwitch((label + "Cursor1_toggle").c_str(), &Cursor1toggle, GenColour);

            ImGui::TableNextColumn(); ImGui::Text("Cursor 2");
            ImGui::TableNextColumn(); ToggleSwitch((label + "Cursor2_toggle").c_str(), &Cursor2toggle, GenColour);

            ImGui::TableNextColumn(); ImGui::Text("Signal Properties");
            ImGui::TableNextColumn(); ToggleSwitch((label + "sig_prop_toggle").c_str(), &SignalPropertiesToggle, GenColour);

            ImGui::EndTable();
        }

        // --- General / Trigger ---
        labWidth = 100.0f;
        float labWidth2 = 70.0f;
        controlWidth = (width - labWidth - labWidth2) / 2;

        ImGui::Dummy(ImVec2(0, 10.0f));
        ImGui::SeparatorText("General");
        if (ImGui::BeginTable("GeneralTable", 4))
        {
            ImGui::TableSetupColumn("One", ImGuiTableColumnFlags_WidthFixed, labWidth);
            ImGui::TableSetupColumn("Two", ImGuiTableColumnFlags_WidthFixed, controlWidth);
            ImGui::TableSetupColumn("Three", ImGuiTableColumnFlags_WidthFixed, labWidth2);
            ImGui::TableSetupColumn("Four", ImGuiTableColumnFlags_WidthFixed, controlWidth);

            // Trigger enable
            ImGui::TableNextColumn(); ImGui::Text("Trigger");
            ImGui::TableNextColumn(); ToggleSwitch((label + "Trigger1_toggle").c_str(), &Trigger, GenColour);

            // Trigger type
            ImGui::TableNextColumn(); ImGui::Text("Type");
            ImGui::TableNextColumn(); ImGui::SetNextItemWidth(controlWidth);
            ImGui::Combo("##Trigger Type OSC1", &TriggerTypeComboCurrentItem,
                TriggerTypeComboList, IM_ARRAYSIZE(TriggerTypeComboList));

            // Trigger level
            if (!AutoTriggerLevel)
            {
                TriggerLevel.renderInTable(100.0f);
            }
            else
            {
                ImGui::TableNextColumn(); ImGui::Text("Level");
                ImGui::TableNextColumn(); ImGui::Text("%.2f V", TriggerLevel.getValue());
            }

            // Auto level toggle
            ImGui::TableNextColumn(); ImGui::Text("Auto Level");
            ImGui::TableNextColumn(); ToggleSwitch("##Auto1", &AutoTriggerLevel, GenColour);

            // Optional hysteresis UI
            if (HysteresisDisplayOptionEnabled)
            {
                ImGui::Text("Hysteresis Level");
                ImGui::TableNextColumn();
                ImGui::Text("Auto"); ImGui::SameLine(); ImGui::Text("ON"); ImGui::SameLine();
                ToggleSwitch((label + "Auto_trigger1_hysteresis_toggle").c_str(),
                    &AutoTriggerHysteresisToggle, GenColour);
                ImGui::SameLine(); ImGui::Text("OFF");
            }

            ImGui::EndTable();
        }

        // --- Export rows ---
        drawExportRow("OSC1",
            OSC1WritePathComboCurrentItem,
            OSC1ClipboardCopied, OSC1ClipboardCopiedFrame,
            OSC1ClipboardCopyNext,
            OSC1WritePath, FileExtension);

        drawExportRow("OSC2",
            OSC2WritePathComboCurrentItem,
            OSC2ClipboardCopied, OSC2ClipboardCopiedFrame,
            OSC2ClipboardCopyNext,
            OSC2WritePath, FileExtension);

        // --- Math Mode  ---
        ImGui::SeparatorText("Math");
        // ImGui::Text("Display");
        // ImGui::SameLine();
		ImGui::Text(" OFF");
		ImGui::SameLine();
        ToggleSwitch((label + "Math1_toggle").c_str(), &MathControls1.On, ImU32(MathColour));
        ImGui::SameLine();
		ImGui::Text("ON");

        ImGui::Text("Expression");
        ImGui::SameLine();
        MiniHLInput("Expression",
            MathControls1.Text,
            rules,
            /*text*/ ImGui::GetColorU32(ImGuiCol_Text),
            /*bg  */ ImGui::GetColorU32(ImGuiCol_FrameBg),
            "eg. osc1 + osc2,  sin(2*pi*100*t)"
        );

        if (!MathControls1.Parsable && MathControls1.Text.length() > 0)
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"\u2717");
            ImVec2 mn = ImGui::GetItemRectMin();
            ImVec2 mx = ImGui::GetItemRectMax();
            float pad = 6.0f;
            mn.x -= pad; mn.y -= pad; mx.x += pad; mx.y += pad;
            std::string error_tooltip = "Text not parsable!";
            if (ImGui::IsMouseHoveringRect(mn, mx))
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(error_tooltip.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }
        else if (MathControls1.Text.length())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0, 1, 0, 1), u8"\u2714");
        }
    }

    bool controlLab() override { return false; }

    // PlotWidget can poll these:
    bool ShouldCopyOSC1ToClipboard() const { return OSC1ClipboardCopyNext; }
    bool ShouldCopyOSC2ToClipboard() const { return OSC2ClipboardCopyNext; }
    void ClearCopyFlags() { OSC1ClipboardCopyNext = OSC2ClipboardCopyNext = false; }

private:
    const std::string label;

    // Trigger type list
    const char* TriggerTypeComboList[4] = {
        "OSC1 Rising Edge", "OSC1 Falling Edge",
        "OSC2 Rising Edge", "OSC2 Falling Edge"
    };

    // Export helpers
    float OSCWritePathComboWidth = 100.f;
    const char* OSCWritePathComboList[2] = { "clipboard", "csv" };
    int OSC1ClipboardCopiedFrame = -1000;
    int OSC2ClipboardCopiedFrame = -1000;
    int OSCClipboardCopiedLinger = 50;

    void drawExportRow(const char* which,
        int& destComboIdx, bool& copiedFlag, int& copiedFrame,
        bool& copyNext, nfdchar_t*& path, const nfdchar_t* ext)
    {
        std::string btnLabel = std::string("Export ") + which;
        if (copiedFlag) { copiedFrame = ImGui::GetFrameCount(); copiedFlag = false; }
        if ((ImGui::GetFrameCount() - copiedFrame) < OSCClipboardCopiedLinger) btnLabel = "Copied!";

        if (WhiteOutlineButton((btnLabel + std::string("##") + which + "ExportButton").c_str(), ImVec2(100, 30)))
        {
            if (destComboIdx == 0) // clipboard
            {
                copyNext = true;
            }
            else // csv
            {
                nfdresult_t result = NFD_SaveDialog(ext, NULL, &path);
#ifndef NDEBUG
                if (result == NFD_OKAY) { puts("Success!"); puts(path); }
                else if (result == NFD_CANCEL) { puts("User pressed cancel."); }
                else { printf("Error: %s\n", NFD_GetError()); }
#endif
            }
        }
        ImGui::SameLine();
        ImGui::Text("to");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(OSCWritePathComboWidth);
        ImGui::Combo((std::string("##") + which + "WritePathCombo").c_str(),
            &destComboIdx, OSCWritePathComboList, IM_ARRAYSIZE(OSCWritePathComboList));
    }

    // MiniHLInput rules (unchanged)
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
        {"log", constants::FunctionColour, "log(x) (natural log, ln(x))", true, true, true},
        {"abs", constants::FunctionColour, "abs(x) (absolute value, |x|)", true, true, true},
        {"sqrt",constants::FunctionColour, "sqrt(x) (square root, x^(1/2))",true, true, true},
        {"sgn", constants::FunctionColour, "sgn(x) (sign)", true, true, true},
        {"floor",constants::FunctionColour,"floor(x)",true, true, true},
        {"ceil",constants::FunctionColour, "ceil(x)", true, true, true},
        {"round",constants::FunctionColour,"round(x)",true, true, true},
        {"trunc",constants::FunctionColour,"trunc(x)",true, true, true},
        {"frac",constants::FunctionColour, "frac(x)", true, true, true},
        {"log10",constants::FunctionColour,"log10(x)",true, true, true},
        {"asin", constants::FunctionColour, "asin(x)", true, true, true},
        {"acos", constants::FunctionColour, "acos(x)", true, true, true},
        {"atan", constants::FunctionColour, "atan(x)", true, true, true},
        {"sinh", constants::FunctionColour, "sinh(x)", true, true, true},
        {"cosh", constants::FunctionColour, "cosh(x)", true, true, true},
        {"tanh", constants::FunctionColour, "tanh(x)", true, true, true},
        {"pow", constants::FunctionColour,  "pow(x,y) (x^y)", true, true, true},

        // Signals
        {"osc1", ImU32(OSC1Colour), "oscilloscope signal 1", true, true, true},
        {"osc2", ImU32(OSC2Colour), "oscilloscope signal 2", true, true, true},
		{"t",   ImU32(MathColour),  "time variable",            true, true, true},

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
        {"(",  constants::SymbolColour, "", false, false, false},
        {")",  constants::SymbolColour, "",false, false, false},
        {">",  constants::SymbolColour, "greater than", false, true, false},
        {"<",  constants::SymbolColour, "less than",    false, true, false},
        {">=", constants::SymbolColour, "greater than or equal", false, true, false},
        {"<=", constants::SymbolColour, "less than or equal",    false, true, false},
        {"!",  constants::SymbolColour, "logical NOT",           false, true, false}
    };
};
