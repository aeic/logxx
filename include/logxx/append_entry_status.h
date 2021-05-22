// Copyright (c) 2021, A Effective Infrastructure Committee.
// All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LOGXX_APPENDENTRYSTATUS_H_
#define LOGXX_APPENDENTRYSTATUS_H_

namespace logxx {

enum class AppendEntryStatus { kOk, kEndOfFile, kUnknownError };

}

#endif  // LOGXX_APPENDENTRYSTATUS_H_
