#include "Scene.h"
#include <cmath>
#include <sstream>
#include <exprtk.hpp>
#include <windows.h>
#include <complex>
#include "Fourier.h"

static inline ImU32 RGBA(const ImVec4& c) {
    return IM_COL32(int(c.x * 255), int(c.y * 255), int(c.z * 255), int(c.w * 255));
}

struct Scene::Impl {
    using symbol_table_t = exprtk::symbol_table<float>;
    using expression_t = exprtk::expression<float>;
    using parser_t = exprtk::parser<float>;

    symbol_table_t symbols;
    expression_t   expression;
    parser_t       parser;
    float varX = 0.0f;
    bool valid = false;
    std::string lastError;

    Impl() {
        symbols.add_variable("x", varX);
        symbols.add_constants();
        expression.register_symbol_table(symbols);
    }
};

Scene::Scene() : impl(std::make_unique<Impl>()) {}
Scene::~Scene() = default;   // now compiler sees full Impl type

void Scene::SetExpression(const std::string& expr) {
    impl->valid = impl->parser.compile(expr, impl->expression);
    if (!impl->valid) {
        std::ostringstream oss;
        oss << "Parse error in expression: " << expr << "\n";
        for (std::size_t i = 0; i < impl->parser.error_count(); ++i) {
            auto e = impl->parser.get_error(i);
            oss << "Error " << i
                << " at pos " << e.token.position
                << " [" << exprtk::parser_error::to_str(e.mode)
                << "] " << e.diagnostic << "\n";
        }
        impl->lastError = oss.str();
        //MessageBoxA(nullptr, oss.str().c_str(), "Expression Error", MB_OK | MB_ICONERROR);
    }
}

float Scene::Eval(float x) {
    if (!impl->valid) return 0.0f;
    impl->varX = x;
    return impl->expression.value();
}

void Scene::DrawBackground(const ImVec2& windowSize, const AppConfig& cfg) {
    const float centerX = windowSize.x * 0.5f;
    const float centerY = windowSize.y * 0.5f;
    const float step = (cfg.gridScale > 0 ? cfg.gridSpacing * cfg.gridScale : cfg.gridSpacing);

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    auto colGrid = IM_COL32(cfg.gridColor.x * 255, cfg.gridColor.y * 255, cfg.gridColor.z * 255, cfg.gridColor.w * 255);
    auto colAxis = IM_COL32(cfg.axisColor.x * 255, cfg.axisColor.y * 255, cfg.axisColor.z * 255, cfg.axisColor.w * 255);

    const int nX = int(windowSize.x / step) + 1;
    const int nY = int(windowSize.y / step) + 1;

    // vertical grid
    for (int i = -nX; i <= nX; ++i) {
        float x = centerX + i * step;
        dl->AddLine(ImVec2(x, 0), ImVec2(x, windowSize.y), colGrid);
    }
    // horizontal grid
    for (int i = -nY; i <= nY; ++i) {
        float y = centerY + i * step;
        dl->AddLine(ImVec2(0, y), ImVec2(windowSize.x, y), colGrid);
    }

    // axes
    dl->AddLine(ImVec2(0, centerY), ImVec2(windowSize.x, centerY), colAxis);
    dl->AddLine(ImVec2(centerX, 0), ImVec2(centerX, windowSize.y), colAxis);

    // arrows
    dl->AddTriangleFilled({ windowSize.x - 10, centerY - 5 }, { windowSize.x, centerY }, { windowSize.x - 10, centerY + 5 }, colAxis);
    dl->AddTriangleFilled({ centerX - 5, 10 }, { centerX, 0 }, { centerX + 5, 10 }, colAxis);

    // X ticks
    for (int i = -nX; i <= nX; ++i) {
        if (i == 0) continue;
        float x = centerX + i * step;
        dl->AddLine({ x, centerY - 5 }, { x, centerY + 5 }, colAxis);
        dl->AddText({ x + 2, centerY + 10 }, colAxis, std::to_string(i * cfg.gridSpacing).c_str());
    }
    // Y ticks
    for (int i = -nY; i <= nY; ++i) {
        if (i == 0) continue;
        float y = centerY + i * step;
        dl->AddLine({ centerX - 5, y }, { centerX + 5, y }, colAxis);
        dl->AddText({ centerX + 10, y - 8 }, colAxis, std::to_string(-i * cfg.gridSpacing).c_str());
    }
}

