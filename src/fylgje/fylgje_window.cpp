#include <iostream>
#include "fylgje_window.h"
#include "./ui_fylgje_window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->arcRadio1, &QRadioButton::clicked, this, &MainWindow::set_arc_1);
    connect(ui->arcRadio2, &QRadioButton::clicked, this, &MainWindow::set_arc_2);
    connect(ui->arcRadio3, &QRadioButton::clicked, this, &MainWindow::set_arc_3);
    connect(ui->arcRadio4, &QRadioButton::clicked, this, &MainWindow::set_arc_4);
    connect(ui->arcRadio5, &QRadioButton::clicked, this, &MainWindow::set_arc_5);

    connect(ui->tripRadio1, &QRadioButton::clicked, this, &MainWindow::set_triplet_1);
    connect(ui->tripRadio2, &QRadioButton::clicked, this, &MainWindow::set_triplet_2);
    connect(ui->tripRadio3, &QRadioButton::clicked, this, &MainWindow::set_triplet_3);
    connect(ui->tripRadio4, &QRadioButton::clicked, this, &MainWindow::set_triplet_4);
    connect(ui->tripRadio5, &QRadioButton::clicked, this, &MainWindow::set_triplet_5);
    connect(ui->tripRadio6, &QRadioButton::clicked, this, &MainWindow::set_triplet_6);
    connect(ui->tripRadio7, &QRadioButton::clicked, this, &MainWindow::set_triplet_7);
    connect(ui->tripRadio8, &QRadioButton::clicked, this, &MainWindow::set_triplet_8);
    connect(ui->tripRadio9, &QRadioButton::clicked, this, &MainWindow::set_triplet_9);

    connect(ui->int_AB_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_AB);
    connect(ui->int__t_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_t);
    connect(ui->int_xP_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_xP);
    connect(ui->int_xt_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_xt);
    connect(ui->int__A_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_a);
    connect(ui->int_Pt_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_Pt);
    connect(ui->int__P_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_p);
    connect(ui->int__x_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_x);
    connect(ui->int__B_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_b);

    ui->arcRadio1->setChecked(true);
    set_arc_1();
    ui->tripletBox->setChecked(false);
    ui->tripRadio1->setChecked(true);
    ui->intTypeBox->setChecked(true);
    ui->int_AB_Radio->setChecked(true);

    setup();
}

