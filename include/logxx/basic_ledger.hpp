// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_BASICLEDGER_HPP_
#define LOGXX_BASICLEDGER_HPP_

#include <cstddef>  // size_t
#include <cstdint>  // int32_t, int64_t

#include <atomic>        // std::atomic
#include <system_error>  // std::error_code
#include <utility>       // std::move

#include <xlib/filesystem.hpp>
#include <xlib/io/buffer/byte_buffer.hpp>
#include <xlib/mmap.hpp>
#include <xlib/types/byte.hpp>

namespace logxx::detail {

/**
 * @brief A ledger is a sequence of entries, and each entry is a sequence of bytes.
 * @details
 * Entries are written sequentially to a ledger and at most once. Consequently, ledgers have an append-only semantics.
 */
class BasicLedger {
 public:
  BasicLedger(xlib::fs::path ledger_path, int32_t ledger_size) : path_(std::move(ledger_path)), size_(ledger_size) {
    try {
      xlib::fs::ensure_regular_file(path_, size_);
    } catch (const xlib::fs::filesystem_error& ex) {
      // TODO: logging
      throw;
    }

    begin_offset_ = std::stoll(path_.filename());

    std::error_code ec;
    mmap_block_ = xlib::mmap::make_mmap_sink(path_.string(), 0, size_, ec);
    if (ec) {
      throw xlib::fs::filesystem_error("mmap failed", path_, ec);
    }
  }

  bool Full() { return wrote_position_.load(std::memory_order_relaxed) >= size_; }

  xlib::byte_buffer SelectBuffer(int64_t physical_offset, size_t buffer_size) {
    if (physical_offset < begin_offset()) {
      throw std::runtime_error("buffer underflow");
    }
    if (physical_offset + buffer_size > end_offset()) {
      throw std::runtime_error("buffer overflow");
    }
    int64_t logical_offset = physical_offset - begin_offset();
    return xlib::byte_buffer{data() + logical_offset, buffer_size};
  }

  bool operator<(int64_t offset) const { return offset < begin_offset(); }
  bool operator>(int64_t offset) const { return offset >= end_offset(); }

 public:
  int64_t begin_offset() const { return begin_offset_; }
  int64_t end_offset() const { return begin_offset_ + wrote_position(); }

  xlib::byte* data() { return mmap_block_.data(); }
  int32_t size() const { return size_; }

  int32_t wrote_position() const { return wrote_position_.load(std::memory_order_relaxed); }
  void set_wrote_position(int32_t wrote_position) { wrote_position_.store(wrote_position, std::memory_order_relaxed); }

 private:
  xlib::fs::path path_;
  int32_t size_;
  int64_t begin_offset_;

  xlib::mmap::mmap_sink mmap_block_;

  std::atomic<int32_t> wrote_position_{0};
};

}  // namespace logxx::detail

#endif  // LOGXX_BASICLEDGER_HPP_
