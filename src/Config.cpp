#include "Config.h"
#include <fstream>
#include <sstream>
#include <limits>
#include <cstring>

bool AppConfig::Load(const char* file) {
    std::ifstream f(file);
    if (!f.is_open()) return false;

    auto load4 = [&](ImVec4& c) {
        f >> c.x >> c.y >> c.z >> c.w;
        };

    load4(funcColor);
    load4(gridColor);
    load4(axisColor);
    load4(backgroundColor);
    load4(quadColor);
    load4(quadBorderColor);
    f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // expression line
    std::string expr;
    std::getline(f, expr);
    if (!expr.empty()) {
        strncpy_s(funcExpr, kExprBufSize, expr.c_str(), _TRUNCATE);
    }


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
