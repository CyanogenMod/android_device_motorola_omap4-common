/*
 * Copyright (c) 2010, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* -------------------------------------------------------------------------- *
 *
 * @file:OMX_TI_IVCommon.h
 * This header defines the structures specific to the config indices of msp_VPPM.
 *
 * @path ..\OMAPSW_SysDev\multimedia\omx\khronos1_1\omx_core\inc
 *
 * -------------------------------------------------------------------------- */

/* ======================================================================== *!
 *! Revision History
 *! ==================================================================== */

#ifndef OMX_TI_IVCommon_H
#define OMX_TI_IVCommon_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_IVCommon.h>
#include <OMX_Image.h>

#define DCC_PATH  "/data/misc/camera/"

#define MAX_URI_LENGTH      (OMX_MAX_STRINGNAME_SIZE)
#define MAX_ALGOAREAS       (35)

// Size of sensor EEPROM in bytes.
// Note: This size is also defined in
//       WTSD_DucatiMMSW/framework/camera/Cam_sysstate.h
//       WTSD_DucatiMMSW/omx/omx_il_1_x/omx_core/omx_ti_ivcommon.h
//       If you change it here, change it also there.
#define SENSOR_EEPROM_SIZE  (256)

/*======================================================================= */
/* Enumerated values for operation mode for compressed image
 *
 * ENUMS:
 * Chunk         : Chunk based operation
 * NonChunk    : Non-chunk based operation
 */
 /* ======================================================================= */
typedef enum OMX_JPEG_COMPRESSEDMODETYPE {
    OMX_JPEG_ModeChunk = 0,
    OMX_JPEG_ModeNonChunk,
    OMX_JPEG_CompressedmodeMax = 0x7fffffff
}OMX_JPEG_COMPRESSEDMODETYPE ;


/*======================================================================= */
/* Enumerated values for operation mode for uncompressed image
 *
 * ENUMS:
 * Frame   :  Frame based operation
 * Slice   : Slice based operation
 * Stitch  : For stitching between image frames
 * Burst   :  For stitching between image frames
 */
 /* ======================================================================= */
typedef enum OMX_JPEG_UNCOMPRESSEDMODETYPE {
    OMX_JPEG_UncompressedModeFrame = 0,
    OMX_JPEG_UncompressedModeSlice,
    OMX_JPEG_UncompressedModeStitch,
    OMX_JPEG_UncompressedModeBurst,
    OMX_JPEG_UncompressedModeMax = 0x7fffffff
}OMX_JPEG_UNCOMPRESSEDMODETYPE;



/*======================================================================= */
/* Configuration structure for compressed image
 *
 * STRUCT MEMBERS:
 *  nSize                 : Size of the structure in bytes
 *  nVersion              : OMX specification version information
 *  nPortIndex            : Port that this structure applies to
 *  eCompressedImageMode  : Operating mode enumeration for compressed image
 */
 /*======================================================================= */
typedef struct OMX_JPEG_PARAM_COMPRESSEDMODETYPE{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_JPEG_COMPRESSEDMODETYPE eCompressedImageMode;
}OMX_JPEG_PARAM_COMPRESSEDMODETYPE;



/*======================================================================= */
/* Uncompressed image Operating mode configuration structure
 *
 * STRUCT MEMBERS:
 * nSize                     : Size of the structure in bytes
 * nVersion                  : OMX specification version information
 * nPortIndex                : Port that this structure applies to
 * nBurstLength              : No of frames to be dumped in burst mode
 * eUncompressedImageMode    : uncompressed image mode information
 * eSourceType               : Image encode souce info
 * tRotationInfo             : Rotation related information
 */
 /*======================================================================= */
typedef struct OMX_JPEG_PARAM_UNCOMPRESSEDMODETYPE{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nBurstLength;
    OMX_JPEG_UNCOMPRESSEDMODETYPE eUncompressedImageMode;
}OMX_JPEG_PARAM_UNCOMPRESSEDMODETYPE;


/*======================================================================= */
/* Subregion Decode Parameter configuration structure
 *
 * STRUCT MEMBERS:
 * nSize                     : Size of the structure in bytes
 * nVersion                  : OMX specification version information
 * nXOrg                     : Sectional decoding X origin
 * nYOrg                     : Sectional decoding Y origin
 * nXLength                  : Sectional decoding X length
 * nYLength                  : Sectional decoding Y length
 */
 /*======================================================================= */
typedef struct OMX_IMAGE_PARAM_DECODE_SUBREGION{
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nXOrg;
    OMX_U32 nYOrg;
    OMX_U32 nXLength;
    OMX_U32 nYLength;
}OMX_IMAGE_PARAM_DECODE_SUBREGION;


/**
 * sensor select  types
 */
typedef enum OMX_SENSORSELECT{
        OMX_PrimarySensor = 0,
        OMX_SecondarySensor,
        OMX_TI_StereoSensor,
        OMX_SensorTypeMax = 0x7fffffff
}OMX_SENSORSELECT;

/**
 *
 * Sensor Select
 */
typedef  struct OMX_CONFIG_SENSORSELECTTYPE {
OMX_U32  nSize; /**< Size of the structure in bytes */
OMX_VERSIONTYPE nVersion; /**< OMX specification version info */
OMX_U32 nPortIndex; /**< Port that this struct applies to */
OMX_SENSORSELECT eSensor; /**< sensor select */
} OMX_CONFIG_SENSORSELECTTYPE;

/**
 * Flicker cancellation types
 */
typedef enum OMX_COMMONFLICKERCANCELTYPE{
        OMX_FlickerCancelOff = 0,
        OMX_FlickerCancelAuto,
        OMX_FlickerCancel50,
        OMX_FlickerCancel60,
        OMX_FlickerCancel100,
        OMX_FlickerCancel120,
        OMX_FlickerCancelMax = 0x7fffffff
}OMX_COMMONFLICKERCANCELTYPE;

typedef struct OMX_CONFIG_FLICKERCANCELTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_COMMONFLICKERCANCELTYPE eFlickerCancel;
} OMX_CONFIG_FLICKERCANCELTYPE;


/**
 * Sensor caleberation types
 */
typedef enum OMX_SENSORCALTYPE{
        OMX_SensorCalFull = 0,
        OMX_SensorCalQuick,
        OMX_SensorCalMax = 0x7fffffff
}OMX_SENSORCALTYPE;

typedef struct OMX_CONFIG_SENSORCALTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_SENSORCALTYPE eSensorCal;
} OMX_CONFIG_SENSORCALTYPE;

/**
 * Scene mode types
 */
typedef enum OMX_SCENEMODETYPE{

        OMX_Manual = 0,
        OMX_Closeup,
        OMX_Portrait,
        OMX_Landscape,
        OMX_Underwater,
        OMX_Sport,
        OMX_SnowBeach,
        OMX_Mood,
        OMX_NightPortrait,
        OMX_NightIndoor,
        OMX_Fireworks,
        OMX_Document, /**< for still image */
        OMX_Barcode, /**< for still image */
        OMX_SuperNight, /**< for video */
        OMX_Cine, /**< for video */
        OMX_OldFilm, /**< for video */
        OMX_TI_Action,
        OMX_TI_Beach,
        OMX_TI_Candlelight,
        OMX_TI_Night,
        OMX_TI_Party,
        OMX_TI_Portrait,
        OMX_TI_Snow,
        OMX_TI_Steadyphoto,
        OMX_TI_Sunset,
        OMX_TI_Theatre,
        OMX_SceneModeMax = 0x7fffffff
}OMX_SCENEMODETYPE;

typedef struct OMX_CONFIG_SCENEMODETYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_SCENEMODETYPE eSceneMode;
} OMX_CONFIG_SCENEMODETYPE;

 /**
 * Port specific capture trigger
 * useful for the usecases with multiple capture ports.
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  bExtCapturing : Start Captre at the specified port. Can be queried to know the status of a specific port.
 */
typedef struct OMX_CONFIG_EXTCAPTURING {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bExtCapturing;
} OMX_CONFIG_EXTCAPTURING;


 /**
 * Digital Zoom Speed
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nDigitalZoomSpeed      :  Optical zoom speed level. Special values:
 *      0 - stop current movement
 *      values from 1 to 254 are mapped proportionally to supported zoom speeds inside optical zoom driver.
 *      So 1 is slowest available optical zoom speed and 254 is fastest available optical zoom speed
 *      255 - default optical zoom speed value
 */
typedef struct OMX_CONFIG_DIGITALZOOMSPEEDTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_U8 nDigitalZoomSpeed;
} OMX_CONFIG_DIGITALZOOMSPEEDTYPE;


 /**
 * Digital Zoom Target
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nDigitalZoomTarget      :  Default and minimum is 0. Maximum is determined by the current supported range
 */

typedef struct OMX_CONFIG_DIGITALZOOMTARGETTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_U32 nDigitalZoomTarget;
} OMX_CONFIG_DIGITALZOOMTARGETTYPE;


/**
* Scale quality enums
*/
typedef enum OMX_SCALEQUALITY{
        OMX_DefaultScaling = 0, /** <default scaling if nothing is specified > */
        OMX_BetterScaling,   /** <better scaling> */
        OMX_BestScaling,  /** <best  scaling> */
        OMX_AutoScalingQuality,  /** <auto scaling quality> */
        OMX_FastScaling,   /** <fast scaling, prioritizes speed> */
        OMX_ScaleQualityMax = 0x7fffffff
}OMX_SCALEQUALITY;

/**
* Scaling Quality Mode
*/
typedef enum OMX_SCALEQUALITYMODE{
        OMX_SingleFrameScalingMode = 0, /** <default > */
        OMX_MultiFrameScalingMode,   /** <better scaling> */
        OMX_AutoScalingMode,  /** <best  scaling> */
        OMX_ScaleModeMax = 0x7fffffff
}OMX_SCALEQUALITYMODE;

 /**
 * Rescale quality control type
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  eScaleQuality : controls the quality level.
 *  eScaleQualityMode      :  controls the scaling algo types
 */
typedef struct OMX_CONFIG_SCALEQUALITYTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_SCALEQUALITY eScaleQuality;
    OMX_SCALEQUALITYMODE eScaleQualityMode;
} OMX_CONFIG_SCALEQUALITYTYPE;

/**
* Smooth Zoom mode enum
* Starts or stops the Smooth Zoom.  Selecting INCREASE will cause an increasing digital zoom factor (increased cropping),
* with a shrinking viewable area and crop height percentage.  Selecting DECREASE will cause a decreasing digital zoom (decreased cropping),
* with a growing viewable area and crop height percentage.  The CaptureCropHeight will continue to update based on the SmoothZoomRate until
* the SmoothZoomMin or SmoothZoomMax zoom step is reached, the framework minimum zoom step is reached, the SmoothZoomRate becomes 0,
* or the SmoothZoomMode is set to OFF.
* NOTE: The message payload includes all parts of the message that is NOT part of the message header as listed for the CAM_SEND_DATA message.
*/
typedef enum OMX_SMOOTHZOOMMODE{
    OMX_Off=0, /**< default OFF */
    OMX_Increase,
    OMX_Decrease,
    OMX_SmoothZoomModeMax = 0x7fffffff
}OMX_SMOOTHZOOMMODE;


 /**
 * Rescale quality control type
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  eSmoothZoomMode : controls the smooth zoom feature.
 *  nSmoothZoomRate      :  Values from 0 to 65535 which represents percentage to increase per second, where 65535 = 100%, and 0 = 0%.
 *  nSmoothZoomQuantize:
 *  nSmoothZoomThresh
 *  nSmoothZoomMin
 *  nSmoothZoomMax
 */
typedef struct OMX_CONFIG_SMOOTHZOOMTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_SMOOTHZOOMMODE eSmoothZoomMode;
    OMX_U32 nSmoothZoomRate;
    OMX_U32 nSmoothZoomQuantize;
    OMX_U32 nSmoothZoomThresh;
    OMX_U32 nSmoothZoomMin;
    OMX_U32 nSmoothZoomMax;
} OMX_CONFIG_SMOOTHZOOMTYPE;

/**
 * Enumeration of possible Extended image filter types for OMX_CONFIG_IMAGEFILTERTYPE
 */
typedef enum OMX_EXTIMAGEFILTERTYPE {
    OMX_ImageFilterSepia = 0x7F000001,
    OMX_ImageFilterGrayScale,
    OMX_ImageFilterNatural,
    OMX_ImageFilterVivid,
    OMX_ImageFilterColourSwap,
    OMX_ImageFilterOutOfFocus,
    OMX_ImageFilterWaterColour,
    OMX_ImageFilterPastel,
    OMX_ImageFilterFilm,
    OMX_TI_ImageFilterBlackWhite,
    OMX_TI_ImageFilterWhiteBoard,
    OMX_TI_ImageFilterBlackBoard,
    OMX_TI_ImageFilterAqua,
    OMX_TI_ImageFilterPosterize,
    OMX_ImageFilterTypeMax = 0x7fffffff
} OMX_EXTIMAGEFILTERTYPE;


/**
 * Image filter configuration extended
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bBlemish : Enable/Disable Blemish correction
 */
typedef struct OMX_CONFIG_BLEMISHTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bBlemish;
} OMX_CONFIG_BLEMISHTYPE;

/**
 * Enumeration of Bracket types
 * OMX_BracketExposureRelativeInEV:
 *      Exposure value is changed relative to the value set by automatic exposure.
 *      nBracketStartValue and nBracketStep are in Q16. Increment is additive.
 * OMX_BracketExposureAbsoluteMs:
 *      Exposure value is changed in absolute value in ms.
 *      nBracketStartValue and nBracketStep are in Q16. Increment is multiplicative.
 * OMX_BracketFocusRelative:
 *      Focus is adjusted relative to the focus set by auto focus.
 *      The value is S32 integer, and is the same as adjusting nFocusSteps of OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE relatively.
 *      Increment is additive.
 * OMX_BracketFocusAbsolute:
 *      Focus position is adjusted absolutely. It is the same as setting nFocusSteps of
 *      OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE relatively for each captures.
 *      The value should be interpreted as U32 value.  Increment is additive.
 * OMX_BracketFlashPower:
 *      Power of flash is adjusted relative to the automatic level. Increment is multiplicative.
 * OMX_BracketAperture:
 *      Aperture number relative to the automatic setting. Data in Q16 format. Increment is multiplicative.
 * OMX_BracketTemporal:
 *      To suppport temporal bracketing.
 */
typedef enum OMX_BRACKETMODETYPE {
    OMX_BracketExposureRelativeInEV = 0,
    OMX_BracketExposureAbsoluteMs,
    OMX_BracketFocusRelative,
    OMX_BracketFocusAbsolute,
    OMX_BracketFlashPower,
    OMX_BracketAperture,
    OMX_BracketTemporal,
    OMX_BracketExposureGainAbsolute,
    OMX_BracketVectorShot,
    OMX_BrackerTypeKhronosExtensions = 0x6f000000,
    OMX_BrackerTypeVendorStartUnused = 0x7f000000,
    OMX_BracketTypeMax = 0x7FFFFFFF
} OMX_BRACKETMODETYPE;

typedef struct OMX_CONFIG_BRACKETINGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BRACKETMODETYPE eBracketMode;
    OMX_U32             nNbrBracketingValues;
    OMX_S32             nBracketValues[10];     /**< 10 can be assumed */
    OMX_S32             nBracketValues2[10];     /**< 10 can be assumed */
} OMX_CONFIG_BRACKETINGTYPE;


/**
 * Capture mode types
 * Note: this list could get extended modified based on the type of interenal use-case pipelines implemented within the camera component.
 *
 *       OMX_CaptureImageHighSpeedBurst = 0,
 *       OMX_CaptureImageHighSpeedTemporalBracketing,
 *       OMX_CaptureImageProfileBase(Base):
 *       	Base one almost same as Highspeed one.
 *       OMX_CaptureImageProfileLowLight1(LL1):
 *       	Includes NSF2 in addition to Base processing
 *       OMX_CaptureImageProfileLowLight2(LL2):
 *       	Includes NSF2 and LBCE in addition to Base processing.
 *       OMX_CaptureImageProfileOpticalCorr1(OC1):
 *       	Includes LDC in addition to Base processing.
 *       OMX_CaptureImageProfileOpticalCorr2(OC2):
 *       	Includes LDC and CAC in addition to Base processing.
 *       OMX_CaptureImageProfileExtended1(Ext1):
 *       	Includes NSF2, LBCE, LDC, and CAC in addition to Base
 *       OMX_CaptureStereoImageCapture:
 *       	Stereo image capture use-case.
 *       OMX_CaptureImageMemoryInput:
 *       	need to take sensor input from INPUT port.
 *       OMX_CaptureVideo:
 *       OMX_CaptureHighSpeedVideo:
 *       OMX_CaptureVideoMemoryInput:
 *
 */
typedef enum OMX_CAMOPERATINGMODETYPE {
        OMX_CaptureImageHighSpeedBurst = 0,
        OMX_CaptureImageHighSpeedTemporalBracketing,
        OMX_CaptureImageProfileBase,
        OMX_CaptureImageProfileLowLight1,
        OMX_CaptureImageProfileLowLight2,
        OMX_CaptureImageProfileOpticalCorr1,
        OMX_CaptureImageProfileOpticalCorr2,
        OMX_CaptureImageProfileExtended1,
        OMX_CaptureStereoImageCapture,
        OMX_CaptureImageMemoryInput,
        OMX_CaptureVideo,
        OMX_CaptureHighSpeedVideo,
        OMX_CaptureVideoMemoryInput,
        OMX_TI_CaptureDummy,
        OMX_TI_CaptureGestureRecognition,
        OMX_TI_CaptureImageProfileZeroShutterLag,
        OMX_TI_SinglePreview,
        OMX_TI_StereoGestureRecognition,
        OMX_TI_CPCam,
        OMX_TI_StereoVideo,
        OMX_CaptureHighQualityVideo,
        OMX_TI_SimultaneousSensorsGesture,
        OMX_TI_CaptureVideoLowLatency,
        OMX_TI_HQBurst,
        OMX_TI_Reprocessing,
        // Put new entries here so OMX_CamOperatingModeMax always points to
        // the last one
        OMX_TI_CamOperatingModeCount,
        OMX_CamOperatingModeMax = OMX_TI_CamOperatingModeCount - 1,
        OMX_CamOperatingMode = 0x7fffffff
} OMX_CAMOPERATINGMODETYPE;

/**
 * Capture mode setting: applicable to multi shot capture also including bracketing.
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  eCamOperatingMode : specifies the camera operating mode.
 */
typedef struct OMX_CONFIG_CAMOPERATINGMODETYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_CAMOPERATINGMODETYPE eCamOperatingMode;
} OMX_CONFIG_CAMOPERATINGMODETYPE;


/**
 * Capture mode setting: applicable to multi shot capture also including bracketing.
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  nFrameRate   : when bContinuous is FALSE, need to define the frame rate of the muti-shot scenario. Since this would be applicable to IMAGE domain port, there is no port specific frame rate.
 *  nFrameBefore :
 * 	is specifying how many frames before the capture trigger shall be used.
 * 	It is implementation dependent how many is supported. This shall only be supported for images and not for video frames.
 * bPrepareCapture :
 *	should be set to true when nFrameBefore is greater than zero and before capturing of before-frames should start.
 *	The component is not allowed to deliver buffers until capturing starts. This shall only be supported for images and not for video frames.
 * bEnableBracketing :
 *	should be enabled when bracketing is used. In bracketing mode, one parameter can be changed per each capture.
 * tBracketConfigType :
 *	specifies bracket mode to use. Valid only when bEnableBracketing is set.
 */
typedef struct OMX_CONFIG_EXTCAPTUREMODETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nFrameRate;
    OMX_U32 nFrameBefore;
    OMX_BOOL bPrepareCapture;
    OMX_BOOL bEnableBracketing;
    OMX_CONFIG_BRACKETINGTYPE tBracketConfigType;
} OMX_CONFIG_EXTCAPTUREMODETYPE;

/**
 * For Extended Focus region Type -
 */
typedef struct OMX_CONFIG_EXTFOCUSREGIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nRefPortIndex;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
} OMX_CONFIG_EXTFOCUSREGIONTYPE;

/**
 * Digital Flash Control
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bDigitalFlash : Digital flash type Enable/Disable -
 * Specifies whether the digital flash algorithm is enabled or disabled. This overrides the contrast and brightness settings.
 */
typedef struct OMX_CONFIG_DIGITALFLASHTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bDigitalFlash;
} OMX_CONFIG_DIGITALFLASHTYPE;



/**
 * Privacy Indicator Enable/Disable
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bPrivacyIndicator :
 *        Specifies whether the flash should be used to indicate image or video capture. When flash is not used for exposure,
 *        flash will be activated after exposure to indicate image capture.
 *        If video light is not used, the flash can be blinking or constant at low intensity to indicate capture but not affect exposure.
 *        Specifies whether the digital flash algorithm is enabled or disabled. This overrides the contrast and brightness settings.
 */
typedef struct OMX_CONFIG_PRIVACYINDICATOR {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bPrivacyIndicator;
} OMX_CONFIG_PRIVACYINDICATOR;


/**
 * Privacy Indicator Enable/Disable
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bTorchMode :
 *        Enable/Disable
 *      nIntensityLevel : relative intensity from 0 - 100
 *      nDuration : duration in msec
 */
typedef struct OMX_CONFIG_TORCHMODETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bTorchMode;
    OMX_U32 nIntensityLevel;
    OMX_U32 nDuration;
} OMX_CONFIG_TORCHMODETYPE;



/**
 * Privacy Indicator Enable/Disable
 * DISABLE - Fire the xenon flash in the usual manner
 * ENABLE - Reduce the light intensity of the main flash (ex 1EV)
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bSlowSync :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_SLOWSYNCTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bSlowSync;
} OMX_CONFIG_SLOWSYNCTYPE;


/**
 * Focus control extended enums. use this along with OMX_IMAGE_CONFIG_FOCUSCONTROLTYPE
 */
typedef enum OMX_IMAGE_EXTFOCUSCONTROLTYPE {
    OMX_IMAGE_FocusControlAutoMacro = 0x7F000001, /**< Reserved region for introducing Vendor Extensions */
    OMX_IMAGE_FocusControlAutoInfinity,
    OMX_IMAGE_FocusControlHyperfocal,
    OMX_IMAGE_FocusControlPortrait, /**< from Xena */
    OMX_IMAGE_FocusControlExtended, /**< from Xena */
    OMX_IMAGE_FocusControlContinousNormal, /**< from Xena */
    OMX_IMAGE_FocusControlContinousExtended,     /**< from Xena */
    OMX_IMAGE_FocusControlContinousFacePriority,
    OMX_IMAGE_FocusControlContinousRegionPriority,
    OMX_IMAGE_FocusControlContinousPicture,
    OMX_IMAGE_FocusControlTypeMax = 0x7fffffff
} OMX_IMAGE_EXTFOCUSCONTROLTYPE;



/**
 * Specifies whether the LED can be used to assist in autofocus, due to low lighting conditions.
 * ENABLE means use as determined by the auto exposure algorithm.
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bFocusAssist :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_FOCUSASSISTTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bFocusAssist;
} OMX_CONFIG_FOCUSASSISTTYPE;



/**
 *for locking the focus
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bFocusLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_FOCUSLOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bFocusLock;
} OMX_CONFIG_FOCUSLOCKTYPE;


/**
 *for locking the White balance
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bWhiteBalanceLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_WHITEBALANCELOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bWhiteBalanceLock;
} OMX_CONFIG_WHITEBALANCELOCKTYPE;

/**
 *for locking the Exposure
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bExposureLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_EXPOSURELOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bExposureLock;
} OMX_CONFIG_EXPOSURELOCKTYPE;

/**
 *for locking the Exposure
 *  Simultaneously lock focus, white balance and exposure (and relevant other settings).
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bAllLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 */
typedef struct OMX_CONFIG_ALLLOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bAllLock;
} OMX_CONFIG_ALLLOCKTYPE;

/**
 *for locking
 *  Simultaneously lock focus, white balance and exposure (and relevant other settings).
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  bLock :
 *        Enable - OMX_TRUE/Disable - OMX_FALSE
 *  bAtCapture:
 *
 */
typedef struct OMX_IMAGE_CONFIG_LOCKTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bLock;
    OMX_BOOL bAtCapture;
} OMX_IMAGE_CONFIG_LOCKTYPE;

/**
 * processig level types enum
 */
typedef enum OMX_PROCESSINGLEVEL{
        OMX_Min = 0,
        OMX_Low,
        OMX_Medium,
        OMX_High,
        OMX_Max,
        OMX_ProcessingLevelMax = 0x7fffffff
}OMX_PROCESSINGLEVEL;

/**
 *processing level type
 *  Simultaneously lock focus, white balance and exposure (and relevant other settings).
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  nLevel :
 *               nLevel hinting processing amount. Range of values is -100 to 100.
 *               0 causes no change to the image.  Increased values cause increased processing to occur, with 100 applying maximum processing.
 *               Negative values have the opposite effect of positive values.
 *  bAuto:
 *		sets if the processing should be applied according to input data.
 		It is allowed to combine the hint level with the auto setting,
 *		i.e. to give a bias to the automatic setting. When set to false, the processing should not take input data into account.
 */

typedef struct OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE {
OMX_U32 nSize;
OMX_VERSIONTYPE nVersion;
OMX_U32 nPortIndex;
OMX_S32 nLevel;
OMX_BOOL bAuto;
} OMX_IMAGE_CONFIG_PROCESSINGLEVELTYPE;


/**
 * White Balance control type extended enums - to be used along with the structure @OMX_CONFIG_WHITEBALCONTROLTYPE
 *
 *
 *
 */
typedef enum OMX_EXTWHITEBALCONTROLTYPE {
    OMX_WhiteBalControlFacePriorityMode = OMX_WhiteBalControlVendorStartUnused + 1, /**<  */
    OMX_TI_WhiteBalControlSunset,
    OMX_TI_WhiteBalControlShade,
    OMX_TI_WhiteBalControlTwilight,
    OMX_TI_WhiteBalControlWarmFluorescent,
    OMX_TI_WhiteBalControlMax = 0x7fffffff
} OMX_EXTWHITEBALCONTROLTYPE;

