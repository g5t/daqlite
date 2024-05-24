#include <iostream>
#include <sstream>
#include <fmt/core.h>
#include "fylgje_window.h"
#include "./ui_fylgje_window.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  maxBox = {ui->intMax00, ui->intMax01, ui->intMax02,
            ui->intMax10, ui->intMax11, ui->intMax12,
            ui->intMax20, ui->intMax21, ui->intMax22};
//    minBox = {ui->intMin00, ui->intMin01, ui->intMin02,
//              ui->intMin10, ui->intMin11, ui->intMin12,
//              ui->intMin20, ui->intMin21, ui->intMin22};


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

  connect(ui->resetButton, &QPushButton::pressed, this, &MainWindow::reset);
  connect(ui->scaleButton, &QPushButton::pressed, this, &MainWindow::plot);
  connect(ui->autoscaleButton, &QPushButton::toggled, this, &MainWindow::autoscale_toggled);
  connect(ui->intTypeBox, &QGroupBox::toggled, this, &MainWindow::update_intensity_limits);
  connect(ui->tripletBox, &QGroupBox::toggled, this, &MainWindow::update_intensity_limits);
  connect(ui->pauseButton, &QPushButton::toggled, this, &MainWindow::pause_toggled);

  setup_add_bin_boxes();
  setup_intensity_limits();
  setup_gradient_list();
  setup();
}

void MainWindow::setup_add_bin_boxes() {
    auto layout = ui->binsFrame->layout();
    std::tuple<QString, int_t, void (MainWindow::*)(int), void (MainWindow::*)(int)> groups[]{
        {"A", int_t::a, &MainWindow::set_bins_a_1d, &MainWindow::set_bins_a_2d},
        {"B", int_t::b, &MainWindow::set_bins_b_1d, &MainWindow::set_bins_b_2d},
        {"x", int_t::x, &MainWindow::set_bins_x_1d, &MainWindow::set_bins_x_2d},
        {"p", int_t::p, &MainWindow::set_bins_p_1d, &MainWindow::set_bins_p_2d},
        {"t", int_t::t, &MainWindow::set_bins_t_1d, &MainWindow::set_bins_t_2d},
    };
    auto make_label = [](const QString & name){
        auto label = new QLabel();
        label->setText(name);
        label->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        return label;
    };
    for (const auto& [name, t, call1, call2]: groups){
        layout->addWidget(make_label(name));
        auto k1 = std::make_pair(t, 1);
        auto k2 = std::make_pair(t, 2);

        bin_boxes[k1] = new TwoSpinBox(4, 1024);
        layout->addWidget(bin_boxes.at(k1));
        connect(bin_boxes.at(k1), QOverload<int>::of(&TwoSpinBox::valueChanged), this, call1);

        bin_boxes[k2] = new TwoSpinBox(4, 512);
        layout->addWidget(bin_boxes.at(k2));
        connect(bin_boxes.at(k2), QOverload<int>::of(&TwoSpinBox::valueChanged), this, call2);
    }
}

void MainWindow::setup_intensity_limits() {
  for (int arc=0; arc<5; ++arc){
    for (int triplet=0; triplet<9; ++triplet){
      for (const auto & t: ::bifrost::data::TYPEND){
        auto key = make_key(arc, triplet, t);
//        min[key] = 0;
        max[key] = 1;
      }
    }
  }
  auto signal = QOverload<int>::of(&QSpinBox::valueChanged);
  auto slot = &MainWindow::get_intensity_limits;
//  for (auto sender: minBox) connect(sender, signal, this, slot);
  for (auto sender: maxBox) connect(sender, signal, this, slot);
}

void MainWindow::setup_gradient_list(){
  ui->colormapComboBox->clear();
  for (auto name: {"gray", "hot", "cold", "night", "candy", "geography", "thermal"}){
    ui->colormapComboBox->addItem(QString(name));
  }
  ui->colormapComboBox->setCurrentText(QString("geography"));
}

