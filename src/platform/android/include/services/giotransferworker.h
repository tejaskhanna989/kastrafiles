// ── Android include redirect ─────────────────────────────────────────
// This file shadows services/giotransferworker.h on Android. CMake puts
// platform/android/include ahead of the source tree on the include path
// (target_include_directories ... BEFORE), so when fileoperations.cpp does
//   #include "services/giotransferworker.h"
// it resolves *here* and pulls in the pure-Qt twin instead of the GIO one.
// The class name and interface are identical, so no other change is needed.

#pragma once

#include "platform/android/qttransferworker.h"
