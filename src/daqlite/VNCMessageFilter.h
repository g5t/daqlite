// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file VNCMessageFilter.h
///
/// \brief Suppress benign Qt xcb warnings emitted by VNC X servers
///
//===----------------------------------------------------------------------===//

#pragma once

/// \brief Install a Qt message handler that drops the
/// `setNetWmStateOnUnmappedWindow` xcb warning when running under VNC.
///
/// VNC sessions are detected via the `VNCDESKTOP` environment variable;
/// outside a VNC session this function is a no-op.
void installVNCMessageFilter();
