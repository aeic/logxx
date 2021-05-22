// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "logxx/fixed_ledger.h"

#include <cstddef>  // size_t
#include <cstdint>  // int32_t, int64_t

#include <tuple>  // std::tuple, std::make_tuple

#include <xlib/io/buffer/byte_buffer.hpp>

namespace logxx::detail {

bool FixedLedgerBase::AppendEntry(AppendEntryFunction& append_entry_delegate) {
  int32_t current_position = wrote_position();
  if (current_position >= size()) {
    return false;
  }

  int64_t physical_offset = begin_offset() + current_position;
  xlib::byte* entry_start = data() + current_position;

  xlib::byte_buffer byte_buffer{entry_start, static_cast<size_t>(entry_size_)};

  bool result = append_entry_delegate(physical_offset, byte_buffer);
  if (result) {
    set_wrote_position(current_position + entry_size_);
  }

  return result;
}

std::tuple<bool, int32_t> FixedLedgerBase::CheckEntries(CheckEntryFunction& check_entry_delegate) {
  xlib::byte_buffer byte_buffer{data(), static_cast<size_t>(size())};
  int32_t ledger_offset = 0;
  while (byte_buffer.has_remaining()) {
    auto buffer = byte_buffer.slice(entry_size_);
    if (!check_entry_delegate(std::move(buffer))) {
      break;
    }
    byte_buffer.skip(entry_size_);
  }
  return std::make_tuple(!byte_buffer.has_remaining(), static_cast<int32_t>(byte_buffer.position()));
}

}  // namespace logxx::detail
