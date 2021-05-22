// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_APPENDENTRYRESULT_H_
#define LOGXX_APPENDENTRYRESULT_H_

#include <cstdint>

#include "logxx/append_entry_status.h"

namespace logxx {

struct AppendEntryResult {
  AppendEntryStatus status{AppendEntryStatus::kUnknownError};
  int32_t entry_size{0};
  void* extension{nullptr};
};

}  // namespace logxx

#endif  // LOGXX_APPENDENTRYRESULT_H_