/**
 *white balance gain type
 *  xWhiteBalanceGain and xWhiteBalanceOffset represents gain and offset for R, Gr, Gb, B channels respectively in Q16 format. \
 *  For example, new red pixel value = xWhiteBalanceGain[1]* the current pixel value + xWhiteBalanceOffset[1].
 *  All values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *  nWhiteThreshhold  represents thresholds for "white" area measurments in Q16 format.
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_CONFIG_WHITEBALGAINTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_S32 xWhiteBalanceGain[4];
    OMX_S32 xWhiteBalanceOffset[4];
    OMX_S32 nWhiteThreshhold[4];
} OMX_CONFIG_WHITEBALGAINTYPE;

/**
 *  This structure represents linear color conversion from one space to another.  For example, to conversion from one RGB color into another RGB color space can be represented as
 *  R' =  xColorMatrix[1][1]*R + xColorMatrix[1][2]*G + xColorMatrix[1][3]*B + xColorOffset[1]
 *  G' = xColorMatrix[2][1]*R + xColorMatrix[2][2]*G + xColorMatrix[2][3]*B + xColorOffset[2]
 *  B' = xColorMatrix[3][1]*R + xColorMatrix[3][2]*G + xColorMatrix[3][3]*B + xColorOffset[3]
 *  Both xColorMatrix and xColorOffset are represented as Q16 value.
 *  bFullColorRange represents represents whether valid range of color is 0 to 255 (when set to TRUE) or 16 to 235 (for FALSE).
 *  Again all values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_CONFIG_EXT_COLORCONVERSIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_S32 xColorMatrix[3][3];
    OMX_S32 xColorOffset[3];
    OMX_BOOL bFullColorRange;
}OMX_CONFIG_EXT_COLORCONVERSIONTYPE;


/**
 * xGamma represents lool-up table for gamma correction in Q16 format.
 * All values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_CONFIG_GAMMATABLETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 xGamma[3][256];
}OMX_CONFIG_GAMMATABLETYPE;



/**
 * processig types
 */
typedef enum OMX_PROCESSINGTYPE{
        OMX_BloomingReduction = 0,
        OMX_Denoise,
        OMX_Sharpening,
        OMX_Deblurring,
        OMX_Demosaicing,
        OMX_ContrastEnhancement,
        OMX_ProcessingTypeMax = 0x7fffffff
}OMX_PROCESSINGTYPE;


typedef  struct OMX_CONFIGPROCESSINGORDERTYPE {
OMX_U32  nSize; /**< Size of the structure in bytes */
OMX_VERSIONTYPE nVersion; /**< OMX specification version info */
OMX_U32 nPortIndex; /**< Port that this struct applies to */
OMX_U32 nIndex;
OMX_PROCESSINGTYPE eProc;
} OMX_CONFIGPROCESSINGORDERTYPE;

/**
 * HIST TYPE
 */
typedef enum OMX_HISTTYPE{
        OMX_HistControlLuminance = 0, /**< Luminance histogram is calculated (Y)*/
        OMX_HistControlColorComponents, /**< A histogram per color component (R, G, B) is calculated*/
    OMX_HistControlChrominanceComponents,     /**< A histogram per chrominance component (Cb, Cr) is calculated.*/
    OMX_HistControl_32BIT_PATCH = 0x7FFFFFFF
}OMX_HISTTYPE;

/**
 * Histogram Setting
 *  nPortIndex is an output port. The port index decides on which port the extra data structur is sent on.
 *  bFrameLimited is a Boolean used to indicate if measurement shall be terminated after the specified number of
 *  frames if true frame limited measurement is enabled; otherwise the port does not terminate measurement until
 *  instructed to do so by the client.
 *  nFrameLimit is the limit on number of frames measured, this parameter is only valid if bFrameLimited is enabled.
 *  bMeasure is a Boolean that should be set to true when measurement shall begin, otherwise set to false. Query will give status information on if measurement is ongoing or not.
 *  nBins specifies the number of histogram bins. When queried with set to zero, the respons gives the maximum number of bins allowed.
 *  nLeft is the leftmost coordinate of the measurement area rectangle.
 *  nTop is the topmost coordinate of the measurement area rectangle.
 *  nWidth is the width of the measurement area rectangle in pixels.
 *  nHeight is the height of the measurement area rectangle in pixels.
 *  eHistType is an enumeration specifying the histogram type
 *
 *
 */

typedef struct OMX_CONFIG_HISTOGRAMTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bFrameLimited;
    OMX_U32 nFrameLimit;
    OMX_BOOL bMeasure;
    OMX_U32 nBins;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_HISTTYPE eHistType;
} OMX_CONFIG_HISTOGRAMTYPE;

/**
 * OMX_HISTCOMPONENTTYPE Enumerated Value
 */
typedef enum OMX_HISTCOMPONENTTYPE{
        OMX_HISTCOMP_Y = 0, /**<    Luminance histogram (Y) */
        OMX_HISTCOMP_YLOG,  /**< Logarithmic luminance histogram (Y)*/
        OMX_HISTCOMP_R, /**< Red histogram component (R)*/
        OMX_HISTCOMP_G, /**< Green histogram component (G)*/
        OMX_HISTCOMP_B, /**< Blue histogram component (B)*/
        OMX_HISTCOMP_Cb,    /**< Chroma blue histogram component (Cb)*/
    OMX_HISTCOMP_Cr,     /**< Chroma red histogram component (Cr) */
    OMX_HISTCOMP_32BIT_PATCH = 0x7FFFFFFF
}OMX_HISTCOMPONENTTYPE;

/**
 * The OMX_TI_CAMERAVIEWTYPE enumeration is used to identify the
 * particular camera view and frame type that the rest of
 * the data in the structure is associated with.
 */
typedef enum OMX_TI_CAMERAVIEWTYPE {
    OMX_2D_Prv,         /**< Camera view in 2D for preview */
    OMX_2D_Snap,        /**< Camera view in 2D for snapshot */
    OMX_2D_Cap,         /**< Camera view in 2D for capture */
    OMX_3D_Left_Prv,    /**< Left camera view in 3D for preview */
    OMX_3D_Left_Snap,   /**< Left camera view in 3D for snapshot */
    OMX_3D_Left_Cap,    /**< Left camera view in 3D for capture */
    OMX_3D_Right_Prv,   /**< Right camera view in 3D for preview */
    OMX_3D_Right_Snap,  /**< Right camera view in 3D for snapshot */
    OMX_3D_Right_Cap,   /**< Right camera view in 3D for capture */
    OMX_TI_CAMERAVIEWTYPE_32BIT_PATCH = 0x7FFFFFFF
} OMX_TI_CAMERAVIEWTYPE;

/**
 *  nSize is the size of the structure including the length of data field containing
 *  the Histogram Matched for Stereo Gamma data.
 *  nItems is the number of items in the Gamma table.
 *  data[1] first byte of the Gamma data
 */
typedef struct OMX_HMSGAMMATYPE {
    OMX_U32 nSize;                          /**< The size of the structure
                                                 including the length of data field containing the gamma data */
    OMX_VERSIONTYPE       nVersion;
    OMX_U32               nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U32               nItems;            /**< The number of items in Gamma */
    OMX_U8                data[1];
} OMX_HMSGAMMATYPE;

/**
 *  Structure describes properties of pyramid level
 *  nOffset : nOffset from start of buffer in bytes
 *  nWidth  : nWidth  in pixels
 *  nHeight : nHeight in pixels
 *  nStride : nStride in bytes
 */
typedef struct OMX_TI_IMAGEPYRAMIDDESCTYPE {
    OMX_U32     nOffset;
    OMX_U16     nWidth;
    OMX_U16     nHeight;
    OMX_U32     nStride;
} OMX_TI_IMAGEPYRAMIDDESCTYPE;

/**
 *  The extra data having pyramid data,
 *  It is described with the following structure.
 *  nLevelsCount - Number of levels of pyramid
 *  PyramidData - first element of array with description
 *  of levels of pyramid. Size of array will describe
 *  by nLevelsCount.
 */
typedef struct OMX_TI_IMAGEPYRAMIDTYPE {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U32                     nPortIndex;
    OMX_TI_CAMERAVIEWTYPE       eCameraView;
    OMX_U32                     nLevelsCount;
    OMX_TI_IMAGEPYRAMIDDESCTYPE PyramidData[1];
} OMX_TI_IMAGEPYRAMIDTYPE;


#define OMX_OTHER_EXTRADATATYPE_SIZE ((OMX_U32)(((OMX_OTHER_EXTRADATATYPE *)0x0)->data))  /**< Size of OMX_OTHER_EXTRADATATYPE
                                                                                without Data[1] and without padding */

/**
 * The extra data having DCC data is described with the following structure.
 * This data contains single flags and values
 * (not arrays) that have general usage for camera applications.
 */
typedef struct OMX_TI_DCCDATATYPE {
    OMX_U32               nSize;
    OMX_VERSIONTYPE       nVersion;
    OMX_U32               nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U32               nCameraModuleId;
    OMX_U32               nDccDescriptorId;
    OMX_U32               nAlgorithmVendorId;
    OMX_U32               nUseCaseId;
    OMX_U32               nOffset;
    OMX_PTR               pData;
} OMX_TI_DCCDATATYPE;
/**
 * The extra data type to feed the camera re-processing function
 */
typedef struct OMX_TI_CAMREPROCMETATYPE {
    OMX_U32 nExpTime;
    OMX_U32 nGain;
} OMX_TI_CAMREPROCMETATYPE;

/**
 * The extra data vector shot feedback info
 *  nConfigId   : Same id that cames with
 *                OMX_TI_CONFIG_ENQUEUESHOTCONFIGS::nShotConfig[x].nConfigId
 *                for particular shot config.
 *  nFrameNum   : Frame number in vect shot repeat sequence.
 *                Starts from 1 for every shot config.
 *
 *  nExpMin     : The exposure time lower limit,[us]
 *  nExpMax     : The exposure time upper limit,[us]
 *  nGainMin    : The analog gain lower limit,[0,01EV]
 *  nGainMax    : The analog gain upper limit,[0,01EV]
 *
 *  nReqEC      : Requested total exposure compensation
 *  nReqExpTime : Requested exposure time
 *  nReqGain    : Requested gain
 *
 *  nExpTime    : Exposure time of this frame.
 *  nAGain      : Analog gain of this frame.
 *
 *  nSenExpTimeErr : Exposure time error in us.
 *                If the requested exposure time is ExpReq
 *                and the one produced by the sensor is nExpTime then:
 *                nExpTimeErr = nExpTime - ExpReq.
 *  nSenAGainErr: Analog gain error as multiplier (in Q8 format).
 *
 *  nDevEV      : The total exposure deviation,[us]
 *  nDevExpTime : The exposure time deviation after flicker reduction,[us]
 *  nDevAGain   : The analog gain deviation after flicker reduction,[0,01EV]
 */
typedef struct OMX_TI_VECTSHOTINFOTYPE {
    OMX_U32 nConfigId;
    OMX_U32 nFrameNum;
    OMX_U32 nExpMin;
    OMX_U32 nExpMax;
    OMX_U32 nGainMin;
    OMX_U32 nGainMax;
    OMX_S32 nReqEC;
    OMX_S32 nReqExpTime;
    OMX_S32 nReqGain;
    OMX_U32 nExpTime;
    OMX_U32 nAGain;
    OMX_S32 nSenExpTimeErr;
    OMX_U32 nSenAGainErr;
    OMX_S32 nDevEV;
    OMX_S32 nDevExpTime;
    OMX_S32 nDevAGain;
} OMX_TI_VECTSHOTINFOTYPE;

/*
 * LSC gain table size
 */
#define OMX_TI_LSC_GAIN_TABLE_SIZE (80 * 1024)

/**
 * Possible LSC table gain formats
 */
typedef enum OMX_TI_LSC_GAIN_FORMAT_TYPE {
    OMX_TI_LSC_GAIN_FORMAT_0Q8,
    OMX_TI_LSC_GAIN_FORMAT_0Q8_PLUS_1,
    OMX_TI_LSC_GAIN_FORMAT_1Q7,
    OMX_TI_LSC_GAIN_FORMAT_1Q7_PLUS_1,
    OMX_TI_LSC_GAIN_FORMAT_2Q6,
    OMX_TI_LSC_GAIN_FORMAT_2Q6_PLUS_1,
    OMX_TI_LSC_GAIN_FORMAT_3Q5,
    OMX_TI_LSC_GAIN_FORMAT_3Q5_PLUS_1,
    OMX_TI_LSC_GAIN_FORMAT = 0x7FFFFFFF
} OMX_TI_LSC_GAIN_FORMAT_TYPE;

/**
 * The extra data for LSC table
 *  bApplied    : If true the table is applied to the frame.
 *  eGainFormat : Paxel format
 *  nWidth      : LSC table width in paxels
 *  nHeight     : LSC table height in paxels
 *  pGainTable  : LSC gain table
 */
typedef struct OMX_TI_LSCTABLETYPE {
    OMX_BOOL bApplied;
    OMX_TI_LSC_GAIN_FORMAT_TYPE eGainFormat;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_U8 pGainTable[OMX_TI_LSC_GAIN_TABLE_SIZE];
} OMX_TI_LSCTABLETYPE;

/**
 * The extra data having ancillary data is described with the following structure.
 * This data contains single flags and values
 * (not arrays) that have general usage for camera applications.
 */
typedef  struct OMX_TI_ANCILLARYDATATYPE {
    OMX_U32             nSize;
    OMX_VERSIONTYPE     nVersion;
    OMX_U32             nPortIndex;
    OMX_TI_CAMERAVIEWTYPE       eCameraView;
    OMX_U32             nAncillaryDataVersion;
    OMX_U32             nFrameNumber;
    OMX_U32             nShotNumber;
    OMX_U16             nInputImageHeight;
    OMX_U16             nInputImageWidth;
    OMX_U16             nOutputImageHeight;
    OMX_U16             nOutputImageWidth;
    OMX_U16             nDigitalZoomFactor;
    OMX_S16             nCropCenterColumn;
    OMX_S16             nCropCenterRow;
    OMX_U16             nOpticalZoomValue;
    OMX_U8              nFlashConfiguration;
    OMX_U8              nFlashUsage;
    OMX_U32             nFlashStatus;
    OMX_U8              nAFStatus;
    OMX_U8              nAWBStatus;
    OMX_U8              nAEStatus;
    OMX_U32             nExposureTime;
    OMX_U16             nEVCompensation;
    OMX_U8              nDigitalGainValue;
    OMX_U8              nAnalogGainValue;
    OMX_U16             nCurrentISO;
    OMX_U16             nReferenceISO;
    OMX_U8              nApertureValue;
    OMX_U8              nPixelRange;
    OMX_U16             nPixelAspectRatio;
    OMX_U8              nCameraShake;
    OMX_U16             nFocalDistance;
    OMX_U64             nParameterChangeFlags;
    OMX_U8              nNumFacesDetected;
    OMX_U8              nConvergenceMode;
    OMX_U8              nConvergenceStatus;
    OMX_U8              nDCCStatus;
} OMX_TI_ANCILLARYDATATYPE;

/**
 * White Balance Results data
 *  The extra data having white balance results data is
 *  described with the following structure..
 */
typedef struct OMX_TI_WHITEBALANCERESULTTYPE {
    OMX_U32             nSize;          /**< Size */
    OMX_VERSIONTYPE     nVersion;       /**< Version */
    OMX_U32             nPortIndex;     /**< Port Index */
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U16             nColorTemperature;      /**< White Balance Color Temperature in Kelvins */
    OMX_U16             nGainR;         /**< Bayer applied R color channel gain in (U13Q9) */
    OMX_U16             nGainGR;        /**< Bayer applied Gr color channel gain in (U13Q9) */
    OMX_U16             nGainGB;        /**< Bayer applied Gb color channel gain in (U13Q9) */
    OMX_U16             nGainB;         /**< Bayer applied B color channel gain in (U13Q9) */
    OMX_S16             nOffsetR;       /**< Bayer applied R color channel offset */
    OMX_S16             nOffsetGR;      /**< Bayer applied Gr color channel offset */
    OMX_S16             nOffsetGB;      /**< Bayer applied Gb color channel offset */
    OMX_S16             nOffsetB;       /**< Bayer applied B color channel offset */
} OMX_TI_WHITEBALANCERESULTTYPE;

/**
 * Unsaturated Regions data
 * The extra data having unsaturated regions data is
 * described with the following structure..
 */
typedef struct OMX_TI_UNSATURATEDREGIONSTYPE {
    OMX_U32             nSize;          /**< Size */
    OMX_VERSIONTYPE     nVersion;       /**< Version */
    OMX_U32             nPortIndex;     /**< Port Index */
    OMX_U16             nPaxelsX;       /**< The number of paxels in the horizontal direction */
    OMX_U16             nPaxelsY;       /**< The number of paxels in the vertical direction */
    OMX_U16         data[1];     /**< the first value of an array of values that represent
                                     the percentage of unsaturated pixels within the associated paxel */
} OMX_TI_UNSATURATEDREGIONSTYPE;

/**
 * OMX_BARCODETYPE
 */
typedef enum OMX_BARCODETYPE {
        OMX_BARCODE1D = 0,      /**< 1D barcode */
        OMX_BARCODE2D,          /**< 2D barcode */
    OMX_BarcodeMax = 0x7fffffff
}OMX_BARCODETYPE;
/**
 * Brcode detection data
 *	nLeft is the leftmost coordinate of the detected area rectangle.
 *	nTop is the topmost coordinate of the detected area rectangle.
 *	nWidth is the width of the detected area rectangle in pixels.
 *	nHeight is the height of the detected area rectangle in pixels.
 *	nOrientation is the orientation of the axis of the detected object. This refers to the angle between the vertical axis of barcode and the horizontal axis.
 *	eBarcodetype is an enumeration specifying the barcode type, as listed in the given table.
 */
typedef struct OMX_BARCODEDETECTIONTYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_S32               nLeft;     /**< The leftmost coordinate of the detected area rectangle */
    OMX_S32               nTop;     /**< Topmost coordinate of the detected area rectangle */
    OMX_U32               nWidth;     /**< The width of the detected area rectangle in pixels */
    OMX_U32               nHeight;     /**< The height of the detected area rectangle in pixels */
    OMX_S32               nOrientation;     /**< The orientation of the axis of the detected object.
                                         This refers to the angle between the vertical axis of barcode and the horizontal axis */
    OMX_BARCODETYPE eBarcodetype;     /**< An enumeration specifying the barcode type, as listed in the given table */
 } OMX_BARCODEDETECTIONTYPE;

/**
 * Front object detection data
 *	nLeft is the leftmost coordinate of the detected area rectangle.
 *	nTop is the topmost coordinate of the detected area rectangle.
 *	nWidth is the width of the detected area rectangle in pixels.
 *	nHeight is the height of the detected area rectangle in pixels.
 */
typedef struct OMX_FRONTOBJDETECTIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_S32               nLeft;     /**< The leftmost coordinate of the detected area rectangle */
    OMX_S32               nTop;     /**< The topmost coordinate of the detected area rectangle */
    OMX_U32               nWidth;     /**< The width of the detected area rectangle in pixels */
    OMX_U32               nHeight;     /**< The height of the detected area rectangle in pixels */
} OMX_FRONTOBJDETECTIONTYPE;

/**
 * Distance estimation data
 * nDistance is the estimated distance to the object in millimeters.
 * nLargestDiscrepancy is the estimated largest discrepancy of the distance to the object in millimeters. When equal to MAX_INT the discrepancy is unknown.
 */
typedef struct OMX_DISTANCEESTIMATIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U32               nDistance;        /**< Estimated distance to the object in millimeters */
    OMX_U32               nLargestDiscrepancy;     /**< the estimated largest discrepancy of the distance to the object in millimeters.
                                                 When equal to MAX_INT the discrepancy is unknown */
} OMX_DISTANCEESTIMATIONTYPE;

/**
 * Distance estimation data
 * nDistance is the estimated distance to the object in millimeters.
 * nLargestDiscrepancy is the estimated largest discrepancy of the distance to the object in millimeters. When equal to MAX_INT the discrepancy is unknown.
 */

typedef struct OMX_MOTIONESTIMATIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_S32               nPanX;     /**< The detected translation in horizontal direction.
                                     The value is represented as pixels in Q16-format */
    OMX_S32 nPanY;              /**< The detected translation in vertical direction.
                                     The value is represented as pixels in Q16-format */
} OMX_MOTIONESTIMATIONTYPE;


/**
 * Focus region data
 *	nRefPortIndex is the port the image frame size is defined on. This image frame size is used as reference for the focus region rectangle.
 *	nLeft is the leftmost coordinate of the focus region rectangle.
 *	nTop is the topmost coordinate of the focus region rectangle.
 *	nWidth is the width of the focus region rectangle in pixels.
 *	nHeight is the height of the focus region rectangle in pixels.
 *
 */
typedef struct OMX_FOCUSREGIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U32               nRefPortIndex;     /**< The port the image frame size is defined on.
                                     This image frame size is used as reference for the focus region rectangle */
    OMX_S32 nLeft;              /**< The leftmost coordinate of the focus region rectangle */
    OMX_S32 nTop;               /**< The topmost coordinate of the focus region rectangle */
    OMX_U32 nWidth;             /**< The width of the focus region rectangle in pixels */
    OMX_U32 nHeight;            /**< The height of the focus region rectangle in pixels */
} OMX_FOCUSREGIONTYPE;

/**
 * OMX_ISOSETTINGTYPE: specifies its auto or manual setting
 *
 */
typedef enum OMX_ISOSETTINGTYPE{
        OMX_Auto = 0, /**<	*/
    OMX_IsoManual,      /**< */
    OMX_IsoSettingMax = 0x7fffffff
}OMX_ISOSETTINGTYPE;

/**
 *  nSize is the size of the structure including the length of data field containing
 *  the histogram data.
 *  eISOMode:
 *  	specifies the ISO seetting mode - auto/manual
 *  nISOSetting:
 *  	for manual mode client can specify the ISO setting.
 */

typedef struct OMX_CONFIG_ISOSETTINGTYPE{
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_ISOSETTINGTYPE eISOMode;
	OMX_U32 nISOSetting;
}OMX_CONFIG_ISOSETTINGTYPE;

/**
 * custom RAW format
 */
typedef struct OMX_CONFIG_RAWFORMATTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_VERSIONTYPE nFormatVersion;
    OMX_STRING cVendorName;
} OMX_CONFIG_RAWFORMATTYPE;

/**
 * Sensor type
 */
typedef struct OMX_CONFIG_SENSORTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_VERSIONTYPE nSensorVersion;
    OMX_STRING cModelName;
} OMX_CONFIG_SENSORTYPE;

/**
* Sensor Detect
*/
typedef struct OMX_TI_PARAM_SENSORDETECT {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_BOOL        bSensorDetect;
} OMX_TI_PARAM_SENSORDETECT;

/**
 * OMX_BAYERCOMPRESSION
 *
 */
typedef enum OMX_BAYERCOMPRESSION {
    OMX_BAYER_UNPACKED,
    OMX_BAYER_PACKED10,
    OMX_BAYER_ALAW,
    OMX_BAYER_DPCM,
    OMX_BAYER_MAX = 0x7FFFFFFF
} OMX_BAYERCOMPRESSION;

/**
* Sensor Detect
*/
typedef struct OMX_TI_PARAM_BAYERCOMPRESSION {
    OMX_U32              nSize;
    OMX_VERSIONTYPE      nVersion;
    OMX_U32              nPortIndex;
    OMX_BAYERCOMPRESSION eBayerCompression;
} OMX_TI_PARAM_BAYERCOMPRESSION;

/**
 * Sensor custom data type
 */
typedef struct OMX_CONFIG_SENSORCUSTOMDATATYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nDataLength;
    OMX_U8 xSensorData[1];
} OMX_CONFIG_SENSORCUSTOMDATATYPE;

/**
 * OMX_OBJDETECTQUALITY
 *
 */
typedef enum OMX_OBJDETECTQUALITY{
        OMX_FastDetection = 0, /**< A detection that prioritizes speed*/
        OMX_Default,    /**< The default detection, should be used when no control of the detection quality is given.*/
        OMX_BetterDetection,    /**< A detection that levels correct detection with speed*/
        OMX_BestDtection,   /**< A detection that prioritizes correct detection*/
    OMX_AUTODETECTION,       /**< Automatically decide which object detection quality is best.*/
    OMX_ObjDetectQualityMax = 0x7fffffff
}OMX_OBJDETECTQUALITY;

/**
 * OBJECT DETECTION Type
 *      nPortIndex: is an output port. The port index decides on which port the extra data structur of detected object is sent on.
 *      bEnable : this controls ON/OFF for this object detection algirithm.
 *      bFrameLimited: is a Boolean used to indicate if detection shall be terminated after the specified number of frames if
 *          true frame limited detection is enabled; otherwise the port does not terminate detection until instructed to do so by the client.
 *      nFrameLimit: is the limit on number of frames detection is executed for, this parameter is only valid if bFrameLimited is enabled.
 *      nMaxNbrObjects: specifies the maximum number of objects that should be found in each frame. It is implementation dependent which objects are found.
 *      nLeft: is the leftmost coordinate of the detection area rectangle.
 *      nTop: is the topmost coordinate of the detection area rectangle.
 *      nWidth: is the width of the detection area rectangle in pixels.
 *      nHeight: is the height of the detection area rectangle in pixels.
 *      eObjDetectQuality: is an enumeration specifying the quality desired by the detection.
 *      nPriority: represents priority of each object when there are multiple objects detected.
 */

typedef struct OMX_CONFIG_OBJDETECTIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bEnable;
    OMX_BOOL bFrameLimited;
    OMX_U32 nFrameLimit;
    OMX_U32 nMaxNbrObjects;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_OBJDETECTQUALITY eObjDetectQuality;
    OMX_U32 nPriority;
    OMX_U32 nDeviceOrientation;
 } OMX_CONFIG_OBJDETECTIONTYPE;


/**
 * OMX_OBJDETECTQUALITY
 *
 */
typedef enum OMX_DISTTYPE{
        OMX_DistanceControlFocus = 0, /**< focus objects distance type*/
    OMX_DISTANCECONTROL_RECT,       /**< Evaluated distance to the object found in the rectangelar area indicated as input region.  */
    OMX_DistTypeMax = 0x7fffffff
}OMX_DISTTYPE;


/**
 * Distance mesurement
 *	bStarted is a Boolean. The IL client sets it to true to start the measurement .
 *		the IL client sets to false to stop the measurement. The IL client can query it to check if the measurement is ongoing.
 *	nLeft : is the leftmost coordinate of the rectangle.
 *	nTop : is the topmost coordinate of the rectangle.
 *	nWidth:  is the width of the rectangle in pixels.
 *	nHeight:  is the height of the rectangle in pixels.
 *	eDistType:  is an enumeration specifying the distance measurement type, as shown in
 */
