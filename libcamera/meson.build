#
#  Copyright (C) 2024 Intel Corporation
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
libcamera_sources += files([
    'ipu7.cpp',
    'Frames.cpp',
    'ZslCapture.cpp',
    'ParameterConverter.cpp',
    'Camera3AMetadata.cpp',
    'AEStateMachine.cpp',
    'AFStateMachine.cpp',
    'AWBStateMachine.cpp',
    'PrivacyControl.cpp',
    'HwPrivacyControl.cpp',
# LIBCAMERA_NONE_UPSTREAM_S
    'MakerNoteBuilder.cpp',
# LIBCAMERA_NONE_UPSTREAM_E
])

subdir('src')
subdir('modules')

# for master branch build
includes += include_directories(
    '.',
    'include',
    'include/api',
    'include/utils',
    'include/linux',
    'include/ia_imaging',
)

libcamera_args = [
    '-DHAVE_LINUX_OS',
    '-D__STDC_FORMAT_MACROS',
    '-DHAVE_PTHREADS',
    '-DHAVE_IA_TYPES',
    '-DHAVE_PRCTL',
    '-DCAMERA_HAL_DEBUG',
    '-DDUMP_IMAGE',
    '-DHAL_PIXEL_FORMAT_NV12_LINEAR_CAMERA_INTEL=0x10F',
    '-DMAX_PAC_BUFFERS=10',
    '-DIPU_SYSVER_ipu75',
    '-DPAC_ENABLE',
    '-DSTATIC_GRAPH_USE_IA_AIC_TYPES',
# PNP_DEBUG_S
    '-DPNP_DEBUG_TEST',
# PNP_DEBUG_E
]

if get_option('android_platform').contains('cros')
    libcamera_args += [
        '-DCAL_BUILD',
        '-DLIBCAMERA_BUILD',
        '-DFACE_DETECTION',
    ]
endif

libcamera_args += [
    '-Wno-shadow',
    '-Wno-sign-compare',
    '-Wno-missing-field-initializers',
    '-Wno-non-virtual-dtor',
    '-Wno-unused-parameter',
]

ia_aiqb_parser = cc.find_library('ia_aiqb_parser', required : true)
ia_cca = cc.find_library('ia_cca', required : true)
ia_cmc_parser = cc.find_library('ia_cmc_parser', required : true)
ia_exc = cc.find_library('ia_exc', required : true)
ia_log = cc.find_library('ia_log', required : true)
dl = cc.find_library('dl', required : true)
rt = cc.find_library('rt', required : true)
ia_aic = cc.find_library('ia_aic', required : true)
ia_aiq_v1 = cc.find_library('ia_aiq_v1', required : true)
expat = cc.find_library('expat', required : true)
jsoncpp = cc.find_library('jsoncpp', required : true)
perfetto = cc.find_library('perfetto_sdk', required : true)

# internal libraries
libcamera_deps += [
    ia_aiqb_parser,
    ia_cca,
    ia_cmc_parser,
    ia_exc,
    ia_log,
    dl,
    rt,
    expat,
    ia_aic,
    ia_aiq_v1,
    jsoncpp,
    perfetto,
]

if get_option('android_platform').contains('cros')
    jpeg = cc.find_library('jpeg', required : true)

    # chrome packages (feature support)
    libsync = dependency('libsync', required : true)
    libyuv = dependency('libyuv', required : true)

    libcamera_deps += [
        jpeg,
        libsync,
        libyuv,
    ]
endif

# GPU_GLES_PROCESSOR_S
GLESv2 = cc.find_library('GLESv2', required : true)
OpenGL = cc.find_library('OpenGL', required : true)
EGL = cc.find_library('EGL', required : true)

libcamera_deps += [
  EGL,
  GLESv2,
  OpenGL,
]
# GPU_GLES_PROCESSOR_E

# set default as not enabling Sandbox
ipa_sandbox = true

if not ipa_sandbox
    libcamera_sources += files([
        'modules/algowrapper/IntelCca.cpp',
    ])
else
    libcamera_args += [
        '-DIPA_SANDBOXING',
    ]

    subdir('ipa')
endif
