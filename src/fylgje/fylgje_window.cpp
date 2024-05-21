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

    connect(ui->arcCycleCheck, &QCheckBox::clicked, this, &MainWindow::cycle_arc_toggle);

    _fixed_arc = 0;
    ui->arcRadio1->setChecked(true);
    _fixed_triplet = 0;
    ui->tripletBox->setChecked(false);
    ui->tripRadio1->setChecked(true);
    _fixed_type = int_t::ab;
    ui->intTypeBox->setChecked(true);
    ui->int_AB_Radio->setChecked(true);

    setup();
}

void MainWindow::setup(){
    data = new ::bifrost::data::Manager(5, 9);
    plots = new PlotManager(data, ui->plotGrid, 3, 3);

    broker = "localhost:9092";
    topic = "bifrost_detector_samples";
    setup_consumer(broker, topic);
//
    /// Update timer
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::plot);
    timer->start(100);
}
void MainWindow::setup_consumer(std::string b, std::string t){
    delete consumer;
    consumer = new WorkerThread(data, std::move(b), std::move(t));
    // caengraph.WThread = consumer;
    consumer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::set_arc(int n){
    _fixed_arc = n;
    plot();
}

void MainWindow::cycle_arc() {
    if (!ui->arcCycleCheck->isChecked()) return;
    _fixed_arc = (_fixed_arc + 1) % 5;
    switch (_fixed_arc){
        case 0: ui->arcRadio1->setChecked(true); break;
        case 1: ui->arcRadio2->setChecked(true); break;
        case 2: ui->arcRadio3->setChecked(true); break;
        case 3: ui->arcRadio4->setChecked(true); break;
        case 4: ui->arcRadio5->setChecked(true); break;
        default: std::cout << "Error cycling arc value" << std::endl;
    }
    plot();
    if (ui->arcCycleCheck->isChecked())
        QTimer::singleShot(static_cast<int>(ui->arcPeriodSpin->value() * 1000), this, &MainWindow::cycle_arc);
}
void MainWindow::cycle_arc_toggle(bool checked){
    if (checked) cycle_arc();
}

void MainWindow::set_triplet(int n){
    _fixed_triplet = n;
    plot();
}

void MainWindow::set_int(int_t t){
    _fixed_type = t;
    plot();
}

void MainWindow::plot(){
  set_intensity_limits();
  _triplet_fixed = ui->tripletBox->isChecked();
  _type_fixed = ui->intTypeBox->isChecked();
  if (_triplet_fixed && _type_fixed) {
    plot_single(_fixed_arc, _fixed_triplet, _fixed_type);
  } else if (_triplet_fixed) {
    plot_one_triplet(_fixed_arc, _fixed_triplet);
  } else if (_type_fixed) {
    plot_one_type(_fixed_arc, _fixed_type);
  }
  plots->scale(minima, maxima, !ui->scaleButton->isChecked());
}

void MainWindow::plot_single(int arc, int triplet, int_t t){
    PlotManager::Dim d{PlotManager::Dim::none};
    if (int_t::a == t || int_t::b == t || int_t::x == t || int_t::p == t || int_t::t == t){
        d = PlotManager::Dim::one;
    }
    if (int_t::ab == t || int_t::xt == t || int_t::pt == t || int_t::xp == t){
        d = PlotManager::Dim::two;
    }
    plots->make_single(d, t);
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
    } else if (int_t::ab == t || int_t::xt == t || int_t::pt == t || int_t::xp == t){
        d = PlotManager::Dim::two;
    }
    plots->make_all_same(d, t);
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
    int_t ts[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
    plots->make_multi(ts);

    plots->plot(0, 0, data->independent_axis(ts[0]), data->data_1D(arc, triplet, ts[0]));
    plots->plot(0, 1, data->independent_axis(ts[1]), data->data_1D(arc, triplet, ts[1]));
    plots->plot(0, 2, data->independent_axis(ts[2]), data->data_1D(arc, triplet, ts[2]));
    plots->plot(1, 2, data->independent_axis(ts[5]), data->data_1D(arc, triplet, ts[5]));
    plots->plot(2, 2, data->independent_axis(ts[8]), data->data_1D(arc, triplet, ts[8]));

    plots->plot(1, 0, data->data_2D(arc, triplet, ts[3]));
    plots->plot(1, 1, data->data_2D(arc, triplet, ts[4]));
    plots->plot(2, 0, data->data_2D(arc, triplet, ts[6]));
    plots->plot(2, 1, data->data_2D(arc, triplet, ts[7]));
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


void MainWindow::on_autoscaleButton_toggled(bool check)
{
  ui->intMax_a->setReadOnly(check);
  ui->intMax_b->setReadOnly(check);
  ui->intMax_p->setReadOnly(check);
  ui->intMax_x->setReadOnly(check);
  ui->intMax_t->setReadOnly(check);
  ui->intMax_ab->setReadOnly(check);
  ui->intMax_xt->setReadOnly(check);
  ui->intMax_px->setReadOnly(check);
  ui->intMax_pt->setReadOnly(check);
  if (check) set_intensity_limits();
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


void MainWindow::on_intTypeBox_toggled(bool arg1)
{
//  ui->tripletBox->setChecked(!arg1);
  _type_fixed = arg1;
}


void MainWindow::on_tripletBox_toggled(bool arg)
{
//  ui->intTypeBox->setChecked(!arg);
  _triplet_fixed = arg;
}

void MainWindow::set_intensity_limits() {
  int_t ts[]{int_t::a, int_t::b, int_t::x, int_t::p, int_t::t, int_t::ab, int_t::xt, int_t::pt, int_t::xp};
  if (ui->autoscaleButton->isChecked()) {
    for (auto t: ts){
      auto m = static_cast<int>(data->max(_fixed_arc, t));
      maxima[t] = m ? m : 1;
    }
    ui->intMax_a->setValue(maxima[int_t::a]);
    ui->intMax_b->setValue(maxima[int_t::b]);
    ui->intMax_x->setValue(maxima[int_t::x]);
    ui->intMax_p->setValue(maxima[int_t::p]);
    ui->intMax_t->setValue(maxima[int_t::t]);
    ui->intMax_ab->setValue(maxima[int_t::ab]);
    ui->intMax_xt->setValue(maxima[int_t::xt]);
    ui->intMax_pt->setValue(maxima[int_t::pt]);
    ui->intMax_px->setValue(maxima[int_t::xp]);
  } else {
    maxima[int_t::a] = ui->intMax_a->value();
    maxima[int_t::b] = ui->intMax_b->value();
    maxima[int_t::x] = ui->intMax_x->value();
    maxima[int_t::p] = ui->intMax_p->value();
    maxima[int_t::t] = ui->intMax_t->value();
    maxima[int_t::ab] = ui->intMax_ab->value();
    maxima[int_t::xt] = ui->intMax_xt->value();
    maxima[int_t::pt] = ui->intMax_pt->value();
    maxima[int_t::xp] = ui->intMax_px->value();
  }

}