#include "Fourier.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include "Config.h"

Fourier::Fourier(int fs) : Fs_(fs) {}
int Fourier::Fs() const { return Fs_; }

// Generate N samples from a mathematical function f(x)
// Used to create the discrete time-domain signal before applying DFT
std::vector<double> Fourier::sample(std::function<double(double)> f, int N) const {
    std::vector<double> x(N);
    for (int n = 0; n < N; ++n) x[n] = f(n / double(Fs_));
    return x;
}

// Remove DC offset by subtracting mean value from all samples
void Fourier::zeroMean(std::vector<double>& x) {
    double s = 0.0;
    for (double v : x) s += v;
    s /= x.size();
    for (double& v : x) v -= s;
}

// Generate Hann window for spectral smoothing in STFT or DFT
std::vector<double> Fourier::hann(int M) {
    std::vector<double> w(M);
    if (M == 1) { w[0] = 1.0; return w; }
    const double twoPi = 2.0 * M_PI;
    for (int n = 0; n < M; ++n) w[n] = 0.5 - 0.5 * std::cos(twoPi * n / (M - 1));
    return w;
}

// Apply a window function elementwise to the signal
void Fourier::applyWindow(std::vector<double>& frame, const std::vector<double>& w) {
    for (int n = 0; n < (int)frame.size(); ++n) frame[n] *= w[n];
}

// Discrete Fourier Transform: converts signal from time to frequency domain
std::vector<std::complex<double>> Fourier::dft(const std::vector<double>& x) const {
    const int N = (int)x.size();
    std::vector<std::complex<double>> X(N);
    for (int k = 0; k < N; ++k) X[k] = dftAt(x, k);
    return X;
}

// Compute one frequency bin of the DFT
std::complex<double> Fourier::dftAt(const std::vector<double>& x, int k) const {
    const int N = (int)x.size();
    std::complex<double> s(0.0, 0.0);
    for (int n = 0; n < N; ++n) s += x[n] * twiddle(k, n, N);
    return s;
}

// Convert full DFT to single-sided amplitude spectrum (for real signals)
std::vector<double> Fourier::amplitudeSingleSided(const std::vector<std::complex<double>>& X) {
    const int N = (int)X.size();
    const int K = N / 2;
    std::vector<double> A(K + 1, 0.0);
    for (int k = 0; k <= K; ++k) {
        double scale = (k == 0 || (N % 2 == 0 && k == K)) ? 1.0 : 2.0;
        A[k] = scale * std::abs(X[k]) / N;
    }
    return A;
}

// Compute frequency value (Hz) of bin k for given sampling rate
double Fourier::binFreq(int k, int N) const {
    return k * (double)Fs_ / N;
}

// Short-Time Fourier Transform: compute amplitude spectrum for each frame
std::vector<std::vector<double>> Fourier::stftMagnitude(const std::vector<double>& x, int M, int H, const std::vector<double>& w) const {
    std::vector<std::vector<double>> S;
    for (int t = 0; t + M <= (int)x.size(); t += H) {
        std::vector<double> frame(M);
        for (int n = 0; n < M; ++n) frame[n] = x[t + n];
        applyWindow(frame, w);
        auto Y = dftReal(frame);
        S.push_back(amplitudeSingleSided(Y));
    }
    return S;
}

// Compute DFT for real-valued frame
std::vector<std::complex<double>> Fourier::dftReal(const std::vector<double>& frame) const {
    const int M = (int)frame.size();
    std::vector<std::complex<double>> Y(M);
    for (int k = 0; k < M; ++k) {
        std::complex<double> s(0.0, 0.0);
        for (int n = 0; n < M; ++n) s += frame[n] * twiddle(k, n, M);
        Y[k] = s;
    }
    return Y;
}

// Modulate a signal using exp(-j*pi*n) pattern. Used for visualization of Fourier modulation modes
std::vector<double> Fourier::modulate(const std::vector<double>& signal, int mode) const {
    std::vector<double> out(signal.size());
    for (size_t n = 0; n < signal.size(); ++n) {
        std::complex<double> e = std::exp(std::complex<double>(0, -M_PI * (double)n));
        double carrier = 1.0;
        if (mode == FOURIER_REAL)      carrier = e.real();
        else if (mode == FOURIER_IMAG) carrier = e.imag();
        else if (mode == FOURIER_MAG)  carrier = std::abs(e);
        out[n] = signal[n] * carrier;
    }
    return out;
}

// Generate and modulate samples between xMin..xMax using user function f(x)
std::vector<double> Fourier::generateModulated(int N, double xMin, double xMax,
    std::function<double(double)> func,
    int mode) const
{
    std::vector<double> signal(N);
    for (int i = 0; i < N; ++i) {
        double tNorm = (double)i / (double)(N - 1);
        double x = xMin + tNorm * (xMax - xMin);
        signal[i] = func(x);
    }
    return modulate(signal, mode);
}

// Convert a vector of y-values into ImGui screen coordinates using callback
std::vector<ImVec2> Fourier::toPoints(const std::vector<double>& y,
    double xMin, double xMax,
    std::function<ImVec2(double, double)> toScreen) const
{
    std::vector<ImVec2> pts(y.size());
    for (int i = 0; i < (int)y.size(); ++i) {
        double tNorm = (double)i / (double)(y.size() - 1);
        double x = xMin + tNorm * (xMax - xMin);
        pts[i] = toScreen(x, y[i]);
    }
    return pts;
}

