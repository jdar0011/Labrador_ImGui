#pragma once
#include "ControlWidget.hpp"
#include "UIComponents.hpp"
#include "util.h"
#include "implot.h"
#include "NetworkAnalyser.hpp"
#include <string>
#include <algorithm>

class NetworkAnalyserControl : public ControlWidget {
public:
    struct NetworkAcquireState {
        bool acquiring = false;
        double t_start_s = 0.0;
        double elapsed_last_s = 0.0;
        double success_until_time = 0.0;
    };

    struct {
        bool Display = false;
        bool Acquire = false;
        const char* SignalGenComboList[2] = { "SG1", "SG2" };
        const char* OscComboList[2] = { "OSC1", "OSC2" };
        int  StimulusComboCurrentItem = 0;
        int  InputComboCurrentItem = 0;
        int  ResponseComboCurrentItem = 1;
        float ComboBoxWidth = 100.0f;
        bool PhaseOn = false;
        float ItemWidth = 100.0f;
        double f_start = 10;
        double f_stop = 1000;
        const char* UnitsComboList[2] = { "dbV", "V RMS" };
        int  UnitsComboCurrentItem = 0;
        bool Autofit = false;
        bool AcquisitionExists = false;
        int  PointSpacingComboCurrentItem = 0;
        const char* PointSpacingComboList[2] = { "Logarithmic", "Linear" };
    } NA;

    ImColor NetworkAnalyserColour = colourConvert(constants::NETWORK_ANALYSER_ACCENT);

    NetworkAnalyserControl(const char* label, ImVec2 size, const float* borderColor)
        : ControlWidget(label, size, borderColor), label_(label) {
    }

    void SetNetworkAnalyser(NetworkAnalyser* na, NetworkAnalyser::Config* cfg) {
        na_ = na; cfg_ = cfg;
    }

    void renderControl() override {
        ImGui::SeparatorText("Network Analyzer");

        ImGui::Text("Display"); ImGui::SameLine();
        ToggleSwitch("##NetworkAnalyserOn", &NA.Display, ImU32(NetworkAnalyserColour));
        ImGui::SameLine(); ImGui::Text("Phase On"); ImGui::SameLine();
        ToggleSwitch("##NetworkAnalyserPhaseOn", &NA.PhaseOn, ImU32(NetworkAnalyserColour));

        ImGui::Text("Stimulus Generator"); ImGui::SameLine();
        ImGui::SetNextItemWidth(NA.ComboBoxWidth);
        ImGui::Combo("##NetworkAnalyserGenCombo", &NA.StimulusComboCurrentItem,
            NA.SignalGenComboList, IM_ARRAYSIZE(NA.SignalGenComboList));

        ImGui::Text("Reference (Input) Channel"); ImGui::SameLine();
        int& in = NA.InputComboCurrentItem;
        int& out = NA.ResponseComboCurrentItem;
        in = (in != 0); out = (out != 0);
        if (in == out) out = 1 - in;
        ImGui::SetNextItemWidth(NA.ComboBoxWidth);
        bool changed_in = ImGui::Combo("##NetworkAnalyserInputCombo", &in, NA.OscComboList, IM_ARRAYSIZE(NA.OscComboList));
        if (changed_in) out = 1 - in;

        ImGui::Text("Response (Output) Channel"); ImGui::SameLine();
        ImGui::SetNextItemWidth(NA.ComboBoxWidth);
        bool changed_out = ImGui::Combo("##NetworkAnalyserResponseCombo", &out, NA.OscComboList, IM_ARRAYSIZE(NA.OscComboList));
        if (changed_out) in = 1 - out;

        // push into cfg if present
        if (cfg_) {
            cfg_->gen_channel = NA.StimulusComboCurrentItem + 1; // SG1=1, SG2=2
            cfg_->ch_input = (in == 0 ? 1 : 2);
            cfg_->ch_output = (out == 0 ? 1 : 2);
        }

        RangeSliderDoubleLog("Frequency Range", &NA.f_start, &NA.f_stop,
            1, 5000, 0.1, true, true, 0.0f, 5.0f, 8.0f);
        if (cfg_) { cfg_->f_start = NA.f_start; cfg_->f_stop = NA.f_stop; cfg_->use_ifbw_limits = false; }

        // acquire button with overlay/progress
        static NetworkAcquireState st;
        if (DrawNetworkAcquireButton(st, na_, cfg_, ImVec2(200, 30))) {
#ifndef NDEBUG
            printf("Network Analyser acquisition started\n");
#endif
        }
        ImGui::SameLine();
        NA.Autofit = WhiteOutlineButton("Auto Fit##NetworkAnalyser", ImVec2(100, 30));

        ImGui::SetNextItemOpen(false, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Advanced Options##NetworkOptions", ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (ImGui::BeginTable("NetworkAdvTbl", 2, ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.0f);
                ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

                auto row = [&](const char* label, auto widget) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(label);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(NA.ItemWidth > 0 ? NA.ItemWidth : -FLT_MIN);
                    widget();
                };

                row("Vertical Units", [&] {
                    ImGui::Combo("##Vertical Units", &NA.UnitsComboCurrentItem, NA.UnitsComboList, IM_ARRAYSIZE(NA.UnitsComboList));
                });

                row("Number of Data Points", [&] {
                    if (cfg_) ImGui::SliderInt("##Points", &cfg_->points, 2, 501, "%d",
                        ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat);
                    else { int dummy = 201; ImGui::SliderInt("##Points", &dummy, 2, 501); }
                });

                row("Point Spacing", [&] {
                    ImGui::Combo("##PointSpacing", &NA.PointSpacingComboCurrentItem, NA.PointSpacingComboList, IM_ARRAYSIZE(NA.PointSpacingComboList));
                });
                if (cfg_) cfg_->log_spacing = (NA.PointSpacingComboCurrentItem == 0);

                ImGui::EndTable();
            }
        }
    }

    bool controlLab() override { return false; }

