#pragma once
#include <map>
#include <QGridLayout>
#include <QPlot/qcustomplot/qcustomplot.h>

#include "data_manager.h"

class PlotManager{
public:
    using data_t = ::bifrost::data::Manager;
    using layout_t = QGridLayout;
    enum class Dim {none, one, two};

    PlotManager(data_t * h, layout_t * l, int n1, int n2): histograms(h), layout(l), n1(n1), n2(n2) {
        dims[0] = Dim::none;
    }

    void make_single(Dim d){
        if (layout->rowCount() != 1 || layout->columnCount() != 1 || d != dims[0]){
            qDeleteAll(layout->children());
            if (Dim::one == d) return make_1D(0, 0);
            if (Dim::two == d) return make_2D(0, 0);
        }
    }

    void make_all_same(Dim t){
        if (layout->rowCount() != 3 || layout->columnCount() != 3){
            qDeleteAll(layout->children());
        }
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; ++j) {
                // if the plot exists but is the wrong type, remove it.
                if (layout->itemAtPosition(i, j) && t !=dims[key(i, j)]){
                    layout->removeItem(layout->itemAtPosition(i, j));
                }
                // if the plot doesn't exist create it
                if (!layout->itemAtPosition(i, j)) {
                    if (Dim::one == t){
                        make_1D(i, j, (i>0) & (j>1));
                    } else {
                        make_2D(i, j);
                    }
                }
            }
        }
    }

    void make_multi(){
        /*
         * 1 1 1
         * 2 2 -
         * 2 2 -
         */
        if (layout->rowCount() != 3 || layout->columnCount() != 3){
            qDeleteAll(layout->children());
        }
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; ++j) {
                Dim t{i == 0 || j > 1 ? Dim::one : Dim::two};
                // if the plot exists but is the wrong type, remove it.
                if (layout->itemAtPosition(i, j) && t !=dims[key(i, j)]){
                    layout->removeItem(layout->itemAtPosition(i, j));
                }
                // if the plot doesn't exist create it
                if (!layout->itemAtPosition(i, j)) {
                    if (Dim::one == t){
                        make_1D(i, j, (i>0) & (j>1));
                    } else {
                        make_2D(i, j);
                    }
                }
            }
        }
    }

    void plot(int i, int j, const QVector<double> * x, const QVector<double> * y){
        // 1-D data plotting
        auto k = key(i, j);
        if (!dims.count(k) || dims.at(k) != Dim::one) return;
        auto g = lines.at(k);
        g->setData(*x, *y);
    }
    void plot(int i, int j, const QCPColorMapData * data){
        auto k = key(i, j);
        if (!dims.count(k) || dims.at(k) != Dim::two) return;
        if (!images.count(k)) return;
        auto im = images.at(k);
        // Why does setData _require_ a mutable pointer?
        im->setData(const_cast<QCPColorMapData *>(data), true);
    }

private:
    [[nodiscard]] inline int key(int i, int j) const {
        return i * n_ + j;
    }

    void make_plot(int i, int j){
        auto item = layout->itemAtPosition(i, j);
        if (!item){
            auto p = new QCustomPlot();
            p->axisRect()->setAutoMargins(QCP::msNone);
            p->xAxis->ticker()->setTickCount(0);
            p->yAxis->ticker()->setTickCount(0);
            p->xAxis->setTickPen(QPen(Qt::NoPen));
            layout->addWidget(p, i, j);
            plots[key(i, j)] = p;
        }
    }
    void make_1D(int i, int j, bool flip=false){
        dims[key(i, j)] = Dim::one;
        make_plot(i, j);
        auto p = plots[key(i, j)];
        auto g = new QCPGraph(flip ? p->yAxis : p->xAxis, flip ? p->xAxis : p->yAxis);
        // styling of the displayed line
        lines[key(i, j)] = g;
    }
    void make_2D(int i, int j, bool flip=false){
        dims[key(i, j)] = Dim::two;
        make_plot(i, j);
        auto p = plots[key(i, j)];
        auto m = new QCPColorMap(flip ? p->yAxis : p->xAxis, flip ? p->xAxis : p->yAxis);
        m->data()->setSize(n2, n2);
        m->data()->setRange(QCPRange(0, n2-1), QCPRange(0, n2-1));

        auto s = new QCPColorScale(p);
        m->setColorScale(s);
        m->setGradient(QCPColorGradient::gpGrayscale);
        m->rescaleDataRange();
        p->rescaleAxes();

        images[key(i, j)] = m;
    }

private:
    data_t * histograms;
    layout_t * layout{};
    int n1;
    int n2;
    int n_{3};

    std::map<int, QCustomPlot *> plots;
    std::map<int, QCPColorMap *> images;
    std::map<int, QCPGraph *> lines;
    std::map<int, Dim> dims;
};