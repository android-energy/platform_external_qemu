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

#include <memory>

#include "OpenglRender/Renderer.h"
#include "OpenglRender/render_api_types.h"

namespace emugl {

// RenderLib - root interface for the GPU emulation library
//  Use it to set the library-wide parameters (logging, crash reporting) and
//  create indivudual renderers that take care of drawing windows.
class RenderLib {
public:
    virtual ~RenderLib() = default;

    virtual void setLogger(emugl_logger_struct logger) = 0;
    virtual void setCrashReporter(emugl_crash_reporter_t reporter) = 0;

    // initRenderer - initialize the OpenGL renderer object.
    //
    // |width| and |height| are the framebuffer dimensions that will be reported
    // to the guest display driver.
    //
    // |useSubWindow| is true to indicate that renderer has to support an
    // OpenGL subwindow. If false, it only supports setPostCallback().
    // See Renderer.h for more info.
    //
    // There might be only one renderer.
    virtual RendererPtr initRenderer(int width, int height,
                                     bool useSubWindow) = 0;
};

using RenderLibPtr = std::unique_ptr<RenderLib>;

}  // namespace emugl