typedef struct OMX_CONFIG_DISTANCETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bStarted;
    OMX_S32 nLeft;
    OMX_S32 nTop;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_DISTTYPE eDistType;
} OMX_CONFIG_DISTANCETYPE;


/**
 * face detect data - face attribute
 *  nARGBEyeColor: is the indicates a 32-bit eye color of the person, where bits 0-7 are blue,
 *      bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha.
 *  nARGBSkinColor: is the indicates a 32-bit skin color of the person, where bits 0-7 are blue,
 *      bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha.
 *  nARGBHairColor: is the indicates a 32-bit hair color of the person, where bits 0-7 are blue,
 *      bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha.
 *  nSmileScore: a smile detection score between 0 and 100, where 0 means not detecting,
 *      1 means least certain and 100 means most certain a smile is detected.
 *  nBlinkScore: a eye-blink detection score between 0 and 100, where 0 means not detecting,
 *      1 means least certain and 100 means most certain an eye-blink is detected.
 *  xIdentity: represents the identity of the face. With identity equal to zero this is not supported.
 *      This can be used by a face recognition application. The component shall not reuse an identity value unless the same face.
 *      Can be used to track detected faces when it moves between frames. Specific usage of this field is implementation dependent.
 *      It can be some kind of ID.
 *
 */
typedef struct OMX_FACEATTRIBUTE {
    OMX_U32 nARGBEyeColor;      /**< The indicates a 32-bit eye color of the person,
                                     where bits 0-7 are blue, bits 15-8 are green, bits 24-16 are red,
                                     and bits 31-24 are for alpha. */
    OMX_U32 nARGBSkinColor;     /**< The indicates a 32-bit skin color of the person,
                                     where bits 0-7 are blue, bits 15-8 are green, bits 24-16 are red,
                                     and bits 31-24 are for alpha */
    OMX_U32 nARGBHairColor;     /**< the indicates a 32-bit hair color of the person,
                                     where bits 0-7 are blue, bits 15-8 are green, bits 24-16 are red,
                                    and bits 31-24 are for alpha */
    OMX_U32 nSmileScore;        /**< Smile detection score between 0 and 100, where 0 means not detecting,
                                     1 means least certain and 100 means most certain a smile is detected */
    OMX_U32 nBlinkScore;        /**< Eye-blink detection score between 0 and 100, where 0 means not detecting,
                                     1 means least certain and 100 means most certain an eye-blink is detected */
    OMX_U32 xIdentity[4];       /**< represents the identity of the face. With identity equal to zero this is not supported.
                                     This can be used by a face recognition application.
                                     The component shall not reuse an identity value unless the same face.
                                     Can be used to track detected faces when it moves between frames.
                                     Specific usage of this field is implementation dependent.
                                     It can be some kind of ID */
} OMX_FACEATTRIBUTE;

/**
 * xGamma represents lool-up table for gamma correction in Q16 format.
 * All values assume that maximum value is 255. If internal implementation uses higher dynamic range, this value should be adjusted internally.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nScore: is a detection score between 0 and 100, where 0 means unknown score, 1 means least certain and 100 means most certain the detection is correct.
 *  nLeft: is the leftmost coordinate of the detected area rectangle.
 *  nTop: is the topmost coordinate of the detected area rectangle.
 *  nWidth: is the width of the detected area rectangle in pixels.
 *  nHeight: is the height of the detected area rectangle in pixels.
 *  nOrientationRoll/Yaw/Pitch is the orientation of the axis of the detected object. Here roll angle is defined as the angle between the vertical axis of face and the horizontal axis. All angles can have the value of -180 to 180 degree in Q16 format. Some face detection algorithm may not be able to fill in the angles, this is denoted by the use of MAX_INT value.
 *  nPriority represents priority of each object when there are multiple objects detected.
 *  nFaceAttr describe the attributes of the detected face object with the following structure:
 *
 *
 */
typedef struct OMX_TI_FACERESULT {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U32               nScore;     /**< Detection score between 0 and 100, where 0 means unknown score,
                                         1 means least certain and 100 means most certain the detection is correct */
    OMX_S32 nLeft;                  /**< The leftmost coordinate of the detected area rectangle */
    OMX_S32 nTop;                   /**< The topmost coordinate of the detected area rectangle */
    OMX_U32 nWidth;                 /**< The width of the detected area rectangle in pixels */
    OMX_U32 nHeight;                /**< The height of the detected area rectangle in pixels */
    // The orientation of the axis of the detected object.
    // Here roll angle is defined as the angle between the vertical axis of face and the horizontal axis.
    // All angles can have the value of -180 to 180 degree in Q16 format.
    // Some face detection algorithm may not be able to fill in the angles, this is denoted by the use of MAX_INT value.
OMX_S32 nOrientationRoll;
OMX_S32 nOrientationYaw;
OMX_S32 nOrientationPitch;
    //
    OMX_U32           nPriority;     /**< Represents priority of each object when there are multiple objects detected */
    OMX_FACEATTRIBUTE nFaceAttr;     /**< Describe the attributes of the detected face object with the following structure */
} OMX_TI_FACERESULT;


/**
 * Face detection data
 * The extra data having face detection data is described with the following structure.
 * The parser should only assume that the first tFacePosition[ulFaceCount] of the 35 elements
 * of the array should contain valid data.
 */
typedef struct OMX_FACEDETECTIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE    eCameraView;
    OMX_U16               ulFaceCount;     // faces detected
    OMX_TI_FACERESULT tFacePosition[35];// 35 is max faces supported by FDIF
} OMX_FACEDETECTIONTYPE;

/**
 * MTIS Vendor Specific Motion estimation
 * The extra data having MTIS motion estimation data is
 * described with the following structure.
 */
typedef struct OMX_TI_MTISTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_S32 nMaxMVh;            /**< The maximum MV for horizontal direction */
    OMX_S32 nMaxMVv;            /**< The maximum MV for vertical direction */
    OMX_U16 nMVRelY[9];         /**< The mask for MV reliability */
    OMX_U16 nMVRelX[9];         /**< The mask for MV reliability */
    OMX_S32 nMVh[9];            /**< The MVs for horizontal direction */
    OMX_S32 nMVv[9];            /**< The MVs for vertical direction */
} OMX_TI_MTISTYPE;

/**
 * The OMX_EXTRADATATYPE enumeration is used to define the
 * possible extra data payload types.
 */
typedef enum OMX_EXT_EXTRADATATYPE {
    OMX_ExifAttributes = 0x7F000001,    /**< 0x7F000001 Reserved region for introducing Vendor Extensions */
    OMX_AncillaryData,                  /**< 0x7F000002 ancillary data */
    OMX_WhiteBalance,                   /**< 0x7F000003 white balance resultant data */
    OMX_UnsaturatedRegions,             /**< 0x7F000004 unsaturated regions data */
    OMX_FaceDetection,                  /**< 0x7F000005 face detect data */
    OMX_BarcodeDetection,               /**< 0x7F000006 bar-code detct data */
    OMX_FrontObjectDetection,           /**< 0x7F000007 Front object detection data */
    OMX_MotionEstimation,               /**< 0x7F000008 motion Estimation data */
    OMX_MTISType,                       /**< 0x7F000009 MTIS motion Estimation data */
    OMX_DistanceEstimation,             /**< 0x7F00000A disctancedistance estimation */
    OMX_Histogram,                      /**< 0x7F00000B histogram */
    OMX_FocusRegion,                    /**< 0x7F00000C focus region data */
    OMX_ExtraDataPanAndScan,            /**< 0x7F00000D pan and scan data */
    OMX_RawFormat,                      /**< 0x7F00000E custom RAW data format */
    OMX_SensorType,                     /**< 0x7F00000F vendor & model of the sensor being used */
    OMX_SensorCustomDataLength,         /**< 0x7F000010 vendor specific custom data length */
    OMX_SensorCustomData,               /**< 0x7F000011 vendor specific data */
    OMX_TI_FrameLayout,                 /**< 0x7F000012 vendor specific data */
    OMX_TI_SEIinfo2004Frame1,           /**< 0x7F000013 Used for 2004 SEI message to be provided by video decoders */
    OMX_TI_SEIinfo2004Frame2,           /**< 0x7F000014 Used for 2004 SEI message to be provided by video decoders */
    OMX_TI_SEIinfo2010Frame1,           /**< 0x7F000015 Used for 2010 SEI message to be provided by video decoders */
    OMX_TI_SEIinfo2010Frame2,           /**< 0x7F000016 Used for 2010 SEI message to be provided by video decoders */
    OMX_TI_RangeMappingInfo,            /**< 0x7F000017 Used for Range mapping info provided by Video Decoders */
    OMX_TI_RescalingInfo,               /**< 0x7F000018 Used for width/height rescaling info provided by Video Decoders */
    OMX_TI_WhiteBalanceOverWrite,       /**< 0x7F000019 Used for manual AWB settings */
    OMX_TI_CPCamData,                   /**< 0x7F00001A Used for cp cam data */
    OMX_TI_H264ESliceDataInfo,          /**< 0x7F00001B */
    OMX_TI_DccData,                     /**< 0x7F00001C Used for dcc data overwrite in the file system */
    OMX_TI_ProfilerData,                /**< 0x7F00001D Used for profiling data */
    OMX_TI_VectShotInfo,                /**< 0x7F00001E Used for vector shot feedback notification */
    OMX_TI_CamReProcMeta,               /**< 0x7F00001F Used for meta data input to camera re-proc function */
    OMX_TI_LSCTable,                    /**< 0x7F000020 Lens shading table for corresponding frame */
    OMX_TI_CodecExtenderErrorFrame1,    /**< 0x7F000021 Used for Codec Extended Error to be provided byvideo decoders */
    OMX_TI_CodecExtenderErrorFrame2,    /**< 0x7F000022 Used for Codec Extended Error to be provided byvideo decoders */
    OMX_TI_MBInfoFrame1,                /**< 0x7F000023 Used for MBError message to be provided by videodecoders */
    OMX_TI_MBInfoFrame2,                /**< 0x7F000024 Used for MBError message to be provided by videodecoders */
    OMX_TI_SEIInfoFrame1,               /**< 0x7F000025 Used for SEI message to be provided by video decoders*/
    OMX_TI_SEIInfoFrame2,               /**< 0x7F000026 Used for SEI message to be provided by video decoders*/
    OMX_TI_VUIInfoFrame1,               /**< 0x7F000027 Used for VUI message to be provided by video decoders */
    OMX_TI_VUIInfoFrame2,               /**< 0x7F000028 Used for VUI message to be provided by video decoders */
    OMX_TI_FaceDetectionRaw,            /**< 0x7F000029 Face detect data without face tracking calculations */
    OMX_TI_HMSGamma,                    /**< 0x7F00002A Histogram Matched for Stereo Gamma table */
    OMX_TI_ImagePyramid,                /**< 0x7F00002B Describe image piramid sizes for each level of pyramid */
    OMX_TI_ExtraData_AFStatistics,      /**< 0x7F00002C Auto Focus buffer and settings for corresponding frame */
    OMX_TI_ExtraData_AEWBStatistics,    /**< 0x7F00002D Auto Exppsure, white balance buffer and settings for corresponding frame */
    OMX_TI_ExtraData_BSCStatistics,     /**< 0x7F00002E Boundary signal calculator statistics for corresponding frame */
    OMX_TI_ExtraData_AuxiliaryImage,    /**< 0x7F00002F Auxiliary image contains rescaled image at QVGA resolution */
    OMX_TI_ExtraData_Count,
    OMX_TI_ExtraData_Max = OMX_TI_ExtraData_Count - 1,
    OMX_TI_ExtraData_32Bit_Patch = 0x7fffffff
} OMX_EXT_EXTRADATATYPE;

/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port on which this extra data to be assosiated
 *  eExtraDataType :  Extra data type
 *  bEnable      : Eneble/Disable this extra-data through port.
 *
 */
typedef struct OMX_CONFIG_EXTRADATATYPE {
    OMX_U32 nSize;                              /**< The size of the structure including data bytes
                                                     and any padding necessary to ensure 32bit alignment
                                                     of the next OMX_OTHER_EXTRADATATYPE structure */
    OMX_VERSIONTYPE nVersion;
    OMX_U32               nPortIndex;           /**< The read-only value containing the index of the port */
    OMX_EXT_EXTRADATATYPE eExtraDataType;       /**< Identifies the extra data payload type */
    OMX_BOOL bEnable;
} OMX_CONFIG_EXTRADATATYPE;

/**
 * JPEG header type
 * */

typedef enum OMX_JPEGHEADERTYPE{
	OMX_NoHeader = 0,
	OMX_JFIF,
    OMX_EXIF,
    OMX_JpegHeaderTypeMax = 0x7fffffff
}OMX_JPEGHEADERTYPE;
/**
 * Re-start marker configuration
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port on which this extra data to be assosiated
 *  eJpegHeaderType : JPEG header type EXIF, JFIF, or No heeader.
 */

typedef struct OMX_CONFIG_JPEGHEEADERTYPE{
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_JPEGHEADERTYPE eJpegHeaderType;
}OMX_CONFIG_JPEGHEEADERTYPE;

/**
 * Re-start marker configuration
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port on which this extra data to be assosiated
 *  nRstInterval :  interval at which RST markers are to be inserted.
 *  bEnable      : Eneble/Disable this RST marker insertion feature.
 *
 */

typedef struct OMX_CONFIG_RSTMARKER{
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_U32 nRstInterval;
	OMX_BOOL nEnable;
}OMX_CONFIG_RSTMARKER;

/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 * nMaxSize : max size
 *
 *
 */
typedef struct OMX_IMAGE_JPEGMAXSIZE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nMaxSize;
} OMX_IMAGE_JPEGMAXSIZE;


typedef enum OMX_IMAGESTAMPOPERATION{
    OMX_NewImageStamp = 0,
    OMX_Continuation,
    OMX_ImageStapOperationMax = 0x7fffffff
}OMX_IMAGESTAMPOPERATION;


/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 * nMaxSize : max size
 *
 *
 */
typedef struct OMX_PARAM_IMAGESTAMPOVERLAYTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_IMAGESTAMPOPERATION nOp;
    OMX_U32 nLeft;
    OMX_U32 nTop;
    OMX_U32 nHeight;
    OMX_U32 nWidth;
    OMX_COLOR_FORMATTYPE eFormat;
    OMX_U8 * pBitMap;
} OMX_PARAM_IMAGESTAMPOVERLAYTYPE;


/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 * nMaxSize : max size
 *
 *
 */
typedef struct OMX_PARAM_THUMBNAILTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nHeight;
    OMX_U32 nWidth;
    OMX_IMAGE_CODINGTYPE eCompressionFormat;
    OMX_COLOR_FORMATTYPE eColorFormat;
    OMX_U32 nQuality;
    OMX_U32 nMaxSize;
} OMX_PARAM_THUMBNAILTYPE;

/**
 * Red-Eye Removal Enum
 */
typedef enum OMX_REDEYEREMOVALTYPE{
    OMX_RedEyeRemovalOff    = 0, /** No red eye removal*/
    OMX_RedEyeRemovalOn, /**    Red eye removal on*/
    OMX_RedEyeRemovalAuto,  /** Red eye removal will be done automatically when detected*/
    OMX_RedEyeRemovalKhronosExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions*/
    OMX_RedEyeRemovalVendorStartUnused = 0x7F000000,    /** Reserved region for introducing Vendor Extensions*/
    OMX_RedEyeRemovalMax = 0x7FFFFFFF
}OMX_REDEYEREMOVALTYPE;

/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  nLeft: is the leftmost coordinate of the detection area rectangle (such as face region).
 *  nTop: is the topmost coordinate of the detection area rectangle (such as face region).
 *  nWidth: is the width of the detection area rectangle  in pixels.
 *  nHeight: is the height of the detection area rectangle in pixels.
 *  nARGBEyeColor indicates a 32-bit eye color to replace the red-eye, where bits 0-7 are blue, bits 15-8 are green, bits 24-16 are red, and bits 31-24 are for alpha. When all zero indicates automatic choice.

 *
 */
typedef struct OMX_CONFIG_REDEYEREMOVALTYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_S32 nLeft;
   OMX_S32 nTop;
   OMX_U32 nWidth;
   OMX_U32 nHeight;
   OMX_U32 nARGBEyeColor;
   OMX_REDEYEREMOVALTYPE eMode;
} OMX_CONFIG_REDEYEREMOVALTYPE;






/**
 * Video capture YUV Range Enum
 */
typedef enum OMX_VIDEOYUVRANGETYPE{
    OMX_ITURBT601 = 0,
    OMX_Full8Bit,
    OMX_VideoYUVRangeKhronosExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions*/
    OMX_VideoYUVRangeVendorStartUnused = 0x7F000000,    /** Reserved region for introducing Vendor Extensions*/
    OMX_VideoYUVRangeMax = 0x7FFFFFFF
}OMX_VIDEOYUVRANGETYPE;

/**
 * Enable Extra-data on a specific port.
 *
 *
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *
 */
typedef struct OMX_PARAM_VIDEOYUVRANGETYPE {
   OMX_U32 nSize;
   OMX_VERSIONTYPE nVersion;
   OMX_U32 nPortIndex;
   OMX_VIDEOYUVRANGETYPE eYUVRange;
} OMX_PARAM_VIDEOYUVRANGETYPE;

/**
 * Video noise filter mode range enum
 */
typedef enum OMX_VIDEONOISEFILTERMODETYPE{
    OMX_VideoNoiseFilterModeOff = 0,
    OMX_VideoNoiseFilterModeOn,
    OMX_VideoNoiseFilterModeAuto,
    OMX_VideoNoiseFilterModeExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions */
    OMX_VideoNoiseFilterModeStartUnused = 0x7F000000,   /** Reserved region for introducing Vendor Extensions */
    OMX_VideoNoiseFilterModeMax = 0x7FFFFFFF
} OMX_VIDEONOISEFILTERMODETYPE;

/**
 * Enable video noise filter.
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eMode       : Video noise filter mode (on/off/auto)
 */
typedef struct OMX_PARAM_VIDEONOISEFILTERTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_VIDEONOISEFILTERMODETYPE eMode;
} OMX_PARAM_VIDEONOISEFILTERTYPE;


/**
 * High ISO Noise filter mode range enum
 */
typedef enum OMX_ISONOISEFILTERMODETYPE{
    OMX_ISONoiseFilterModeOff = 0,
    OMX_ISONoiseFilterModeOn,
    OMX_ISONoiseFilterModeAuto,
    OMX_ISONoiseFilterModeExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions */
    OMX_ISONoiseFilterModeStartUnused = 0x7F000000,   /** Reserved region for introducing Vendor Extensions */
    OMX_ISONoiseFilterModeMax = 0x7FFFFFFF
} OMX_ISONOISEFILTERMODETYPE;

/**
 * Enable ISO noise filter.
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eMode       : ISO noise filter (NSF2 is used) mode (on/off/auto)
 */
typedef struct OMX_PARAM_ISONOISEFILTERTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_ISONOISEFILTERMODETYPE eMode;
} OMX_PARAM_ISONOISEFILTERTYPE;

/**
 * Structure used to to call OMX_GetParams() for each
 * increment of "Index" starting with "0"
 *
 * STRUCT MEMBERS:
 * nSize            : Size of the structure in bytes
 * nVersion         : OMX specification version information
 * nIndex           : Index of the sDCCURI 0..MAX_URI_LENGTH
 * sDCCURI          : Look-up table containing strings. Ends with '\0'
 */
typedef struct OMX_TI_PARAM_DCCURIINFO {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nIndex;
    OMX_S8 sDCCURI[MAX_URI_LENGTH];
} OMX_TI_PARAM_DCCURIINFO;

/**
 * Manual White Balance color temperature
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  nColorTemperature : Color Temperature in K
 */
typedef struct OMX_TI_CONFIG_WHITEBALANCECOLORTEMPTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nColorTemperature;
} OMX_TI_CONFIG_WHITEBALANCECOLORTEMPTYPE;

/**
 * Focus spot weighting range enum
 */
typedef enum OMX_TI_CONFIG_FOCUSSPOTMODETYPE {
    OMX_FocusSpotDefault = 0,                           /** Makes CommonFocusRegion to be used. */
    OMX_FocusSpotSinglecenter,                          /** Only central part of the image is used for focus. */
    OMX_FocusSpotMultiNormal,                           /** Middle part of the image is used with 100% weight, upper and lower parts are with 50%. */
    OMX_FocusSpotMultiAverage,                          /** All the image is used with 100% weight. */
    OMX_FocusSpotMultiCenter,                           /** Central part of the image is used with 100% weight, the rest is used with 50%. */
    OMX_FocusSpotExtensions = 0x6F000000,               /** Reserved region for introducing Khronos Standard Extensions */
    OMX_FocusSpotModeStartUnused = 0x7F000000,          /** Reserved region for introducing Vendor Extensions */
    OMX_FocusSpotModeMax = 0x7FFFFFFF
} OMX_TI_CONFIG_FOCUSSPOTMODETYPE;

/**
 * Focus Spot Weighting configuration.
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eMode       : Spot Weighting mode
 */
typedef struct OMX_TI_CONFIG_FOCUSSPOTWEIGHTINGTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_TI_CONFIG_FOCUSSPOTMODETYPE eMode;
} OMX_TI_CONFIG_FOCUSSPOTWEIGHTINGTYPE;

/**
 * Enumeration of possible Exposure control types for OMX_EXPOSURECONTROLTYPE
 */
typedef enum OMX_TI_EXTEXPOSURECONTROLTYPE {
    OMX_TI_ExposureControlVeryLong = OMX_ExposureControlVendorStartUnused + 1,
    OMX_TI_ExposureControlFacePriority,
    OMX_TI_ExposureControlMax = 0x7fffffff
} OMX_TI_EXTEXPOSURECONTROLTYPE;

/**
 * Variable frame rate configuration.
 *
 * STRUCT MEMBERS:
 *  nSize         : Size of the structure in bytes
 *  nVersion      : OMX specification version information
 *  nPortIndex    : Port that this structure applies to
 *  xMinFramerate : Minimum variable frame rate value
 */
typedef struct OMX_TI_PARAM_VARFRAMERATETYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_U32 xMinFramerate;
} OMX_TI_PARAM_VARFRAMERATETYPE;

/**
 * Exposure config for right frame
 */
typedef struct OMX_TI_CONFIG_EXPOSUREVALUERIGHTTYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_U32 nApertureFNumber;  /**< e.g. nApertureFNumber = 2 implies "f/2" - Q16 format */
	OMX_U32 nShutterSpeedMsec; /**< Shutterspeed in milliseconds */
	OMX_U32 nSensitivity;      /**< e.g. nSensitivity = 100 implies "ISO 100" */
} OMX_TI_CONFIG_EXPOSUREVALUERIGHTTYPE;

/**
 * Auto Convergence mode enum
 */
typedef enum OMX_TI_AUTOCONVERGENCEMODETYPE {
	OMX_TI_AutoConvergenceModeDisable,
	OMX_TI_AutoConvergenceModeFrame,
	OMX_TI_AutoConvergenceModeCenter,
	OMX_TI_AutoConvergenceModeFocusFaceTouch,
	OMX_TI_AutoConvergenceModeManual,
	OMX_TI_AutoConvergenceExtensions = 0x6F000000,    /** Reserved region for introducing Khronos Standard Extensions */
	OMX_TI_AutoConvergenceStartUnused = 0x7F000000,   /** Reserved region for introducing Vendor Extensions */
	OMX_TI_AutoConvergenceModeMax = 0x7FFFFFFF
} OMX_TI_AUTOCONVERGENCEMODETYPE;

/**
 * Variable farame rate configuration.
 *
 * STRUCT MEMBERS:
 *  nSize             : Size of the structure in bytes
 *  nVersion          : OMX specification version information
 *  nPortIndex        : Port that this structure applies to
 *  eACMode           : Auto convergence mode
 *  nManualConverence : Manual Converence value
 *  nACProcWinStartX  : Start X AC Window
 *  nACProcWinStartY  : Start Y AC Window
 *  nACProcWinWidth   : Width of AC Window
 *  nACProcWinHeight  : Height of AC Window
 *  bACStatus         : output status from AL alg
 */
typedef struct OMX_TI_CONFIG_CONVERGENCETYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_TI_AUTOCONVERGENCEMODETYPE eACMode;
	OMX_S32 nManualConverence;
	OMX_U32 nACProcWinStartX;
	OMX_U32 nACProcWinStartY;
	OMX_U32 nACProcWinWidth;
	OMX_U32 nACProcWinHeight;
	OMX_BOOL bACStatus;
} OMX_TI_CONFIG_CONVERGENCETYPE;

/**
 * Camera specific version.
 *
 * STRUCT MEMBERS:
 *  nBranch        : Branch
 *  nCommitID      : Commit ID
 *  nBuildDateTime : Build date and time
 *  nExtraInfo     : rederved for future use
 */
typedef struct OMX_TI_CAMERASPECVERSIONTYPE {
	OMX_U8 nBranch[64];
	OMX_U8 nCommitID[64];
	OMX_U8 nBuildDateTime[64];
	OMX_U8 nExtraInfo[64];
} OMX_TI_CAMERASPECVERSIONTYPE;

/**
 * Stereo frame layout enum
 */
typedef enum OMX_TI_STEREOFRAMELAYOUTTYPE {
	OMX_TI_StereoFrameLayout2D,
	OMX_TI_StereoFrameLayoutTopBottom,
	OMX_TI_StereoFrameLayoutLeftRight,
    OMX_TI_StereoFrameLayoutTopBottomSubsample,
    OMX_TI_StereoFrameLayoutLeftRightSubsample,
	OMX_TI_StereoFrameLayoutMax = 0x7FFFFFFF
} OMX_TI_STEREOFRAMELAYOUTTYPE;

/**
 * Camera frame layout type.
 *
 * STRUCT MEMBERS:
 *  eFrameLayout    : frame layout
 *  nSubsampleRatio : subsample ratio
 */
typedef struct OMX_TI_FRAMELAYOUTTYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_TI_STEREOFRAMELAYOUTTYPE eFrameLayout;
	OMX_U32 nSubsampleRatio; /**  Subsampling ratio, Q15.7 */
} OMX_TI_FRAMELAYOUTTYPE;

