#include <QTableWidget>
#include <QStringList>
#include <QLineEdit>

#include "fylgje_window.h"
#include "./ui_fylgje_window.h"

void MainWindow::setup_calibration(){
  setup_calibration_info();
  setup_calibration_table();
  connect(ui->actionSave_Calibration, &QAction::triggered, this, &MainWindow::save_calibration);
}

void MainWindow::setup_calibration_info(){
  auto layout = ui->calibrationLayout->layout();
  QString message{calibration.info().c_str()};
  auto line = new QLineEdit(message, this);
  line->setToolTip("Info");
  layout->addWidget(line);
  connect(line, &QLineEdit::textChanged, this, [&](const QString & text){
    calibration.set_info(text.toStdString());
  });
}

void MainWindow::setup_calibration_table(){
  using Item = QTableWidgetItem;
  auto layout = ui->calibrationLayout->layout();

  std::vector<float> arcs{2.7, 3.2, 3.8, 4.4, 5.0};

  auto rows = configuration.Instrument.groups * configuration.Instrument.units_per_group;
  std::vector<std::pair<std::string, std::function<Item*(int,int,int,CalibrationUnit *)>>> columns{
      {"arc/meV", [&arcs](int arc, int, int, CalibrationUnit *){
        return new FloatTableItem(arcs[arc], false);
      }},
      {"triplet", [](int, int triplet, int, CalibrationUnit *){
        return new IntTableItem(triplet, false); //Item(tr("%1").arg(triplet));
      }},
      {"tube", [](int, int, int tube, CalibrationUnit *){
        return new IntTableItem(tube, false); // Item(tr("%1").arg(tube));
      }},
      {"x left", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitLeftItem(unit);
      }},
      {"x right", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitRightItem(unit); // Item(tr("%1").arg(unit.right));
      }},
      {"height min", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitMinItem(unit);
      }},
      {"height max", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitMaxItem(unit);
      }},
      {"c0", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC0Item(unit);
      }},
      {"c1", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC1Item(unit);
      }},
      {"c2", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC2Item(unit);
      }},
      {"c3", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC3Item(unit);
      }},
  };
  QStringList headers;
  for (const auto & [label, _]: columns){
    headers << label.c_str();
  }

  std::vector<QTableWidgetItem *> table_cells;
  table_cells.reserve(rows * headers.size());

  auto table = new QTableWidget(rows, headers.size(), this);
  layout->addWidget(table);
  table->setHorizontalHeaderLabels(headers);
  for (int group=0; group<configuration.Instrument.groups; ++group){
    for (int unit=0; unit<configuration.Instrument.units_per_group; ++unit){
      auto arc = group / 9;
      auto triplet = group % 9;
      auto row = group * configuration.Instrument.units_per_group + unit;
      for (int column=0; column < headers.size(); ++column){
        // this item handles updating the editable unit fields without a separate callback
        auto item = columns[column].second(arc, triplet, unit, calibration.unit_pointer(group, unit));
        table->setItem(row, column, item);
      }
    }
  }
}


void MainWindow::save_calibration() {
  using namespace std::filesystem;
  auto q_filename = QFileDialog::getSaveFileName(this, tr("Save Configuration"), "", tr("JSON files (*.json *.JSON)"));
  auto p = path(q_filename.toStdString());
  if (!p.has_filename()) return;
  auto ext = std::string(p.extension());
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
  if (ext != ".json"){
    p.replace_extension(path(".json"));
  }
  nlohmann::json j;
  calibration.set_date();
  j = calibration;
  to_json_file(j, std::string(p));
}