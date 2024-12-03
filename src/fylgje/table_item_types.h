#pragma once
#include <QTableWidgetItem>
#include <optional>
#include <iostream>

class FylgjeTableItem: public QTableWidgetItem {
public:
  std::string CalibrationUnitStr(){return "";}
protected:
  virtual void setCalibrationUnit(){}
};

class IntTableItem: public FylgjeTableItem {
public:
  IntTableItem(qint32 data=0, bool editable=true){
    data_ = data;
    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (editable){
      flags |= Qt::ItemIsEditable;
    }
    setFlags(flags);
    setSelected(false);
  }
  QVariant data(int role) const {
    if (role == Qt::EditRole) return data_;
    if (role == Qt::DisplayRole) return data_;
    if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    return QTableWidgetItem::data(role);
  }
  void setData(int role, const QVariant &value){
    if (role == Qt::EditRole) {
      data_ = value.toInt();
      tableWidget()->itemChanged(this);
    }
  }
  bool operator<(const IntTableItem & other) const {
    return data_ < other.data_;
  }
protected:
  qint32 data_;
};

class OptIntItem: public FylgjeTableItem {
public:
  OptIntItem(std::optional<int> data){
    data_ = data;
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    setSelected(false);
  }
  QVariant data(int role) const {
    if (role == Qt::EditRole) return data_.has_value() ? data_.value() : QVariant();
    if (role == Qt::DisplayRole) return data_.has_value() ? data_.value() : QVariant();
    if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    return QTableWidgetItem::data(role);
  }
  void setData(int role, const QVariant &value){
    if (role == Qt::EditRole) {
      auto v = value.toInt();
      if (v == data_.value_or(std::numeric_limits<int>::lowest())){
        data_ = std::nullopt;
      } else {
        data_ = v;
      }
      setCalibrationUnit();
      tableWidget()->itemChanged(this);
    }
  }
  bool operator<(const OptIntItem & other) const {
    if (data_.has_value() && other.data_.has_value()){
      return data_.value() < other.data_.value();
    }
    if (data_.has_value()) return false;
    return true;
  }
protected:
  std::optional<int> data_;
};

class CalibrationUnitMinItem: public OptIntItem {
public:
  CalibrationUnitMinItem(CalibrationUnit * unit): OptIntItem(unit->min), unit_(unit){}
protected:
  void setCalibrationUnit(){unit_->min = data_;}
  CalibrationUnit * unit_;
public:
  std::string CalibrationUnitStr(){return std::to_string(unit_->min.value_or(-1));}
};
class CalibrationUnitMaxItem: public OptIntItem {
public:
  CalibrationUnitMaxItem(CalibrationUnit * unit): OptIntItem(unit->max), unit_(unit){}
protected:
  void setCalibrationUnit(){unit_->max = data_;}
  CalibrationUnit * unit_;
public:
  std::string CalibrationUnitStr(){return std::to_string(unit_->max.value_or(-1));}
};


class FloatTableItem: public FylgjeTableItem {
public:
  FloatTableItem(float data=0, bool editable=true){
    data_ = data;
    auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (editable){
      flags |= Qt::ItemIsEditable;
    }
    setFlags(flags);
    setSelected(false);
  }
  QVariant data(int role) const {
    if (role == Qt::EditRole) return data_;
    if (role == Qt::DisplayRole) return data_;
    if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    return QTableWidgetItem::data(role);
  }
  void setData(int role, const QVariant &value){
    if (role == Qt::EditRole) {
      data_ = value.toFloat();
      setCalibrationUnit();
      tableWidget()->itemChanged(this);
    }
  }
  bool operator<(const FloatTableItem & other) const {
    return data_ < other.data_;
  }
protected:
  float data_;
};

class CalibrationUnitLeftItem: public FloatTableItem {
public:
  CalibrationUnitLeftItem(CalibrationUnit * unit): FloatTableItem(unit->left), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->left = data_;
  }
  CalibrationUnit * unit_;
};
class CalibrationUnitRightItem: public FloatTableItem {
public:
  CalibrationUnitRightItem(CalibrationUnit * unit): FloatTableItem(unit->right), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->right = data_;
  }
  CalibrationUnit * unit_;
};


class OptDoubleItem: public FylgjeTableItem {
public:
  OptDoubleItem(std::optional<double> data=0){
    data_ = data;
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    setSelected(false);
  }
  QVariant data(int role) const {
    if (role == Qt::EditRole) return data_.has_value() ? data_.value() : QVariant();
    if (role == Qt::DisplayRole) return data_.has_value() ? data_.value() : QVariant();
    if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    return QTableWidgetItem::data(role);
  }
  void setData(int role, const QVariant &value){
    if (role == Qt::EditRole) {
      auto v = value.toDouble();
      if (v == data_.value_or(std::numeric_limits<double>::lowest())){
        data_ = std::nullopt;
      } else {
        data_ = v;
      }
      setCalibrationUnit();
      tableWidget()->itemChanged(this);
    }
  }
  bool operator<(const OptDoubleItem & other) const {
    if (data_.has_value() && other.data_.has_value()){
      return data_.value() < other.data_.value();
    }
    if (data_.has_value()) return false;
    return true;
  }
protected:
  std::optional<double> data_;
};

class CalibrationUnitC0Item: public OptDoubleItem {
public:
  CalibrationUnitC0Item(CalibrationUnit * unit): OptDoubleItem(unit->c0), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c0 = data_;
  }
  CalibrationUnit * unit_;
};
class CalibrationUnitC1Item: public OptDoubleItem {
public:
  CalibrationUnitC1Item(CalibrationUnit * unit): OptDoubleItem(unit->c1), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c1 = data_;
  }
  CalibrationUnit * unit_;
};

class CalibrationUnitC2Item: public OptDoubleItem {
public:
  CalibrationUnitC2Item(CalibrationUnit * unit): OptDoubleItem(unit->c2), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c2 = data_;
  }
  CalibrationUnit * unit_;
};
class CalibrationUnitC3Item: public OptDoubleItem {
public:
  CalibrationUnitC3Item(CalibrationUnit * unit): OptDoubleItem(unit->c3), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c3 = data_;
  }
  CalibrationUnit * unit_;
};