// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_BASICLOGSTREAM_HPP_
#define LOGXX_BASICLOGSTREAM_HPP_

#include <cstdint>  // int32_t, int64_t

#include <algorithm>   // std::lower_bound
#include <functional>  // std::function
#include <memory>      // std::unique_ptr
#include <string>      // std::string
#include <utility>     // std::move
#include <vector>      // std::vector

#include <absl/strings/str_format.h>
#include <xlib/filesystem.hpp>
#include <xlib/memory/owner.hpp>

namespace logxx::detail {

constexpr size_t kLedgerFilenameLength = 20;

/**
 * @brief LogStream are "ledgers", and each unit of a log (aka record) is a "ledger entry".
 */
template <typename Ledger>
class BasicLogStream {
 public:
  BasicLogStream(xlib::fs::path ledger_dir, int32_t ledger_size)
      : ledger_dir_{std::move(ledger_dir)}, ledger_size_{ledger_size} {}

  /**
   * @brief Scan ledger directory to build ledger sequence
   */
  void Load() {
    try {
      xlib::fs::ensure_directory(ledger_dir_);
    } catch (const xlib::fs::filesystem_error& ex) {
      // TODO: logging
      throw;
    }

    // scan ledgers
    std::vector<std::pair<xlib::fs::path, size_t>> ledger_condidates;
    for (const auto& entry : xlib::fs::directory_iterator{ledger_dir_}) {
      // check file type
      if (!entry.is_regular_file()) {
        continue;
      }

      // check file name
      const auto& filename = entry.path().filename().native();
      if (filename.length() != kLedgerFilenameLength && filename.find_first_not_of("0123456789") != std::string::npos) {
        continue;
      }

      // check file size
      if (entry.file_size() != ledger_size_) {
        // TODO: loging
        // log.warn(file + "\t" + file.length() + " length not matched message store config value, please check it
        // manually");
      }

      ledger_condidates.emplace_back(entry.path(), entry.file_size());
    }

    if (ledger_condidates.empty()) {
      return;
    }

    // desc
    std::sort(ledger_condidates.begin(), ledger_condidates.end(),
              [](const std::pair<xlib::fs::path, size_t>& a, const std::pair<xlib::fs::path, size_t>& b) {
                return a.first > b.first;
              });

    const auto& [latest_ledger_path, latest_ledger_size] = ledger_condidates.back();
    size_t last_offset = std::stoll(latest_ledger_path.filename().native()) + latest_ledger_size;

    // double check
    for (const auto& [condidate_path, condidate_size] : ledger_condidates) {
      const auto& condidate_filename = condidate_path.filename().native();

      // check offset
      size_t next_offset = std::stoll(condidate_filename) + condidate_size;
      if (next_offset != last_offset) {
        // discontinuous
        break;
      }
      last_offset = next_offset;

      try {
        auto ledger = std::make_unique<Ledger>(condidate_path, condidate_size);
        ledger->set_wrote_position(ledger->size());
        // Ledger.setFlushedPosition(this.LedgerSize);
        // Ledger.setCommittedPosition(this.LedgerSize);
        ledgers_.push_back(std::move(ledger));
        // log.info("load " + file.getPath() + " OK");
      } catch (const std::exception& e) {
        // TODO: logging
        throw;
      }
    }

    std::reverse(ledgers_.begin(), ledgers_.end());
  }

  /**
   * @brief Check ledgers and return flushed offset
   */
  template <typename... Arguments>
  int64_t Recover(Arguments... arguments) {
    if (ledgers_.empty()) {
      // Commitlog case files are deleted
      // log.warn("The commitlog files are deleted, and delete the consume queue files");
      // this.mappedFileQueue.setFlushedWhere(0);
      // this.mappedFileQueue.setCommittedWhere(0);
      // this.defaultMessageStore.destroyLogics();
      return 0;
    }

    // Began to recover from the last third file
    size_t index = 0;
    if (ledgers_.size() > 3) {
      index = ledgers_.size() - 3;
    }

    int64_t physical_offset = 0;
    for (; index < ledgers_.size(); ++index) {
      auto* ledger = ledgers_[index].get();
      // log.info("recover next physics file, " + mappedFile.getFileName());
      auto [check_next, ledger_offset] = ledger->CheckEntries(arguments...);
      physical_offset = ledger->begin_offset() + ledger_offset;
      if (!check_next) {
        // Intermediate file read error
        // log.info("recover physics file end, " + mappedFile.getFileName());
        break;
      }
    }

    // this.mappedFileQueue.setFlushedWhere(processOffset);
    // this.mappedFileQueue.setCommittedWhere(processOffset);
    TruncateDirtyEntries(physical_offset);

    return physical_offset;
  }

