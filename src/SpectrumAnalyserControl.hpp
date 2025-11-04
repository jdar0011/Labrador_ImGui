#pragma once
#include "ControlWidget.hpp"
#include "UIComponents.hpp"
#include "util.h"
#include <array>
#include <string>
#include <chrono>
#include <algorithm>
#include "implot.h"

using Clock = std::chrono::steady_clock;

class SpectrumAnalyserControl : public ControlWidget {
public:
    struct SpectrumAcquireState {
        bool acquiring = false;
        float  max_time_s = 1.0f;
        double elapsed_last_s = 0.0;
        Clock::time_point t0{};
        double success_until_time = 0.0;
    };

    struct {
        bool On = false;
        bool AcquisitionExists = false;
        int  SignalComboCurrentItem = 0;
        bool Acquire = false;
        int  SampleRatesComboCurrentItem = 0;
        int  WindowComboCurrentItem = 0;
        float TimeWindow = 1.f;
        float LookbackTimeWindow = 1.f;
        const char* SignalComboList[2] = { "OSC1", "OSC2" };

        std::array<const char*, 56> SampleRatesComboList{};
        std::array<std::string, 56> SampleRatesLabels{};
        std::array<int, 56> SampleRatesValues{};

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
    } SA;

    ImColor SpectrumAnalyserColour = colourConvert(constants::SPECTRUM_ANALYSER_ACCENT);

    SpectrumAnalyserControl(const char* label, ImVec2 size, const float* borderColor)
        : ControlWidget(label, size, borderColor), label_(label) {
        buildSampleRatesCombo();
    }

    void renderControl() override {
        ImGui::SeparatorText("Spectrum Analyzer (FFT)");
        ImGui::Text("Display"); ImGui::SameLine();
        ToggleSwitch("##SpectrumAnalyserOn", &SA.On, ImU32(SpectrumAnalyserColour));

        ImGui::Text("Signal");
        ImGui::Combo("##SpectrumAnalyserSignalCombo", &SA.SignalComboCurrentItem,
            SA.SignalComboList, IM_ARRAYSIZE(SA.SignalComboList));

        switch (SA.AcquisitionModeComboCurrentItem) {
        case 0: // gated
            if (DrawAcquireButton(SA.AcquireState)) {
                SA.Acquire = true;
                SA.TimeWindow = std::clamp(SA.AcquireState.elapsed_last_s, 0.01, (double)SA.AcquireState.max_time_s);
            }
            else SA.Acquire = false;
            break;
        case 1: // lookback
            SA.Acquire = WhiteOutlineButton("Acquire", ImVec2(100, 30));
            if (SA.Acquire) SA.TimeWindow = SA.LookbackTimeWindow;
            break;
        }
        ImGui::SameLine();
        SA.Autofit = WhiteOutlineButton("Auto Fit##SpectrumAnalyser", ImVec2(100, 30));

        if (SA.AcquisitionExists) {
            if (SA.SignalComboCurrentItem == 0) ImGui::TextDisabled("Last captured: %.2f s", SA.OSC1_last_captured);
            else                                 ImGui::TextDisabled("Last captured: %.2f s", SA.OSC2_last_captured);
        }

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Advanced Options##SpectrumOptions", ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (ImGui::BeginTable("SpectrumAdvTbl", 2, ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.0f);
                ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                auto row = [&](const char* label, auto widget) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(label);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(SA.ItemWidth > 0 ? SA.ItemWidth : -FLT_MIN);
                    widget();
                };

                row("Sample Rate", [&] {
                    ImGui::Combo("##SampleRate",
                        &SA.SampleRatesComboCurrentItem,
                        SA.SampleRatesComboList.data(),
                        (int)SA.SampleRatesComboList.size());
                });

                if (SA.AcquisitionModeComboCurrentItem == 0) {
                    row("Max Time Window", [&] {
                        ImGui::SliderFloat("##MaxTimeWindow", &SA.AcquireState.max_time_s, 0.1f, 20.0f, "%.1f s");
                    });
                }
                else {
                    row("Time Window", [&] {
                        ImGui::SliderFloat("##TimeWindow", &SA.LookbackTimeWindow, 0.1f, 20.0f, "%.1f s");
                    });
                }

                row("Windowing Function", [&] {
                    ImGui::Combo("##Windowing", &SA.WindowComboCurrentItem, SA.WindowComboList, IM_ARRAYSIZE(SA.WindowComboList));
                });

                row("Vertical Units", [&] {
                    ImGui::Combo("##Vertical Units", &SA.UnitsComboCurrentItem, SA.UnitsComboList, IM_ARRAYSIZE(SA.UnitsComboList));
                });