void MainWindow::setup(){
    data = new ::bifrost::data::Manager(5, 9);
    plots = new PlotManager(ui->plotGrid, 3, 3);

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
    update_intensity_limits();
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
    update_intensity_limits();
    plot();
}

void MainWindow::set_int(int_t t){
    _fixed_type = t;
    update_intensity_limits();
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
  //plots->scale(minima, maxima, !ui->scaleButton->isChecked());
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
    auto key = make_key(arc, triplet, t);
    auto is_log = ui->scaleButton->isChecked();
    auto gradient = ui->colormapComboBox->currentText().toStdString();
    auto is_inverted = ui->colormapInvertedCheck->isChecked();
    if (PlotManager::Dim::one == d){
        plots->plot(0, 0, data->axis(t), data->data_1D(arc, triplet, t), 0.0, 1.0*max[key], is_log);
    }
    if (PlotManager::Dim::two == d){
        plots->plot(0, 0, data->data_2D(arc, triplet, t), 0.0, 1.0*max[key], is_log, gradient, is_inverted);
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
    auto is_log = ui->scaleButton->isChecked();
    if (PlotManager::Dim::one == d){
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; ++j) {
              auto key = make_key(arc, i*3+j, t);
              plots->plot(i, j, data->axis(t), data->data_1D(arc, i*3+j, t), 0.0, 1.0*max[key], is_log);
            }
        }
    }
    if (PlotManager::Dim::two == d){
      auto gradient = ui->colormapComboBox->currentText().toStdString();
      auto is_inverted = ui->colormapInvertedCheck->isChecked();
        for (int i=0; i<3; ++i) {
            for (int j=0; j<3; ++j) {
              auto key = make_key(arc, i*3+j, t);
              plots->plot(i, j, data->data_2D(arc, i*3+j, t), 0.0, 1.0*max[key], is_log, gradient, is_inverted);
            }
        }
    }
}
void MainWindow::plot_one_triplet(int arc, int triplet){
  int_t ts[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  plots->make_multi(ts);

  int i[]{0,0,0,1,1,1,2,2,2};
  int j[]{0,1,2,0,1,2,0,1,2};
  auto is_log = ui->scaleButton->isChecked();
  for (int t: {0, 1, 2, 5, 8}){
    auto key = make_key(arc, triplet, ts[t]);
    plots->plot(i[t], j[t], data->axis(ts[t]), data->data_1D(arc, triplet, ts[t]), 0.0, 1.0*max[key], is_log);
  }
  auto gradient = ui->colormapComboBox->currentText().toStdString();
  auto is_inverted = ui->colormapInvertedCheck->isChecked();
  for (int t: {3, 4, 6, 7}){
    auto key = make_key(arc, triplet, ts[t]);
    plots->plot(i[t], j[t], data->data_2D(arc, triplet, ts[t]), 0.0, 1.0*max[key], is_log, gradient, is_inverted);
  }
}

void MainWindow::reset(){
  data->clear();
}


void MainWindow::pause_toggled(bool checked)
{
  std::cout << "pause button is now " << (checked ? "on" : "off") << std::endl;
  if (!checked) plot();
  std::cout << "Intensity limits" << std::endl;
  for (auto [key, value]: max){
    auto [arc, triplet, type] = key;
    std::stringstream stype;
    stype << type;
    fmt::print("{:1d} {:1d} {:6s} ({}, {})\n", arc, triplet, stype.str(), 0, value);
  }
}


void MainWindow::autoscale_toggled(bool check)
{
  for (auto x: maxBox) x->setReadOnly(check);
//  for (auto x: minBox) x->setReadOnly(check);
  if (check) set_intensity_limits();
}

void MainWindow::set_intensity_limits_triplets(){
  for (int triplet=0; triplet<9; ++triplet){
    auto key = make_key(_fixed_arc, triplet, _fixed_type);
    maxBox[triplet]->setValue(max.at(key));
//    minBox[triplet]->setValue(min.at(key));
  }
}

void MainWindow::get_intensity_limits_triplets(){
  for (int triplet=0; triplet<9; ++triplet){
    auto key = make_key(_fixed_arc, triplet, _fixed_type);
    max.at(key) = maxBox[triplet]->value();
//    min.at(key) = minBox[triplet]->value();
  }
}

void MainWindow::auto_intensity_limits_triplets(){
  for (int triplet=0; triplet<9; ++triplet){
    auto key = make_key(_fixed_arc, triplet, _fixed_type);
    max.at(key) = static_cast<int>(data->max(_fixed_arc, triplet, _fixed_type));
//    min.at(key) = static_cast<int>(data->min(_fixed_arc, triplet, _fixed_type));
  }
}

void MainWindow::set_intensity_limits_types(){
  int_t types[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  for (int type=0; type<9; ++type){
    auto key = make_key(_fixed_arc, _fixed_triplet, types[type]);
    maxBox[type]->setValue(max.at(key));
//    minBox[type]->setValue(min.at(key));
  }
}

void MainWindow::get_intensity_limits_types(){
  int_t types[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  for (int type=0; type<9; ++type){
    auto key = make_key(_fixed_arc, _fixed_triplet, types[type]);
    max.at(key) = maxBox[type]->value();
//    min.at(key) = minBox[type]->value();
  }
}

void MainWindow::auto_intensity_limits_types(){
  int_t types[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  for (auto & type : types){
    auto key = make_key(_fixed_arc, _fixed_triplet, type);
    max.at(key) = static_cast<int>(data->max(_fixed_arc, _fixed_triplet, type));
//    min.at(key) = static_cast<int>(data->min(_fixed_arc, _fixed_triplet, type));
  }
}

void MainWindow::set_intensity_limits_singular(){
  auto key = make_key(_fixed_arc, _fixed_triplet, _fixed_type);
  maxBox[0]->setValue(max.at(key));
//  minBox[0]->setValue(min.at(key));
}

void MainWindow::get_intensity_limits_singular(){
  auto key = make_key(_fixed_arc, _fixed_triplet, _fixed_type);
  max.at(key) = maxBox[0]->value();
//  min.at(key) = minBox[0]->value();
}

void MainWindow::auto_intensity_limits_singular(){
  auto key = make_key(_fixed_arc, _fixed_triplet, _fixed_type);
  max.at(key) = static_cast<int>(data->max(_fixed_arc, _fixed_triplet, _fixed_type));
//  min.at(key) = static_cast<int>(data->min(_fixed_arc, _fixed_triplet, _fixed_type));
}

void MainWindow::set_intensity_limits() {
  if (ui->autoscaleButton->isChecked()) {
    auto_intensity_limits();
    update_intensity_limits();
  } else {
    get_intensity_limits();
  }
}

void MainWindow::auto_intensity_limits() {
  if (ui->tripletBox->isChecked() && !ui->intTypeBox->isChecked()){
    auto_intensity_limits_types();
  } else if (ui->intTypeBox->isChecked() && !ui->tripletBox->isChecked()){
    auto_intensity_limits_triplets();
  } else if (ui->intTypeBox->isChecked() && ui->tripletBox->isChecked()){
    auto_intensity_limits_singular();
  }
}

void MainWindow::update_intensity_limits() {
  if (ui->tripletBox->isChecked() && !ui->intTypeBox->isChecked()){
    set_intensity_limits_types();
  } else if (ui->intTypeBox->isChecked() && !ui->tripletBox->isChecked()){
    set_intensity_limits_triplets();
  } else if (ui->intTypeBox->isChecked() && ui->tripletBox->isChecked()){
    set_intensity_limits_singular();
  }
}

void MainWindow::get_intensity_limits(){
  if (ui->tripletBox->isChecked() && !ui->intTypeBox->isChecked()){
    get_intensity_limits_triplets();
  } else if (ui->intTypeBox->isChecked() && !ui->tripletBox->isChecked()){
    get_intensity_limits_types();
  } else if (ui->intTypeBox->isChecked() && ui->tripletBox->isChecked()){
    get_intensity_limits_singular();
  }
}