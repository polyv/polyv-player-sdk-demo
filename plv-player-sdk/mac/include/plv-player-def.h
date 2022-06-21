#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define PLAYERSDK_CALL __cdecl
#if defined(PLAYERSDK_EXPORT)
#define PLAYERSDK_API extern "C" __declspec(dllexport)
#else
#define PLAYERSDK_API extern "C" __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#define PLAYERSDK_API __attribute__((visibility("default"))) extern "C"
#define PLAYERSDK_CALL
#elif defined(__ANDROID__) || defined(__linux__)
#define PLAYERSDK_API extern "C" __attribute__((visibility("default")))
#define PLAYERSDK_CALL
#else
#define PLAYERSDK_API extern "C"
#define PLAYERSDK_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** the device string length. */
const int PLV_MAX_DEVICE_ID_LENGTH = 512;

/** the video rate. */
enum VIDEO_RATE_TYPE{
	/** auto to set rate. */
	VIDEO_RATE_AUTO = 0,///< only in play can use,do not use in download. 

	VIDEO_RATE_SD,///< Video standard definition.
	VIDEO_RATE_HD,///< Video HD.
	VIDEO_RATE_BD,///< Video ultra clear.
};

/** Output log filter level. */
enum LOG_FILTER_TYPE{
	/** Do not output any log information. */
	LOG_FILTER_OFF = 0,

	/** Output all log information.
	* Set your log filter as debug if you want to get the most complete log file. 
	*/
	LOG_FILTER_DEBUG,
	/** Output FATAL, ERROR, WARNING, and INFO level log information.
	 * We recommend setting your log filter as this level.
	 */
	LOG_FILTER_INFO ,
	/** Outputs FATAL, ERROR, and WARNING level log information.
	 */
	LOG_FILTER_WARN ,
	/** Outputs FATAL and ERROR level log information. */
	LOG_FILTER_ERROR,
	/** Outputs FATAL level log information. */
	LOG_FILTER_FATAL,
};

/** HDMI Device Changed type */
enum HDMI_DEVICE_TYPE {
	/** no hdmi device. */
	HDMI_DEVICE_NONE = 0,
	/** the device can use. */
	HDMI_DEVICE_USE,
	/** the device can not use. */
	HDMI_DEVICE_UNUSE,
};

/** Video output drivers are interfaces to different video output facilities. 
*@note	If there is no video problem, it is generally not recommended to use.
*/
enum VIDEO_OUTPUT_DEVICE{
	/** default system. */
	VIDEO_OUTPUT_NONE = 0,

	/** Shader - based GPU Renderer
	* General purpose, customizable, GPU-accelerated video output driver. 
	* It supports extended scaling methods, dithering, color management, custom shaders, HDR, and more.
	*/
	VIDEO_OUTPUT_GPU,

	/** Direct3D 9 Renderer
	* Video output driver that uses the Direct3D interface.
	* This driver is for compatibility with systems that don't provide proper OpenGL drivers, and where ANGLE does not perform well.
	* @note only use in windows
	*/
	VIDEO_OUTPUT_DIRECT3D,
};

/** first http request. */
enum SDK_HTTP_REQUEST{
	/** will use first use http to request data, if fails, use it again https. */
	FIRST_HTTP_REQUEST = 0,
	/** will use first use https to request data, if fails, use it again http.
	* @note default value.
	*/
	FIRST_HTTPS_REQUEST,
	/** only use http protocol. */
	ONLY_HTTP_REQUEST,
	/** only use https protocol. */
	ONLY_HTTPS_REQUEST,
};

/** the error code. */
enum SDK_ERROR_TYPE{
	E_NO_ERR = 0,///< success.
	E_NO_INIT,///< sdk no init.
	E_NO_FILE,///< no this file.
	E_NO_RATE,///< the video no this rate.
	E_NO_SPACE ,///< user no space.
	E_NO_SUPPORT,///< sdk no support.
	E_VID_ERR,///< the vid format error.
	E_KEY_ERR,///< download key file error or key format error.
	E_ZIP_ERR,///< download zip file error or decompression error.
	E_M3U8_ERR,///< download m3u8 file error or m3u8 format error.
	E_HTTP_ERR,///< http request error.
	E_PATH_ERR,///< the path is illegal.
	E_PARAM_ERR,///< the param is illegal.
	E_DELETE_ERR,///< delete file error.
	E_INVOKE_ERR,///< invoke api error.
	E_NETWORK_ERR,///< network error.
	E_DOWNLOAD_ERR,///< download error, will be open file error, or path error.
	E_DOWNLOADING,///< the object is downloading.
	E_ABORT_DOWNLOAD,///< user abort download.
	E_DELETE_VIDEO,///< cancel download and delete.
	E_MEDIA_UNINIT,///< the mdeia not init(no set vid).
	E_MEDIA_UNLOAD,///< the media is unload.
	E_MEDIA_LOADING,///< the media is loading.
	E_TSFILESIZE,///< ts file size is zero.
	E_OUTFLOW, ///< the media return outflow.
	E_OSDFONT_ERR,///< no font file.
	E_FILE_INCOMPLETE,///< file incomplete.
	E_OBJECT_NULL,///< the object is nullptr.
	E_NO_AUDIO_DEVICE,///< no audio device.

	E_ERROR_MAX,///< over this value will be core code error.
};

