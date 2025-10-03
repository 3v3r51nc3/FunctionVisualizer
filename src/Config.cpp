#include <fstream>
#include <sstream>
#include <limits>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include "Config.h"
#include "Scene.h"

static inline bool starts_with(const std::string& s, const char* p) {
    return s.rfind(p, 0) == 0;
}

static inline void trim_inplace(std::string& s) {
    auto ns = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), ns));
    s.erase(std::find_if(s.rbegin(), s.rend(), ns).base(), s.end());
}

static inline bool parse_bool(std::istringstream& iss, bool& b) {
    std::string v; if (!(iss >> v)) return false;
    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
    if (v == "1" || v == "true" || v == "yes") { b = true; return true; }
    if (v == "0" || v == "false" || v == "no") { b = false; return true; }
    return false;
}

static inline bool read_vec4(std::istream& is, ImVec4& c) {
    return static_cast<bool>(is >> c.x >> c.y >> c.z >> c.w);
}

// ---------- Load ----------
bool AppConfig::Load(const char* file, Scene& scene) {
    std::ifstream f(file);
    if (!f.is_open()) return false;

    // peek first line
    std::string first;
    std::getline(f, first);
    std::string line = first;

    if (starts_with(first, "AppConfig")) {
        // new KV format
        while (true) {
            if (!std::getline(f, line)) break;
            trim_inplace(line);
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string key; iss >> key;

            if (key == "funcColor") { read_vec4(iss, funcColor); }
            else if (key == "gridColor") { read_vec4(iss, gridColor); }
            else if (key == "axisColor") { read_vec4(iss, axisColor); }
            else if (key == "backgroundColor") { read_vec4(iss, backgroundColor); }
            else if (key == "quadColor") { read_vec4(iss, quadColor); }
            else if (key == "quadBorderColor") { read_vec4(iss, quadBorderColor); }
            else if (key == "fourierColor") { read_vec4(iss, fourierColor); }
            else if (key == "fourierRangeColor") { read_vec4(iss, fourierRangeColor); }

            else if (key == "samples") { iss >> samples; }
            else if (key == "gridSpacing") { iss >> gridSpacing; }
            else if (key == "gridScale") { iss >> gridScale; }

            else if (key == "fourierFunction") { parse_bool(iss, fourierFunction); }
            else if (key == "showFourierRange") { parse_bool(iss, showFourierRange); }
            else if (key == "fourierCenter") { iss >> fourierCenter; }
            else if (key == "fourierRange") { iss >> fourierRange; }
            else if (key == "fourierMode") { iss >> fourierMode; }
            else if (key == "fourierDisplayMode") { iss >> fourierDisplayMode; }

            else if (key == "funcExpr") {
                std::string expr; std::getline(iss, expr);
                if (!expr.empty() && expr[0] == ' ') expr.erase(0, 1);
                trim_inplace(expr);
                if (!expr.empty()) {
#ifdef _MSC_VER
                    strncpy_s(funcExpr, kExprBufSize, expr.c_str(), _TRUNCATE);
#else
                    std::strncpy(funcExpr, expr.c_str(), kExprBufSize - 1);
                    funcExpr[kExprBufSize - 1] = '\0';
#endif
                    scene.SetExpression(expr);
                }
            }
            // unknown keys are ignored for forward compatibility
        }
        return true;
    }

    // -------- legacy fallback (старый последовательный формат) --------
    f.clear();
    f.seekg(0);

    auto load4 = [&](ImVec4& c) { f >> c.x >> c.y >> c.z >> c.w; };

    load4(funcColor);
    load4(gridColor);
    load4(axisColor);
    load4(backgroundColor);
    load4(quadColor);
    load4(quadBorderColor);
    f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string expr;
    std::getline(f, expr);
    if (!expr.empty()) {
#ifdef _MSC_VER
        strncpy_s(funcExpr, kExprBufSize, expr.c_str(), _TRUNCATE);
#else
        std::strncpy(funcExpr, expr.c_str(), kExprBufSize - 1);
        funcExpr[kExprBufSize - 1] = '\0';
#endif
        scene.SetExpression(expr);
    }

    f >> samples >> gridSpacing >> gridScale;
    // остальные поля останутся со значениями по умолчанию
    return true;
}

// ---------- Save ----------
void AppConfig::Save(const char* file) const {
    std::ofstream f(file);
    if (!f.is_open()) return;

    f << "AppConfig v2\n";
    f.setf(std::ios::fixed);
    f << std::setprecision(6);

    auto dump4 = [&](const char* name, const ImVec4& c) {
        f << name << " " << c.x << " " << c.y << " " << c.z << " " << c.w << "\n";
        };

    dump4("funcColor", funcColor);
    dump4("gridColor", gridColor);
    dump4("axisColor", axisColor);
    dump4("backgroundColor", backgroundColor);
    dump4("quadColor", quadColor);
    dump4("quadBorderColor", quadBorderColor);
    dump4("fourierColor", fourierColor);
    dump4("fourierRangeColor", fourierRangeColor);

    f << "samples " << samples << "\n";
    f << "gridSpacing " << gridSpacing << "\n";
    f << "gridScale " << gridScale << "\n";

    f << "fourierFunction " << (fourierFunction ? "true" : "false") << "\n";
    f << "showFourierRange " << (showFourierRange ? "true" : "false") << "\n";
    f << "fourierCenter " << fourierCenter << "\n";
    f << "fourierRange " << fourierRange << "\n";
    f << "fourierMode " << fourierMode << "\n";
    f << "fourierDisplayMode " << fourierDisplayMode << "\n";

    // expr — остаток строки, без кавычек
    f << "funcExpr " << funcExpr << "\n";
}
