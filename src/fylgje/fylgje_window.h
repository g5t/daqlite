#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QPlot/qcustomplot/qcustomplot.h>

#include "WorkerThread.h"
#include "plot_manager.h"
#include "data_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    using int_t = ::bifrost::data::Type;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void set_arc_1(){set_arc(0);}
    void set_arc_2(){set_arc(1);}
    void set_arc_3(){set_arc(2);}
    void set_arc_4(){set_arc(3);}
    void set_arc_5(){set_arc(4);}

    void set_triplet_1(){set_triplet(0);}
    void set_triplet_2(){set_triplet(1);}
    void set_triplet_3(){set_triplet(2);}
    void set_triplet_4(){set_triplet(3);}
    void set_triplet_5(){set_triplet(4);}
    void set_triplet_6(){set_triplet(5);}
    void set_triplet_7(){set_triplet(6);}
    void set_triplet_8(){set_triplet(7);}
    void set_triplet_9(){set_triplet(8);}

    void set_int_x(){set_int(int_t::x);}
    void set_int_a(){set_int(int_t::a);}
    void set_int_b(){set_int(int_t::b);}
    void set_int_p(){set_int(int_t::p);}
    void set_int_t(){set_int(int_t::t);}
    void set_int_xP(){set_int(int_t::xp);}
    void set_int_AB(){set_int(int_t::ab);}
    void set_int_xt(){set_int(int_t::xt);}
    void set_int_Pt(){set_int(int_t::pt);}

    // gateway, uses private flags to determine which plot type is called
    void plot();

private:
    void set_arc(int n);
    void set_triplet(int n);
    void set_int(int_t t);

    // ensure there is only 1 full-size canvas, draw the fully specified plot
    void plot_single(int arc, int triplet, int_t int_type);
    // plot all triplets for the specified intensity plot type
    void plot_one_type(int arc, int_t int_type);
    // plot all intensity plots for the specified triplet
    void plot_one_triplet(int arc, int triplet);

    void setup();
    void setup_consumer(std::string broker, std::string topic);
private slots:
    void on_resetButton_pressed();

    void on_resetButton_released();

    void on_pauseButton_toggled(bool checked);

    void on_autoscaleButton_toggled(bool checked);

    void on_scaleButton_toggled(bool checked);

    void on_colormapComboBox_currentIndexChanged(int index);

    void on_colormapComboBox_currentTextChanged(const QString &arg1);

    void on_intensityMinSpin_valueChanged(int arg1);

    void on_intensityMaxSpin_valueChanged(int arg1);

    void on_intTypeBox_toggled(bool arg1);

    void on_tripletBox_toggled(bool arg);

private:
    Ui::MainWindow *ui;
    bool _triplet_fixed{false};
    bool _type_fixed{false};
    int _fixed_arc{1};
    int _fixed_triplet{1};
    int_t _fixed_type{int_t::unknown};

    ::bifrost::data::Manager * data;
    PlotManager * plots;
    WorkerThread * consumer{};
};
#endif // MAINWINDOW_H
