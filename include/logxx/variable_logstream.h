// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_VARIABLELOGSTREAM_H_
#define LOGXX_VARIABLELOGSTREAM_H_

#include <cstdint>  // int64_t

#include <utility>  // std::move

#include "logxx/basic_logstream.hpp"
#include "logxx/variable_ledger.h"

namespace logxx {

class VariableLogStream : public detail::BasicLogStream<VariableLedger> {
  using LedgerType = VariableLedger;
  using BaseType = detail::BasicLogStream<VariableLedger>;

 public:
  // Types:
  using AppendEntryFunction = LedgerType::AppendEntryFunction;
  using CheckEntryFunction = LedgerType::CheckEntryFunction;

 public:
  using BaseType::BasicLogStream;

  AppendEntryResult AppendEntry(AppendEntryFunction append_entry_delegate) {
    auto* ledger = GetLastLedger(0, true);
    if (ledger == nullptr) {
      // TODO
    }

    auto result = ledger->AppendEntry(append_entry_delegate);
    if (result.status == AppendEntryStatus::kEndOfFile) {
      ledger = GetLastLedger(0, true);
      if (ledger == nullptr) {
        // TODO
      }
      result = ledger->AppendEntry(append_entry_delegate);
    }

    return result;
  }

  int64_t Recover(CheckEntryFunction check_entry_delegate = nullptr) {
    return BaseType::Recover(std::move(check_entry_delegate));
  }

 private:
  void TruncateDirtyEntities(int64_t offset);
};

}  // namespace logxx

#endif  // LOGXX_VARIABLELOGSTREAM_H_
