/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "OpenGLESDispatch/GLESv2Dispatch.h"

#include <stdio.h>
#include <stdlib.h>

#include "emugl/common/shared_library.h"

static emugl::SharedLibrary *s_gles2_lib = NULL;

#define DEFAULT_GLES_V2_LIB EMUGL_LIBNAME("GLES_V2_translator")

// An unimplemented function which prints out an error message.
// To make it consistent with the guest, all GLES2 functions not supported by
// the driver should be redirected to this function.

static void gles2_unimplemented() {
    fprintf(stderr, "Called unimplemented GLESv2 API\n");
}

//
// This function is called only once during initialiation before
// any thread has been created - hence it should NOT be thread safe.
//
bool init_gles2_dispatch(gles2_server_context_t *dispatch_table)
{
    const char *libName = getenv("ANDROID_GLESv2_LIB");
    if (!libName) libName = DEFAULT_GLES_V2_LIB;

    //
    // Load the GLES library
    //
    char error[256];
    s_gles2_lib = emugl::SharedLibrary::open(libName, error, sizeof(error));
    if (!s_gles2_lib) {
        fprintf(stderr, "%s: Could not load %s [%s]\n", __FUNCTION__,
                libName, error);
        return false;
    }

    //
    // init the GLES dispatch table
    //
    dispatch_table->initDispatchByName(gles2_dispatch_get_proc_func, NULL);
    return true;
}

//
// This function is called only during initialiation before
// any thread has been created - hence it should NOT be thread safe.
//
void *gles2_dispatch_get_proc_func(const char *name, void *userData)
{
    void* func = NULL;
    if (s_gles2_lib) {
        func = (void *)s_gles2_lib->findSymbol(name);
    }
    // To make it consistent with the guest, redirect any unsupported functions
    // to gles2_unimplemented.
    if (!func) {
        func = (void *)gles2_unimplemented;
    }
    return func;
}
