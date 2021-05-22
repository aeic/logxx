// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "logxx/variable_ledger.h"

#include <cstddef>  // size_t
#include <cstdint>  // int32_t, int64_t

#include <stdexcept>  // std::runtime_error
#include <tuple>      // std::tuple, std::make_tuple

#include <xlib/io/buffer/byte_buffer.hpp>

namespace logxx {

namespace {

constexpr int32_t kEntrySizeFieldLength = 4;
constexpr int32_t kMagicCodeFieldLength = 4;
constexpr int32_t kEntryHeaderLength = kEntrySizeFieldLength + kMagicCodeFieldLength;
constexpr int32_t kEndFileMinBlankLength = kEntryHeaderLength;

constexpr int32_t kEntryMagicCode = -626843481;
constexpr int32_t kBlankMagicCode = -875286124;

}  // namespace

AppendEntryResult VariableLedger::AppendEntry(AppendEntryFunction& append_entry_delegate) {
  int32_t current_position = wrote_position();
  if (current_position >= size()) {
    return {AppendEntryStatus::kUnknownError, 0};
  }

  int64_t physical_offset = begin_offset() + current_position;
  xlib::byte* entry_start = data() + current_position;
  size_t blank_size = size() - current_position;

  // entry_frame: entry_size(4) + magic_code(4) + entry_body
  xlib::byte_buffer byte_buffer{entry_start, blank_size, blank_size - kEndFileMinBlankLength, kEntryHeaderLength};

  auto result = append_entry_delegate(physical_offset, byte_buffer);
  if (result.status == AppendEntryStatus::kOk) {
    // fill header of entry_frame
    byte_buffer.clear();
    byte_buffer.put<int32_t>(kEntryHeaderLength + result.entry_size);
    byte_buffer.put<int32_t>(kEntryMagicCode);
    set_wrote_position(current_position + kEntryHeaderLength + result.entry_size);
  } else if (result.status == AppendEntryStatus::kEndOfFile) {
    // fill black_frame
    byte_buffer.clear();
    byte_buffer.put<int32_t>(static_cast<int32_t>(blank_size));
    byte_buffer.put<int32_t>(kBlankMagicCode);
    set_wrote_position(size());
  }

  return result;
}

xlib::byte_buffer VariableLedger::SelectEntry(int64_t physical_offset, int32_t expected_size) {
  auto header_buffer = SelectBuffer(physical_offset, kEntryHeaderLength);
  auto entry_size = header_buffer.get<int32_t>();
  auto magic_code = header_buffer.get<int32_t>();
  if (magic_code != kEntryMagicCode) {
    throw std::runtime_error("not entry");
  }
  if (expected_size > entry_size) {
    throw std::runtime_error("buffer overflow");
  }
  return {header_buffer.raw(), static_cast<size_t>(entry_size), static_cast<size_t>(expected_size), kEntryHeaderLength};
}

namespace {

int32_t CheckEntry(xlib::byte_buffer& byte_buffer, VariableLedger::CheckEntryFunction& check_entry_delegate) {
  if (byte_buffer.remaining() < kEntryHeaderLength) {
    [[unlikely]] return -2;
  }

  auto entry_size = byte_buffer.get<int32_t>();

  auto magic_code = byte_buffer.get<int32_t>();
  if (magic_code == kBlankMagicCode) {
    [[unlikely]] return 0;
  }
  if (magic_code != kEntryMagicCode) {
    [[unlikely]] return -1;
  }

  if (check_entry_delegate) {
    auto buffer = byte_buffer.slice(entry_size - kEntryHeaderLength);
    if (!check_entry_delegate(std::move(buffer))) {
      return -1;
    }
  }

  byte_buffer.skip(entry_size - kEntryHeaderLength);

  return entry_size;
}

}  // namespace

std::tuple<bool, int32_t> VariableLedger::CheckEntries(CheckEntryFunction& check_entry_delegate) {
  xlib::byte_buffer byte_buffer{data(), static_cast<size_t>(size())};
  int32_t ledger_offset = 0;
  for (;;) {
    int32_t entry_size = CheckEntry(byte_buffer, check_entry_delegate);
    if (entry_size > 0) {
      // Normal data
      ledger_offset += entry_size;
      continue;
    }
    // Come the end of the file, or intermediate file read error
    return std::make_tuple(entry_size == 0, ledger_offset);
  }
}

}  // namespace logxx