// Compute full symmetric DFT spectrum with frequency scaling (rad/s)
// Output: freqs[], magn[] and normalization factors
FourierSpectrum Fourier::computeTransform(std::function<double(double)> f,
    double center, double range, int N) const
{
    FourierSpectrum out;
    double T = (center + range) - (center - range);
    double dt = T / N;
    out.wMax = M_PI / dt;

    std::vector<double> signal(N);
    for (int n = 0; n < N; ++n)
        signal[n] = f((center - range) + n * dt);

    auto X = dft(signal);

    std::vector<std::complex<double>> Xs(N);
    for (int i = 0; i < N; ++i)
        Xs[i] = X[(i + N / 2) % N]; // center spectrum around zero frequency

    out.freqs.resize(N);
    out.magn.resize(N);
    for (int k = 0; k < N; ++k) {
        double w = -out.wMax + 2.0 * out.wMax * (double)k / N;
        out.freqs[k] = w;
        out.magn[k] = std::abs(Xs[k]) / N;
    }

    out.maxAmp = *std::max_element(out.magn.begin(), out.magn.end());
    return out;
}

// Render complete frequency-domain graph with grid and labels
// Draws axes, grid lines, frequency ticks, amplitude labels and spectrum curve
void Fourier::renderTransform(const FourierSpectrum& spec,
    const ImVec2& p0, const ImVec2& p1,
    ImDrawList* draw, ImU32 color) const
{
    draw->AddRectFilled(p0, p1, IM_COL32(25, 25, 25, 255));
    draw->AddRect(p0, p1, IM_COL32(90, 90, 90, 255));

    const float left = p0.x + 50.0f;
    const float right = p1.x - 10.0f;
    const float top = p0.y + 10.0f;
    const float bottom = p1.y - 25.0f;

    const ImU32 gridCol = IM_COL32(60, 60, 60, 255);
    const ImU32 textCol = IM_COL32(200, 200, 200, 255);

    const int gridX = 6;
    const int gridY = 4;

    const double wMin = -spec.wMax;
    const double wRange = 2.0 * spec.wMax;
    const double ampMax = (spec.maxAmp > 1e-12) ? spec.maxAmp : 1.0;

    for (int gx = 0; gx <= gridX; ++gx) {
        float t = (float)gx / (float)gridX;
        float x = left + t * (right - left);
        double w = wMin + t * wRange;
        draw->AddLine(ImVec2(x, top), ImVec2(x, bottom), gridCol);
        char label[48];
        std::snprintf(label, sizeof(label), "%.1f", w);
        draw->AddText(ImVec2(x - 18.0f, bottom + 5.0f), textCol, label);
    }

    for (int gy = 0; gy <= gridY; ++gy) {
        float t = (float)gy / (float)gridY;
        float y = bottom - t * (bottom - top);
        draw->AddLine(ImVec2(left, y), ImVec2(right, y), gridCol);
        char label[48];
        std::snprintf(label, sizeof(label), "%.2f", t * ampMax);
        draw->AddText(ImVec2(p0.x + 5.0f, y - 7.0f), textCol, label);
    }

    draw->AddLine(ImVec2(left, bottom), ImVec2(right, bottom), textCol, 1.0f);
    draw->AddLine(ImVec2(left, top), ImVec2(left, bottom), textCol, 1.0f);
    draw->AddText(ImVec2(right - 25.0f, bottom + 5.0f), textCol, "w (rad/s)");
    draw->AddText(ImVec2(left - 35.0f, top - 10.0f), textCol, "|F(w)|");

    const size_t N = std::min(spec.freqs.size(), spec.magn.size());
    if (N < 2) return;

    for (size_t i = 1; i < N; ++i) {
        float t0 = (float)((spec.freqs[i - 1] - wMin) / wRange);
        float t1 = (float)((spec.freqs[i] - wMin) / wRange);
        float x0 = left + t0 * (right - left);
        float x1 = left + t1 * (right - left);
        float y0 = bottom - (float)(spec.magn[i - 1] / ampMax) * (bottom - top);
        float y1 = bottom - (float)(spec.magn[i] / ampMax) * (bottom - top);
        draw->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, 2.0f);
    }
}

// Compute modulated visualization points for "Fourier modulated signal" mode
std::vector<ImVec2> Fourier::computeModulatedPoints(int N, double xMin, double xMax,
    std::function<double(double)> f,
    int mode,
    std::function<ImVec2(double, double)> toScreen) const
{
    std::vector<double> signal(N);
    for (int i = 0; i < N; ++i) {
        double tNorm = (double)i / (N - 1);
        double x = xMin + tNorm * (xMax - xMin);
        signal[i] = f(x);
    }

    std::vector<ImVec2> pts(N);
    for (int i = 0; i < N; ++i) {
        std::complex<double> e = std::exp(std::complex<double>(0, -M_PI * i));
        double carrier = (mode == FOURIER_REAL) ? e.real()
            : (mode == FOURIER_IMAG) ? e.imag()
            : std::abs(e);
        double y = signal[i] * carrier;
        double tNorm = (double)i / (N - 1);
        double x = xMin + tNorm * (xMax - xMin);
        pts[i] = toScreen((float)x, (float)y);
    }
    return pts;
}

// Compute complex exponential for a given frequency bin
// Twiddle factor = exp(-j * 2π * k * n / N)
inline std::complex<double> Fourier::twiddle(int k, int n, int N) {
    double ang = -2.0 * M_PI * k * n / N;
    return { std::cos(ang), std::sin(ang) };
}