/**
 * The OMX_TI_COLOR_FORMATTYPE enumeration is used to define the
 * extended color format types.
 */
typedef enum OMX_TI_COLOR_FORMATTYPE {
	OMX_TI_COLOR_FormatRawBayer10bitStereo =
	    OMX_COLOR_FormatVendorStartUnused + 2, /**< 10 bit raw for stereo */
	OMX_TI_COLOR_FormatYUV420PackedSemiPlanar =
            (OMX_COLOR_FORMATTYPE) OMX_COLOR_FormatVendorStartUnused  + 0x100, /* 0x100 is used since it is the corresponding HAL pixel fromat */
    OMX_TI_ColorFormatTypeMax = 0x7fffffff
} OMX_TI_COLOR_FORMATTYPE;

// Motorola specific -begin

/**
 * Target exposure configuration of the exposure algorythm.
 *
 * STRUCT MEMBERS:
 *  nSize              : Size of the structure in bytes
 *  nVersion           : OMX specification version information
 *  nTargetExposure    : Value to be passed to the exposure algorythm
 *                       Range 0..255, default 128.
 *  nPortIndex         : Port that this structure applies to
 *  bUseTargetExposure : Flag to enable usage of the nTargetExposure value.
 *                       Set to true, to use the nTargetExposure value;
 *                       false means the algorythm shall use the
 *                       Target Exposure value from tuning data.
 */
typedef struct OMX_CONFIG_TARGETEXPOSURE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U8 nTargetExposure;
    OMX_BOOL bUseTargetExposure;
} OMX_CONFIG_TARGETEXPOSURE;

/**
 * Led Flash & Torch Intensity
 *
 *  STRUCT MEMBERS:
 *  nSize              : Size of the structure in bytes
 *  nVersion           : OMX specification version information
 *  nPortIndex         : Port that this structure applies to
 *  nLedFlashIntens    : Led Flash intensity
 *  nLedTorchIntens    : Led Torch intensity
 */
typedef struct OMX_CONFIG_LEDINTESITY {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32	nLedFlashIntens;
    OMX_U32	nLedTorchIntens;
} OMX_CONFIG_LEDINTESITY;
// Motorola specific - end

/**
 * The OMX_TI_EXIFTAGSTATUS enumeration is used to define the
 * tag status types.
 */
typedef enum OMX_TI_EXIFTAGSTATUS {
	OMX_TI_TagReadOnly,     /**< implies this tag is generated within omx-camera >*/
	OMX_TI_TagReadWrite,    /**< implies this tag can be overwritten by client >*/
	OMX_TI_TagUpdated,      /**< client has to use this to indicate the specific tag is overwritten >*/
	OMX_TI_ExifStatus_Max = 0x7fffffff
} OMX_TI_EXIFTAGSTATUS;

