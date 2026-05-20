// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file DaqliteMsgFilter.h
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

#pragma once

/// \brief Install a Qt message handler that drops the
/// `setNetWmStateOnUnmappedWindow` xcb warning when running under VNC.
///
/// VNC sessions are detected via the `VNCDESKTOP` environment variable;
/// outside a VNC session this function is a no-op.
void installDaqliteMsgFilter();
