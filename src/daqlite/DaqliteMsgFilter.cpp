// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file DaqliteMsgFilter.cpp
///
/// \brief Suppress benign Qt xcb warnings emitted by VNC X servers
///
/// \note If more messages need to be suppressed, this handler can be
/// generalized by replacing the inline `{category, needle}` check with a
/// table of suppression rules iterated by the filter callback. Each rule
/// can carry its own activation gate (e.g. an environment variable) so
/// VNC-only and always-on rules coexist cleanly.
///
//===----------------------------------------------------------------------===//

#include <DaqliteMsgFilter.h>

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

void installDaqliteMsgFilter() {
  if (std::getenv("VNCDESKTOP") != nullptr) {
    PrevHandler = qInstallMessageHandler(filter);
  }
}
