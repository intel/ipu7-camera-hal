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
    '3a/AiqCore.cpp',
    '3a/AiqEngine.cpp',
    '3a/AiqResult.cpp',
    '3a/AiqResultStorage.cpp',
    '3a/AiqSetting.cpp',
    '3a/AiqUnit.cpp',
    '3a/AiqUtils.cpp',
    '3a/I3AControlFactory.cpp',
    '3a/MakerNote.cpp',
    '3a/LensManager.cpp',
    '3a/SensorManager.cpp',
    '3a/intel3a/Intel3AParameter.cpp',
    'core/BufferQueue.cpp',
    'core/CameraBuffer.cpp',
    'core/CameraContext.cpp',
    'core/CameraEvent.cpp',
    'core/CameraStream.cpp',
    'core/CaptureUnit.cpp',
    'core/DeviceBase.cpp',
    'core/LensHw.cpp',
    'core/IProcessingUnitFactory.cpp',
    'core/ProcessingUnit.cpp',
    'core/RequestThread.cpp',
    'core/SensorHwCtrl.cpp',
    'core/PSysDevice.cpp',
    'core/SofSource.cpp',
    'core/processingUnit/PipeManager.cpp',
    'core/processingUnit/IPipeManagerFactory.cpp',
    'core/processingUnit/PipeLine.cpp',
    'core/processingUnit/CBStage.cpp',
# FILE_SOURCE_S
    'core/FileSource.cpp',
# FILE_SOURCE_E
# PNP_DEBUG_S
    'platformdata/PnpDebugControl.cpp',
    'core/processingUnit/PipeManagerStub.cpp',
    'iutils/SwImageConverter.cpp',
    'core/MockPSysDevice.cpp',
# PNP_DEBUG_E
    'iutils/CameraDump.cpp',
    'iutils/CameraLog.cpp',
    'iutils/PerfettoTrace.cpp',
    'iutils/Trace.cpp',
    'iutils/Utils.cpp',
    'platformdata/AiqInitData.cpp',
    'platformdata/CameraParserInvoker.cpp',
    'platformdata/CameraSensorsParser.cpp',
    'platformdata/JsonCommonParser.cpp',
    'platformdata/JsonParserBase.cpp',
    'platformdata/PlatformData.cpp',
    'platformdata/gc/GraphConfig.cpp',
    'platformdata/gc/GraphConfigManager.cpp',
    'platformdata/gc/GraphUtils.cpp',
    'scheduler/CameraScheduler.cpp',
    'scheduler/CameraSchedulerPolicy.cpp',
    'v4l2/MediaControl.cpp',
    'v4l2/SysCall.cpp',
    'v4l2/V4l2DeviceFactory.cpp',
    'v4l2/NodeInfo.cpp',
    'iutils/ModuleTags.cpp',
    'iutils/LogSink.cpp',
    'core/IpuPacAdaptor.cpp',
])

# for master branch build
includes += include_directories(
    '.',
    'fd',
    'platformdata',
    'platformdata/gc',
    'v4l2',
    'iutils',
    'core',
    'core/processingUnit',
    'scheduler',
    '3a/intel3a',
    '3a',
)
# POST_PROCESS_FEATURE_S
if get_option('android_platform').contains('cros')
    libcamera_sources += files([
        'core/processingUnit/PostProcessStage.cpp',
        'core/SwPostProcessUnit.cpp',
        'image_process/sw/ImageConverter.cpp',
        'image_process/sw/ImageScalerCore.cpp',
        'image_process/PostProcessorBase.cpp',
        'image_process/PostProcessorCore.cpp',
# GPU_GLES_PROCESSOR_S
        'image_process/gpu/GPUPostProcessor.cpp',
# GPU_GLES_PROCESSOR_E
        'image_process/chrome/ImageProcessorCore.cpp',
    ])

    includes += include_directories(
        'image_process',
        'image_process/sw',
        'image_process/gpu',
        'image_process/chrome',
    )
endif
# POST_PROCESS_FEATURE_E