typedef struct OMX_TI_CONFIG_EXIF_TAGS {
	OMX_U32                 nSize;
	OMX_VERSIONTYPE         nVersion;
	OMX_U32                 nPortIndex;
	OMX_TI_EXIFTAGSTATUS    eStatusImageWidth;
	OMX_U32                 ulImageWidth;
	OMX_TI_EXIFTAGSTATUS    eStatusImageHeight;
	OMX_U32                 ulImageHeight;
	OMX_TI_EXIFTAGSTATUS    eStatusBitsPerSample;
	OMX_U16                 usBitsPerSample[3];
	OMX_TI_EXIFTAGSTATUS    eStatusCompression;
	OMX_U16                 usCompression;
	OMX_TI_EXIFTAGSTATUS    eStatusPhotometricInterpretation;
	OMX_U16                 usPhotometricInterpretation;
	OMX_TI_EXIFTAGSTATUS    eStatusOrientation;
	OMX_U16                 usOrientation;
	OMX_TI_EXIFTAGSTATUS    eStatusSamplesPerPixel;
	OMX_U16                 usSamplesPerPixel;
	OMX_TI_EXIFTAGSTATUS    eStatusPlanarConfiguration;
	OMX_U16                 usPlanarConfiguration;
	OMX_TI_EXIFTAGSTATUS    eStatusYCbCrSubSampling;
	OMX_U16                 usYCbCrSubSampling[2];
	OMX_TI_EXIFTAGSTATUS    eStatusYCbCrPositioning;
	OMX_U16                 usYCbCrPositioning;
	OMX_TI_EXIFTAGSTATUS    eStatusXResolution;
	OMX_U32                 ulXResolution[2];
	OMX_TI_EXIFTAGSTATUS    eStatusYResolution;
	OMX_U32                 ulYResolution[2];
	OMX_TI_EXIFTAGSTATUS    eStatusResolutionUnit;
	OMX_U16                 usResolutionUnit;

	OMX_TI_EXIFTAGSTATUS    eStatusRowsPerStrip;
	OMX_U32                 ulRowsPerStrip;
	OMX_TI_EXIFTAGSTATUS    eStatusDataSize;
	OMX_U32                 ulDataSize;

	OMX_TI_EXIFTAGSTATUS    eStatusTransferFunction;
	OMX_U16                 usTransferFunction[3*256];
	OMX_TI_EXIFTAGSTATUS    eStatusWhitePoint;
	OMX_U32                 ulWhitePoint[4]; //2x2
	OMX_TI_EXIFTAGSTATUS    eStatusPrimaryChromaticities;
	OMX_U32                 ulPrimaryChromaticities[12]; //2x6
	OMX_TI_EXIFTAGSTATUS    eStatusYCbCrCoefficients;
	OMX_U32                 ulYCbCrCoefficients[6]; //2x3
	OMX_TI_EXIFTAGSTATUS    eStatusReferenceBlackWhite;
	OMX_U32                 ulReferenceBlackWhite[12]; //2x6
	OMX_TI_EXIFTAGSTATUS    eStatusDateTime;
	OMX_S8*                 pDateTimeBuff;
	OMX_U32                 ulDateTimeBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusImageDescription;
	OMX_S8*                 pImageDescriptionBuff;
	OMX_U32                 ulImageDescriptionBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusMake;
	OMX_S8*                 pMakeBuff;
	OMX_U32                 ulMakeBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusModel;
	OMX_S8*                 pModelBuff;
	OMX_U32                 ulModelBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusSoftware;
	OMX_S8*                 pSoftwareBuff;
	OMX_U32                 ulSoftwareBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusArtist;
	OMX_S8*                 pArtistBuff;
	OMX_U32                 ulArtistBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusCopyright;
	OMX_S8*                 pCopyrightBuff;
	OMX_U32                 ulCopyrightBuffSizeBytes;

	OMX_TI_EXIFTAGSTATUS    eStatusExifVersion;
	OMX_S8                  cExifVersion[4];
	OMX_TI_EXIFTAGSTATUS    eStatusFlashpixVersion;
	OMX_S8                  cFlashpixVersion[4];
	OMX_TI_EXIFTAGSTATUS    eStatusColorSpace;
	OMX_U16                 usColorSpace;
	OMX_TI_EXIFTAGSTATUS    eStatusComponentsConfiguration;
	OMX_S8                  cComponentsConfiguration[4];
	OMX_TI_EXIFTAGSTATUS    eStatusCompressedBitsPerPixel;
	OMX_U32                 ulCompressedBitsPerPixel[2];
	OMX_TI_EXIFTAGSTATUS    eStatusPixelXDimension;
	OMX_U32                 ulPixelXDimension;
	OMX_TI_EXIFTAGSTATUS    eStatusPixelYDimension;
	OMX_U32                 ulPixelYDimension;
	OMX_TI_EXIFTAGSTATUS    eStatusMakerNote;
	OMX_S8*                 pMakerNoteBuff;
	OMX_U32                 ulMakerNoteBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusUserComment;
	OMX_S8*                 pUserCommentBuff;
	OMX_U32                 ulUserCommentBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusRelatedSoundFile;
	OMX_S8                  cRelatedSoundFile[13];
	OMX_TI_EXIFTAGSTATUS    eStatusDateTimeOriginal;
	OMX_S8*                 pDateTimeOriginalBuff;
	OMX_U32                 ulDateTimeOriginalBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusDateTimeDigitized;
	OMX_S8*                 pDateTimeDigitizedBuff;
	OMX_U32                 ulDateTimeDigitizedBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusSubSecTime;
	OMX_S8*                 pSubSecTimeBuff;
	OMX_U32                 ulSubSecTimeBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusSubSecTimeOriginal;
	OMX_S8*                 pSubSecTimeOriginalBuff;
	OMX_U32                 ulSubSecTimeOriginalBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusSubSecTimeDigitized;
	OMX_S8*                 pSubSecTimeDigitizedBuff;
	OMX_U32                 ulSubSecTimeDigitizedBuffSizeBytes;

	OMX_TI_EXIFTAGSTATUS    eStatusExposureTime;
	OMX_U32                 ulExposureTime[2];
	OMX_TI_EXIFTAGSTATUS    eStatusFNumber;
	OMX_U32                 ulFNumber[2];
	OMX_TI_EXIFTAGSTATUS    eStatusExposureProgram;
	OMX_U16                 usExposureProgram;
	OMX_TI_EXIFTAGSTATUS    eStatusSpectralSensitivity;
	OMX_S8*                 pSpectralSensitivityBuff;
	OMX_U32                 ulSpectralSensitivityBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusISOCount;
	OMX_U16                 usISOCount;
	OMX_TI_EXIFTAGSTATUS    eStatusISOSpeedRatings;
	OMX_U16*                pISOSpeedRatings;
	OMX_TI_EXIFTAGSTATUS    eStatusOECF;
	OMX_S8*                 pOECFBuff;
	OMX_U32                 ulOECFBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusShutterSpeedValue;
	OMX_S32                 slShutterSpeedValue[2];
	OMX_TI_EXIFTAGSTATUS    eStatusApertureValue;
	OMX_U32                 ulApertureValue[2];
	OMX_TI_EXIFTAGSTATUS    eStatusBrightnessValue;
	OMX_S32                 slBrightnessValue[2];
	OMX_TI_EXIFTAGSTATUS    eStatusExposureBiasValue;
	OMX_S32                 slExposureBiasValue[2];
	OMX_TI_EXIFTAGSTATUS    eStatusMaxApertureValue;
	OMX_U32                 ulMaxApertureValue[2];
	OMX_TI_EXIFTAGSTATUS    eStatusSubjectDistance;
	OMX_U32                 ulSubjectDistance[2];
	OMX_TI_EXIFTAGSTATUS    eStatusMeteringMode;
	OMX_U16                 usMeteringMode;
	OMX_TI_EXIFTAGSTATUS    eStatusLightSource;
	OMX_U16                 usLightSource;
	OMX_TI_EXIFTAGSTATUS    eStatusFlash;
	OMX_U16                 usFlash;
	OMX_TI_EXIFTAGSTATUS    eStatusFocalLength;
	OMX_U32                 ulFocalLength[2];
	OMX_TI_EXIFTAGSTATUS    eStatusSubjectArea;
	OMX_U16                 usSubjectArea[4];
	OMX_TI_EXIFTAGSTATUS    eStatusFlashEnergy;
	OMX_U32                 ulFlashEnergy[2];
	OMX_TI_EXIFTAGSTATUS    eStatusSpatialFrequencyResponse;
	OMX_S8*                 pSpatialFrequencyResponseBuff;
	OMX_U32                 ulSpatialFrequencyResponseBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusFocalPlaneXResolution;
	OMX_U32                 ulFocalPlaneXResolution[2];
	OMX_TI_EXIFTAGSTATUS    eStatusFocalPlaneYResolution;
	OMX_U32                 ulFocalPlaneYResolution[2];
	OMX_TI_EXIFTAGSTATUS    eStatusFocalPlaneResolutionUnit;
	OMX_U16                 usFocalPlaneResolutionUnit;
	OMX_TI_EXIFTAGSTATUS    eStatusSubjectLocation;
	OMX_U16                 usSubjectLocation[2];
	OMX_TI_EXIFTAGSTATUS    eStatusExposureIndex;
	OMX_U32                 ulExposureIndex[2];
	OMX_TI_EXIFTAGSTATUS    eStatusSensingMethod;
	OMX_U16                 usSensingMethod;
	OMX_TI_EXIFTAGSTATUS    eStatusFileSource;
	OMX_S8                  cFileSource;
	OMX_TI_EXIFTAGSTATUS    eStatusSceneType;
	OMX_S8                  cSceneType;
	OMX_TI_EXIFTAGSTATUS    eStatusCFAPattern;
	OMX_S8*                 pCFAPatternBuff;
	OMX_U32                 ulCFAPatternBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusCustomRendered;
	OMX_U16                 usCustomRendered;
	OMX_TI_EXIFTAGSTATUS    eStatusExposureMode;
	OMX_U16                 usExposureMode;
	OMX_TI_EXIFTAGSTATUS    eStatusWhiteBalance;
	OMX_U16                 usWhiteBalance;
	OMX_TI_EXIFTAGSTATUS    eStatusDigitalZoomRatio;
	OMX_U32                 ulDigitalZoomRatio[2];
	OMX_TI_EXIFTAGSTATUS    eStatusFocalLengthIn35mmFilm;
	OMX_U16                 usFocalLengthIn35mmFilm;
	OMX_TI_EXIFTAGSTATUS    eStatusSceneCaptureType;
	OMX_U16                 usSceneCaptureType;
	OMX_TI_EXIFTAGSTATUS    eStatusGainControl;
	OMX_U16                 usGainControl;
	OMX_TI_EXIFTAGSTATUS    eStatusContrast;
	OMX_U16                 usContrast;
	OMX_TI_EXIFTAGSTATUS    eStatusSaturation;
	OMX_U16                 usSaturation;
	OMX_TI_EXIFTAGSTATUS    eStatusSharpness;
	OMX_U16                 usSharpness;
	OMX_TI_EXIFTAGSTATUS    eStatusDeviceSettingDescription;
	OMX_S8*                 pDeviceSettingDescriptionBuff;
	OMX_U32                 ulDeviceSettingDescriptionBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusSubjectDistanceRange;
	OMX_U16                 usSubjectDistanceRange;

	OMX_TI_EXIFTAGSTATUS    eStatusImageUniqueID;
	OMX_S8                  cImageUniqueID[33];
	OMX_U8*                 pPrivateNextIFDPointer;    //Should not be used by the application
	OMX_U8*                 pPrivateThumbnailSize;     //Should not be used by the application
	OMX_U8*                 pPrivateTiffHeaderPointer; //Should not be used by the application

	OMX_TI_EXIFTAGSTATUS    eStatusGpsVersionId;
	OMX_U8                  ucGpsVersionId[4];
	OMX_TI_EXIFTAGSTATUS    eStatusGpslatitudeRef;
	OMX_S8                  cGpslatitudeRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsLatitude;
	OMX_U32                 ulGpsLatitude[6];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsLongitudeRef;
	OMX_S8                  cGpsLongitudeRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsLongitude;
	OMX_U32                 ulGpsLongitude[6];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsAltitudeRef;
	OMX_U8                  ucGpsAltitudeRef;
	OMX_TI_EXIFTAGSTATUS    eStatusGpsAltitude;
	OMX_U32                 ulGpsAltitude[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsTimeStamp;
	OMX_U32                 ulGpsTimeStamp[6];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsSatellites;
	OMX_S8*                 pGpsSatellitesBuff;
	OMX_U32                 ulGpsSatellitesBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusGpsStatus;
	OMX_S8                  cGpsStatus[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsMeasureMode;
	OMX_S8                  cGpsMeasureMode[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDop;
	OMX_U32                 ulGpsDop[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsSpeedRef;
	OMX_S8                  cGpsSpeedRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsSpeed;
	OMX_U32                 ulGpsSpeed[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsTrackRef;
	OMX_S8                  cGpsTrackRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsTrack;
	OMX_U32                 ulGpsTrack[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsImgDirectionRef;
	OMX_S8                  cGpsImgDirectionRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsImgDirection;
	OMX_U32                 ulGpsImgDirection[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsMapDatum;
	OMX_S8*                 pGpsMapDatumBuff;
	OMX_U32                 ulGpsMapDatumBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestLatitudeRef;
	OMX_S8                  cGpsDestLatitudeRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestLatitude;
	OMX_U32                 ulGpsDestLatitude[6];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestLongitudeRef;
	OMX_S8                  cGpsDestLongitudeRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestLongitude;
	OMX_U32                 ulGpsDestLongitude[6];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestBearingRef;
	OMX_S8                  cGpsDestBearingRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestBearing;
	OMX_U32                 ulGpsDestBearing[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestDistanceRef;
	OMX_S8                  cGpsDestDistanceRef[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDestDistance;
	OMX_U32                 ulGpsDestDistance[2];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsProcessingMethod;
	OMX_S8*                 pGpsProcessingMethodBuff;
	OMX_U32                 ulGpsProcessingMethodBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusGpsAreaInformation;
	OMX_S8*                 pGpsAreaInformationBuff;
	OMX_U32                 ulGpsAreaInformationBuffSizeBytes;
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDateStamp;
	OMX_S8                  cGpsDateStamp[11];
	OMX_TI_EXIFTAGSTATUS    eStatusGpsDifferential;
	OMX_U16                 usGpsDifferential;
} OMX_TI_CONFIG_EXIF_TAGS;

/**
 * The OMX_TI_SENFACING_TYPE enumeration is used to define the
 * sensor facing.
 */
typedef enum OMX_TI_SENFACING_TYPE {
    OMX_TI_SENFACING_FRONT,
    OMX_TI_SENFACING_BACK,
    OMX_TI_SENFACING_MAX = 0x7FFFFFFF
}OMX_TI_SENFACING_TYPE;

/**
 * Structure used to configure current OMX_TI_SENMOUNT_TYPE
 *
 * @param nSenId
 * @param nRotation
 * @param bMirror
 * @param bFlip
 * @param eFacing
 */
typedef struct OMX_TI_SENMOUNT_TYPE {
    OMX_U32             nSenId;
    OMX_U32             nRotation;
    OMX_BOOL                bMirror;
    OMX_BOOL                bFlip;
    OMX_TI_SENFACING_TYPE   eFacing;
}OMX_TI_SENMOUNT_TYPE;

/**
 * Structure used to configure current OMX_TI_VARFPSTYPE
 *
 * @param nVarFPSMin    Number of the smallest FPS supported.
 * @param nVarFPSMax    Number of the biggest FPS supported.
 */
typedef struct OMX_TI_VARFPSTYPE {
    OMX_U32                 nVarFPSMin;
    OMX_U32                 nVarFPSMax;
} OMX_TI_VARFPSTYPE;

/**
 * Structure used to configure current OMX_TI_CONFIG_SHAREDBUFFER
 *
 * STRUCT MEMBERS:
 * nSize            : Size of the structure in bytes
 * nVersion         : OMX specification version information
 * nPortIndex       : Port that this structure applies to
 * nSharedBuffSize  : Size of the pSharedBuff in bytes
 * pSharedBuff      : Pointer to a buffer
 */
typedef struct OMX_TI_CONFIG_SHAREDBUFFER {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_U32 nSharedBuffSize;
	OMX_U8* pSharedBuff;
} OMX_TI_CONFIG_SHAREDBUFFER;

/**
 * Structure used to configure current OMX_TI_CAPRESTYPE
 *
 * STRUCT MEMBERS:
 * nSize            : Size of the structure in bytes
 * nVersion         : OMX specification version information
 * nPortIndex       : Port that this structure applies to
 * nWidthMin        : Number of the smallest width supported
 * nHeightMin       : Number of the smallest height supported
 * nWidthMax        : Number of the biggest width supported
 * nHeightMax       : Number of the biggest height supported
 * nMaxResInPixels  : Max resolution in pixels. Used for description of 3d resolutions.
 */
typedef struct OMX_TI_CAPRESTYPE {
    OMX_U32         nSize;          //- OMX struct header not required as this struct wont be queried on its own?
	OMX_VERSIONTYPE nVersion;
	OMX_U32         nPortIndex;
	OMX_U32         nWidthMin;  // smallest width supported
	OMX_U32         nHeightMin; // smallest height supported
	OMX_U32         nWidthMax;  // biggest width supported
	OMX_U32         nHeightMax; // biggest height supported
	OMX_U32         nMaxResInPixels;// max resolution in pixels
} OMX_TI_CAPRESTYPE;

/**
 * Structure used to configure current OMX_TI_CAPTYPE
 *
 * STRUCT MEMBERS:
 * nSize                                : Size of the structure in bytes
 * nVersion                             : OMX specification version information
 * nPortIndex                           : Port that this structure applies to
 * ulPreviewFormatCount                 : Number of the supported preview pixelformat count
 * ePreviewFormats                      : Array containing the supported preview pixelformat count
 * ulImageFormatCount                   : Number of the supported image pixelformat count
 * eImageFormats                        : Array containing the supported image pixelformat count
 * tPreviewResRange                     : Supported preview resolution range
 * tRotatedPreviewResRange              : Supported rotated preview resolution range
 * tImageResRange                       : Supported image resolution range
 * tThumbResRange                       : Supported thumbnail resolution range
 * ulWhiteBalanceCount                  : Supported whitebalance mode count
 * eWhiteBalanceModes                   : Array containing the whitebalance modes
 * ulColorEffectCount                   : Supported effects count
 * eColorEffects                        : Array containing the supported effects
 * xMaxWidthZoom                        : Fixed point value stored as Q16 representing the maximum value for the Zoom allowed on Width
 * xMaxHeightZoom                       : Fixed point value stored as Q16 representing the maximum value for the Zoom allowed on Height
 * ulFlickerCount                       : Number of the supported anti-flicker modes
 * eFlicker                             : Array containing the supported anti-flicker modes
 * ulExposureModeCount                  : Number of the supported exposure modes
 * eExposureModes                       : Array containing the supported exposure modes
 * bLensDistortionCorrectionSupported   : Flag for Lens Distortion Correction Algorithm support
 * bISONoiseFilterSupported             : Flag for Noise Filter Algorithm support
 * xEVCompensationMin                   : Fixed point value stored as Q16 representing the EVCompensation minumum allowed value
 * xEVCompensationMax                   : Fixed point value stored as Q16 representing the EVCompensation maximum allowed value
 * nSensitivityMax                      : nSensitivityMax = 100 implies maximum supported equal to "ISO 100"
 * ulFocusModeCount                     : Number of the supported focus modes
 * eFocusModes                          : Array containing the supported focus modes
 * ulSceneCount                         : Number of the supported scenes
 * eSceneModes                          : Array containing the supported scenes
 * ulFlashCount                         : Number of the supported flash modes
 * eFlashModes                          : Array containing the supported flash modes
 * xFramerateMin                        : Fixed point value stored as Q16 representing the minimum framerate allowed
 * xFramerateMax                        : Fixed point value stored as Q16 representing the maximum framerate allowed
 * bContrastSupported                   : Flag showing if the contrast is supported
 * bSaturationSupported                 : Flag showing if the saturation is supported
 * bBrightnessSupported                 : Flag showing if the brightness is supported
 * bProcessingLevelSupported            : Flag showing if the processing level is supported
 * bQFactorSupported                    : Flag showing if the QFactor is supported
 * ulPrvVarFPSModesCount                : Number of preview FPS modes
 * tPrvVarFPSModes                      : Preview FPS modes
 * ulCapVarFPSModesCount                : Number of capture FPS modes
 * tCapVarFPSModes                      : Capture FPS modes
 * tSenMounting                         : Sensor mount information
 * ulAutoConvModesCount                 : Supported auto convergence modes count
 * eAutoConvModes                       : Array containing the auto convergence modes
 * ulBracketingModesCount               : Supported bracketing modes count
 * eBracketingModes                     : Array containing the bracketing modes
 * bGbceSupported                       : Flag showing if the Gbce is supported
 * bRawJpegSupported                    : Flag showing if the Raw + Jpeg is supported
 * ulImageCodingFormatCount             : Supported image coding formats count
 * eImageCodingFormat                   : Array containing the image coding formats
 * uSenNativeResWidth                   : Sensor native resolution width
 * uSenNativeResHeight                  : Sensor native resolution height
 * ulAlgoAreasFocusCount                : Supported number of AlgoAreas for focus areas
 * ulAlgoAreasExposureCount             : Supported number of AlgoAreas for exposure areas
 * bAELockSupported                     : Flag showing if the AE Lock is supported
 * bAWBLockSupported                    : Flag showing if the AWB Lock is supported
 * bAFLockSupported                     : Flag showing if the Af Lock is supported
 * nFocalLength                         : Focal length defined in terms of 0.01mm
 * ulPrvFrameLayoutCount                : supported frame layout count for preview
 * ePrvFrameLayout                      : Array containing the frame layouts for preview
 * ulCapFrameLayoutCount                : supported frame layout count for capture
 * eCapFrameLayout                      : Array containing the frame layouts for capture
 * bVideoNoiseFilterSupported           : Flag showing if the video noise filter is supported
 * bVideoStabilizationSupported         : Flag showing if the video stabilization is supported
 * bStillCapDuringVideoSupported        : Flag showing if the still capture is supported during video
 * bMechanicalMisalignmentSupported     : Flag showing if the mechanical misalignment is supported
 * bFacePrioritySupported               : Flag showing if the face priority is supported
 * bRegionPrioritySupported             : Flag showing if the region priority is supported
 * bGlbceSupported                      : Flag showing if the GLBCE is supported
 * nManualConvMin                       : Manual convergence min value
 * nManualConvMax                       : Manual convergence max value
 * nManualExpMin                        : Manual exposure time min value
 * nManualExpMax                        : Manual exposure time max value
 * nBrightnessMin                       : Brightness min value
 * nBrightnessMax                       : Brightness max value
 * nContrastMin                         : Contrast min value
 * nContrastMax                         : Contrast max value
 * nSharpnessMin                        : Sharpness min value
 * nSharpnessMax                        : Sharpness max value
 * nSaturationMin                       : Saturation min value
 * nSaturationMax                       : Saturation max value
 */
typedef struct OMX_TI_CAPTYPE {
	OMX_U32                         nSize;
	OMX_VERSIONTYPE                 nVersion;
	OMX_U32                         nPortIndex;
	OMX_U16                         ulPreviewFormatCount;   // supported preview pixelformat count
	OMX_COLOR_FORMATTYPE            ePreviewFormats[32];
	OMX_U16                         ulImageFormatCount;     // supported image pixelformat count
	OMX_COLOR_FORMATTYPE            eImageFormats[32];
	OMX_TI_CAPRESTYPE               tPreviewResRange;       // supported preview resolution range
	OMX_TI_CAPRESTYPE               tRotatedPreviewResRange;     // supported rotated preview resolution range
	OMX_TI_CAPRESTYPE               tImageResRange;         // supported image resolution range
	OMX_TI_CAPRESTYPE               tThumbResRange;         // supported thumbnail resolution range
	OMX_U16                         ulWhiteBalanceCount;    // supported whitebalance mode count
	OMX_WHITEBALCONTROLTYPE         eWhiteBalanceModes[32];
	OMX_U16                         ulColorEffectCount;     // supported effects count
	OMX_IMAGEFILTERTYPE             eColorEffects[32];
	OMX_S32                         xMaxWidthZoom;          // Fixed point value stored as Q16
	OMX_S32                         xMaxHeightZoom;         // Fixed point value stored as Q16
	OMX_U16                         ulFlickerCount;         // supported anti-flicker mode count
	OMX_COMMONFLICKERCANCELTYPE     eFlicker[32];
	OMX_U16                         ulExposureModeCount;    // supported exposure mode count
	OMX_EXPOSURECONTROLTYPE         eExposureModes[32];
	OMX_BOOL                        bLensDistortionCorrectionSupported;
	OMX_BOOL                        bISONoiseFilterSupported;
	OMX_S32                         xEVCompensationMin;     // Fixed point value stored as Q16
	OMX_S32                         xEVCompensationMax;     // Fixed point value stored as Q16
	OMX_U32                         nSensitivityMax;        // nSensitivityMax = 100 implies maximum supported equal to "ISO 100"
	OMX_U16                         ulFocusModeCount;       // supported focus mode count
	OMX_IMAGE_FOCUSCONTROLTYPE      eFocusModes[32];
	OMX_U16                         ulSceneCount;           // supported scene count
	OMX_SCENEMODETYPE               eSceneModes[64];
	OMX_U16                         ulFlashCount;           // supported flash modes count
	OMX_IMAGE_FLASHCONTROLTYPE      eFlashModes[32];
	OMX_U32                         xFramerateMin;          // Fixed point value stored as Q16
	OMX_U32                         xFramerateMax;          // Fixed point value stored as Q16
	OMX_BOOL                        bContrastSupported;
	OMX_BOOL                        bSaturationSupported;
	OMX_BOOL                        bBrightnessSupported;
	OMX_BOOL                        bProcessingLevelSupported;
	OMX_BOOL                        bQFactorSupported;
	OMX_U16                         ulPrvVarFPSModesCount;  // supported variable FPS preview modes count
	OMX_TI_VARFPSTYPE               tPrvVarFPSModes[10];
	OMX_U16                         ulCapVarFPSModesCount;  // supported variable FPS capture modes count
	OMX_TI_VARFPSTYPE               tCapVarFPSModes[10];
	OMX_TI_SENMOUNT_TYPE            tSenMounting;
	OMX_U16                         ulAutoConvModesCount;   // supported auto convergence modes count
	OMX_TI_AUTOCONVERGENCEMODETYPE  eAutoConvModes[32];
	OMX_U16                         ulBracketingModesCount; // supported bracketing modes count
	OMX_BRACKETMODETYPE             eBracketingModes[32];
	OMX_BOOL                        bGbceSupported;         // Flag showing if the Gbce is supported
	OMX_BOOL                        bRawJpegSupported;      // Flag showing if the Raw + Jpeg issupported
	OMX_U16                         ulImageCodingFormatCount;
	OMX_IMAGE_CODINGTYPE            eImageCodingFormat[32];
	OMX_U16                         uSenNativeResWidth;
	OMX_U16                         uSenNativeResHeight;
        OMX_U16                        ulAlgoAreasFocusCount;
        OMX_U16                        ulAlgoAreasExposureCount;
    OMX_BOOL                       bAELockSupported;
    OMX_BOOL                       bAWBLockSupported;
    OMX_BOOL                       bAFLockSupported;
    OMX_U16                        nFocalLength;
    OMX_U16                        ulPrvFrameLayoutCount;       // supported frame layout count
    OMX_TI_STEREOFRAMELAYOUTTYPE   ePrvFrameLayout[16];
    OMX_U16                        ulCapFrameLayoutCount;       // supported frame layout count
    OMX_TI_STEREOFRAMELAYOUTTYPE   eCapFrameLayout[16];
    OMX_BOOL                       bVideoNoiseFilterSupported;
    OMX_BOOL                       bVideoStabilizationSupported;
    OMX_BOOL                       bStillCapDuringVideoSupported;
    OMX_BOOL                       bMechanicalMisalignmentSupported;
    OMX_BOOL                       bFacePrioritySupported;
    OMX_BOOL                       bRegionPrioritySupported;
    OMX_BOOL                       bGlbceSupported;
    OMX_S16                        nManualConvMin;
    OMX_S16                        nManualConvMax;
    OMX_U16                        nManualExpMin;
    OMX_U16                        nManualExpMax;
    OMX_S16                        nBrightnessMin;
    OMX_S16                        nBrightnessMax;
    OMX_S16                        nContrastMin;
    OMX_S16                        nContrastMax;
    OMX_S16                        nSharpnessMin;
    OMX_S16                        nSharpnessMax;
    OMX_S16                        nSaturationMin;
    OMX_S16                        nSaturationMax;
} OMX_TI_CAPTYPE;



/**
 * Defines 3A Face priority mode.
 *
 * STRUCT MEMBERS:
 *  nSize               : Size of the structure in bytes
 *  nVersion            : OMX specification version information
 *  nPortIndex          : Port that this structure applies to
 *  bAwbFaceEnable      : Enable Face priority for Auto White Balance
 *  bAeFaceEnable       : Enable Face priority for Auto Exposure
 *  bAfFaceEnable       : Enable Face priority for Auto Focus
 */
typedef struct OMX_TI_CONFIG_3A_FACE_PRIORITY {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_BOOL bAwbFaceEnable;
	OMX_BOOL bAeFaceEnable;
	OMX_BOOL bAfFaceEnable;
} OMX_TI_CONFIG_3A_FACE_PRIORITY;

/**
 * Defines 3A Region priority mode.
 *
 * STRUCT MEMBERS:
 *  nSize               : Size of the structure in bytes
 *  nVersion            : OMX specification version information
 *  nPortIndex          : Port that this structure applies to
 *  bAwbFaceEnable      : Enable Region priority for Auto White Balance
 *  bAeFaceEnable       : Enable Region priority for Auto Exposure
 *  bAfFaceEnable       : Enable Region priority for Auto Focus
 */
typedef struct OMX_TI_CONFIG_3A_REGION_PRIORITY {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_BOOL bAwbRegionEnable;
	OMX_BOOL bAeRegionEnable;
	OMX_BOOL bAfRegionEnable;
} OMX_TI_CONFIG_3A_REGION_PRIORITY;

/*
* STRUCT MEMBERS:
* nSize         : Size of the structure in bytes
* nVersion      : OMX specification version information
* nPortIndex    : Port that this structure applies to
* bAutoConvergence : Enable/Disable Auto Convergence
*/
typedef struct OMX_TI_PARAM_AUTOCONVERGENCETYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_BOOL bAutoConvergence;
} OMX_TI_PARAM_AUTOCONVERGENCETYPE;

/**
 * Focus distance configuration
 *
 *  STRUCT MEMBERS:
 *  nSize: Size of the structure in bytes
 *  nVersion: OMX specification version information
 *  nPortIndex: Port that this structure applies to
 *  nFocusDistanceNear : Specifies the near focus distance in mm ( 0 equals infinity )
 *  nFocusDistanceOptimal : Specifies the optimal focus distance in mm ( 0 equals infinity )
 *  nFocusDistanceFar : Specifies the far focus distance in mm ( 0 equals infinity )
 *  nLensPosition : Specifies the current lens position in driver units
 */
typedef struct OMX_TI_CONFIG_FOCUSDISTANCETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nFocusDistanceNear;
    OMX_U32 nFocusDistanceOptimal;
    OMX_U32 nFocusDistanceFar;
    OMX_S32 nLensPosition;
} OMX_TI_CONFIG_FOCUSDISTANCETYPE;

/**
 * The OMX_TI_BRIGHTNESSCONTRASTCRTLTYPE enumeration is used to define the
 * brightness and contrast mode types.
 */
typedef enum OMX_TI_BRIGHTNESSCONTRASTCRTLTYPE {
	OMX_TI_BceModeOff = 0,
	OMX_TI_BceModeOn,
	OMX_TI_BceModeAuto,
	OMX_TI_BceModeMax = 0x7FFFFFFF
} OMX_TI_BRIGHTNESSCONTRASTCRTLTYPE;

/**
 * Local and global brightness contrast type.
 *
 * STRUCT MEMBERS:
 *  nSize             : Size of the structure in bytes
 *  nVersion          : OMX specification version information
 *  nPortIndex        : Port that this structure applies to
 *  eControl          : Control field for GLBCE
 */
typedef struct OMX_TI_CONFIG_LOCAL_AND_GLOBAL_BRIGHTNESSCONTRASTTYPE {
	OMX_U32 nSize;
	OMX_VERSIONTYPE nVersion;
	OMX_U32 nPortIndex;
	OMX_TI_BRIGHTNESSCONTRASTCRTLTYPE eControl;
} OMX_TI_CONFIG_LOCAL_AND_GLOBAL_BRIGHTNESSCONTRASTTYPE;

/**
 * Uncompressed image operating mode configuration structure.
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param xMin          The minimum frame rate allowed.
 *                      Units are Q16 frames per second.
 * @param xMax          The maximum frame rate allowed.
 *                      Units are Q16 frames per second.
 */

typedef struct OMX_TI_CONFIG_VARFRMRANGETYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 xMin;
    OMX_U32 xMax;
} OMX_TI_CONFIG_VARFRMRANGETYPE;

/**
 * Single preview capture modes
 */
    typedef enum OMX_TI_SINGLEPREVIEWMODETYPE {
        OMX_TI_SinglePreviewMode_PreviewOnly,
        OMX_TI_SinglePreviewMode_Video,
        OMX_TI_SinglePreviewMode_ImageCapture,
        OMX_TI_SinglePreviewMode_ImageCaptureHighSpeed,
        OMX_TI_SinglePreviewMode_Reprocess,
        OMX_TI_SinglePreviewMode = 0x7FFFFFFF
    } OMX_TI_SINGLEPREVIEWMODETYPE;

/**
 * Define configuration structure for
 * single preview capture mode
 *
 * STRUCT MEMBERS:
 *  nSize               : Size of the structure in bytes
 *  nVersion            : OMX specification version information
 *  eMode               : Select the subusecase mode (Video/HQ/HS)
 */
    typedef struct OMX_TI_CONFIG_SINGLEPREVIEWMODETYPE {
        OMX_U32                      nSize;
        OMX_VERSIONTYPE              nVersion;
        OMX_TI_SINGLEPREVIEWMODETYPE eMode;
    } OMX_TI_CONFIG_SINGLEPREVIEWMODETYPE;


/**
 * Configuratin structure for freeze AWB parameter modifications.
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param nPortIndex    Port index to which to apply.
 * @param nTimeDelay    Time for which the AWB parameters to be frozen.
 *                                             measured in milliseconds
 */
    typedef struct OMX_TI_CONFIG_FREEZE_AWB {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nPortIndex;
        OMX_U32         nTimeDelay;
    } OMX_TI_CONFIG_FREEZE_AWB;

/**
 * Configuration structure used to set
 * minimum time between two sequential WB coefficients modifications.
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param nPortIndex    Port index to which to apply.
 * @param nDelayTime    The time in milliseconds.
 */
    typedef struct OMX_TI_CONFIG_AWB_DELAY {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nPortIndex;
        OMX_U32         nDelayTime;
    } OMX_TI_CONFIG_AWB_DELAY;

/**
 * Configuration structure used to set
 * minimum time delay between
 * two sequential AE parameters modifications
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param nPortIndex    Port index to which to apply.
 * @param nDelayTime    The time in milliseconds.
 */
    typedef struct OMX_TI_CONFIG_AE_DELAY {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nPortIndex;
        OMX_U32         nDelayTime;
    } OMX_TI_CONFIG_AE_DELAY;


/**
 * Configuration structure used to freeze AE modifications
 * for a nTimeDelay milliseconds
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param nPortIndex    Port index to which to apply.
 * @param nTimeDelay    The time in milliseconds.
 */
    typedef struct OMX_TI_CONFIG_FREEZE_AE {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nPortIndex;
        OMX_U32         nTimeDelay;
    } OMX_TI_CONFIG_FREEZE_AE;

/**
 * Configuration structure used to set
 * the AE gain threshold
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param uMinTH        Minimum value for AE gain.
 * @param uMaxTH        Maximum value for AE gain.
 */
    typedef struct OMX_TI_CONFIG_AE_THRESHOLD {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nPortIndex;
        OMX_U32         uMinTH;
        OMX_U32         uMaxTH;
    } OMX_TI_CONFIG_AE_THRESHOLD;

/**
 * Enumeration describing the main gestures
 */
    typedef enum OMX_TI_GESTURES_TYPE {
        OMX_TI_GESTURE_NO_GESTURE = 0x70000001,
        OMX_TI_GESTURE_SWIPE_RIGHT,
        OMX_TI_GESTURE_SWIPE_LEFT,
        OMX_TI_GESTURE_FIST_RIGHT,
        OMX_TI_GESTURE_FIST_LEFT,

        OMX_TI_GESTURE_COUNT,
        OMX_TI_GESTURE_MAX = 0x7FFFFFFF
    } OMX_TI_GESTURES_TYPE;

/**
 * Enumeration describing the main gesture objects
 */
    typedef enum OMX_TI_OBJECT_TYPE {
        OMX_TI_OBJECT_PALM,
        OMX_TI_OBJECT_FIST,
        OMX_TI_OBJECT_FACE,

        OMX_TI_OBJECT_MAX = 0x7FFFFFFF
    } OMX_TI_OBJECT_TYPE;

/**
 * Data structure carrying information about
 * objects located at a certain area of frame buffer.
 *
 * @param nSize         Size of the structure in bytes.
 * @param nVersion      OMX specification version information.
 * @param nPortIndex    Port index to which to apply.
 * @param eType         The object type.
 * @param nTop          The top coordinate.
 * @param nLeft         The left coordinate.
 * @param nWidth        The width of the object.
 * @param nHeight       The height of the object.
 */
    typedef struct OMX_CONFIG_OBJECT_RECT_TYPE {
        OMX_U32            nSize;
        OMX_VERSIONTYPE    nVersion;
        OMX_U32            nPortIndex;
        OMX_TI_OBJECT_TYPE eType;
        OMX_S32            nTop;
        OMX_S32            nLeft;
        OMX_U32            nWidth;
        OMX_U32            nHeight;
    } OMX_CONFIG_OBJECT_RECT_TYPE;

/**
 * Data structure carrying information about
 * gestures detected at a certain frame.
 *
 * @param nSize                 Size of the structure in bytes.
 * @param nVersion              OMX specification version information.
 * @param nPortIndex            Port index to which to apply.
 * @param nTimeStamp            Frame id.
 * @param eType                 Type of the gesture detected at that frame.
 * @param nNumDetectedGestures  Number ot the areas of the frame in which this gesture is detected.
 * @param nGestureAreas         The areas where this gesture is detected.
 */
    typedef struct OMX_TI_CONFIG_GESTURES_INFO {
        OMX_U32                     nSize;
        OMX_VERSIONTYPE             nVersion;
        OMX_U32                     nPortIndex;
        OMX_TICKS                   nTimeStamp;
        OMX_TI_GESTURES_TYPE        eType;
        OMX_U32                     nNumDetectedGestures;
        OMX_CONFIG_OBJECT_RECT_TYPE nGestureAreas[35];
    } OMX_TI_CONFIG_GESTURES_INFO;

/**
* Define the frames queue len for ZSL
*
* STRUCT MEMBERS:
* nSize: Size of the structure in bytes
* nVersion: OMX specification version information
* nHistoryLen: History len in number of frames
*/
    typedef struct OMX_TI_PARAM_ZSLHISTORYLENTYPE {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nHistoryLen;
    } OMX_TI_PARAM_ZSLHISTORYLENTYPE;

/**
* Define the frame delay in ms for ZSL
*
* STRUCT MEMBERS:
* nSize: Size of the structure in bytes
* nVersion: OMX specification version information
* nDelay: Capture frame delay in ms
*/
    typedef struct OMX_TI_CONFIG_ZSLDELAYTYPE {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_S32         nDelay;
    } OMX_TI_CONFIG_ZSLDELAYTYPE;

/**
* AlogAreas purpose
* This type specifies the purpose of areas specified in OMX_ALGOAREASTYPE.
* */
    typedef enum OMX_ALGOAREAPURPOSE{
        OMX_AlgoAreaFocus = 0, // Multi region focus
        OMX_AlgoAreaExposure,
    }OMX_ALGOAREAPURPOSE;

    typedef  struct OMX_ALGOAREA {
        OMX_S32 nLeft;                      /**< The leftmost coordinate of the area rectangle */
        OMX_S32 nTop;                       /**< The topmost coordinate of the area rectangle */
        OMX_U32 nWidth;                     /**< The width of the area rectangle in pixels */
        OMX_U32 nHeight;                    /**< The height of the area rectangle in pixels */
        OMX_U32 nPriority;                  /**< Priority - ranges from 1 to 1000 */
    }OMX_ALGOAREA;

/**
* Algorythm areas type
* This type defines areas for Multi Region Focus,
* or another algorithm region parameters,
* such as Multi Region Auto Exposure.
*
* STRUCT MEMBERS:
*  nSize            : Size of the structure in bytes
*  nVersion         : OMX specification version information
*  nPortIndex       : Port index
*  tAreaPosition    : Area definition - coordinates and purpose - Multi Region Focus, Auto Exposure, etc.
*  nNumAreas        : Number of areas defined in the array
*  nAlgoAreaPurpose : Algo area purpose - eg. Multi Region Focus is OMX_AlgoAreaFocus
*/
    typedef  struct OMX_ALGOAREASTYPE {
        OMX_U32 nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32 nPortIndex;

        OMX_U32 nNumAreas;
        OMX_ALGOAREA tAlgoAreas[MAX_ALGOAREAS];
        OMX_ALGOAREAPURPOSE nAlgoAreaPurpose;
    } OMX_ALGOAREASTYPE;

/*==========================================================================*/
/*!
@brief OMX_TI_PARAM_ENHANCEDPORTRECONFIG : Suport added to new port reconfig usage
@param bUsePortReconfigForCrop       Enables port reconfig for crop.
@param bUsePortReconfigForPadding    Enables port reconfig for padding
*/
/*==========================================================================*/

typedef struct OMX_TI_PARAM_ENHANCEDPORTRECONFIG {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bUsePortReconfigForCrop;
    OMX_BOOL bUsePortReconfigForPadding;
} OMX_TI_PARAM_ENHANCEDPORTRECONFIG;

/**
* OMX_TI_H3aPAXELCFG: AF/AEWB paxel description structure
* @param nVPos: vertical start point of paxel grid w.r.t first pixel of input image frame
* @param nVSize: vertical paxel size
* @param nHPos: horizontal start point of paxel grid w.r.t first pixel of input image frame
* @param nHSize: horizontal paxel size
* @param nVCount: num of vert paxels. AF/AEWB paxels are always adjacent to each other
* @param nVIncr: num of pixels to skip within a paxel, vertically
* @param nHCount: num of horz paxels. AF/AEWB paxels are always adjacent to each other
* @param nHIncr: num of pixels to skip within a paxel, horizontally
**/
typedef struct {
    OMX_U16 nVPos;
    OMX_U8  nVSize;
    OMX_U16 nHPos;
    OMX_U8  nHSize;
    OMX_U8  nVCount;
    OMX_U8  nVIncr;
    OMX_U8  nHCount;
    OMX_U8  nHIncr;
} OMX_TI_H3aPAXELCFG;

/**
* OMX_TI_AF_COLOR_PAXEL_STATS: struct af_h3a_color_paxel
* @param nSum                 : Sum of the pixels used to arrive at the statistics for a paxel
* @param nFVSum             : Focus Value (sum/peak) for a paxel
* @param nFVSquaredSum : Focus Value Squared (sum/peak) for a paxel
* @param nReserved           : To be ignored
**/
typedef struct OMX_TI_AF_COLOR_PAXEL_STATS {
    OMX_U32 nSum;
    OMX_U32 nFVSum;
    OMX_U32 nFVSquaredSum;
    OMX_U32 nReserved;
} OMX_TI_AF_COLOR_PAXEL_STATS;

/**
* OMX_TI_AFPAXELDATA
* @param tGPaxel     : Paxel information for green color
* @param tRBPaxel   : Paxel information for red/blue color
* @param tBRPaxel   : Paxel information for blue/red color
**/
typedef struct {
    OMX_TI_AF_COLOR_PAXEL_STATS tGPaxel;
    OMX_TI_AF_COLOR_PAXEL_STATS tRBPaxel;
    OMX_TI_AF_COLOR_PAXEL_STATS tBRPaxel;
} OMX_TI_AFPAXELDATA;

/**
* OMX_TI_AF_RGB_POS_TYPE
* Defines the RGB bayer pattern assumed while extracting AF statistics individually for R,G,B colour channels
**/
typedef enum OMX_TI_AF_RGB_POS_TYPE {
    OMX_TI_AF_RGBPOSITION_BAYER_GR_GB = 0,
    OMX_TI_AF_RGBPOSITION_BAYER_RG_GB = 1,
    OMX_TI_AF_RGBPOSITION_BAYER_GR_BG = 2,
    OMX_TI_AF_RGBPOSITION_BAYER_RG_BG = 3,
    OMX_TI_AF_RGBPOSITION_CUSTOM_GG_RB = 4,
    OMX_TI_AF_RGBPOSITION_CUSTOM_RB_GG = 5,
    OMX_TI_AF_RGBPOSITION_BAYER_FORMAT = 0x7FFFFFFF
} OMX_TI_AF_RGB_POS_TYPE;

/* Max size of AF buffer output by ISP H3A engine */
#define OMX_TI_AF_MAX_NUM_PAXELS (127 * 35)

/**
 * The extra data for AutoFocus data
 * @param eAFBayerRgbPosition : When Vertical focus is disabled, R,G,B location w.r.t. to paxel start location is specified by this field.
 * @param bAFPeakModeEnable : If enabled, peak for FV, FV^2 is computed for a paxel. If disabled, average of FV, FV^2 is computed for a paxel
 * @param bAFVerticalFocusEnable : Whether vertical focus is enabled.
 * @param tAFPaxelWindow : AF paxel description
 * @param ptAFPaxelStatistics : Output AF buffer
 */
typedef struct OMX_TI_AF_STATISTICS_TYPE {
    OMX_TI_AF_RGB_POS_TYPE eAFBayerRgbPosition;
    OMX_BOOL               bAFPeakModeEnable;
    OMX_BOOL               bAFVerticalFocusEnable;
    OMX_TI_H3aPAXELCFG     tAFPaxelWindow;
    OMX_TI_AFPAXELDATA     ptAFPaxelStatistics[OMX_TI_AF_MAX_NUM_PAXELS];
} OMX_TI_AF_STATISTICS_TYPE;


typedef struct {
    /** Average value for red pixels in current paxel */
    OMX_U16 red;
    /** Average value for green pixels in current paxel */
    OMX_U16 green;
    /** Average value for blue pixels in current paxel */
    OMX_U16 blue;
    /** Flag indicating whether current paxel is valid 0:invalid, !0:valid */
    OMX_U16 valid;
} OMX_TI_H3AAEWBPAXELDATA;

typedef struct OMX_TI_H3AAFDATA {
    OMX_U32 nSize;                                          /**< The size of the structure
                                                                                               including the length of data field containing the histogram data */
    OMX_VERSIONTYPE       nVersion;
    OMX_U32               nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U8               *data;
} OMX_TI_H3AAFDATA;


#define OMX_TI_AEWB_MAX_NUM_PAXELS (35 * 127)

/**
* OMX_TI_AEWBPAXELDATA
* @param nSubsampleAccumulatedValues[4]         : Sub sample accumulator(s), not-clipped. Seperate for each pixel in 2x2 sub-sample
* @param nSaturatorAccumulatedValues[4]           : Saturator accumulator(s), clipped based upon threshold. Seperate for each pixel in 2x2 sub-sample
* @param nUnsaturatedCount : Count of unsaturated 2x2 sub-samples in a paxel
**/
typedef struct OMX_TI_AEWBPAXELDATA {
    OMX_U16 nSubsampleAccumulatedValues[4];
    OMX_U16 nSaturatorAccumulatedValues[4];
    OMX_U32 nUnsaturatedCount;
} OMX_TI_AEWBPAXELDATA;

/**
* OMX_TI_AEWBMODE
* @enum OMX_TI_AEWB_MODE_SUM_OF_SQUARE    : Sum of square calculated across sub-samples in a paxel
* @enum OMX_TI_AEWB_MODE_MINMAX                  : Min-max calculted across sub-samples in a paxel
* @enum OMX_TI_AEWB_MODE_SUM_ONLY              : Only Sum calculated across sub-samples in a paxel
**/
typedef enum {
    OMX_TI_AEWB_MODE_SUM_OF_SQUARE=0,
    OMX_TI_AEWB_MODE_MINMAX=1,
    OMX_TI_AEWB_MODE_SUM_ONLY=2,
    OMX_TI_AEWB_MODE_MAX = 0x7FFFFFFF
} OMX_TI_AEWBMODE;

/**
 * The extra data for AutoExposure, AutoWhiteBalance data
 * @param eAEWBMode : AEWB mode
 * @param nAEWBThresholdPixelValue : Threshold against which pixel values are compared
 * @param nAccumulationShift : Right shift value applied on result of pixel accumulation
 * @param tAEWBPaxelWindow : AE/AWB paxel description
 * @param ptAEWBPaxelStatistics : Output AE/AWB buffer
 */
typedef struct OMX_TI_AEWB_STATISTICS_TYPE {
    OMX_TI_AEWBMODE               eAEWBMode;
    OMX_U16                       nAEWBThresholdPixelValue;
    OMX_U8                        nAccumulationShift;
    OMX_TI_H3aPAXELCFG            tAEWBPaxelWindow;
    OMX_TI_AEWBPAXELDATA          ptAEWBPaxelStatistics[OMX_TI_AEWB_MAX_NUM_PAXELS];
} OMX_TI_AEWB_STATISTICS_TYPE;



typedef enum OMX_TI_BSC_COLOUR_ELEMENT_TYPE {
    OMX_TI_BSC_COLOUR_ELEMENT_Y = 0,
    OMX_TI_BSC_COLOUR_ELEMENT_Cb = 1,
    OMX_TI_BSC_COLOUR_ELEMENT_Cr = 2,
    OMX_TIBSC_COLOUR_ELEMENT_MAX = 0x7FFFFFFF
} OMX_TI_BSC_COLOUR_ELEMENT_TYPE;


/**
 * OMX_TI_BSC_POSITIONPARAMETERS
 * @param nVectors : number of row/column sum vectors. Max value = 4
 * @param nShift : down-shift of input data
 * @param nVPos : vertical position of first pixel to be summed
 * @param nHPos : horizontal position of first pixel to be summed
 * @param nVNum : number of pixels sampled vertically
 * @param nHNum : number of pixels sampled horizontally
 * @param nVSkip : vertical spacing between adjacent pixels to be summed
 * @param nHSkip : horizontal pixel spacing between adjacent pixels to be summed
 *
 * The number of row/column sums cannot exceed 1920, implies:
 *   -  (nVectors + 1) * (nVNum) <=1920, for row sums
 *   -  (nVectors + 1) * (nHNum) <=1920, for column sums
 */
typedef struct OMX_TI_BSC_POSITIONPARAMETERS {
    OMX_U8  nVectors;
    OMX_U8  nShift;
    OMX_U16 nVPos;
    OMX_U16 nHPos;
    OMX_U16 nVNum;
    OMX_U16 nHNum;
    OMX_U8  nVSkip;
    OMX_U8  nHSkip;
} OMX_TI_BSC_POSITIONPARAMETERS;

/* Max number of row/column sums supported by Bsc engine*/
#define OMX_TI_BSC_MAX_NUM_ROW_COLUMN_SUMS (1920)

/**
 * The extra data for ISP Boundary Signal Calculator engine
 * @param eBscColourElement : Selects the element to be summed (Y, Cb or Cr)
 * @param nRowPosition : Bsc row sum descriptor
 * @param nColumnPosition : Bsc column sum descriptor
 * @param ptBscRowSumData : Each value corresponds to sum value in a row.
 *                          Num of row sums = nRowPosition.nVectors * nRowPosition.VNum
 * @param ptBscColumnSumData : Each value corresponds to sum value in a column.
 *                             Num of column sums = nColumnPosition.nVectors * nColumnPosition.HNum
 */
typedef struct OMX_TI_BSC_STATISTICS_TYPE {
    OMX_TI_BSC_COLOUR_ELEMENT_TYPE eBscColourElement;
    OMX_TI_BSC_POSITIONPARAMETERS  nRowPosition;
    OMX_TI_BSC_POSITIONPARAMETERS  nColumnPosition;
    OMX_U16                        ptBscRowSumData[OMX_TI_BSC_MAX_NUM_ROW_COLUMN_SUMS];
    OMX_U16                        ptBscColumnSumData[OMX_TI_BSC_MAX_NUM_ROW_COLUMN_SUMS];
} OMX_TI_BSC_STATISTICS_TYPE;


#define AUX_IMAGEDATA_MAX_SIZE_BYTES     ((320 * 240 * 3)/2)

/**
 * The extra data for AuxiliaryImageData
 * @param eAuxImageFormat : Format of image. In case of Aux buffer, it is OMX_COLOR_FormatYUV420SemiPlanar (YUV 4:2:0)
 * @param nAuxImageWidth : Width of Auxiliary Image (<=320)
 * @param nAuxImageHeight : Height of Auxiliary Image (<=240)
 * @param nAuxImageStrideBytes : Width in bytes for auxiliary buffer. In case of YUV4:2:0, stride for UV component is half of this value.
 * @param ptAuxImage : Data Pointer for Auxiliary data. First 320*240 bytes is Y Component. Remaining is UV component.
 */
typedef struct OMX_TI_AUX_IMAGEDATA_TYPE {
    OMX_COLOR_FORMATTYPE      eAuxImageFormat;
    OMX_U16                   nAuxImageWidth;
    OMX_U16                   nAuxImageHeight;
    OMX_U16                   nAuxImageStrideBytes;
    OMX_U8                    ptAuxImage[AUX_IMAGEDATA_MAX_SIZE_BYTES];
} OMX_TI_AUX_IMAGEDATA_TYPE;


/**
* Data structure carrying information about
* VTC slice height.
*
* @param nSize Size of the structure in bytes.
* @param nVersion OMX specification version information.
* @param nSliceHeight Definition of slice height.
*
*
*
*
*/
typedef struct OMX_TI_PARAM_VTCSLICE {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nSliceHeight;
    OMX_U32         nInternalBuffers;
    OMX_PTR         IonBufhdl[2];
} OMX_TI_PARAM_VTCSLICE;


/**
 *  nSize is the size of the structure including the length of data field containing
 *  the histogram data.
 *  nBins is the number of bins in the histogram.
 *  eComponentType specifies the type of the histogram bins according to enum.
 *  It can be selected to generate multiple component types, then the extradata struct
 *  is repeated for each component type.
 */
typedef struct OMX_TI_HISTOGRAMTYPE {
    OMX_U32 nSize;                      /**< The size of the structure
                                             including the length of data field containing the histogram data */
    OMX_VERSIONTYPE       nVersion;
    OMX_U32               nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;
    OMX_U32               nBins;        /**< The number of bins in the histogram */
    OMX_HISTCOMPONENTTYPE eComponentType; /**< Specifies the type of the histogram bins according to enum.
                                             It can be selected to generate multiple component types,
                                             then the extradata struct is repeated for each component type */
    OMX_U8 *data;
} OMX_TI_HISTOGRAMTYPE;



typedef struct OMX_TI_CPCAMDATA {
    OMX_U32               nSize;
    OMX_VERSIONTYPE       nVersion;
    OMX_U32               nPortIndex;
    OMX_TI_CAMERAVIEWTYPE eCameraView;

    /* Face Detect */
    OMX_U16           ulFaceCount;     // faces detected
    OMX_TI_FACERESULT tFacePosition[35];     // 35 is max faces supported by FDIF


    /**** Ancillary Data ******/
    OMX_U32 nFrameNumber;
    OMX_U16 nInputImageHeight;
    OMX_U16 nInputImageWidth;
    OMX_U16 nOutputImageHeight;
    OMX_U16 nOutputImageWidth;
    OMX_U16 nDigitalZoomFactor;
    OMX_S16 nCropCenterColumn;
    OMX_S16 nCropCenterRow;
    OMX_U16 nOpticalZoomValue;
    OMX_U8  nAFStatus;
    OMX_U8  nAWBStatus;
    OMX_U8  nAEStatus;
    OMX_U32 nExposureTime;
    OMX_U16 nEVCompensation;
    OMX_U8  nDigitalGainValue;
    OMX_U8  nAnalogGainValue;
    OMX_U16 nCurrentISO;
    OMX_U16 nReferenceISO;
    OMX_U8  nApertureValue;
    OMX_U8  nPixelRange;
    OMX_U8  nCameraShake;
    OMX_U8  nNumFacesDetected;

    /* Not Yet Supported */
    OMX_U16 nFocalDistance;
    OMX_U16 nShotNumber;
    OMX_U8  nFlashStatus;


    /***  White Balance gains ****/
    /**< White Balance Color Temperature in Kelvins */
    OMX_U16 nColorTemperature;

    /**< Bayer applied R color channel gain in (U13Q9) */
    OMX_U16 nGainR;

    /**< Bayer applied Gr color channel gain in (U13Q9) */
    OMX_U16 nGainGR;

    /**< Bayer applied Gb color channel gain in (U13Q9) */
    OMX_U16 nGainGB;

    /**< Bayer applied B color channel gain in (U13Q9) */
    OMX_U16 nGainB;

    /* BELOW  ARE NOT   SUPPORTED , Default set to 0 */
    OMX_S16 nOffsetR;              /**< Bayer applied R color channel offset */
    OMX_S16 nOffsetGR;             /**< Bayer applied Gr color channel offset */
    OMX_S16 nOffsetGB;             /**< Bayer applied Gb color channel offset */
    OMX_S16 nOffsetB;              /**< Bayer applied B color channel offset */


    /* AEWB,AF,HIST data size */
    OMX_U32 nAewbDataSize;
    OMX_U32 nAfDataSize;
    OMX_U32 nHistSize;



    /*** H3A AF-AEW DATA ***/
    OMX_TI_H3aPAXELCFG       tAfPaxelWin;
    OMX_TI_H3aPAXELCFG       tAewbPaxelWin;
    OMX_TI_H3AAEWBPAXELDATA *tpPaxel;
    OMX_TI_H3AAFDATA         tH3A_Af;
    /* Histogram */
    OMX_TI_HISTOGRAMTYPE Histogram;


} OMX_TI_CPCAMDATA;

/**
* Start/Stop mechanical misalignment
*
* STRUCT MEMBERS:
* nSize: Size of the structure in bytes
* nVersion: OMX specification version information
* nPortIndex: Index of the port on which Mechanical Misalignment to be enabled/disabled
* bMM: Mechanical Misalignment enable/disable flag
*/
    typedef struct OMX_TI_CONFIG_MM {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_U32         nPortIndex;
        OMX_BOOL        bMM;
    } OMX_TI_CONFIG_MM;

/**
* Start/Stop Affine transformation for Mm/Ac
*
* STRUCT MEMBERS:
* nSize: Size of the structure in bytes
* nVersion: OMX specification version information
* bAffine: Enable / Disable
*/
    typedef struct OMX_TI_PARAM_AFFINE {
        OMX_U32         nSize;
        OMX_VERSIONTYPE nVersion;
        OMX_BOOL        bAffine;
        } OMX_TI_PARAM_AFFINE;

/**
* A pointer to this struct is passed to the OMX_SetParameter when the extension
* index for the 'OMX.google.android.index.enableAndroidNativeBuffers' extension
* is given.
* The corresponding extension Index is OMX_TI_IndexUseNativeBuffers.
* This will be used to inform OMX about the presence of gralloc pointers instead
* of virtual pointers
*/
typedef struct OMX_TI_PARAMUSENATIVEBUFFER {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_BOOL bEnable;
} OMX_TI_PARAMUSENATIVEBUFFER;

/**
* A pointer to this struct is passed to OMX_GetParameter when the extension
* index for the 'OMX.google.android.index.getAndroidNativeBufferUsage'
* extension is given.
* The corresponding extension Index is OMX_TI_IndexAndroidNativeBufferUsage.
* The usage bits returned from this query will be used to allocate the Gralloc
* buffers that get passed to the useAndroidNativeBuffer command.
*/
typedef struct OMX_TI_PARAMNATIVEBUFFERUSAGE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nUsage;
} OMX_TI_PARAMNATIVEBUFFERUSAGE;

/**
 * OMX_TI_ZSL_PRIORITY_TYPE Enumerated Value
 */
typedef enum OMX_TI_ZSL_PRIORITY_TYPE {
    OMX_TI_ZSL_PRIORITY_TIME,
    OMX_TI_ZSL_PRIORITY_FOCUS,
    OMX_TI_ZSL_PRIORITY = 0x7FFFFFFF
} OMX_TI_ZSL_PRIORITY_TYPE;

/**
* Define the priority tha twill be used to select ZSL frame
*
* STRUCT MEMBERS:
* nSize: Size of the structure in bytes
* nVersion: OMX specification version information
* ePriority: Priority
*/
typedef struct OMX_TI_CONFIG_ZSLFRAMESELECTPRIOTYPE {
    OMX_U32                  nSize;
    OMX_VERSIONTYPE          nVersion;
    OMX_TI_ZSL_PRIORITY_TYPE ePriority;
} OMX_TI_CONFIG_ZSLFRAMESELECTPRIOTYPE;

/**
* MIPI, ECC, and CRC counters
* Mipi counter counts the frames from the MIPI receiver (CSI_RX).
* TCMD application will use this test
* to validate the MIPI channel integrity (TX to RX).
*
* STRUCT MEMBERS:
*  nSize              : Size of the structure in bytes
*  nVersion           : OMX specification version information
*  nPortIndex         : Port that this structure applies to
*  bResetMIPICounter  : if OMX_SetConfig() is called with value True
*                       for this parameter, the MIPICounter shall be reset to 0, by ducati.
*  nMIPICounter       : MIPI frame counter
*  nECCCounter        : ECC counter
*  nCRCCounter        : CRC counter
*/
typedef struct OMX_CONFIG_MIPICOUNTERS {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_BOOL        bResetMIPICounter;
    OMX_U32         nMIPICounter;
    OMX_U32         nECCCounter;
    OMX_U32         nCRCCounter;
    OMX_U32         nFifoOvfCounter;
    OMX_U32         nOCPCounter;
    OMX_U32         nEccCorrCounter;
    OMX_U32         SoTErrCnt;
    OMX_U32         SoTSyncErrCnt;
    OMX_U32         ULPMCnt;
    OMX_U32         ULPMExitCnt;
    OMX_U32         ULPMEnterCnt;
    OMX_U32         ControlErrCnt;
    OMX_U32         ErrEscapeCnt;
    OMX_U32         CSIRxTimeoutCnt;
    OMX_U32         bStopStartCntrs;
} OMX_CONFIG_MIPICOUNTERS;

/**
* CSI Timing Register
*
* STRUCT MEMBERS:
*  nSize              : Size of the structure in bytes
*  nVersion           : OMX specification version information
*  nPortIndex         : Port that this structure applies to
*  nReadWrite         : if OMX_SetConfig() is called with value True
*                       for this parameter, the ISS_CAMERARX_CORE1_REG0 register will be
*                       written with the supplied values below.
*  nThsSettle         :
*  nThsTerm           :
*  nHsClkCfg          :
*/
typedef struct OMX_CONFIG_CSITIMINGRW {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_U8          nReadWrite;
    OMX_U8          nThsSettle;
    OMX_U8          nThsTerm;
    OMX_U8          nHsClkCfg;
} OMX_CONFIG_CSITIMINGRW;

/**
* CSI Complex IO Data
*
* STRUCT MEMBERS:
*  nSize              : Size of the structure in bytes
*  nVersion           : OMX specification version information
*  nPortIndex         : Port that this structure applies to
*  nFrameCount        : Recieved Frames on the CSI2Rx
*  nLaneCount         : Number of active lanes
*  nCSISpeed          : CSI2Rx speed
*/
typedef struct OMX_CONFIG_CSICMPXIO {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_U32         nFrameCount;
    OMX_U32         nLaneCount;
    OMX_U32         nCSISpeed;
} OMX_CONFIG_CSICMPXIO;

/**
 * Auto Focus Score
 *
 *  STRUCT MEMBERS:
 *  nSize              : Size of the structure in bytes
 *  nVersion           : OMX specification version information
 *  nPortIndex         : Port that this structure applies to
 *  nAutoFocusScore    : Auto Focus Score
 */
typedef struct OMX_CONFIG_AUTOFOCUSSCORE {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_U32         nAutoFocusScore;
} OMX_CONFIG_AUTOFOCUSSCORE;

/**
 * Color Bar test pattern
 *
 *  STRUCT MEMBERS:
 *  nSize              : Size of the structure in bytes
 *  nVersion           : OMX specification version information
 *  nPortIndex         : Port that this structure applies to
 *  bEnableColorBars   : Enable Color Bars test pattern
 */
typedef struct OMX_CONFIG_COLORBARS {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_U32         bEnableColorBars;
} OMX_CONFIG_COLORBARS;

/**
* Sensor OTP EEEPROM data
*
*  STRUCT MEMBERS:
*  nSize              : Size of the structure in bytes
*  nVersion           : OMX specification version information
*  nPortIndex         : Port that this structure applies to
*  pData              : pointer to the client's buffer
*  nDataSize          : size of the EEPROM data in bytes
*  nClientDataSize    : size of the client's buffer
*  SensorIndex        : index of the eeprom buffer
*/
typedef struct OMX_CONFIG_OTPEEPROM {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_PTR         pData;
    OMX_U32         nDataSize;
    OMX_U32         nClientDataSize;
    OMX_U8          SensorIndex;
}OMX_CONFIG_OTPEEPROM;

/**
 * The OMX_ISP_TYPE enumeration is used to define the
 * TI ISP & ST ISP types.
 */
typedef enum OMX_ISP_TYPE {
    OMX_TIISP = 0,
    OMX_STISP= 1,
    OMX_ISPUnknown
} OMX_ISP_TYPE;

/**
* ISP Information
*
*  STRUCT MEMBERS:
*  nSize              : Size of the structure in bytes
*  nVersion           : OMX specification version information
*  nPortIndex         : Port that this structure applies to
*  eIspType              : ISP Type (TI ISP/ ST ISP)
*  nHardwareVersion    : Hardware version of ISP
*  nSoftwareVersion        : Software version of ISP
*/
typedef struct OMX_CONFIG_ISPINFO {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_ISP_TYPE    eIspType;
    OMX_U32         nHardwareVersion;
    OMX_U32         nSoftwareVersion;
    OMX_S8          cDucatiVersion[32];
}OMX_CONFIG_ISPINFO;

typedef enum OMX_TI_PORTTAPPOINTTYPE {
    OMX_TI_PortTap_Bayer_SensorOutput,
    OMX_TI_PortTap_Bayer_PostLsc,
    OMX_TI_PortTap_Bayer_PreBayerToYUVConversion,
    OMX_TI_PortTap_YUV_PostBayerToYUVConversion,
    OMX_TI_PortTap_YUV_PreJPEGCompression,
    OMX_TI_PortTap = 0x7FFFFFFF
} OMX_TI_PORTTAPPOINTTYPE;

/**
 * Define configuration structure for
 * tap in/out points for the selected port
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  eTapPoint   : Select the tap in/out point for the port
 */
typedef struct OMX_TI_CONFIG_PORTTAPPOINTTYPE {
    OMX_U32                 nSize;
    OMX_VERSIONTYPE         nVersion;
    OMX_U32                 nPortIndex;
    OMX_TI_PORTTAPPOINTTYPE eTapPoint;
} OMX_TI_CONFIG_PORTTAPPOINTTYPE;

/**
 * Available methods to apply vect shot exposure and gain
 */
typedef enum OMX_TI_EXPGAINAPPLYMETHODTYPE {
    OMX_TI_EXPGAINAPPLYMETHOD_ABSOLUTE,
    OMX_TI_EXPGAINAPPLYMETHOD_RELATIVE,
    OMX_TI_EXPGAINAPPLYMETHOD_FORCE_RELATIVE,
    OMX_TI_EXPGAINAPPLYMETHOD_FORCE_ABSOLUTE,
    OMX_TI_EXPGAINAPPLYMETHOD = 0x7FFFFFFF
} OMX_TI_EXPGAINAPPLYMETHODTYPE;

/**
 * Define configuration structure for
 * shot configuration for the selected port
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  nConfigId   : A unique config identification number that will be
 *                put in ancillary data for the corresponding output frame
 *  nFrames     : Number of sequential frames that will use this
 *                configuration
 *  nEC         : Total exposure compensation value
 *  nExp        : Exposure value for this configuration slot
 *  nGain       : Gain value for this configuration slot
 *  eExpGainApplyMethod : Selects the method which will be used to apply exposure and gain
 *  bNoSnapshot : Determinates whether a snapshot image will be send
 *                on the preview port for this shot config
 */
typedef struct OMX_TI_CONFIG_SHOTCONFIG {
    OMX_U32                         nConfigId;
    OMX_U32                         nFrames;
    OMX_S32                         nEC;
    OMX_S32                         nExp;
    OMX_S32                         nGain;
    OMX_TI_EXPGAINAPPLYMETHODTYPE   eExpGainApplyMethod;
    OMX_BOOL                        bNoSnapshot;
} OMX_TI_CONFIG_SHOTCONFIG;

/**
 * Define configuration structure for
 * shot configuration vector for the selected port
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  bFlushQueue     : If TRUE: Flush queue and abort processing before enqueing
 *                    new shot configurations
 *  nNumConfigs     : Number of valid configurations in the nShotConfig array
 *  nShotConfig     : Array of shot configurations
 *  nSlotsAvilable  : Return value with number of available slots in the queue
 */
typedef struct OMX_TI_CONFIG_ENQUEUESHOTCONFIGS {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U32                     nPortIndex;
    OMX_BOOL                    bFlushQueue;
    OMX_U32                     nNumConfigs;
    OMX_TI_CONFIG_SHOTCONFIG    nShotConfig[5];
} OMX_TI_CONFIG_ENQUEUESHOTCONFIGS;

/**
 * Define configuration structure to
 * query available/free shots in shot queue.
 * Will be supported only as GetConfig function.
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  nAvailableShots : Number of available shots
 */
typedef struct OMX_TI_CONFIG_QUERYAVAILABLESHOTS {
    OMX_U32         nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32         nPortIndex;
    OMX_U32         nAvailableShots;
} OMX_TI_CONFIG_QUERYAVAILABLESHOTS;

/**
 * Available vector shot capture stop methods
 */
typedef enum OMX_TI_VECTSHOTSTOPMETHOD {
    OMX_TI_VECTSHOTSTOPMETHOD_GOTO_PREVIEW,
    OMX_TI_VECTSHOTSTOPMETHOD_WAIT_IN_CAPTURE,
    OMX_TI_VECTSHOTSTOPMETHOD_MAX = 0x7FFFFFFF
} OMX_TI_VECTSHOTSTOPMETHOD;

/**
 * Define configuration structure to
 * specify the beahvior of vector shot capture
 * when the shot queue is empty
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  eStopMethod     : Select the stop method
 */
typedef struct OMX_TI_CONFIG_VECTSHOTSTOPMETHODTYPE {
    OMX_U32                     nSize;
    OMX_VERSIONTYPE             nVersion;
    OMX_U32                     nPortIndex;
    OMX_TI_VECTSHOTSTOPMETHOD   eStopMethod;
} OMX_TI_CONFIG_VECTSHOTSTOPMETHODTYPE;

/* Number of elements per color */
#define OMX_TI_GAMMATABLE_SIZE (512)

/**
 * Describes each element of gamma table
 */
typedef struct OMX_TI_GAMMATABLE_ELEM_TYPE {
    OMX_S16 nOffset;
    OMX_S16 nSlope;
} OMX_TI_GAMMATABLE_ELEM_TYPE;

/**
 * Gamma table
 *
 * STRUCT MEMBERS:
 *  pR  : Red color table
 *  pG  : Blue color table
 *  pB  : Green color table
 */
typedef struct OMX_TI_CONFIG_GAMMATABLE_TYPE {
    OMX_TI_GAMMATABLE_ELEM_TYPE pR[OMX_TI_GAMMATABLE_SIZE];
    OMX_TI_GAMMATABLE_ELEM_TYPE pG[OMX_TI_GAMMATABLE_SIZE];
    OMX_TI_GAMMATABLE_ELEM_TYPE pB[OMX_TI_GAMMATABLE_SIZE];
} OMX_TI_CONFIG_GAMMATABLE_TYPE;
/**
 * Define configuration structure to
 * specify computing of image pyramids
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  nLevelsCount    : Number of levels of the pyramid
 *  nScalingFactor  : (Format: Q16) Scaling factor for
 *                    the levels of the pyramid
 */
typedef struct OMX_TI_PARAM_IMAGEPYRAMIDTYPE {
    OMX_U32                 nSize;
    OMX_VERSIONTYPE         nVersion;
    OMX_U32                 nPortIndex;
    OMX_U32                 nLevelsCount;
    OMX_U32                 nScalingFactor; // Q16
} OMX_TI_PARAM_IMAGEPYRAMIDTYPE;



/* 16 Extended to allow precise H3A for preview */
#define OMX_MAX_WINHC                       (36)
/* 32 Extended to allow precise H3A for preview */
#define OMX_MAX_WINVC                       (128)
/* One paxel is 32 Bytes + on every 8 paxel 8*2 Bytes for number of unsaturated pixels in previouse 8 paxels */
#define OMX_AWWB_H3A_PAXEL_SIZE_BYTES       (32 + 2)
/*AF SCM Range Constants */
#define OMX_AF_PAXEL_VERTICAL_COUNT_MAX     (127)
/*AF SCM Range Constants */
#define OMX_AF_PAXEL_HORIZONTAL_COUNT_MAX   (35)
/* Max sensor with */
#define OMX_ISP_IN_WIDTH                    (4032)
/* Max sensor height */
#define OMX_ISP_IN_HEIGHT                   (3024)


//!< Noise filter number of THR coefficients
#define OMX_ISS_NF_THR_COUNT        (8)
//!< Noise filter number of STR coefficients
#define OMX_ISS_NF_STR_COUNT        (8)
//!< Noise filter number of SPR coefficients
#define OMX_ISS_NF_SPR_COUNT        (8)
//!< Numbers of values in Gamma table
#define OMX_ISS_PREV_GAMMA_TABLE    (1024)
//!< Number of offset values on table
#define OMX_ISS_PREV_RGB2RGB_OFFSET (3)
//!< Numbers of values in rows and colomns
#define OMX_ISS_PREV_RGB2RGB_MATRIX (3)
//!< GBCE enhancement table size
#define OMX_ISS_GBCE_TABLE_SIZE     (1024)
//!< Edge enhancement table size
#define OMX_ISS_EE_TABLE_SIZE       (1024)
//!< Edge enhancement number of coefficients
#define OMX_ISS_COEFF               (9)
//!< Histogram dims count
#define OMX_ISS_HIST_DIMS_COUNT     (4)
//!< Histogram gain table size
#define OMX_ISS_HIST_GAIN_TBL       (4)
//!< Number of offset values on table
#define OMX_ISS_PREV_RGB2YUV_OFFSET (3)
//!< Numbers of values in rows and colomns
#define OMX_ISS_PREV_RGB2YUV_MATRIX (3)

/*--------data declarations -----------------------------------*/
typedef enum {
    OMX_IPIPE_BAYER_PATTERN_RGGB,
    OMX_IPIPE_BAYER_PATTERN_GRBG,
    OMX_IPIPE_BAYER_PATTERN_GBRG,
    OMX_IPIPE_BAYER_PATTERN_BGGR,
    OMX_IPIPE_BAYER_PATTERN_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_BAYER_PATTERNTYPE;

typedef enum {
    OMX_IPIPE_BAYER_MSB_BIT15,
    OMX_IPIPE_BAYER_MSB_BIT14,
    OMX_IPIPE_BAYER_MSB_BIT13,
    OMX_IPIPE_BAYER_MSB_BIT12,
    OMX_IPIPE_BAYER_MSB_BIT11,
    OMX_IPIPE_BAYER_MSB_BIT10,
    OMX_IPIPE_BAYER_MSB_BIT9,
    OMX_IPIPE_BAYER_MSB_BIT8,
    OMX_IPIPE_BAYER_MSB_BIT7,
    OMX_IPIPE_BAYER_MSB_BIT_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_BAYER_MSB_POSTYPE;

typedef enum {
    OMX_ISIF_LSC_8_PIXEL       = 3,
    OMX_ISIF_LSC_16_PIXEL      = 4,
    OMX_ISIF_LSC_32_PIXEL      = 5,
    OMX_ISIF_LSC_64_PIXEL      = 6,
    OMX_ISIF_LSC_128_PIXEL     = 7,
    OMX_ISIF_LSC_128_PIXEL_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE;

typedef enum {
    OMX_LSC_GAIN_0Q8        = 0,
    OMX_LSC_GAIN_0Q8_PLUS_1 = 1,
    OMX_LSC_GAIN_1Q7        = 2,
    OMX_LSC_GAIN_1Q7_PLUS_1 = 3,
    OMX_LSC_GAIN_2Q6        = 4,
    OMX_LSC_GAIN_2Q6_PLUS_1 = 5,
    OMX_LSC_GAIN_3Q5        = 6,
    OMX_LSC_GAIN_3Q5_PLUS_1 = 7,
    OMX_LSC_GAIN_MAX        = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_LSC_GAIN_FORMATTYPE;

typedef enum {
    OMX_ISIF_LSC_OFFSET_NO_SHIFT         = 0,
    OMX_ISIF_LSC_OFFSET_1_LEFT_SHIFT     = 1,
    OMX_ISIF_LSC_OFFSET_2_LEFT_SHIFT     = 2,
    OMX_ISIF_LSC_OFFSET_3_LEFT_SHIFT     = 3,
    OMX_ISIF_LSC_OFFSET_4_LEFT_SHIFT     = 4,
    OMX_ISIF_LSC_OFFSET_5_LEFT_SHIFT     = 5,
    OMX_ISIF_LSC_OFFSET_5_LEFT_SHIFT_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_LSC_OFFSET_SHIFTTYPE;

typedef enum {
    OMX_ISIF_LSC_OFFSET_OFF = 0,
    OMX_ISIF_LSC_OFFSET_ON  = 1,
    OMX_ISIF_LSC_OFFSET_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_LSC_OFFSET_ENABLETYPE;

typedef struct {
    //!< DLSCCFG ENABLE-- enable 2d LSC module
    OMX_U8 nEnable;
    //!< HVAL LSCHVAL--LSC HSIZE
    OMX_U16 nLscHSize;
    //!< HVAL LSCHVAL--LSC VSIZE
    OMX_U16 nLscVSize;
    //!< HOFST LSCHOFST DATAHOFST 0-16383-- H-direction data offset
    OMX_U16 nHDirDataOffset;
    //!< VOFST LSCVOFST DATAHOFST 0-16383-- V-direction data offset
    OMX_U16 nVDirDataOffset;
    //!< X DLSCINI   6:0-- H-position of the paxel
    OMX_U8 nHPosInPaxel;
    //!< Y DLSCINI   6:0-- V-position of the paxel
    OMX_U8 nVPosInPaxel;

    //!< GAIN_MODE_M DLSCCFG
    OMX_TI_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE ePaxHeight;
    //!< GAIN_MODE_N DLSCCFG
    OMX_TI_3ASKIP_ISIF_LSC_GAIN_MODE_MNTYPE ePaxLength;
    //!< GAIN_FORMAT DLSCCFG
    OMX_TI_3ASKIP_ISIF_LSC_GAIN_FORMATTYPE eGainFormat;
    //!< offset scaling factor
    OMX_U8 nOffsetScalingFactor;
    //!< OFSTSFT DLSCOFST--offset shift value
    OMX_TI_3ASKIP_ISIF_LSC_OFFSET_SHIFTTYPE eOffsetShiftVal;
    //!< OFSTSFT DLSCOFST--offset enable value
    OMX_TI_3ASKIP_ISIF_LSC_OFFSET_ENABLETYPE eOffsetEnable;
    //!< gain table address--32 bit aligned
    OMX_U32 pGainTableAddress[256];
    //!< gain table length
    OMX_U16 nGainTableLength;
    //!< offset table address
    OMX_U32 pOffsetTableAddress[256];
    //!< offset table length
    OMX_U16 nOffsetTableLength;
} OMX_TI_3ASKIP_ISIF_2DLSC_CFGTYPE;

typedef enum {
    OMX_ISIF_HORIZONTAL_CLAMP_DISABLED             = 0,
    OMX_ISIF_HORIZONTAL_CLAMP_ENABLED              = 1,
    OMX_ISIF_PREVIOUS_HORIZONTAL_CLAMP_ENABLED     = 2,
    OMX_ISIF_CLAMP_MAX                             = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE;

typedef enum {
    OMX_ISIF_ONE_COLOR_CLAMP    = 0,
    OMX_ISIF_FOUR_COLOR_CLAMP   = 1,
    OMX_ISIF_COLOR_CLAMP_MAX    = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE;

typedef enum {
    OMX_ISIF_HBLACK_2PIXEL_TALL    = 0,
    OMX_ISIF_HBLACK_4PIXEL_TALL    = 1,
    OMX_ISIF_HBLACK_8PIXEL_TALL    = 2,
    OMX_ISIF_HBLACK_16PIXEL_TALL   = 3,
    OMX_ISIF_HBLACK_PIXEL_TALL_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_HBLACK_PIXEL_HEIGHTTYPE;

typedef enum {
    OMX_ISIF_HBLACK_32PIXEL_WIDE   = 0,
    OMX_ISIF_HBLACK_64PIXEL_WIDE   = 1,
    OMX_ISIF_HBLACK_128PIXEL_WIDE  = 2,
    OMX_ISIF_HBLACK_256PIXEL_WIDE  = 3,
    OMX_ISIF_HBLACK_PIXEL_WIDE_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_HBLACK_PIXEL_WIDTHTYPE;

typedef enum {
    OMX_ISIF_VBLACK_PIXEL_NOT_LIMITED = 0,
    OMX_ISIF_VBLACK_PIXEL_LIMITED     = 1,
    OMX_ISIF_VBLACK_PIXEL_LIMITED_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_HBLACK_PIXEL_VALUE_LIMTYPE;

typedef enum {
    OMX_ISIF_VBLACK_BASE_WINDOW_LEFT  = 0,
    OMX_ISIF_VBLACK_BASE_WINDOW_RIGHT = 1,
    OMX_ISIF_VBLACK_BASE_WINDOW_MAX   = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_HBLACK_BASE_WINDOWTYPE;

typedef struct {
    //!< CLHSV CLHWIN2 12-0--Vertical black clamp.
    //!< Window Start position (V).Range: 0 - 8191
    OMX_U16 nVPos;
    //!< CLHWN  CLHWIN0--[Horizontal Black clamp]
    //!< Vertical dimension of a Window (2^N).
    OMX_TI_3ASKIP_ISIF_HBLACK_PIXEL_HEIGHTTYPE eVSize;
    //!< CLHSH CLHWIN1 12-0--Horizontal black clamp.
    //!< Window Start position (H).Range: 0 - 8191
    OMX_U16 nHPos;
    //!< CLHWM  CLHWIN0--[Horizontal Black clamp]
    //!< Horizontal dimension of a Window (2^M).
    OMX_TI_3ASKIP_ISIF_HBLACK_PIXEL_WIDTHTYPE eHSize;
    //!< CLHLMT CLHWIN0--Horizontal Black clamp. Pixel value
    //!< limitation for the Horizontal clamp value calculation
    OMX_TI_3ASKIP_ISIF_HBLACK_PIXEL_VALUE_LIMTYPE ePixelValueLimit;
    //!< CLHWBS CLHWIN0--[Horizontal Black clamp] Base Window select
    OMX_TI_3ASKIP_ISIF_HBLACK_BASE_WINDOWTYPE eRightWindow;
    //!< CLHWC  CLHWIN0--[Horizontal Black clamp]
    //!< Window count per color. Window count = CLHWC+1. Range: 1 - 32
    OMX_U8 nWindowCountPerColor;
} OMX_TI_3ASKIP_ISIFHBLACKPARAMSTYPE;

typedef enum {
    OMX_ISIF_VBLACK_2PIXEL_WIDE  = 0,
    OMX_ISIF_VBLACK_4PIXEL_WIDE  = 1,
    OMX_ISIF_VBLACK_8PIXEL_WIDE  = 2,
    OMX_ISIF_VBLACK_16PIXEL_WIDE = 3,
    OMX_ISIF_VBLACK_32PIXEL_WIDE = 4,
    OMX_ISIF_VBLACK_64PIXEL_WIDE = 5,
    OMX_ISIF_VBLACK_PIXEL_WIDE   = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE;

typedef enum {
    OMX_ISIF_VALUE_HORIZONTAL_DIRECTION = 0,
    OMX_ISIF_VALUE_CONFIG_REGISTER      = 1,
    OMX_ISIF_VALUE_NOUPDATE             = 2,
    OMX_ISIF_VALUE_MAX                  = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_VBLACK_RESET_MODETYPE;

typedef struct {
    //!< CLVSV CLVWIN2 12-0--Vertical black clamp.
    //!< Window Start position (V).Range: 0 - 8191
    OMX_U16 nVPos;
    //!< CLVOBV CLVWIN3 12-0 range 0-8192-- Vertical black clamp.
    //!< Optical black V valid (V).Range: 0 - 8191
    OMX_U16 nVSize;
    //!< CLVSH CLVWIN1 12-0--Vertical black clamp. Window Start
    //!< position (H).Range: 0 - 8191
    OMX_U16 nHPos;
    //!< CLVOBH  CLVWIN0-- Vertical Black clamp. Optical Black H valid (2^L).
    OMX_TI_3ASKIP_ISIF_VBLACK_PIXEL_WIDTHTYPE HSize;
    //!< CLVCOEF CLVWIN0-- Vertical Black clamp .Line average coefficient (k).
    OMX_U8 line_avg_coef;
    //!< CLVRVSL CLVWIN0-- Vertical Black clamp. Select the reset value
    //!< for the Clamp value of the Previous line
    OMX_TI_3ASKIP_ISIF_VBLACK_RESET_MODETYPE reset_mode;
    //!< CLVRV reset value U12--Vertical black clamp reset value
    //!< (U12) Range: 0 to 4095
    OMX_U16 reset_value;
} OMX_TI_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE;

typedef struct {
    //!< CLEN  CLAMPCFG-- clamp module enablement
    OMX_U8 nEnable;
    //!< CLMD  CLAMPCFG-- horizontal clamp mode
    OMX_TI_3ASKIP_ISIF_HORIZONTAL_CLAMP_MODETYPE eHClampMode;
    //!< CLHMD CLAMPCFG-- black clamp mode
    OMX_TI_3ASKIP_ISIF_BLACK_CLAMP_MODETYPE eBlackClampMode;
    //!< CLDCOFST CLDC s13-- clamp dc-offset value
    OMX_U16 nDCOffsetClampVal;
    //!< CLSV 12-0 (range 0-8191)--black clamp v-start position
    OMX_U16 nBlackClampVStartPos;
    //!< CLHWIN0-- horizontal black clamp parameters
    OMX_TI_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE tHorizontalBlack;
    //!< CLVWIN0-- vertical black clamp parameters
    OMX_TI_3ASKIP_ISIF_VERTICAL_BLACK_PARAMSTYPE tVerticalBlack;
} OMX_TI_3ASKIP_ISIF_CLAMP_CFGTYPE;

typedef struct {
    //!< FLSHEN FLSHCFG0-- flash enable
    OMX_U8 nEnable;
    //!< SFLSH FLSHCFG1--Start line to set the FLASH timing signal.
    OMX_U16 nFlashTimingStartLine;
    //!< VFLSH FLSHCFG--Valid width of the FLASH timing signal.
    //!< Valid width = Crystal-clock x 2 x (VFLSH + 1)
    OMX_U16 nFlashTimingWidth;
} OMX_TI_3ASKIP_ISIF_FLASH_CFGTYPE;

typedef struct {
    //!< gain offset feature-flag
    OMX_U8 gain_offset_featureflag;
    //!< CGR  CRGAIN-- gain R
    OMX_U16 gain_r;
    //!< CGGR CGRGAIN-- gain GR
    OMX_U16 gain_gr;
    //!< CGGB CGBGAIN-- gain GB
    OMX_U16 gain_gb;
    OMX_U16 gain_bg;
    //!< COFT COFSTA--offset
    OMX_U16 offset;
} OMX_TI_3ASKIP_ISIF_GAINOFFSET_CFGTYPE;

typedef enum {
    OMX_ISIF_VDLC_WHOLE_LINE                = 0,
    OMX_ISIF_VDLC_DISABLE_ABOVE_UPPER_PIXEL = 1,
    OMX_ISIF_VDLC_MAX                       = 0x7FFFFFFF

} OMX_TI_3ASKIP_ISIF_VDLC_PIXEL_DEPENDENCYTYPE;

typedef enum {
    OMX_ISIF_VLDC_FED_THRO_ONSATURATION                 = 0,
    OMX_ISIF_VLDC_HORIZONTAL_INTERPOLATION_ONSATURATION = 1,
    OMX_ISIF_VLDC_HORIZONTAL_INTERPOLATION              = 2,
    OMX_ISIF_VLDC_MAX                                   = 0x7FFFFFFF
} OMX_TI_3ASKIP_ISIF_VLDC_MODE_SELECTTYPE;

typedef struct {
    //!< DFCMEM0--Vertical Defect position
    OMX_U16 nVerticalDefectPosition;
    //!< DFCMEM1 12-0--horizontal defect position
    OMX_U16 nHorizontalDefectPosition;
    //!< DFCMEM2--Defect correction Memory 2
    OMX_U8 nSub1ValueVldc;
    //!< DFCMEM3--Defect correction Memory 3
    OMX_U8 nSub2LessThanVldc;
    //!< DFCMEM4--Defect correction Memory 4
    OMX_U8 nSub3GreaterThanVldc;
} OMX_TI_3ASKIP_ISIF_VLDCDEFECT_LINEPARAMSTYPE;

typedef struct {
    //!< VDFCEN  DFCCTL--enable VLDC module
    OMX_U8 nEnable;
    //!< VDFCUDA DFCCTL--pixel dependency
    OMX_TI_3ASKIP_ISIF_VDLC_PIXEL_DEPENDENCYTYPE eDisableVldcUpperPixels;
    //!< VDFLSFT DFCCTL-- VLDC shift values
    OMX_U8 nVldcShiftVal;
    //!< VDFCSL  DFCCTL--VLDC mode select
    OMX_TI_3ASKIP_ISIF_VLDC_MODE_SELECTTYPE eVldcModeSelect;
    //!< VDFSLV VDFSATLV OMX_U12 range 0 - 4095-- VLDC saturation level
    OMX_U16 nVldcSaturationLvl;
    //!< number of defect lines-maximum8
    OMX_U8 nDefectLines;
    //!< DFCMEM0 -8--  defect line paramaters
    OMX_TI_3ASKIP_ISIF_VLDCDEFECT_LINEPARAMSTYPE tVldcDefectLineParams;
} OMX_TI_3ASKIP_ISIF_VLDC_CFGTYPE;

typedef enum {
    OMX_NOISE_FILTER_1   = 1,
    OMX_NOISE_FILTER_2   = 2,
    OMX_NOISE_FILTER_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_NOISE_FILTERTYPE;

typedef enum {
    OMX_IPIPE_NF_SPR_SINGLE = 0,
    OMX_IPIPE_NF_SPR_LUT    = 1,
    OMX_IPIPE_NF_SPR_MAX    = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_NF_SELTYPE;

typedef enum {
    OMX_IPIPE_NF_LSC_GAIN_OFF  = 0,
    OMX_IPIPE_NF_LSC_GAIN_ON   = 1,
    OMX_IPIPE_NF_LSC_GAIN_MAX  = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_NF_LSC_GAINTYPE;

typedef enum {
    OMX_IPIPE_NF_SAMPLE_BOX     = 0,
    OMX_IPIPE_NF_SAMPLE_DIAMOND = 1,
    OMX_IPIPE_NF_SAMPLE_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE;

typedef struct {
    //!< Enable-disable for Noise filter in ipipe
    OMX_U8 nEnable;
    //!< Noise filter number
    OMX_TI_3ASKIP_IPIPE_NOISE_FILTERTYPE eNFNum;
    //!< Selecting the spread in NF
    OMX_TI_3ASKIP_IPIPE_NF_SELTYPE eSel;
    //!< Controling the lsc gain applied in Noise Filter
    OMX_TI_3ASKIP_IPIPE_NF_LSC_GAINTYPE eLscGain;
    //!< Selecting the sampling method
    OMX_TI_3ASKIP_IPIPE_NF_SAMPLE_METHODTYPE eTyp;
    OMX_U8                                   nDownShiftVal;
    OMX_U8                                   nSpread;
    OMX_U16                                  pThr[OMX_ISS_NF_THR_COUNT];
    OMX_U8                                   pStr[OMX_ISS_NF_STR_COUNT];
    OMX_U8                                   pSpr[OMX_ISS_NF_SPR_COUNT];
    OMX_U16                                  nEdgeMin;
    OMX_U16                                  nEdgeMax;
} OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE;

typedef enum {
    OMX_IPIPE_GIC_LSC_GAIN_OFF = 0,
    OMX_IPIPE_GIC_LSC_GAIN_ON  = 1,
    OMX_IPIPE_GIC_LSC_GAIN_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_GIC_LSC_GAINTYPE;

typedef enum {
    OMX_IPIPE_GIC_GICTHR = 0,
    OMX_IPIPE_GIC_NF2THR = 1,
    OMX_IPIPE_GIC_MAX    = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_GIC_SELTYPE;

typedef enum {
    OMX_IPIPE_GIC_DIFF_INDEX = 0,
    OMX_IPIPE_GIC_HPD_INDEX  = 1,
    OMX_IPIPE_GIC_INDEX_MAX  = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_GIC_INDEXTYPE;

typedef struct {
    /* works only when data format is GR,GB */
    //!< Enable-disable for diffrerent components in ipipe
    OMX_U16 nEnable;
    //!< Selecting LSC gain in GIC
    OMX_TI_3ASKIP_IPIPE_GIC_LSC_GAINTYPE eLscGain;
    //!< Slection of threshold vaue in GIC filter
    OMX_TI_3ASKIP_IPIPE_GIC_SELTYPE eSel;
    //!< Selecting the index in GIC
    OMX_TI_3ASKIP_IPIPE_GIC_INDEXTYPE eTyp;
    OMX_U8                            nGicGain;
    OMX_U8                            nGicNfGain;
    OMX_U16                           nGicThr;
    OMX_U16                           nGicSlope;
} OMX_TI_3ASKIP_IPIPE_GIC_CFGTYPE;

typedef struct {
    /*offseet after R,GR,GB,B*/
    OMX_U16 pOffset[4];
    /*gain for R gr gb B*/
    OMX_U16 pGain[4];
} OMX_TI_3ASKIP_IPIPE_WB_CFGTYPE;

typedef enum {
    OMX_IPIPE_CFA_MODE_2DIR    = 0,
    OMX_IPIPE_CFA_MODE_2DIR_DA = 1,
    OMX_IPIPE_CFA_MODE_DAA     = 2,
    OMX_IPIPE_CFA_MODE_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_CFA_MODETYPE;

typedef struct {
    OMX_U16 nHpfThr;
    OMX_U16 nHpfSlope;
    OMX_U16 nMixThr;
    OMX_U16 nMixSlope;
    OMX_U16 nDirThr;
    OMX_U16 nDirSlope;
    OMX_U16 nDirNdwt;
} OMX_TI_3ASKIP_IPIPE_CFA_DIRTYPE;

typedef struct {
    OMX_U8  nMonoHueFra;
    OMX_U8  nMonoEdgThr;
    OMX_U16 nMonoThrMin;
    OMX_U16 nMonoThrSlope;
    OMX_U16 nMonoSlpMin;
    OMX_U16 nMonoSlpSlp;
    OMX_U16 nMonoLpwt;
} OMX_TI_3ASKIP_IPIPE_CFA_DAATYPE;

typedef struct {
    OMX_U8                           nEnable;
    OMX_TI_3ASKIP_IPIPE_CFA_MODETYPE eMode;
    OMX_TI_3ASKIP_IPIPE_CFA_DIRTYPE  tDir;
    OMX_TI_3ASKIP_IPIPE_CFA_DAATYPE  tDaa;
} OMX_TI_3ASKIP_IPIPE_CFA_CFGTYPE;

typedef enum {
    OMX_IPIPE_GAMMA_TBL_64  = 0,
    OMX_IPIPE_GAMMA_TBL_128 = 1,
    OMX_IPIPE_GAMMA_TBL_256 = 2,
    OMX_IPIPE_GAMMA_TBL_512 = 3,
    OMX_IPIPE_GAMMA_TBL_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_GAMMA_TABLE_SIZETYPE;

typedef enum {
    OMX_IPIPE_GAMMA_BYPASS_ENABLE  = 1,
    OMX_IPIPE_GAMMA_BYPASS_DISABLE = 0,
    OMX_IPIPE_GAMMA_BYPASS_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_GAMMA_BYPASSTYPE;

typedef struct {
    OMX_U8                                   nEnable;
    OMX_TI_3ASKIP_IPIPE_GAMMA_TABLE_SIZETYPE eGammaTblSize;
    //!< May not be needed, since table is always in RAM
    OMX_U8 nTbl;
    //!< o not bypassed
    OMX_TI_3ASKIP_IPIPE_GAMMA_BYPASSTYPE eBypassB;
    OMX_TI_3ASKIP_IPIPE_GAMMA_BYPASSTYPE eBypassG;
    OMX_TI_3ASKIP_IPIPE_GAMMA_BYPASSTYPE eBypassR;
    /*poOMX_Ser to red gamma table      Red gamma table -   (U8Q0)
      *                                 Blue gamma table -  (U8Q0)
      *                                 Green gamma table - (U8Q0)
      */
    OMX_S8 pRedTable[OMX_ISS_PREV_GAMMA_TABLE];
    OMX_S8 pBlueTable[OMX_ISS_PREV_GAMMA_TABLE];
    OMX_S8 pGreenTable[OMX_ISS_PREV_GAMMA_TABLE];
} OMX_TI_3ASKIP_IPIPE_GAMMA_CFGTYPE;

typedef struct {
    /*      [RR] [GR] [BR]
     *      [RG] [GG] [BG]
     *      [RB] [GB] [BB]*/
    /* Blending values(S12Q8 format) */
    /*RR,GR,BR,RG,GG,BG,RB,GB,BB each 11 bits*/
    OMX_U16 pMulOff[OMX_ISS_PREV_RGB2RGB_MATRIX][OMX_ISS_PREV_RGB2RGB_MATRIX];
    /* Blending offset value for R,G,B - (S10Q0) */
    /*R,G,B each 13 bits*/
    OMX_U16 pOft[OMX_ISS_PREV_RGB2RGB_OFFSET];
} OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE;

typedef struct {
    OMX_U8 nBrightness;
    OMX_U8 nContrast;
    /*      [CSCRY]   [CSCGY]  [CSCBY]
     *      [CSCRCB] [CSCGCB] [CSCBCB]
     *      [CSCRCR] [CSCGCR] [CSCBCR] */
    /* Color space conversion coefficients(S10Q8) */
    /*RY,GY,BY,RCB,GCB,BCB ,RCR,GCR,BCR 12 bits*/
    OMX_S16 pMulVal[OMX_ISS_PREV_RGB2YUV_MATRIX][OMX_ISS_PREV_RGB2YUV_MATRIX];
    /* CSC offset values for Y offset, CB offset
     *  and CR offset respectively (S8Q0) */
    /*Y,CB,CR -11bits*/
    OMX_S16 pOffset[OMX_ISS_PREV_RGB2YUV_OFFSET];
} OMX_TI_3ASKIP_IPIPE_RGBYUV_CFGTYPE;

typedef enum {
    //!< Cr CB unmodified
    OMX_IPIPE_GBCE_METHOD_Y_VALUE  = 0,
    OMX_IPIPE_GBCE_METHOD_GAIN_TBL = 1,
    OMX_IPIPE_GBCE_METHOD_MAX      = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_GBCE_METHODTYPE;

typedef struct {
    //!< Enable-disable for GBCE in ipipe
    OMX_U16 nEnable;
    //!< Selecting the type of GBCE method
    OMX_TI_3ASKIP_IPIPE_GBCE_METHODTYPE nTyp;
    //!< GBCE LookUp Tabale
    OMX_U16 pLookupTable[OMX_ISS_GBCE_TABLE_SIZE];
} OMX_TI_3ASKIP_IPIPE_GBCE_CFGTYPE;

typedef enum {
    OMX_IPIPE_PROC_COMPR_NO,
    OMX_IPIPE_PROC_COMPR_DPCM,
    OMX_IPIPE_PROC_COMPR_ALAW,
    OMX_IPIPE_PROC_COMPR_PACK,
    OMX_IPIPE_PROC_COMPR_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_PROC_COMPRESSIONTYPE;

typedef enum {
    OMX_IPIPE_YUV_PHS_POS_COSITED  = 0,
    OMX_IPIPE_YUV_PHS_POS_CENTERED = 1,
    OMX_IPIPE_YUV_PHS_POS_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_YUV_PHASE_POSTYPE;

typedef enum  {
    OMX_IPIPE_VP_DEV_CSIA,
    OMX_IPIPE_VP_DEV_CSIB,
    OMX_IPIPE_VP_DEV_CCP,
    OMX_IPIPE_VP_DEV_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_VP_DEVICETYPE;

//ISS configuration structure which controls the operation of yuv444 to yuv 422
typedef struct {
    OMX_TI_3ASKIP_IPIPE_YUV_PHASE_POSTYPE ePos;
    OMX_U8                                nLpfEn;
} OMX_TI_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE;

typedef enum {
    OMX_IPIPE_HALO_REDUCTION_ENABLE  = 1,
    OMX_IPIPE_HALO_REDUCTION_DISABLE = 0,
    OMX_IPIPE_HALO_REDUCTION_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_EE_HALO_CTRLTYPE;

/* ISS configuration structure Edge enhancement */
typedef struct {
    /** Defect Correction Enable */
    OMX_U16 nEnable;
    //Enable-disable for halo reduction in Edge enhancement
    OMX_TI_3ASKIP_IPIPE_EE_HALO_CTRLTYPE eHaloReduction;
    // 9 coefficients
    OMX_S16 pMulVal[OMX_ISS_COEFF];
    OMX_U8  nSel;
    OMX_U8  nShiftHp;
    OMX_U16 nThreshold;
    OMX_U16 nGain;
    OMX_U16 nHpfLowThr;
    OMX_U8  nHpfHighThr;
    OMX_U8  nHpfGradientGain;
    OMX_U8  nHpfgradientOffset;
    OMX_S16 pEeTable[OMX_ISS_EE_TABLE_SIZE];
} OMX_TI_3ASKIP_IPIPE_EE_CFGTYPE;

// ISS configuration structure for CAR module in ipipe
typedef struct {
    // Enable-disable for CAR module in ipipe
    OMX_U8  nEnable;
    OMX_U8  nTyp;
    OMX_U8  nSw0Thr;
    OMX_U8  nSw1Thr;
    OMX_U8  nHpfType;
    OMX_U8  nHpfShift;
    OMX_U8  nHpfThr;
    OMX_U8  nGn1Gain;
    OMX_U8  nGn1Shift;
    OMX_U16 nGn1Min;
    OMX_U8  nGn2Gain;
    OMX_U8  nGn2Shift;
    OMX_U16 nGn2Min;
} OMX_TI_3ASKIP_IPIPE_CAR_CFGTYPE;

//!< ISS configuration structure for LSC
typedef struct {
    OMX_U16 nVOffset;
    //<! va1
    OMX_S16 nVLinearCoeff;
    //<! va2
    OMX_S16 nVQuadraticCoeff;
    //<! vs1
    OMX_U8 nVLinearShift;
    //<! vs2
    OMX_U8  nVQuadraticShift;
    OMX_U16 nHOffset;
    //<! va1
    OMX_S16 nHLinearCoeff;
    //<! va2
    OMX_S16 nHQuadraticCoeff;
    //<! vs1
    OMX_U8 nHLinearShift;
    //<! vs2
    OMX_U8 nHQuadraticShift;
    //!< Gain value for R
    OMX_U8 nGainR;
    //!< Gain value for GR
    OMX_U8 nGainGR;
    //!< Gain value for GB
    OMX_U8 nGainGB;
    //!< Gain value for B
    OMX_U8 nGainB;
    //!< Offset value for R
    OMX_U8 nOffR;
    //!< Offset value for GR
    OMX_U8 nOffGR;
    //!< Offset value for GB
    OMX_U8 nOffGB;
    //!< Offset value for B
    OMX_U8 nOffB;
    //<! LSC_SHIFT
    OMX_U8 nShift;
    //<! LSC_MAX
    OMX_U16 nMax;
} OMX_TI_3ASKIP_IPIPE_LSC_CFGTYPE;

typedef struct {
    OMX_U16 nVPos;
    OMX_U16 nVSize;
    OMX_U16 nHPos;
    OMX_U16 nHSize;
} OMX_TI_3ASKIP_IPIPE_HIST_DIMTYPE;

//!< ISS configuration structure for Histogram in ipipe
typedef struct {
    //!< Enable-disable for Histogram in ipipe
    OMX_U8 nEnable;
    OMX_U8 nOst;
    OMX_U8 nSel;
    OMX_U8 nType;
    OMX_U8 nBins;
    OMX_U8 nShift;
    //!< Bits [3:0], 0 is disable
    OMX_U8 nCol;
    //!< [3:0], 0 is disable
    OMX_U8 nRegions;
    //!< Pointer to array of 4 structs
    OMX_TI_3ASKIP_IPIPE_HIST_DIMTYPE pHistDim[OMX_ISS_HIST_DIMS_COUNT];
    OMX_U8                           nClearTable;
    OMX_U8                           nTableSel;
    //!< r,gr,gb,b
    OMX_U8 pGainTbl[OMX_ISS_HIST_GAIN_TBL];
} OMX_TI_3ASKIP_IPIPE_HIST_CFGTYPE;

typedef enum {
    OMX_BOXCAR_DISABLED     = 0,
    OMX_BOXCAR_ENABLED      = 1,
    OMX_BOXCAR_ENB_DSB_MAX  = 0x7FFFFFFF
} OMX_TI_3ASKIP_BOXCAR_ENABLETYPE;

typedef enum {
    OMX_BOXCAR_FREE_RUN     = 0,
    OMX_BOXCAR_ONE_SHOT     = 1,
    OMX_BOXCAR_FREE_ONE_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_BOXCAR_MODETYPE;

typedef enum {
    OMX_BOXCAR_8x8      = 0,
    OMX_BOXCAR_16x16    = 1,
    OMX_BOXCAR_SIZE_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_BOXCAR_SIZETYPE;

typedef struct {
    OMX_U8 nVectors;
    OMX_U8 nShift;
    //!< Vertical position
    OMX_U16 nVPos;
    //!< Horizontal position
    OMX_U16 nHPos;
    //!< Vertical number
    OMX_U16 nVNum;
    //!< Horizontal number
    OMX_U16 nHNum;
    //!< Horizontal skip
    OMX_U8 nVSkip;
    //!< Vertical skip
    OMX_U8 nHSkip;
} OMX_TI_3ASKIP_IPIPE_BSCPOS_PARAMSTYPE;

typedef struct {
    //!< Enable-disable for BSC in ipipe
    OMX_U8 nEnable;
    //!< BSC mode in ipipe
    OMX_U8 nMode;
    //!< BSC Color Sample
    OMX_U8 nColSample;
    //!< BSC Row Sample
    OMX_U8 nRowSample;
    //!< Y or CB or CR
    OMX_U8 nElement;
    //!< Color Position parameters
    OMX_TI_3ASKIP_IPIPE_BSCPOS_PARAMSTYPE nColPos;
    //!< Row Position parameters
    OMX_TI_3ASKIP_IPIPE_BSCPOS_PARAMSTYPE nRowPos;
} OMX_TI_3ASKIP_IPIPE_BSC_CFGTYPE;

//!< Enable-disable enum for DFS in ipipeif
typedef enum {
    OMX_IPIPEIF_FEATURE_ENABLE  = 1,
    OMX_IPIPEIF_FEATURE_DISABLE = 0,
    OMX_IPIPEIF_FEATURE_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE;

typedef struct {
    //!< Enable-disable for DFS in ipipeif
    OMX_TI_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE eDfsGainEn;
    //!< Valid only if eDfsGainEn = OMX_IPIPEIF_FEATURE_ENABLE
    OMX_U16 nDfsGainVal;
    //!< Valid only if eDfsGainEn = OMX_IPIPEIF_FEATURE_ENABLE
    OMX_U16 nDfsGainThr;
    //!< Valid only if eDfsGainEn = OMX_IPIPEIF_FEATURE_ENABLE
    OMX_U16 nOclip;
    //!< Set to 0 if Sensor Parallel interface data is
    //!< to be subtracted by DRK frm in SDRAM
    OMX_U8 nDfsDir;
} OMX_TI_3ASKIP_IPIPEIF_DFS_CFGTYPE;

//!< ISS struct to control defect pixel corrction in ipipeif
typedef struct {
    //!< Enable-disable for DPC in ipipeif
    OMX_TI_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE eDpcEn;
    OMX_U16                                  eDpcThr;
} OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE;

typedef enum {
    OMX_IPIPEIF_DPCM_PREDICTION_SIMPLE   = 0,
    OMX_IPIPEIF_DPCM_PREDICTION_ADVANCED = 1,
    OMX_IPIPEIF_DPCM_PREDICTION_MAX      = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE;

typedef enum {
    OMX_IPIPEIF_DPCM_BIT_SIZE_8_10 = 0,
    OMX_IPIPEIF_DPCM_BIT_SIZE_8_12 = 1,
    OMX_IPIPEIF_DPCM_BIT_SIZE_MAX = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE;

typedef struct {
    //!< Enable-disable for DPCM in ipipeif
    OMX_TI_3ASKIP_IPIPEIF_FEATURE_SELECTTYPE nDpcmEn;
    //!< Valid only if DPCM is enabled; dpcm_en=1
    OMX_TI_3ASKIP_IPIPEIF_DPCM_PRED_TYPTYPE nDpcmPredictor;
    //!< Valid only if DPCM is enabled; dpcm_en=1
    OMX_TI_3ASKIP_IPIPEIF_DPCM_BIT_SIZETYPE nDpcmBitSize;
} OMX_TI_3ASKIP_IPIPEIF_DPCM_CFGTYPE;

typedef enum {
    OMX_RSZ_IP_IPIPE   = 0,
    OMX_RSZ_IP_IPIPEIF = 1,
    OMX_RSZ_IP_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_RSZ_IP_PORT_SELTYPE;

typedef enum {
    OMX_MEM_INPUT_IPIPEIF,
    OMX_MEM_INPUT_CCP,
    OMX_MEM_INPUT_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_MEM_PROC_INPUT_SELECTTYPE;

//!< ISS low pass filter params for Horizontal resizing
typedef struct {
    OMX_U8 nCIntensity;
    OMX_U8 nYIntensity;
} OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE;

//!< Enable-Disable H3A Features
typedef enum {
    OMX_H3A_FEATURE_DISABLE = 0,
    OMX_H3A_FEATURE_ENABLE  = 1,
    OMX_H3A_FEATURE_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE;

typedef struct {
    //!< AEWINBLK WINSV single row of black line vpos
    OMX_U16 nVPos;
    //!< AEWINBLK WINH  win height
    OMX_U16 nHPos;
} OMX_TI_3ASKIP_H3A_AEWB_BLKDIMSTYPE;

typedef struct {
    OMX_U8 pFirCoef[5];
    OMX_U8 nVfvThres;
} OMX_TI_3ASKIP_H3A_AF_FIRPARAMTYPE;

typedef struct {
    //!< Enable-disable for H3A AF Median engine
    OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE eAfMedianEn;
    //!< Enable-disable for H3A AEWB Median engine
    OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE eAewbMedianEn;
    //!< Valid only if eAfMedianEn is set to OMX_H3A_FEATURE_ENABLE
    OMX_U8 nMedianFilterThreshold;
    //!< Enable-disable for AFAlaw engine
    OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE eAfAlawEn;
    //!< Enable-disable for AEWBAlaw engine
    OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE eAewbAlawEn;
    //!< Enable-disable for IPIPEIFAve filter engine
    OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE eIpipeifAveFiltEn;
    //!< Enable-disable for H3A decimation
    OMX_TI_3ASKIP_H3A_FEATURE_ENABLETYPE eH3aDecimEnable;
    //!< Reserved
    OMX_U32 nReserved;
} OMX_TI_3ASKIP_H3A_COMMON_CFGTYPE;

typedef enum {
    OMX_IPIPE_DPC_OTF_ALG_MINMAX2 = 0,
    OMX_IPIPE_DPC_OTF_ALG_MINMAX3 = 1,
    OMX_IPIPE_DPC_OTF_ALG_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_DPC_ALGOTYPE;

typedef enum {
    OMX_IPIPE_DPC_OTF_MAX1_MIN1 = 0,
    OMX_IPIPE_DPC_OTF_MAX2_MIN2 = 1,
    OMX_IPIPE_DPC_OTF_MAX       = 0x7FFFFFFF
} OMX_TI_3ASKIP_IPIPE_DPC_OTFTYPE;

typedef struct {
    OMX_U16 thr_cor_r;
    OMX_U16 thr_cor_gr;
    OMX_U16 thr_cor_gb;
    OMX_U16 thr_cor_b;

    OMX_U16 thr_det_r;
    OMX_U16 thr_det_gr;
    OMX_U16 thr_det_gb;
    OMX_U16 thr_det_b;
} OMX_TI_3ASKIP_IPIPE_DPCOTF_DPC2TYPE;

typedef struct {
    OMX_U8  nDThr;
    OMX_U8  nDSlp;
    OMX_U16 nDMin;
    OMX_U16 nDMax;
} OMX_TI_3ASKIP_IPIPE_DPCOFT_FILTERTYPE;

typedef struct {
    OMX_U8                                eShift;
    OMX_TI_3ASKIP_IPIPE_DPCOFT_FILTERTYPE eOtfCorr;
    OMX_TI_3ASKIP_IPIPE_DPCOFT_FILTERTYPE eOtfDett;
} OMX_TI_3ASKIP_IPIPE_DPCOTF_DPC3TYPE;

/* Union which helps selec either dpc2-dpc 3 params */
typedef union {
    OMX_TI_3ASKIP_IPIPE_DPCOTF_DPC2TYPE tDpc2Params;
    OMX_TI_3ASKIP_IPIPE_DPCOTF_DPC3TYPE tDpc3Params;
} OMX_TI_3ASKIP_IPIPE_DPCOTF_FILTER_PARAMSTYPE;

typedef struct {
    //!< Enable-disable for DPC
    OMX_U8 nEnable;
    //!< ISS dpc otf type
    OMX_TI_3ASKIP_IPIPE_DPC_OTFTYPE eType;
    //!< ISS dpc otf definitions
    OMX_TI_3ASKIP_IPIPE_DPC_ALGOTYPE eAlgo;
    //!< Union which helps selec either dpc2-dpc 3 params
    OMX_TI_3ASKIP_IPIPE_DPCOTF_FILTER_PARAMSTYPE tDpcData;
} OMX_TI_3ASKIP_IPIPE_DPCOTF_CFGTYPE;

typedef struct {
    OMX_U16 thr;
    OMX_U16 gain;
    OMX_U16 shift;
    OMX_U16 min;
} OMX_TI_3ASKIP_IPIPE_CHROMA_PARAMSTYPE;

typedef struct {
    //!< Enable-disable for CGC in ipipe
    OMX_U16                               enable;
    OMX_TI_3ASKIP_IPIPE_CHROMA_PARAMSTYPE y_chroma_low;
    OMX_TI_3ASKIP_IPIPE_CHROMA_PARAMSTYPE y_chroma_high;
    OMX_TI_3ASKIP_IPIPE_CHROMA_PARAMSTYPE c_chroma;
} OMX_TI_3ASKIP_IPIPE_CGS_CFGTYPE;

typedef enum {
    OMX_TRANSFER_ONLINE  = 0,
    OMX_TRANSFER_OFFLINE = 1,
    OMX_TRANSFER_MAX     = 0x7FFFFFFF
} OMX_TI_3ASKIP_CAM_TRANSFERTYPE;

typedef struct {
    OMX_U32 exp;
    OMX_U32 a_gain;
    OMX_U8  mask;
    OMX_U32 nAgainErr;
    OMX_U32 nDigitalISPGain;
} OMX_TI_3ASKIP_CAM_CONTROL_EXPGAINTYPE;

typedef enum {
    /* IDs for L sensor and mono usecases */
    OMX_TI_3aSkipIndexLLeftStart = 0,
    OMX_TI_3aSkipIndexL_ColorPattern = OMX_TI_3aSkipIndexLLeftStart,    /**< 0x00000000 reference: OMX_TI_3ASKIP_IPIPE_BAYER_PATTERNTYPE */
    OMX_TI_3aSkipIndexL_MsbPos,             /**< 0x00000001 reference: OMX_TI_3ASKIP_IPIPE_BAYER_MSB_POSTYPE */
    OMX_TI_3aSkipIndexL_VpDevice,           /**< 0x00000002 reference: OMX_TI_3ASKIP_IPIPE_VP_DEVICETYPE */

    OMX_TI_3aSkipIndexL_Lsc2D,              /**< 0x00000003 reference: OMX_TI_3ASKIP_ISIF_2DLSC_CFGTYPE */
    OMX_TI_3aSkipIndexL_Clamp,              /**< 0x00000004 reference: OMX_TI_3ASKIP_ISIF_CLAMP_CFGTYPE */
    OMX_TI_3aSkipIndexL_Flash,              /**< 0x00000005 reference: OMX_TI_3ASKIP_ISIF_FLASH_CFGTYPE */
    OMX_TI_3aSkipIndexL_GainOffset,         /**< 0x00000006 reference: OMX_TI_3ASKIP_ISIF_GAINOFFSET_CFGTYPE */
    OMX_TI_3aSkipIndexL_Vlcd,               /**< 0x00000007 reference: OMX_TI_3ASKIP_ISIF_VLDC_CFGTYPE */

    OMX_TI_3aSkipIndexL_Nf1,                /**< 0x00000008 reference: OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE */
    OMX_TI_3aSkipIndexL_Nf2,                /**< 0x00000009 reference: OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE */
    OMX_TI_3aSkipIndexL_GIC,                /**< 0x0000000A reference: OMX_TI_3ASKIP_IPIPE_GIC_CFGTYPE */
    OMX_TI_3aSkipIndexL_WB,                 /**< 0x0000000B reference: OMX_TI_3ASKIP_IPIPE_WB_CFGTYPE */
    OMX_TI_3aSkipIndexL_CFA,                /**< 0x0000000C reference: OMX_TI_3ASKIP_IPIPE_CFA_CFGTYPE */
    OMX_TI_3aSkipIndexL_Gamma,              /**< 0x0000000D reference: OMX_TI_3ASKIP_IPIPE_GAMMA_CFGTYPE */
    OMX_TI_3aSkipIndexL_Rgb2Rgb1,           /**< 0x0000000E reference: OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE */
    OMX_TI_3aSkipIndexL_Rgb2Rgb2,           /**< 0x0000000F reference: OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE */
    OMX_TI_3aSkipIndexL_Rgb2Yuv,            /**< 0x00000010 reference: OMX_TI_3ASKIP_IPIPE_RGBYUV_CFGTYPE */
    OMX_TI_3aSkipIndexL_GBCE,               /**< 0x00000011 reference: OMX_TI_3ASKIP_IPIPE_GBCE_CFGTYPE */
    OMX_TI_3aSkipIndexL_Yuv2Yuv,            /**< 0x00000012 reference: OMX_TI_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE */
    OMX_TI_3aSkipIndexL_Ee,                 /**< 0x00000013 reference: OMX_TI_3ASKIP_IPIPE_EE_CFGTYPE */
    OMX_TI_3aSkipIndexL_Car,                /**< 0x00000014 reference: OMX_TI_3ASKIP_IPIPE_CAR_CFGTYPE */
    OMX_TI_3aSkipIndexL_Lsc,                /**< 0x00000015 reference: OMX_TI_3ASKIP_IPIPE_LSC_CFGTYPE */
    OMX_TI_3aSkipIndexL_Histogram,          /**< 0x00000016 reference: OMX_TI_3ASKIP_IPIPE_HIST_CFGTYPE */
    OMX_TI_3aSkipIndexL_Bsc,                /**< 0x00000017 reference: OMX_TI_3ASKIP_IPIPE_BSC_CFGTYPE */
    OMX_TI_3aSkipIndexL_DpcOtf,             /**< 0x00000018 reference: OMX_TI_3ASKIP_IPIPE_DPCOTF_CFGTYPE */
    OMX_TI_3aSkipIndexL_Cgs,                /**< 0x00000019 reference: OMX_TI_3ASKIP_IPIPE_CGS_CFGTYPE */

    OMX_TI_3aSkipIndexL_Dfs,                /**< 0x0000001A reference: OMX_TI_3ASKIP_IPIPEIF_DFS_CFGTYPE */
    OMX_TI_3aSkipIndexL_Dpc1,               /**< 0x0000001B reference: OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE */
    OMX_TI_3aSkipIndexL_Dpc2,               /**< 0x0000001C reference: OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE */
    OMX_TI_3aSkipIndexL_Dpcm,               /**< 0x0000001D reference: OMX_TI_3ASKIP_IPIPEIF_DPCM_CFGTYPE */

    OMX_TI_3aSkipIndexL_HLpf,               /**< 0x0000001E reference: OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE */
    OMX_TI_3aSkipIndexL_VLpf,               /**< 0x0000001F reference: OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE */

    OMX_TI_3aSkipIndexL_H3aCommonParams,    /**< 0x00000020 reference: OMX_TI_3ASKIP_H3A_COMMON_CFGTYPE */
    OMX_TI_3aSkipIndexL_CamControlExpGain,  /**< 0x00000021 reference: OMX_TI_3ASKIP_CAM_CONTROL_EXPGAINTYPE */
    OMX_TI_3aSkipIndexLLeftMax,             /**< 0x00000022 */

    /* IDs for R sensor */
    OMX_TI_3aSkipIndexRightStart = 0x01000000,
    OMX_TI_3aSkipIndexR_ColorPattern = OMX_TI_3aSkipIndexRightStart,    /**< 0x01000000 reference: OMX_TI_3ASKIP_IPIPE_BAYER_PATTERNTYPE */
    OMX_TI_3aSkipIndexR_MsbPos,             /**< 0x01000001 reference: OMX_TI_3ASKIP_IPIPE_BAYER_MSB_POSTYPE */
    OMX_TI_3aSkipIndexR_VpDevice,           /**< 0x01000002 reference: OMX_TI_3ASKIP_IPIPE_VP_DEVICETYPE */

    OMX_TI_3aSkipIndexR_Lsc2D,              /**< 0x01000003 reference: OMX_TI_3ASKIP_ISIF_2DLSC_CFGTYPE */
    OMX_TI_3aSkipIndexR_Clamp,              /**< 0x01000004 reference: OMX_TI_3ASKIP_ISIF_CLAMP_CFGTYPE */
    OMX_TI_3aSkipIndexR_Flash,              /**< 0x01000005 reference: OMX_TI_3ASKIP_ISIF_FLASH_CFGTYPE */
    OMX_TI_3aSkipIndexR_GainOffset,         /**< 0x01000006 reference: OMX_TI_3ASKIP_ISIF_GAINOFFSET_CFGTYPE */
    OMX_TI_3aSkipIndexR_Vlcd,               /**< 0x01000007 reference: OMX_TI_3ASKIP_ISIF_VLDC_CFGTYPE */

    OMX_TI_3aSkipIndexR_Nf1,                /**< 0x01000008 reference: OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE */
    OMX_TI_3aSkipIndexR_Nf2,                /**< 0x01000009 reference: OMX_TI_3ASKIP_IPIPE_NOISE_FILTER_CFGTYPE */
    OMX_TI_3aSkipIndexR_GIC,                /**< 0x0100000A reference: OMX_TI_3ASKIP_IPIPE_GIC_CFGTYPE */
    OMX_TI_3aSkipIndexR_WB,                 /**< 0x0100000B reference: OMX_TI_3ASKIP_IPIPE_WB_CFGTYPE */
    OMX_TI_3aSkipIndexR_CFA,                /**< 0x0100000C reference: OMX_TI_3ASKIP_IPIPE_CFA_CFGTYPE */
    OMX_TI_3aSkipIndexR_Gamma,              /**< 0x0100000D reference: OMX_TI_3ASKIP_IPIPE_GAMMA_CFGTYPE */
    OMX_TI_3aSkipIndexR_Rgb2Rgb1,           /**< 0x0100000E reference: OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE */
    OMX_TI_3aSkipIndexR_Rgb2Rgb2,           /**< 0x0100000F reference: OMX_TI_3ASKIP_IPIPE_RGBRGB_CFGTYPE */
    OMX_TI_3aSkipIndexR_Rgb2Yuv,            /**< 0x01000010 reference: OMX_TI_3ASKIP_IPIPE_RGBYUV_CFGTYPE */
    OMX_TI_3aSkipIndexR_GBCE,               /**< 0x01000011 reference: OMX_TI_3ASKIP_IPIPE_GBCE_CFGTYPE */
    OMX_TI_3aSkipIndexR_Yuv2Yuv,            /**< 0x01000012 reference: OMX_TI_3ASKIP_IPIPE_YUV444YUV422_CFGTYPE */
    OMX_TI_3aSkipIndexR_Ee,                 /**< 0x01000013 reference: OMX_TI_3ASKIP_IPIPE_EE_CFGTYPE */
    OMX_TI_3aSkipIndexR_Car,                /**< 0x01000014 reference: OMX_TI_3ASKIP_IPIPE_CAR_CFGTYPE */
    OMX_TI_3aSkipIndexR_Lsc,                /**< 0x01000015 reference: OMX_TI_3ASKIP_IPIPE_LSC_CFGTYPE */
    OMX_TI_3aSkipIndexR_Histogram,          /**< 0x01000016 reference: OMX_TI_3ASKIP_IPIPE_HIST_CFGTYPE */
    OMX_TI_3aSkipIndexR_Bsc,                /**< 0x01000017 reference: OMX_TI_3ASKIP_IPIPE_BSC_CFGTYPE */
    OMX_TI_3aSkipIndexR_DpcOtf,             /**< 0x01000018 reference: OMX_TI_3ASKIP_IPIPE_DPCOTF_CFGTYPE */
    OMX_TI_3aSkipIndexR_Cgs,                /**< 0x01000019 reference: OMX_TI_3ASKIP_IPIPE_CGS_CFGTYPE */

    OMX_TI_3aSkipIndexR_Dfs,                /**< 0x0100001A reference: OMX_TI_3ASKIP_IPIPEIF_DFS_CFGTYPE */
    OMX_TI_3aSkipIndexR_Dpc1,               /**< 0x0100001B reference: OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE */
    OMX_TI_3aSkipIndexR_Dpc2,               /**< 0x0100001C reference: OMX_TI_3ASKIP_IPIPEIF_DPC_CFGTYPE */
    OMX_TI_3aSkipIndexR_Dpcm,               /**< 0x0100001D reference: OMX_TI_3ASKIP_IPIPEIF_DPCM_CFGTYPE */

    OMX_TI_3aSkipIndexR_HLpf,               /**< 0x0100001E reference: OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE */
    OMX_TI_3aSkipIndexR_VLpf,               /**< 0x0100001F reference: OMX_TI_3ASKIP_RSZ_LPF_CFGTYPE */

    OMX_TI_3aSkipIndexR_H3aCommonParams,    /**< 0x01000020 reference: OMX_TI_3ASKIP_H3A_COMMON_CFGTYPE */
    OMX_TI_3aSkipIndexR_CamControlExpGain,  /**< 0x01000021 reference: OMX_TI_3ASKIP_CAM_CONTROL_EXPGAINTYPE */
    OMX_TI_Index3aSkipRightMax,             /**< 0x01000022 */
    OMX_TI_Index3aSkip_MAX = 0x7FFFFFFF

} OMX_TI_3ASKIP_TI_3ASKIPINDEXTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


