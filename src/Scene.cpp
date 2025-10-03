#include "Scene.h"
#include <cmath>
#include <sstream>
#include <exprtk.hpp>
#include <windows.h>
#include <complex>

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
        float x = -nX + t * (2 * nX);   // sample across screen
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
    // scales and constants
    const float unitScale = (cfg.gridScale > 0 ? cfg.gridSpacing * cfg.gridScale : cfg.gridSpacing);
    const int   sampleCount = (cfg.samples > 2 ? cfg.samples : 2);
    const double kPi = 3.14159265358979323846;
    const double kTwoPi = 2.0 * kPi;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // screen <-> world helpers
    auto toScreen = [&](float worldX, float worldY) -> ImVec2 {
        return ImVec2(center.x + worldX * unitScale, center.y - worldY * unitScale);
        };

    if (cfg.fourierDisplayMode == FOURIER_TRANSFORM)
    {
        // selected bin window [binRangeMin..binRangeMax]
        const float binRangeMin = cfg.fourierCenter - cfg.fourierRange;
        const float binRangeMax = cfg.fourierCenter + cfg.fourierRange;

        if (cfg.showFourierRange)
        {
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

        // spectrum slice integrand plotted over i (normalized x in [0..1])
        std::vector<ImVec2> points;
        points.reserve(sampleCount);

        // clamp bin range
        int binStart = std::max(0, (int)std::floor(binRangeMin));
        int binEnd = std::min(sampleCount - 1, (int)std::ceil(binRangeMax));
        if (binEnd < binStart) std::swap(binStart, binEnd);

        for (int i = 0; i < sampleCount; ++i)
        {
            const float  tNorm = (float)i / (float)(sampleCount - 1); // normalized 0..1
            const float  worldX = tNorm;
            const double sampleValue = (double)Eval(worldX);

            // twiddle base = exp(-j * 2π * i / N)
            const double phaseStep = -kTwoPi * (double)i / (double)sampleCount;
            const std::complex<double> twiddleBase(std::cos(phaseStep), std::sin(phaseStep));

            // start at k = binStart: twiddle = twiddleBase^binStart
            std::complex<double> twiddle = std::polar(1.0, phaseStep * (double)binStart);

            std::complex<double> accumulator(0.0, 0.0);
            for (int k = binStart; k <= binEnd; ++k) {
                accumulator += sampleValue * twiddle; // s * exp(-j 2π k i / N)
                twiddle *= twiddleBase;               // advance k
            }

            const int binsCount = std::max(1, binEnd - binStart + 1);
            accumulator /= (double)binsCount;

            float worldY =
                (cfg.fourierMode == FOURIER_REAL) ? (float)accumulator.real() :
                (cfg.fourierMode == FOURIER_IMAG) ? (float)accumulator.imag() :
                (float)std::abs(accumulator);

            points.emplace_back(toScreen(worldX, worldY));
        }

        for (int i = 1; i < sampleCount; ++i)
            drawList->AddLine(points[i - 1], points[i], RGBA(cfg.fourierColor), 2.0f);
    }
    else // FOURIER_MODULATED_SIGNAL
    {
        std::vector<ImVec2> points;
        points.reserve(sampleCount);

        // cover visible world-X across the window
        const int   halfSpanUnits = int(windowSize.x / unitScale) + 1;
        const float worldXMin = (float)-halfSpanUnits;
        const float worldXMax = (float)+halfSpanUnits;

        // specific bin = N/2 => exp(-j*pi*i) = (-1)^i (fast path)
        bool flip = false; // (-1)^i

        for (int i = 0; i < sampleCount; ++i)
        {
            const float tNorm = (float)i / (float)(sampleCount - 1);     // 0..1
            const float worldX = worldXMin + tNorm * (worldXMax - worldXMin);

            float carrier =
                (cfg.fourierMode == FOURIER_REAL) ? (flip ? -1.0f : 1.0f) :
                (cfg.fourierMode == FOURIER_IMAG) ? 0.0f :
                1.0f; // magnitude of exp() is 1

            const float worldY = (float)Eval(worldX) * carrier;

            points.emplace_back(toScreen(worldX, worldY));
            flip = !flip;
        }

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