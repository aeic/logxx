// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_FIXEDLOGSTREAM_H_
#define LOGXX_FIXEDLOGSTREAM_H_

#include <cstdint>  // int64_t

#include "logxx/basic_logstream.hpp"
#include "logxx/fixed_ledger.h"

namespace logxx {

template <int32_t entry_size>
class FixedLogStream : public detail::BasicLogStream<FixedLedger<entry_size>> {
  using LedgerType = FixedLedger<entry_size>;
  using BaseType = detail::BasicLogStream<LedgerType>;

 public:
  // Types:
  using AppendEntryFunction = typename LedgerType::AppendEntryFunction;
  using CheckEntryFunction = typename LedgerType::CheckEntryFunction;

 public:
  FixedLogStream(xlib::fs::path ledger_dir, int32_t ledger_size) : BaseType(std::move(ledger_dir), ledger_size) {}

  bool AppendEntry(AppendEntryFunction append_entry_delegate) {
    auto* ledger = BaseType::GetLastLedger(0, true);
    if (ledger == nullptr) {
      return false;
    }

    return ledger->AppendEntry(append_entry_delegate);
  }

  int64_t Recover(CheckEntryFunction check_entry_delegate) {
    return BaseType::Recover(std::move(check_entry_delegate));
  }
};

}  // namespace logxx

#endif  // LOGXX_FIXEDLOGSTREAM_H_
