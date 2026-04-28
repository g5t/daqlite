// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file VNCMessageFilter.cpp
///
/// \brief Suppress benign Qt xcb warnings emitted by VNC X servers
///
//===----------------------------------------------------------------------===//

#include "VNCMessageFilter.h"

#include <QString>
#include <QtGlobal>

#include <cstdlib>
#include <cstring>

namespace {
  QtMessageHandler PrevHandler = nullptr;

  void filter(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    const bool IsXcbInternal =
        ctx.category != nullptr &&
        std::strcmp(ctx.category, "qt.qpa.xcb") == 0 &&
        msg.contains(QStringLiteral("setNetWmStateOnUnmappedWindow"));
    if (IsXcbInternal) {
      return;
    }
    if (PrevHandler != nullptr) {
      PrevHandler(type, ctx, msg);
    }
  }
}

void installVNCMessageFilter() {
  if (std::getenv("VNCDESKTOP") != nullptr) {
    PrevHandler = qInstallMessageHandler(filter);
  }
}
