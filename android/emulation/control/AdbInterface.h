// Copyright (C) 2016 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "android/base/Optional.h"
#include "android/base/StringView.h"
#include "android/base/async/Looper.h"
#include "android/base/system/System.h"
#include "android/base/threads/ParallelTask.h"

#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace android {
namespace emulation {

class AdbCommand;
using AdbCommandPtr = std::shared_ptr<AdbCommand>;

// Encapsulates the result of running an ADB command.
struct AdbCommandResult {
    // Exit code returned by ADB.
    android::base::System::ProcessExitCode exit_code;

    // The command's output.
    // NOTE: we're using unique_ptr here because there's a bug in g++ 4 due to
    // which file streams are unmovable.
    mutable std::unique_ptr<std::ifstream> output;
};
using OptionalAdbCommandResult = android::base::Optional<AdbCommandResult>;

// A utility class used to automatically locate the ADB binary and
// asynchronously run ADB commands.
class AdbInterface {
public:
    explicit AdbInterface(android::base::Looper* looper);

    // Returns true is the ADB version is fresh enough.
    bool isAdbVersionCurrent() const { return mAdbVersionCurrent; }

    // Returns the automatically detected path to adb
    const std::string& detectedAdbPath() const { return mAdbPath; }

    // Runs an adb command asynchronously.
    // |args| - the arguments to pass to adb, i.e. "shell dumpsys battery"
    // |result_callback| - the callback function that will be invoked on the
    // calling
    //                     thread after the command completes.
    // |timeout_ms| - how long to wait for the command to complete, in
    // milliseconds.
    // |want_output| - if set to true, the argument passed to the callback will
    // contain the
    //                 output of the command.
    AdbCommandPtr runAdbCommand(
            const std::vector<std::string>& args,
            std::function<void(const OptionalAdbCommandResult&)>
                    result_callback,
            base::System::Duration timeout_ms,
            bool want_output = true);

private:
    android::base::Looper* mLooper;
    std::string mAdbPath;
    bool mAdbVersionCurrent;
};

// Representation of an asynchronously running ADB command.
// These shouldn't be created directly, use AdbInterface::runAdbCommand.
class AdbCommand : public std::enable_shared_from_this<AdbCommand> {
    friend class ::android::emulation::AdbInterface;

public:
    using ResultCallback = std::function<void(const OptionalAdbCommandResult&)>;

    // Returns true if the command is currently in the process of execution.
    bool inFlight() const { return static_cast<bool>(mTask); }

    // Cancels the running command (has no effect if the command isn't running).
    void cancel() { mCancelled = true; }

private:
    AdbCommand(android::base::Looper* looper,
               const std::string& adb_path,
               const std::vector<std::string>& command,
               bool want_output,
               base::System::Duration timeout,
               ResultCallback callback);
    void start();
    void taskFunction(OptionalAdbCommandResult* result);
    void taskDoneFunction(const OptionalAdbCommandResult& result);

    android::base::Looper* mLooper;
    std::unique_ptr<android::base::ParallelTask<OptionalAdbCommandResult>>
            mTask;
    ResultCallback mResultCallback;
    bool mCancelled = false;
    std::string mOutputFilePath;
    std::vector<std::string> mCommand;
    bool mWantOutput;
    int mTimeout;
    bool mFinished;
};
}
}