void MainWindow::setup(){
    data = new ::bifrost::data::Manager(5, 9);
    plots = new PlotManager(data, ui->plotGrid, 3, 3);
//    setup_consumer("", "");
//
//    /// Update timer
//    auto *timer = new QTimer(this);
//    connect(timer, &QTimer::timeout, this, &MainWindow::plot);
//    timer->start(3000);
}
void MainWindow::setup_consumer(std::string broker, std::string topic){
    delete consumer;
    consumer = new WorkerThread(data, std::move(broker), std::move(topic));
    // caengraph.WThread = consumer;
    consumer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::set_arc(int n){
    _fixed_arc = n;
}


void MainWindow::set_triplet(int n){
    _fixed_triplet = n;
}

void MainWindow::set_int(int_t t){
    _fixed_type = t;
}

void MainWindow::plot(){
    _triplet_fixed = ui->tripletBox->isChecked();
    _type_fixed = ui->intTypeBox->isChecked();
    if (_triplet_fixed && _type_fixed) return plot_single(_fixed_arc, _fixed_triplet, _fixed_type);
    if (_triplet_fixed) return plot_one_triplet(_fixed_arc, _fixed_triplet);
    if (_type_fixed) return plot_one_type(_fixed_arc, _fixed_type);
}

void MainWindow::plot_single(int arc, int triplet, int_t t){
    std::cout << "plot single doesn't work properly, this should be unreachable" << std::endl;
    PlotManager::Dim d{PlotManager::Dim::none};
    if (int_t::a == t || int_t::b == t || int_t::x == t || int_t::p == t || int_t::t == t){
        d = PlotManager::Dim::one;
    }
    if (int_t::ab == t || int_t::xt == t || int_t::pt == t || int_t::xp == t){
        d = PlotManager::Dim::two;
    }
    plots->make_single(d);
    if (PlotManager::Dim::one == d){
        plots->plot(0, 0, data->independent_axis(t), data->data_1D(arc, triplet, t));
    }
    if (PlotManager::Dim::two == d){
        plots->plot(0, 0, data->data_2D(arc, triplet, t));
    }
}

void MainWindow::plot_one_type(int arc, int_t t){
    PlotManager::Dim d{PlotManager::Dim::none};
    if (int_t::a == t || int_t::b == t || int_t::x == t || int_t::p == t || int_t::t == t){
        d = PlotManager::Dim::one;
    }
    if (int_t::ab == t || int_t::xt == t || int_t::pt == t || int_t::xp == t){
        d = PlotManager::Dim::two;
    }
    plots->make_all_same(d);
    if (PlotManager::Dim::one == d){
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; ++j) {
                plots->plot(i, j, data->independent_axis(t), data->data_1D(arc, i*3+j, t));
            }
        }
    }
    if (PlotManager::Dim::two == d){
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; ++j) {
                plots->plot(i, j, data->data_2D(arc, i*3+j, t));
            }
        }
    }
}
void MainWindow::plot_one_triplet(int arc, int triplet){
    plots->make_multi();
    plots->plot(0, 0, data->independent_axis(int_t::x), data->data_1D(arc, triplet, int_t::x));
    plots->plot(0, 1, data->independent_axis(int_t::a), data->data_1D(arc, triplet, int_t::a));
    plots->plot(0, 2, data->independent_axis(int_t::p), data->data_1D(arc, triplet, int_t::p));
    plots->plot(1, 2, data->independent_axis(int_t::b), data->data_1D(arc, triplet, int_t::b));
    plots->plot(2, 2, data->independent_axis(int_t::t), data->data_1D(arc, triplet, int_t::t));

    plots->plot(1, 0, data->data_2D(arc, triplet, int_t::xp));
    plots->plot(1, 1, data->data_2D(arc, triplet, int_t::ab));
    plots->plot(2, 0, data->data_2D(arc, triplet, int_t::xt));
    plots->plot(2, 1, data->data_2D(arc, triplet, int_t::pt));
}



void MainWindow::on_resetButton_pressed()
{
    std::cout << "reset button pressed, erase buffers now!" << std::endl;
    plot();
}


void MainWindow::on_resetButton_released()
{
    std::cout << "reset button released, re-enable filling buffers now!" << std::endl;
}


void MainWindow::on_pauseButton_toggled(bool checked)
{
    std::cout << "pause button is now " << (checked ? "on" : "off") << std::endl;
    if (!checked) plot();
}


void MainWindow::on_autoscaleButton_toggled(bool checked)
{
    ui->intensityMaxSpin->setReadOnly(checked);
    ui->intensityMinSpin->setReadOnly(checked);
    std::cout << "autoscale is now " << (checked ? "enabled" : "disabled") << std::endl;
}


void MainWindow::on_scaleButton_toggled(bool checked)
{
    std::cout << "scale is now " << (checked ? "log" : "linear") << std::endl;
}


void MainWindow::on_colormapComboBox_currentIndexChanged(int index)
{
    std::cout << "the colormap selector is now set to " << index << std::endl;
}


void MainWindow::on_colormapComboBox_currentTextChanged(const QString &arg1)
{
    std::cout << "the colormap selector is now set to " << arg1.toStdString() << std::endl;
}


void MainWindow::on_intensityMinSpin_valueChanged(int arg1)
{
    std::cout << "The intensity minimum is now " << arg1 << std::endl;
}


void MainWindow::on_intensityMaxSpin_valueChanged(int arg1)
{
    std::cout << "The intensity maximum is now " << arg1 << std::endl;
}


void MainWindow::on_intTypeBox_toggled(bool arg1)
{
    if (arg1) ui->tripletBox->setChecked(false);
    _type_fixed = arg1;
}


void MainWindow::on_tripletBox_toggled(bool arg)
{
    if (arg) ui->intTypeBox->setChecked(false);
    _triplet_fixed = arg;
}