void Scene::DrawFunction(const ImVec2& center, const ImVec2& windowSize, const AppConfig& cfg) {
    const float unit = (cfg.gridScale > 0 ? cfg.gridSpacing * cfg.gridScale : cfg.gridSpacing);

    const int nX = int(windowSize.x / unit) + 1;
    const int N = (cfg.samples > 2 ? cfg.samples : 2);

    std::vector<ImVec2> pts;
    pts.reserve(N);

    for (int i = 0; i < N; ++i) {
        float t = float(i) / float(N - 1);
        float x = -nX + t * (2 * nX);
        float y = Eval(x);

        float sx = center.x + x * unit;
        float sy = center.y - y * unit;
        pts.emplace_back(sx, sy);
    }

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    for (int i = 1; i < N; ++i) {
        dl->AddLine(pts[i - 1], pts[i], RGBA(cfg.funcColor), 2.0f);
    }
}

void Scene::DrawFourierTransform(const ImVec2& center,
    const ImVec2& windowSize,
    const AppConfig& cfg)
{
    const float unitScale = (cfg.gridScale > 0 ? cfg.gridSpacing * cfg.gridScale : cfg.gridSpacing);
    const int sampleCount = (cfg.samples > 2 ? cfg.samples : 2);
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    auto toScreen = [&](float worldX, float worldY) -> ImVec2 {
        return ImVec2(center.x + worldX * unitScale, center.y - worldY * unitScale);
        };

    Fourier F(sampleCount);

    if (cfg.fourierDisplayMode == FOURIER_TRANSFORM)
    {
        if (cfg.showFourierRange)
        {
            const float binRangeMin = cfg.fourierCenter - cfg.fourierRange;
            const float binRangeMax = cfg.fourierCenter + cfg.fourierRange;

            const float bracketHeight = 1.0f;   // world units up/down from axis
            const float bracketPad = 0.5f;   // world units horizontal ticks
            const float bracketThickness = 4.0f;

            // left vertical
            drawList->AddLine(toScreen(binRangeMin, -bracketHeight), toScreen(binRangeMin, bracketHeight),
                RGBA(cfg.fourierRangeColor), bracketThickness);
            // right vertical
            drawList->AddLine(toScreen(binRangeMax, -bracketHeight), toScreen(binRangeMax, bracketHeight),
                RGBA(cfg.fourierRangeColor), bracketThickness);
            // top caps
            drawList->AddLine(toScreen(binRangeMin, bracketHeight), toScreen(binRangeMin + bracketPad, bracketHeight),
                RGBA(cfg.fourierRangeColor), bracketThickness);
            drawList->AddLine(toScreen(binRangeMax, bracketHeight), toScreen(binRangeMax - bracketPad, bracketHeight),
                RGBA(cfg.fourierRangeColor), bracketThickness);
            // bottom caps
            drawList->AddLine(toScreen(binRangeMin, -bracketHeight), toScreen(binRangeMin + bracketPad, -bracketHeight),
                RGBA(cfg.fourierRangeColor), bracketThickness);
            drawList->AddLine(toScreen(binRangeMax, -bracketHeight), toScreen(binRangeMax - bracketPad, -bracketHeight),
                RGBA(cfg.fourierRangeColor), bracketThickness);
        }

        auto spec = F.computeTransform([&](double x) { return Eval((float)x); },
            cfg.fourierCenter, cfg.fourierRange, sampleCount);

        ImGui::Begin("Fourier Transform");
        ImGui::Text("Samples: %d | Range: [%.3f, %.3f] rad/s | Max amplitude: %.4f",
            sampleCount, -spec.wMax, spec.wMax, spec.maxAmp);

        const ImVec2 canvasSize(ImGui::GetContentRegionAvail().x, 260.0f);
        ImGui::InvisibleButton("FourierCanvas", canvasSize);
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(p0, p1, IM_COL32(25, 25, 25, 255));
        draw->AddRect(p0, p1, IM_COL32(90, 90, 90, 255));

        F.renderTransform(spec, p0, p1, draw, RGBA(cfg.fourierColor));
        ImGui::End();
    }
    else // FOURIER_MODULATED_SIGNAL
    {
        int halfSpanUnits = int(windowSize.x / unitScale) + 1;
        float worldXMin = (float)-halfSpanUnits;
        float worldXMax = (float)+halfSpanUnits;

        auto points = F.computeModulatedPoints(sampleCount, worldXMin, worldXMax,
            [&](double x) { return Eval((float)x); },
            cfg.fourierMode, toScreen);

        for (int i = 1; i < sampleCount; ++i)
            drawList->AddLine(points[i - 1], points[i], RGBA(cfg.fourierColor), 2.0f);
    }
}


bool Scene::HasError() const {
    return !impl->valid;
}

const std::string& Scene::GetLastError() const {
    return impl->lastError;
}