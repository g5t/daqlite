// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HelpWindow.h
///
/// \brief Help window with daqlite keyboard/mouse shortcuts
//===----------------------------------------------------------------------===//

#pragma once

#include <QTextEdit>

// Forward declarations
class QLineEdit;
class QToolButton;

/// \brief Window showing a table with all daqlite keyboard and mouse shortcuts
class HelpWindow : public QTextEdit {
  Q_OBJECT

public:
  /// \brief Constructor
  /// \param parent  Parent widget
  HelpWindow(QWidget *parent = nullptr);

  /// \brief Position the clear button in the top right corner
  void updateClearPosition();

  /// \brief  Ensure that the help window is positioned intelligently close to
  ///         the cursor
  /// \param pos  Cursor/anchor position to place the help window near
  void placeHelp(const QPoint &pos);

  /// \brief  Hide/close the window when ESC is pressed
  /// \param event  Key event delivered by Qt
  void keyPressEvent(QKeyEvent *event) override;

  /// \brief Update clear button position when the help window is shown
  /// \param event  Show event delivered by Qt
  void showEvent(QShowEvent* event) override;

  /// \brief Update clear button position when the help window is resized
  /// \param event  Resize event delivered by Qt
  void resizeEvent(QResizeEvent* event) override;

  /// \return a size that matches the size of the help table
  QSize sizeHint() const override;

private:
  /// \brief  Hide this line edit and "steal" its clear button
  QLineEdit *mLineEdit;

  /// \brief Button used to close/hide the help window
  QToolButton *mClearButton;
};

