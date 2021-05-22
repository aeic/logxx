// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_FIXEDLEDGER_H_
#define LOGXX_FIXEDLEDGER_H_

#include <sys/_types/_int32_t.h>
#include <cstdint>  // int32_t, int64_t

#include <functional>  // std::function
#include <stdexcept>   // std::invalid_argument
#include <tuple>       // std::tuple

#include <xlib/filesystem.hpp>
#include <xlib/io/buffer/byte_buffer.hpp>

#include "logxx/basic_ledger.hpp"

namespace logxx {

namespace detail {

class FixedLedgerBase : public detail::BasicLedger {
  using BaseType = detail::BasicLedger;

 public:
  // Types:
  using AppendEntryFunction = std::function<bool(int64_t physical_offset, xlib::byte_buffer byte_buffer)>;
  using CheckEntryFunction = std::function<bool(xlib::byte_buffer)>;

 public:
  FixedLedgerBase(xlib::fs::path ledger_path, int32_t ledger_size, int32_t entry_size)
      : BaseType(std::move(ledger_path), ledger_size), entry_size_(entry_size) {
    if (ledger_size % entry_size != 0) {
      throw std::invalid_argument("ledger_size must be a multiple of entry_size");
    }
  }

  bool AppendEntry(AppendEntryFunction& append_entry_delegate);

  std::tuple<bool, int32_t> CheckEntries(CheckEntryFunction& check_entry_delegate);

 private:
  int32_t entry_size_;
};
}  // namespace detail

template <int32_t entry_size>
class FixedLedger : public detail::FixedLedgerBase {
 public:
  FixedLedger(xlib::fs::path ledger_path, int32_t ledger_size)
      : FixedLedgerBase(std::move(ledger_path), ledger_size, entry_size) {}
};

}  // namespace logxx

#endif  // LOGXX_FIXEDLEDGER_H_