/** the player media state. */
enum PLAYER_MEDIA_STATE {
	MEDIA_STATE_LOADING = 0,///< the player media file is loading.
	MEDIA_STATE_LOADED,///< the player media file is loaded.
	MEDIA_STATE_PLAY,///< playing the media.
	MEDIA_STATE_PAUSE,///< pause the media.
	MEDIA_STATE_BEGIN_CACHE,///< the media is begin cache.
	MEDIA_STATE_END_CACHE,///< the media is end cache.
	MEDIA_STATE_BEGIN_SEEKING,///< the media is begin seeking.
	MEDIA_STATE_END_SEEKING,///< the media is end seeking.
	MEDIA_STATE_FAIL,///< the media play fail, see log.
	MEDIA_STATE_END,///< the media is play end.
};
/** the player media property. */
enum PLAYER_MEDIA_PROPERTY {
	MEDIA_PROPERTY_DURATION = 0,///< the media duration.
	MEDIA_PROPERTY_POSTION,///< the media play pos. @attention no use, please use callback.
	MEDIA_PROPERTY_HWDEC,///< the media Hardware coding name.
	MEDIA_PROPERTY_VIDEO_CODEC,///< the video codec name.
	MEDIA_PROPERTY_VIDEO_BITRATE,///< the video bitrate.
	MEDIA_PROPERTY_VIDEO_FPS,///< the video fps.
	MEDIA_PROPERTY_VIDEO_WIDTH,///< the video resolution width.
	MEDIA_PROPERTY_VIDEO_HEIGHT,///< the video resolution height.
	MEDIA_PROPERTY_AUDIO_CODEC,///< the audio codec name.
	MEDIA_PROPERTY_AUDIO_BITRATE,///< the audio bitrate.
	MEDIA_PROPERTY_CACHE_SPEED,///< cache speed , how many bytes per second MB/S  speed/1024*1024.
	MEDIA_PROPERTY_CACHE_PROGRESS, ///< cache progress, percent 100%.
};
/** the player media data format. */
enum PLAYER_MEDIA_FORMAT {
	MEDIA_FORMAT_INT64 = 0,///< int type.
	MEDIA_FORMAT_DOUBLE,///< double type.
	MEDIA_FORMAT_STRING,///< string type.
};

/** OSD display type. */
enum OSD_DISPLAY_TYPE {
	OSD_DISPALY_ROLL = 0,///< Scroll right to left.
	OSD_DISPALY_BLINK,///< Random position flicker.
};

/** OSD info setting. */
struct OSDConfigInfo {
	OSDConfigInfo() :
		text(0),
		textSize(16),
		textColor(0),
		textAlpha(255),
		border(false),
		borderColor(0),
		borderAlpha(255),
		borderWidth(5),
		animationEffect(OSD_DISPALY_ROLL),
		displayDuration(5),
		displayInterval(1),
		fadeDuration(1)
	{
	}
	const char* text;///< osd text display, must set. use utf8.
	int textSize;///< font size, [1, 255], default 55.
	const char* textColor;///< font color RGB, default #000000.
	int textAlpha;///< text transparency, [0, 255], default 255.
	bool border;///< border or not, default false.
	const char* borderColor;///< border color RGB.
	int borderAlpha;///< border transparency,[0, 255], default 255.
	int borderWidth;///< border width, [1, 255], default 1.
	OSD_DISPLAY_TYPE animationEffect;///< animation effect of running lantern, default OSD_DISPALY_ROLL.@see OSD_DISPLAY_TYPE.
	/**
	@note OSD_DISPALY_ROLL:roll indicates the duration of a single roll (from the beginning of roll in to full roll out).
	@note OSD_DISPALY_BLINK:blink indicates the time required from the beginning of display to the complete disappearance.
	*/
	int displayDuration;///< The duration of a single running lantern, in seconds.
	/**
	@note OSD_DISPALY_ROLL:roll indicates the time interval between two scrolls (from full roll out to next roll in).
	@note OSD_DISPALY_BLINK:blink indicates the interval between two flashes (the interval from complete disappearance to next appearance).
	*/
	int displayInterval;///< The time interval between two running lights, in seconds.
	/**	
	@note When the value is 0, the gradient effect is not displayed.
	@note When fadeduration < displayduration, the gradient effect is cycled.
	@note It works for roll and blink.
	*/
	int fadeDuration;///< The duration of the gradient animation of the lantern text from display to disappearance, in seconds.
};


/** logo text setting. */
struct LogoTextInfo {
	LogoTextInfo() :
		text(0),
		textFontName(0),
		textSize(16),
		textColor(0),
		borderWidth(1),
		borderColor(0),	
		alignX(1),
		alignY(-1)
	{
	}
	const char* text;///< logo text display, must set. use utf8.
	const char* textFontName;///< text font name, can be null, use system.
	int textSize;///< font size, [1, 255], default 16.
	const char* textColor;///< font color ARGB, default #FFFFFFFF.
	int borderWidth;///< border width, [0, 255], A value of 0 disables borders, default 1.
	const char* borderColor;///< border color ARGB, default #FF000000.	
	int alignX;///< [-1 0 1] left center right, default 1.
	int alignY;///< [-1 0 1] top center bottom, default -1.
};

#ifdef __cplusplus
}
#endif

