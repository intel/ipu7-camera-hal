#
#  Copyright (C) 2017-2021 Intel Corporation
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

if(TARGET ia_aiq)
    message("libcamhal found ia_aiq target")
    return()
endif()

# Get include and lib paths for IA_IMAGING from pkgconfig
find_package(PkgConfig)

pkg_check_modules(IA_IMAGING${TARGET_SUFFIX} ia_imaging${TARGET_SUFFIX})
if(NOT IA_IMAGING${TARGET_SUFFIX}_FOUND)
    message(FATAL_ERROR "IA_IMAGING${TARGET_SUFFIX} not found")
endif()

set(IA_IMAGING${TARGET_SUFFIX}_LIBRARIES
    ia_cca${TARGET_SUFFIX}
    ia_log${TARGET_SUFFIX}
)