private:
    const std::string label_;
    NetworkAnalyser* na_ = nullptr;
    NetworkAnalyser::Config* cfg_ = nullptr;

    // Log-range slider (same implementation you had; kept inline)
    // Log-range slider (same implementation you had; kept inline)
    inline bool RangeSliderDoubleLog(const char* label,
        double* v_lower, double* v_upper,
        double v_min, double v_max,
        double min_span_hz = 0.0, bool auto_swap = true, bool snap_125_on_rel = false,
        float widget_width = 0.0f, float bar_thickness_px = 4.0f, float knob_radius_px = 7.0f,
        float top_label_margin_px = 4.0f, float left_pad_px = 0.0f, float right_pad_px = 16.0f)
    {
        IM_ASSERT(v_lower && v_upper);
        if (!(v_min > 0.0 && v_min < v_max)) return false;

        auto clampd = [](double x, double a, double b) { return x < a ? a : (x > b ? b : x); };
        auto snap125 = [](double x)->double {
            if (x <= 0) return x;
            double e = std::floor(std::log10(x));
            double m = x / std::pow(10.0, e);
            double t = (m < 1.5) ? 1.0 : (m < 3.5 ? 2.0 : (m < 7.5 ? 5.0 : 10.0));
            return t * std::pow(10.0, e);
        };
        auto format_hz_si = [](char* buf, size_t n, double hz) {
            const char* unit = "Hz"; double v = hz;
            if (hz >= 1e9) { v = hz / 1e9; unit = "GHz"; }
            else if (hz >= 1e6) { v = hz / 1e6; unit = "MHz"; }
            else if (hz >= 1e3) { v = hz / 1e3; unit = "kHz"; }
            std::snprintf(buf, n, "%.4g %s", v, unit);
        };
        struct LogMap {
            double lmin, lmax, inv_span;
            LogMap(double vmin, double vmax) { lmin = std::log10(vmin); lmax = std::log10(vmax); inv_span = 1.0 / (lmax - lmin); }
            double v_to_t(double v) const { return (std::log10(v) - lmin) * inv_span; }
            double t_to_v(double t) const { return std::pow(10.0, lmin + (lmax - lmin) * t); }
        };

        double lo = clampd(*v_lower, v_min, v_max);
        double hi = clampd(*v_upper, v_min, v_max);
        auto apply_constraints = [&](double& a, double& b) {
            a = clampd(a, v_min, v_max); b = clampd(b, v_min, v_max);
            if (auto_swap) {
                if (a > b) std::swap(a, b);
                if (min_span_hz > 0 && b - a < min_span_hz) b = clampd(a + min_span_hz, v_min, v_max);
            }
            else {
                if (min_span_hz > 0) {
                    if (a > b - min_span_hz) a = b - min_span_hz;
                    if (b < a + min_span_hz) b = a + min_span_hz;
                }
                a = clampd(a, v_min, v_max); b = clampd(b, v_min, v_max);
            }
        };
        apply_constraints(lo, hi);

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { *v_lower = lo; *v_upper = hi; return false; }

        const ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiID id = ImGui::GetID(label);

        ImGui::BeginGroup();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);

        // First label format pass (reserve height)
        char lbl_lo[32], lbl_hi[32];
        format_hz_si(lbl_lo, sizeof(lbl_lo), lo);
        format_hz_si(lbl_hi, sizeof(lbl_hi), hi);
        ImVec2 sz_lo = ImGui::CalcTextSize(lbl_lo), sz_hi = ImGui::CalcTextSize(lbl_hi);
        const float labels_h = (sz_lo.y > sz_hi.y ? sz_lo.y : sz_hi.y);

        float avail = ImGui::GetContentRegionAvail().x;
        float inner_w = (widget_width > 0.0f ? widget_width : avail);
        inner_w = ImMax(1.0f, inner_w - (left_pad_px + right_pad_px));

        const float grab_w = knob_radius_px * 2.0f;
        const float base_h = ImMax(knob_radius_px * 2.0f + 4.0f, bar_thickness_px + 8.0f);
        const float widget_h = labels_h + top_label_margin_px + base_h;

        ImVec2 pos = ImGui::GetCursorScreenPos(); pos.x += left_pad_px;
        ImVec2 size(inner_w, widget_h);
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) { *v_lower = lo; *v_upper = hi; ImGui::EndGroup(); return false; }

        const float bar_y = bb.Max.y - (base_h * 0.5f);
        const float bar_x0 = bb.Min.x + grab_w * 0.5f;
        const float bar_x1 = bb.Max.x - grab_w * 0.5f;
        const float bar_w = ImMax(1.0f, bar_x1 - bar_x0);

        LogMap lm(v_min, v_max);
        auto v_to_x = [&](double v)->float { return (float)(bar_x0 + lm.v_to_t(v) * bar_w); };
        auto x_to_v = [&](float x)->double { return lm.t_to_v(clampd((x - bar_x0) / bar_w, 0.0, 1.0)); };

        float x_lo = v_to_x(lo), x_hi = v_to_x(hi);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 col_bar_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
        const ImU32 col_bar_sel = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
        const ImU32 col_grab = ImGui::GetColorU32(ImGuiCol_SliderGrab);
        const ImU32 col_grab_a = ImGui::GetColorU32(ImGuiCol_SliderGrabActive);
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

        ImGui::PushID(id);
        ImRect lo_rect(ImVec2(x_lo - grab_w * 0.5f, bb.Min.y), ImVec2(x_lo + grab_w * 0.5f, bb.Max.y));
        ImGui::SetCursorScreenPos(lo_rect.Min);
        ImGui::InvisibleButton("lo", lo_rect.GetSize());
        bool lo_active = ImGui::IsItemActive(), lo_hover = ImGui::IsItemHovered();

        ImRect hi_rect(ImVec2(x_hi - grab_w * 0.5f, bb.Min.y), ImVec2(x_hi + grab_w * 0.5f, bb.Max.y));
        ImGui::SetCursorScreenPos(hi_rect.Min);
        ImGui::InvisibleButton("hi", hi_rect.GetSize());
        bool hi_active = ImGui::IsItemActive(), hi_hover = ImGui::IsItemHovered();

        // Click on track to pick nearest handle
        if (!lo_active && !hi_active && ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(bb.Min, bb.Max)) {
            float mx = ImGui::GetIO().MousePos.x;
            if (fabsf(mx - x_lo) <= fabsf(mx - x_hi)) { ImGui::SetActiveID(ImGui::GetID("lo"), window); lo_active = true; }
            else { ImGui::SetActiveID(ImGui::GetID("hi"), window); hi_active = true; }
        }

        bool changed_now = false;
        if (lo_active) { lo = clampd(x_to_v(ImGui::GetIO().MousePos.x), v_min, v_max); apply_constraints(lo, hi); changed_now = true; }
        if (hi_active) { hi = clampd(x_to_v(ImGui::GetIO().MousePos.x), v_min, v_max); apply_constraints(lo, hi); changed_now = true; }

        // Knobs
        x_lo = v_to_x(lo); x_hi = v_to_x(hi);
        dl->AddCircleFilled(ImVec2(x_lo, bar_y), knob_radius_px, (lo_active || lo_hover) ? col_grab_a : col_grab, 20);
        dl->AddCircleFilled(ImVec2(x_hi, bar_y), knob_radius_px, (hi_active || hi_hover) ? col_grab_a : col_grab, 20);

        // Second label format pass (reuse the SAME buffers — no redefinition)
        format_hz_si(lbl_lo, sizeof(lbl_lo), lo);
        format_hz_si(lbl_hi, sizeof(lbl_hi), hi);
        sz_lo = ImGui::CalcTextSize(lbl_lo); sz_hi = ImGui::CalcTextSize(lbl_hi);
        const float text_baseline = bb.Min.y + (sz_lo.y > sz_hi.y ? sz_lo.y : sz_hi.y);
        dl->AddText(ImVec2(x_lo - sz_lo.x * 0.5f, text_baseline - sz_lo.y), col_text, lbl_lo);
        dl->AddText(ImVec2(x_hi - sz_hi.x * 0.5f, text_baseline - sz_hi.y), col_text, lbl_hi);

        static bool lo_was = false, hi_was = false;
        if (lo_was && !lo_active && snap_125_on_rel) { lo = snap125(lo); apply_constraints(lo, hi); changed_now = true; }
        if (hi_was && !hi_active && snap_125_on_rel) { hi = snap125(hi); apply_constraints(lo, hi); changed_now = true; }
        lo_was = lo_active; hi_was = hi_active;

        ImGui::PopID();
        ImGui::EndGroup();

        bool any_change = changed_now || (lo != *v_lower) || (hi != *v_upper);
        *v_lower = lo; *v_upper = hi;
        return any_change;
    }


    static bool DrawNetworkAcquireButton(
        NetworkAcquireState& st, NetworkAnalyser* na, NetworkAnalyser::Config* cfg,
        ImVec2 size = ImVec2(200, 30))
    {
        const double now = ImGui::GetTime();
        bool clicked = false;

        if (st.acquiring && na) {
            st.elapsed_last_s = now - st.t_start_s;
            if (na->done()) {
                st.acquiring = false;
                st.success_until_time = now + 1.2;
            }
        }

        char label[128];
        if (st.acquiring && na && na->CurrentConfig().points > 0) {
            const int total = na->CurrentConfig().points;
            const int done = std::clamp(na->current_index(), 0, total);
            std::snprintf(label, sizeof(label), "Acquiring: %d / %d (%.1f s)", done, total, st.elapsed_last_s);
            ImVec2 p_min = ImGui::GetCursorScreenPos();
            ImVec2 p_max = ImVec2(p_min.x + size.x, p_min.y + size.y);
            float w = (float)((p_max.x - p_min.x) * ((double)done / (double)total));
            ImGui::GetWindowDrawList()->AddRectFilled(p_min, ImVec2(p_min.x + w, p_max.y),
                IM_COL32(80, 255, 100, 120), ImGui::GetStyle().FrameRounding);
        }
        else if (now < st.success_until_time) {
            std::snprintf(label, sizeof(label), u8"Completed in %.1f s  \u2713", st.elapsed_last_s);
        }
        else {
            const double est = na ? na->EstimateSweepSeconds_UI(*cfg) : 0.0;
            std::snprintf(label, sizeof(label), "Acquire (est. %.0f s)", est);
        }

        if (WhiteOutlineButton(label, size)) {
            if (!st.acquiring && na) {
                na->StartSweep(*cfg);
                st.acquiring = true; st.t_start_s = now; st.elapsed_last_s = 0.0; st.success_until_time = 0.0;
                clicked = true;
            }
        }

        if (st.acquiring && na && na->CurrentConfig().points > 0) {
            const int total = na->CurrentConfig().points;
            const int done = std::clamp(na->current_index(), 0, total);
            ImVec2 r_min = ImGui::GetItemRectMin();
            ImVec2 r_max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(
                r_min, ImVec2(r_min.x + (r_max.x - r_min.x) * ((double)done / (double)total), r_max.y),
                IM_COL32(80, 255, 100, 120), ImGui::GetStyle().FrameRounding);
        }

        return clicked;
    }
};
