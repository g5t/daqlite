
#pragma once
#include <cmath>

#include "data_manager.h"

namespace datastore::bifrost {

    enum class HType {unknown, x, a, p, xp, ab, b, xt, pt, t};

    class Histograms {
    public:
        Histograms() {
            reset();
        };

        ~Histograms() = default;


        inline static int arc(int group) {
            return group / 3;
        }

        inline static int module(int fiber) {
            return fiber / 2;
        }

        inline static int triplet(int fiber, int group) {
            int type = group % 3;
            return module(fiber) * 3 + type;
        }

        bool add(int fiber, int group, int a, int b, double time) {
            auto arc_ = arc(group);
            auto triplet_ = triplet(fiber, group);
            if (arc_ < 0 || arc_ >= ARCS || triplet_ < 0 || triplet_ >= TRIPLETS) {
                return false;
            }
            bool ok{true};
            ok &= add_1D(arc_, triplet_, a, b, time);
            ok &= add_2D(arc_, triplet_, a, b, time);
            return ok;
        }

        [[nodiscard]] const int * data_1D(int arc, int triplet, HType t) const {
            if (HType::a == t) return a_[arc][triplet];
            if (HType::b == t) return b_[arc][triplet];
            if (HType::x == t) return x_[arc][triplet];
            if (HType::p == t) return p_[arc][triplet];
            if (HType::t == t) return t_[arc][triplet];
            return nullptr;
        }
        [[nodiscard]] const int * data_2D(int arc, int triplet, HType t) const {
            if (HType::ab == t) return ab_[arc][triplet];
            if (HType::xp == t) return xp_[arc][triplet];
            if (HType::xt == t) return xt_[arc][triplet];
            if (HType::pt == t) return pt_[arc][triplet];
            return nullptr;
        }

        static constexpr int ARCS = 5;
        static constexpr int TRIPLETS = 9;
        static constexpr int RANGE = 1 << 15;
        static constexpr int SHIFT2D = 6;
        static constexpr int SHIFT1D = 5;
        static constexpr int BIN2D = RANGE >> SHIFT2D;
        static constexpr int TOTAL2D = BIN2D * BIN2D;
        static constexpr int BIN1D = RANGE >> SHIFT1D;

        [[nodiscard]] bool is_empty(int arc, int triplet) const {
            for (int i = 0; i < BIN1D; ++i) {
                if (p_[arc][triplet][i]) {
                    return false;
                }
            }
            return true;
        }

    private:
        // statically sized arrays for the 9 histogram types
        // 1-D histograms
        int p_[ARCS][TRIPLETS][BIN1D]{};
        // I(x) -- charge-division position reconstruction
        int x_[ARCS][TRIPLETS][BIN1D]{};
        // I(t) -- frame time
        int t_[ARCS][TRIPLETS][BIN1D]{};
        // I(A) -- single-end pulse height spectrum
        int a_[ARCS][TRIPLETS][BIN1D]{};
        // I(B) -- single-end pulse height spectrum
        int b_[ARCS][TRIPLETS][BIN1D]{};
        // 2-D histograms
        // I(A,B)
        int ab_[ARCS][TRIPLETS][TOTAL2D]{};
        // I(x,P)
        int xp_[ARCS][TRIPLETS][TOTAL2D]{};
        // I(P,t)
        int pt_[ARCS][TRIPLETS][TOTAL2D]{};
        // I(x,t)
        int xt_[ARCS][TRIPLETS][TOTAL2D]{};


        void reset() {
            memset(&p_, 0, sizeof(p_));
            memset(&x_, 0, sizeof(x_));
            memset(&t_, 0, sizeof(t_));
            memset(&a_, 0, sizeof(a_));
            memset(&b_, 0, sizeof(b_));

            memset(&ab_, 0, sizeof(ab_));
            memset(&xp_, 0, sizeof(xp_));
            memset(&pt_, 0, sizeof(pt_));
            memset(&xt_, 0, sizeof(xt_));
        }

        static int hist_a_or_b(int x, int shift, int bins){
            int y = x >> shift;
            if (y < 0 || y >= bins) y = -1;
            return y;
        }
        static int hist_p(int x, int shift, int bins){
            // a and b are effectively 15-bit integers
            // so a+b is 16-bits, but we want this to fit into BIN2D bins,
            // so we must shift by an extra bit compared to a or b above
            int y = x >> (shift + 1);
            if (y < 0 || y >= bins) y = -1;
            return y;
        }
        static int hist_t(double x, int bins){
            // time at ESS resets every 1/14 Hz ~= 70 msec.
            // find the modulus and bin that range
            double period = 1.0 / 14.0;
            auto frac = fmod(x, period) / period;
            auto y = static_cast<int>(frac * bins);
            if (y < 0 || y >= bins) y = -1;
            return y;
        }
        static int hist_x(int a, int b, int bins){
            int num = a - b;
            int den = a + b;
            if (den == 0) {
                return false;
            }
            double ratio = static_cast<double>(num) / static_cast<double>(den);
            if (ratio < -1.0 || ratio > 1.0) {
                return false;
            }
            // full range is (-1, 1) so shift up by 1, multiply by 512 and convert to an integer
            auto x = static_cast<int>((ratio + 1.0) / 2.0 * (bins - 1));
            if (x < 0 || x >= bins) x = -1;
            return x;
        }

        bool add_2D(int arc, int triplet, int full_a, int full_b, double full_t){
            auto a = hist_a_or_b(full_a, SHIFT2D, BIN2D);
            auto b = hist_a_or_b(full_b, SHIFT2D, BIN2D);
            auto p = hist_p(full_a+full_b, SHIFT2D, BIN2D);
            auto x = hist_x(full_a, full_b, BIN2D);
            auto t = hist_t(full_t, BIN2D);
            if (a < 0 || b < 0 || p < 0 || x < 0 || t < 0) return false;
            ++ab_[arc][triplet][a * BIN2D + b];
            ++xp_[arc][triplet][x * BIN2D + p];
            ++pt_[arc][triplet][p * BIN2D + t];
            ++xt_[arc][triplet][x * BIN2D + t];
            return true;
        }
        bool add_1D(int arc, int triplet, int full_a, int full_b, double full_t){
            auto a = hist_a_or_b(full_a, SHIFT1D, BIN1D);
            auto b = hist_a_or_b(full_b, SHIFT1D, BIN1D);
            auto p = hist_p(full_a+full_b, SHIFT1D, BIN1D);
            auto x = hist_x(full_a, full_b, BIN1D);
            auto t = hist_t(full_t, BIN1D);
            if (a < 0 || b < 0 || p < 0 || x < 0 || t < 0) return false;
            ++x_[arc][triplet][x];
            ++a_[arc][triplet][a];
            ++b_[arc][triplet][b];
            ++p_[arc][triplet][p];
            ++t_[arc][triplet][t];
            return true;
        }
    };

} // namespace datastore