  Ledger* FindLedgerByOffset(int64_t physical_offset) {
    if (ledgers_.empty()) {
      return nullptr;
    }
    int64_t begin_offset = (*ledgers_.begin())->begin_offset();
    if (physical_offset < begin_offset) {
      return nullptr;
    }
    int64_t end_offset = (*ledgers_.begin())->end_offset();
    if (physical_offset >= end_offset) {
      return nullptr;
    }
    auto ledger_count = ledgers_.size();
    auto hint_index = (physical_offset - begin_offset) / ledger_size_;
    if (hint_index >= ledger_count) {
      hint_index = ledger_count - 1;
    }
    auto hint_iter = ledgers_.begin() + hint_index;
    if (physical_offset >= (*hint_iter)->begin_offset()) {
      if (physical_offset < (*hint_iter)->end_offset()) {
        // O(1)
        return (*hint_iter).get();
      }
      return std::lower_bound(hint_iter + 1, ledgers_.end(), physical_offset,
                              [](const std::unique_ptr<Ledger>& ledger, int64_t offset) { return *ledger < offset; })
          ->get();
    }
    return std::lower_bound(ledgers_.begin(), hint_iter, physical_offset,
                            [](const std::unique_ptr<Ledger>& ledger, int64_t offset) { return *ledger < offset; })
        ->get();
  }

 protected:
  Ledger* GetLastLedger() {
    Ledger* last_ledger = nullptr;

    if (!ledgers_.empty()) {
      last_ledger = ledgers_.back().get();
    }

    return last_ledger;
  }

  Ledger* GetLastLedger(int64_t start_offset, bool need_create = true) {
    int64_t create_offset = -1;
    auto* last_ledger = GetLastLedger();

    if (nullptr == last_ledger) {
      create_offset = start_offset - (start_offset % ledger_size_);
    } else if (last_ledger->Full()) {
      create_offset = last_ledger->begin_offset() + ledger_size_;
    }

    if (create_offset != -1 && need_create) {
      xlib::owner<Ledger*> ledger = nullptr;

      try {
        auto ledger_path = ledger_dir_ / OffsetToFilename(create_offset);
        ledger = new Ledger{ledger_path, ledger_size_};
      } catch (const std::exception& e) {
        // log.error("create Ledger exception", e);
        throw;
      }

      if (ledger != nullptr) {
        if (ledgers_.empty()) {
          // ledger.setFirstCreateInQueue(true);
        }
        ledgers_.emplace_back(ledger);
      }

      return ledger;
    }

    return last_ledger;
  }

 private:
  void TruncateDirtyEntries(int64_t physical_offset) {
    std::vector<std::unique_ptr<Ledger>> will_remove_ledgers;

    for (auto it = ledgers_.begin(); it != ledgers_.end();) {
      auto& ledger = *it;
      int64_t tail_offset = ledger->begin_offset() + ledger->size();
      if (physical_offset < tail_offset) {
        if (ledger->begin_offset() <= physical_offset) {
          ledger->set_wrote_position(static_cast<int32_t>(physical_offset - ledger->begin_offset()));
          // file.setCommittedPosition((int) (offset % this.mappedFileSize));
          // file.setFlushedPosition((int) (offset % this.mappedFileSize));
        } else {
          // TODO: delete ledger
          // file.destroy(1000);
          will_remove_ledgers.emplace_back(std::move(ledger));
          it = ledgers_.erase(it);
          continue;
        }
      }
      ++it;
    }

    // this.deleteExpiredFile(willRemoveFiles)
  }

  std::string OffsetToFilename(int64_t offset) { return absl::StrFormat("%020i", offset); }

 private:
  xlib::fs::path ledger_dir_;
  int32_t ledger_size_;

  std::vector<std::unique_ptr<Ledger>> ledgers_;
};

}  // namespace logxx::detail

#endif  // LOGXX_BASICLOGSTREAM_HPP_
