// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_VARIABLELEDGER_H_
#define LOGXX_VARIABLELEDGER_H_

#include <cstdint>  // int32_t, int64_t

#include <functional>  // std::function
#include <tuple>       // std::tuple

#include <xlib/io/buffer/byte_buffer.hpp>

#include "logxx/append_entry_result.h"
#include "logxx/basic_ledger.hpp"

namespace logxx {

class VariableLedger : public detail::BasicLedger {
  using BaseType = detail::BasicLedger;

 public:
  // Types:
  using AppendEntryFunction = std::function<AppendEntryResult(int64_t physical_offset, xlib::byte_buffer byte_buffer)>;
  using CheckEntryFunction = std::function<bool(xlib::byte_buffer)>;

 public:
  using BaseType::BasicLedger;

  AppendEntryResult AppendEntry(AppendEntryFunction& append_entry_delegate);

  xlib::byte_buffer SelectEntry(int64_t physical_offset, int32_t expected_size);

  std::tuple<bool, int32_t> CheckEntries(CheckEntryFunction& check_entry_delegate);
};

}  // namespace logxx

#endif  // LOGXX_VARIABLELEDGER_H_