                row("Acquisition Mode", [&] {
                    ImGui::Combo("##AcquisitionType", &SA.AcquisitionModeComboCurrentItem,
                        SA.AcquisitionModeComboList, IM_ARRAYSIZE(SA.AcquisitionModeComboList));
                });

                ImGui::EndTable();
            }
        }
    }

    bool controlLab() override { return false; }

private:
    const std::string label_;

    static bool DrawAcquireButton(SpectrumAcquireState& st, const char* base_label = "Start Acquiring") {
        double elapsed = 0.0;
        if (st.acquiring) {
            elapsed = std::chrono::duration<double>(Clock::now() - st.t0).count();
            if (elapsed >= st.max_time_s) {
                st.acquiring = false;
                st.elapsed_last_s = st.max_time_s;
                st.success_until_time = ImGui::GetTime() + 1.2;
            }
        }

        static float kAcquireBtnWidth = 0.0f;
        {
            const ImGuiStyle& s = ImGui::GetStyle();
            float w1 = ImGui::CalcTextSize(base_label).x;
            float w2 = ImGui::CalcTextSize("Acquiring: 9999.9 s").x;
            float w3 = ImGui::CalcTextSize(u8"\u2713  9999.99 s").x;
            float need = ImMax(ImMax(w1, w2), w3) + s.FramePadding.x * 2.0f + 8.0f;
            if (need > kAcquireBtnWidth) kAcquireBtnWidth = need;
        }

        bool pressed = WhiteOutlineButton("##AcquireBtn", ImVec2(kAcquireBtnWidth, 30));
        ImVec2 rmin = ImGui::GetItemRectMin();
        ImVec2 rmax = ImGui::GetItemRectMax();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float rounding = ImGui::GetStyle().FrameRounding;

        if (st.acquiring) {
            float t = (float)std::min(elapsed / st.max_time_s, 1.0);
            ImU32 col = ImGui::GetColorU32(ImVec4(0.30f, 0.75f, 0.40f, 0.35f));
            ImVec2 a = ImVec2(rmin.x + 1.0f, rmin.y + 1.0f);
            ImVec2 b = ImVec2(rmin.x + 1.0f + t * (rmax.x - rmin.x - 2.0f), rmax.y - 1.0f);
            dl->AddRectFilled(a, b, col, rounding);
        }

        const double now_ui = ImGui::GetTime();
        const bool show_success = (!st.acquiring && now_ui < st.success_until_time);

        char label_buf[96];
        ImU32 txt_col = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
        if (show_success) {
            std::snprintf(label_buf, sizeof(label_buf), u8"\u2713  %.2f s", st.elapsed_last_s);
            txt_col = ImGui::GetColorU32(ImVec4(0.20f, 0.90f, 0.40f, 1.0f));
        }
        else if (st.acquiring) {
            std::snprintf(label_buf, sizeof(label_buf), "Acquiring: %.1f s", elapsed);
        }
        else {
            std::snprintf(label_buf, sizeof(label_buf), "%s", base_label);
        }

        ImVec2 ts = ImGui::CalcTextSize(label_buf);
        ImVec2 c = ImVec2((rmin.x + rmax.x - ts.x) * 0.5f, (rmin.y + rmax.y - ts.y) * 0.5f);
        dl->AddText(c, txt_col, label_buf);

        bool do_capture = false;
        if (pressed) {
            if (!st.acquiring) {
                st.acquiring = true; st.t0 = Clock::now();
            }
            else {
                st.acquiring = false;
                st.elapsed_last_s = std::max(0.0, std::chrono::duration<double>(Clock::now() - st.t0).count());
                st.success_until_time = ImGui::GetTime() + 1.2;
                do_capture = true;
            }
        }
        if (!st.acquiring && elapsed >= st.max_time_s && st.elapsed_last_s > 0.0) do_capture = true;
        return do_capture;
    }

    void buildSampleRatesCombo() {
        auto fmt_hz_khz = [](int hz)->std::string {
            if (hz >= 1000) {
                if (hz % 1000 == 0) return std::to_string(hz / 1000) + " kHz";
                char buf[32]; std::snprintf(buf, sizeof(buf), "%.1f kHz", hz / 1000.0); return std::string(buf);
            }
            else return std::to_string(hz) + " Hz";
        };
        for (int i = 0; i < 56; ++i) {
            SA.SampleRatesLabels[i] = fmt_hz_khz(constants::DIVISORS_375000[55 - i]);
            SA.SampleRatesComboList[i] = SA.SampleRatesLabels[i].c_str();
            SA.SampleRatesValues[i] = constants::DIVISORS_375000[55 - i];
        }
    }
};
