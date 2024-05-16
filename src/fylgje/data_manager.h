#pragma once
#include <map>
#include <vector>
#include <QVector>
#include <QPlot/qcustomplot/qcustomplot.h>

namespace bifrost {
    inline int arc(int group) {
        return group / 3;
    }

    inline int module(int fiber) {
        return fiber / 2;
    }

    inline int triplet(int fiber, int group) {
        int type = group % 3;
        return module(fiber) * 3 + type;
    }
}

namespace bifrost::data {
    int hist_a_or_b(int x, int shift, int bins);
    int hist_p(int x, int shift, int bins);
    int hist_t(double x, int bins);
    int hist_x(int a, int b, int bins);

    template<class T, class R>
    void iota_step(T begin, T end, R step, R first=R(0)){
        *begin = first;
        for (auto p=begin; p != end; ++p){
            auto n = p+1;
            if (n != end) *n = *p + step;
        }
    }

    enum class Type {unknown, x, a, p, xp, ab, b, xt, pt, t};

    constexpr Type TYPE1D[]{Type::a, Type::b, Type::x, Type::p, Type::t};
    constexpr Type TYPE2D[]{Type::xp, Type::ab, Type::xt, Type::pt};

    constexpr int SHIFT1D = 5;
    constexpr int BIN1D = (1 << 15) >> SHIFT1D;
    constexpr int SHIFT2D = 6;
    constexpr int BIN2D = (1 << 15) >> SHIFT2D;

    class Manager{
    public:
        using AX = std::vector<double>;
        using axis_key_t = Type;
        using D1 = std::vector<double>;
        using D2 = QCPColorMapData;
        using data_key_t = std::tuple<int, int, Type>; // arc, triplet, plot_t
    private:
        std::map<axis_key_t, AX *> independent;
        std::map<axis_key_t, AX *> dependent;
        std::map<data_key_t, D1 *> data_1d;
        std::map<data_key_t, D2 *> data_2d;

        int arcs;
        int triplets;

    public:
        Manager(int arcs, int triplets): arcs(arcs), triplets(triplets) {
            // setup data objects ...
            for (int a = 0; a<arcs; ++a){
                for (int t = 0; t<triplets; ++t) {
                    for (auto k: TYPE1D){
                        data_1d[std::make_tuple(a, t, k)] = new D1(BIN1D, 0);
                    }
                    for (auto k: TYPE2D){
                        auto key = std::make_tuple(a, t, k);
                        data_2d[key] = new D2(BIN2D, BIN2D, QCPRange(0, BIN2D-1), QCPRange(0, BIN2D-1));
                        data_2d[key]->fill(0);
                    }
                }
            }
            // setup axes vectors
            auto a1 = new AX(BIN1D);
            auto b1 = new AX(BIN1D);
            auto x1 = new AX(BIN1D);
            auto p1 = new AX(BIN1D);
            auto t1 = new AX(BIN1D);
            iota_step(a1->begin(), a1->end(), (1u << SHIFT1D) * 1.0);
            iota_step(b1->begin(), b1->end(), (1u << SHIFT1D) * 1.0);
            iota_step(p1->begin(), p1->end(), (1u << (SHIFT1D + 1)) * 1.0);
            iota_step(x1->begin(), x1->end(), 2.0 / (BIN1D + 1), -1.0 + 1.0 / (BIN1D + 1));
            iota_step(t1->begin(), t1->end(), 1.0 / 14.0 / (BIN1D + 1), 1.0 / 14.0 / 2.0 / (BIN1D + 1));
            independent[Type::a] = a1;
            independent[Type::b] = b1;
            independent[Type::p] = p1;
            independent[Type::x] = x1;
            independent[Type::t] = t1;

            auto a2 = new AX(BIN2D);
            auto b2 = new AX(BIN2D);
            auto x2 = new AX(BIN2D);
            auto p2 = new AX(BIN2D);
            auto t2 = new AX(BIN2D);
            iota_step(a2->begin(), a2->end(), (1u << SHIFT2D) * 1.0);
            iota_step(b2->begin(), b2->end(), (1u << SHIFT2D) * 1.0);
            iota_step(p2->begin(), p2->end(), (1u << (SHIFT2D + 1)) * 1.0);
            iota_step(x2->begin(), x2->end(), 2.0 / (BIN2D + 1), -1.0 + 1.0 / (BIN2D + 1));
            iota_step(t2->begin(), t2->end(), 1.0 / 14.0 / (BIN2D + 1), 1.0 / 14.0 / 2.0 / (BIN2D + 1));
            independent[Type::ab] = a2;
            independent[Type::xt] = x2;
            independent[Type::pt] = p2;
            independent[Type::xp] = x2;
            dependent[Type::ab] = b2;
            dependent[Type::xt] = t2;
            dependent[Type::pt] = t2;
            dependent[Type::xp] = p2;
        }
        ~Manager() = default;

        void clear(){
//            for (auto [k, x]: data_1d) x->fill(0);
            for (auto [k, x]: data_1d) std::fill(x->begin(), x->end(), 0);
            for (auto [k, x]: data_2d) x->fill(0);
        }
        bool add(int arc, int triplet, int a, int b, double time);

        [[nodiscard]] double max() const;
        [[nodiscard]] double max(int arc) const;
        [[nodiscard]] double max(int arc, int triplet) const;
        [[nodiscard]] double max(int arc, Type t) const;
        [[nodiscard]] double max(int arc, int triplet, Type t) const;

      [[nodiscard]] double min() const;
      [[nodiscard]] double min(int arc) const;
      [[nodiscard]] double min(int arc, int triplet) const;
      [[nodiscard]] double min(int arc, Type t) const;
      [[nodiscard]] double min(int arc, int triplet, Type t) const;

        [[nodiscard]] D1 * data_1D(int arc, int triplet, Type t) const;
        [[nodiscard]] D2 * data_2D(int arc, int triplet, Type t) const;

        [[nodiscard]] AX * independent_axis(Type t) const {
            return independent.count(t) ? independent.at(t) : nullptr;
        }

    private:
        bool add_1D(int arc, int triplet, int a, int b, double time);
        bool add_2D(int arc, int triplet, int a, int b, double time);
        [[nodiscard]] double max_1D(int arc, int triplet, Type t) const;
        [[nodiscard]] double max_2D(int arc, int triplet, Type t) const;
      [[nodiscard]] double min_1D(int arc, int triplet, Type t) const;
      [[nodiscard]] double min_2D(int arc, int triplet, Type t) const;
    };
}