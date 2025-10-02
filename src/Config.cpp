#include "Config.h"
#include <fstream>
#include <sstream>

void AppConfig::EnsureFuncBufSize(size_t n) {
    if (funcExpr.capacity() < n) funcExpr.reserve(n);
    if (funcExpr.size() + 1 < n) funcExpr.resize(n - 1, '\0');
}

bool AppConfig::Load(const char* file) {
    std::ifstream f(file);
    if (!f.is_open()) return false;

    // plain text lines
    // colors
    f >> funcColor.x >> funcColor.y >> funcColor.z >> funcColor.w;
    f >> gridColor.x >> gridColor.y >> gridColor.z >> gridColor.w;
    f >> axisColor.x >> axisColor.y >> axisColor.z >> axisColor.w;
    f >> backgroundColor.x >> backgroundColor.y >> backgroundColor.z >> backgroundColor.w;
    f >> quadColor.x >> quadColor.y >> quadColor.z >> quadColor.w;
    f >> quadBorderColor.x >> quadBorderColor.y >> quadBorderColor.z >> quadBorderColor.w;
    f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // expr line
    std::string expr;
    std::getline(f, expr);
    if (!expr.empty()) funcExpr = expr;

    // numbers
    f >> samples >> gridSpacing >> gridScale;
    return true;
}

void AppConfig::Save(const char* file) const {
    std::ofstream f(file);
    if (!f.is_open()) return;

    auto dump4 = [&](const ImVec4& c) {
        f << c.x << " " << c.y << " " << c.z << " " << c.w << "\n";
        };
    dump4(funcColor);
    dump4(gridColor);
    dump4(axisColor);
    dump4(backgroundColor);
    dump4(quadColor);
    dump4(quadBorderColor);

    f << funcExpr << "\n";
    f << samples << " " << gridSpacing << " " << gridScale << "\n";
}
