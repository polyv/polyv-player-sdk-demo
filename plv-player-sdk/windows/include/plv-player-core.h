#pragma once
#include "plv-player-def.h"

///////////////////////////////////////////////////////////////////
/** @defgroup Global Global Function
* @{
*/
/**
* @brief get the sdk version.
* @return 
* - string: SDK version.
*/
PLAYERSDK_API const char* PLAYERSDK_CALL PLVGetSdkVersion(void);

/**
* @brief get the code description.
* @param err: error code @see SDK_ERROR_TYPE.
* @return
* - string: description of the error code.
*/
PLAYERSDK_API const char* PLAYERSDK_CALL PLVGetSdkErrorDescription(int err);

/**
* @brief check the video is completed.
* @param vid: the video id.
* @param videoFilePath: the video file path, use UTF8.
* @param rate: the video rate. @see VIDEO_RATE_TYPE.
* @return
* - true: the video is complete.
* - false: the video is no complete.
* @note if the file is mp4, only check the file is exist.
* @attention path must use UTF8.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVCheckFileComplete(const char* vid, const char* videoFilePath, int rate);

/**
* @brief set the sdk log file.
* @param fileName: the log file, use UTF8.
* @return
* - 0: Success.
* - > 0: Failure.
* @note if not have call, will create in %appdata%/plv-player-sdk/logs/.
* @attention path must use UTF8.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkLogFile(const char* fileName);

/**
* @brief set the sdk log filter level.
* @param level: the log level. @see LOG_FILTER_TYPE.
* @return
* - 0: Success.
* - > 0: Failure.
* @note default value for LOG_FILTER_INFO. 
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkLogLevel(LOG_FILTER_TYPE level);

/**
* @brief for thd sdk http request type.
* @param type: http protocol first request type. @see FIRST_HTTPS_REQUEST.
* @return
* - 0: Success.
* - > 0: Failure.
* @note default value for FIRST_HTTPS_REQUEST. 
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkHttpRequest(SDK_HTTP_REQUEST type);

/**
 * @brief set upload ca file, if use https, must set the ca file, otherwise error.
 * @param fileName: the ca file,include path. use UTF8.
* @return
* - 0: Success.
* - > 0: Failure.
* @note if not have call, will find in app path.
* @attention path must use UTF8
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkCacertFile(const char* fileName);

/** 
* @brief Hardware coding.
* @param enable: video hardware speedup enable.
* @return
* - 0: Success.
* - > 0: Failure.
* @note if not have call will default hwdec enable.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkHwdecEnable(bool enable);

/** 
* @brief end video will keep in last frame.
* @param enable: open or no.
* @return
* - 0: Success.
* - > 0: Failure.
* @note if not have call will default false.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkKeepLastFrame(bool enable);

/** 
* @brief Video output drivers are interfaces to different video output facilities. 
*	If there is no video problem, it is generally not recommended to use.
* @param type: output type. @see VIDEO_OUTPUT_DEVICE.
* @param context: it can be "auto, d3d11, win, angle, dxinterop".
* @return
* - 0: Success.
* - > 0: Failure.
* @note @arg type for VIDEO_OUTPUT_GPU, the context can set value.
* @par Sample
* @code
*	int ret = PLVSetSdkVideoOutputDevice(VIDEO_OUTPUT_GPU, "d3d11");
* @endcode
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkVideoOutputDevice(VIDEO_OUTPUT_DEVICE type, const char* context = NULL);

/**
* @brief this api will use report qos, recommended settings.
* @param viewerId: user custom viewer id, use in location quality.
* @param viewerName: user custom viewer name.
* @param viewerAvatar: user custom viewer avatar.
* @param viewerExtraInfo1: extend field.
* @param viewerExtraInfo2: extend field.
* @param viewerExtraInfo3: extend field.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVSetSdkViewerInfo(const char* viewerId, const char* viewerName, const char* viewerAvatar,
	const char* viewerExtraInfo1 = NULL, const char* viewerExtraInfo2 = NULL, const char* viewerExtraInfo3 = NULL);

/**
* @brief init the sdk.
* @param userId: user id, Get from SDK provider.
* @param secretKey: user secret key, Get from SDK provider.
* @param readToken: user read token, Get from SDK provider.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVInitSdkLibrary(const char* userId, const char* secretKey, const char* readToken);

/**
* @brief release sdk.
*/
PLAYERSDK_API void PLAYERSDK_CALL PLVReleaseSdkLibrary(void);
/**
* @}
*/

///////////////////////////////////////////////////////////////////
/** @defgroup Record Software Record or Hardware Record Function 
* @{
*/
#ifdef _WIN32
/**
* Only support windows platform
*/
/**
 * @brief for prevent screen recording or screenshot. 
 * @param window: A handle to the top-level window, must be top-level eg: win WS_EX_APPWINDOW.
 * @param enable: will append to the queue, to prevent or not, and remove the queue.
 * @return
 * - 0: Success.
 * - > 0: Failure.
 */
PLAYERSDK_API int PLAYERSDK_CALL PLVSetPreventRecord(void* window, bool enable);
/**
 * @brief for prevent screen recording or screenshot.
 * @param window: A handle to the top-level window, must be top-level eg: win WS_EX_APPWINDOW.
 * @param[out] enable: to prevent or not.
 * @return
 * - 0: Success.
 * - > 0: Failure.
 */
PLAYERSDK_API int PLAYERSDK_CALL PLVGetPreventRecord(void* window, bool* enable);
/**
 * @brief for plugin inject callback.
 * @param[out] data: user context.
*/
typedef void(*OnPluginInjectHandler)(void* data);
/** 
 * @brief for check plugin inject to record your screen.
 * @param enable: for check to callback or not.
 * @param handler: callback function.
 * @param data: user context data.
 * @return
 * - 0: Success.
 * - > 0: Failure.
 */
PLAYERSDK_API int PLAYERSDK_CALL PLVSetPluginInjectHandler(bool enable, OnPluginInjectHandler handler, void* data);

///////////////////////////////////////////////////////////////////
/**
 * @brief for device recording callback.
 * @param[out] type: for device changed type.
 * @param[out] device: for device info.
 * @param[out] data: user context data.
 * @note @arg type for HDMI_DEVICE_NONE, device be null.
*/
typedef void(*OnHDMIDeviceChangedHandler)(HDMI_DEVICE_TYPE type, const char* device, void* data);
/** 
 * @brief for check HDMI device changed callback.
 * @param enable: for check to callback or not.
 * @param handler: callback function.
 * @param data: user context data.
 * @return
 * - 0: Success.
 * - > 0: Failure.
 */
PLAYERSDK_API int PLAYERSDK_CALL PLVSetHDMIDeviceChangedHandler(bool enable, OnHDMIDeviceChangedHandler handler, void* data);
#endif // end _WIN32
/**
* @}
*/

///////////////////////////////////////////////////////////////////
/** @defgroup Download Download Function
* @{
*/
typedef void* PLVDownloadPtr;///< download object type.
/**
 * @brief for download error callback.
 * @param[out] vid: video id.
 * @param[out] code: error code.
 * @param[out] data: user context data.
*/
typedef void(*OnDownloadErrorHandler)(const char* vid, int code, void* data);
/**
 * @brief for download progress callback.
 * @param[out] vid: video id.
 * @param[out] receivedBytes: download pos.
 * @param[out] totalBytes: file all size.
 * @param[out] data: user context data.
*/
typedef void(*OnDownloadProgressHandler)(const char* vid, long long receivedBytes, long long totalBytes, void* data);
/**
 * @brief for download result callback.
 * @param[out] vid: video id.
 * @param[out] rate: the video rate.
 * @param[out] code: error code.
 * @param[out] data: user context data.
*/
typedef void(*OnDownloadResultHandler)(const char* vid, int rate, int code, void* data);
/** 
 * @brief Creates the IPLVMediaDownload object and returns the pointer.
 * @return 
 * - Pointer: to the IPLVMediaDownload object.
 */
PLAYERSDK_API PLVDownloadPtr PLAYERSDK_CALL PLVDownloadCreate();
/** 
 * @brief Destory the IPLVMediaDownload object.
 * @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
 */
PLAYERSDK_API void PLAYERSDK_CALL PLVDownloadDestroy(PLVDownloadPtr download);
/** 
 * @brief for in the download process error callback.
 * @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
 * @param handler: callback function.
 * @param data: user context data.
 * @return
 * - 0: Success.
 * - > 0: Failure.
 */
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadSetErrorHandler(
	PLVDownloadPtr download, OnDownloadErrorHandler handler, void* data);
/** 
 * @brief for download progress callback.
 * @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
 * @param handler: callback function.
 * @param data: user context data.
 * @return
 * - 0: Success.
 * - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadSetProgressHandler(
	PLVDownloadPtr download, OnDownloadProgressHandler handler, void* data);
/** 
 * @brief for download result callback.
 * @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
 * @param handler: callback function.
 * @param data: user context data.
 * @return
 * - 0: Success.
 * - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadSetResultHandler(
	PLVDownloadPtr download, OnDownloadResultHandler handler, void* data);
/** 
* @brief for reset all handler, will clear all callback function.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadResetHandler(PLVDownloadPtr download);
/** 
* @brief set the video info before download.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @param vid: the video id.
* @param path: the video file path, use UTF8.
* @param rate: the video rate.@see VIDEO_RATE_TYPE.
* @return
* - 0: Success.
* - > 0: Failure.
* @attention path must use UTF8
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadSetVideo(PLVDownloadPtr download, const char* vid, const char* path, int rate);
/** 
* @brief start download the video.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @param autoDownRate: if false will no exist the rate return error.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadStart(PLVDownloadPtr download, bool autoDownRate);
/** 
* @brief pause download the video, the thread not exit.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadPause(PLVDownloadPtr download);
/** 
* @brief stop download the video, the thread will exit.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadStop(PLVDownloadPtr download);
/** 
* @brief delete the video file, will abort the download.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVDownloadDelete(PLVDownloadPtr download);
/** 
* @brief check download status.
* @param download: the download object, for PLVDownloadCreate return. @see PLVDownloadCreate.
* @return
* - true: Downloading.
* - false: Idle.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVDownloadIsDownloading(PLVDownloadPtr download);
/**
* @}
*/
///////////////////////////////////////////////////////////////////
/** @defgroup Player Player Function
* @{
*/
typedef void* PLVPlayerPtr;///< player object type.
/**
* @brief player media state.
* @param[out] vid: the video id.
* @param[out] state: the player state. @see PLAYER_MEDIA_STATE.
* @param[out] data: user context data.
*/
typedef void(*OnPlayerStateHandler)(const char* vid, int state, void* data);
/**
* @brief player media property value.
* @param[out] vid: the video id.
* @param[out] property: the property type. @see PLAYER_MEDIA_PROPERTY.
* @param[out] format: the value type. @see PLAYER_MEDIA_FORMAT.
* @param[out] value: property value.
* @param[out] data: user context data.
*/
typedef void(*OnPlayerPropertyHandler)(const char* vid, int property, int format, const char* value, void* data);
/**
* @brief the player rate change.
* @param[out] vid: video id.
* @param[out] inputRate: user input rate.
* @param[out] realRate: actual real rate.
* @param[out] data: user context data.
*/
typedef void(*OnPlayerRateChangeHandler)(const char* vid, int inputRate, int realRate, void* data);
/**
* @brief the player play media progress.
* @param[out] vid: video id.
* @param[out] millisecond: time pos.
* @param[out] data: user context data.
* @note replace MEDIA_PROPERTY_POSTION.
*/
typedef void(*OnPlayerProgressHandler)(const char* vid, int millisecond, void* data);

/**
* @brief the player audio device list change.
* @param[out] vid: video id.
* @param[out] audioDeviceCount: audio device count.
* @param[out] data: user context data.
*/
typedef void(*OnPlayerAudioDeviceHandler)(const char* vid, int audioDeviceCount, void* data);
/** 
 * @brief Creates the IPLVMediaPlayer object and returns the pointer.
 * @param window: for the window handler.
 * @return 
 * - Pointer: to the IPLVMediaPlayer object.
 */
PLAYERSDK_API PLVPlayerPtr PLAYERSDK_CALL PLVPlayerCreate(void* window);
/** 
 * @brief Destory the IPLVMediaPlayer object.
 * @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
 */
PLAYERSDK_API void PLAYERSDK_CALL PLVPlayerDestroy(PLVPlayerPtr player);
/** 
 * @brief for state change callback.
 * @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
 * @param handler: callback function.
 * @param data: user context data.
 * @return
 * - 0: Success.
 * - > 0: Failure.
 */
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetStateHandler(
	PLVPlayerPtr player, OnPlayerStateHandler handler, void* data);
/** 
 * @brief for property change callback.
 * @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
 * @param handler: callback function.
 * @param data: user context data.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetPropertyHandler(
	PLVPlayerPtr player, OnPlayerPropertyHandler handler, void* data);
/** 
* @brief for current bit chanage callback, the input rate is VIDEO_RATE_AUTO will callback.
 * @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
 * @param handler: callback function.
 * @param data: user context data.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetRateChangeHandler(
	PLVPlayerPtr player, OnPlayerRateChangeHandler handler, void* data);
/** 
* @brief for play progress callback.
 * @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
 * @param handler: callback function.
 * @param data: user context data.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetProgressHandler(
	PLVPlayerPtr player, OnPlayerProgressHandler handler, void* data);
/** 
* @brief for play audio device list callback.
 * @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
 * @param handler: callback function.
 * @param data: user context data.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetAudioDeviceHandler(
	PLVPlayerPtr player, OnPlayerAudioDeviceHandler handler, void* data);
/** 
* @brief for reset all handler.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerResetHandler(PLVPlayerPtr player);
/** 
* @brief set the video OSD config info
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param enable: use or not OSD.
* @param config: OSD info. @see OSDConfigInfo.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetOSDConfig(PLVPlayerPtr player, bool enable, const OSDConfigInfo* config);
/** 
* @brief get the video OSD config info.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param[out] config: OSD info. @see OSDConfigInfo.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetOSDConfig(PLVPlayerPtr player, OSDConfigInfo& config);
/**
* @brief set the video Logo text info
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param enable: use or not logo text.
* @param config: Logo info. @see LogoTextInfo.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetLogoText(PLVPlayerPtr player, bool enable, const LogoTextInfo* config);
/**
* @brief get the video Logo text info.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param[out] config: Logo info. @see LogoTextInfo.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetLogoText(PLVPlayerPtr player, LogoTextInfo& config);
/** 
* @brief set the video cache config info.
* @note If your computer memory is large enough, it is not recommended that you call this interface, just use the default value.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param enable: use or not cache, decide whether to use network cache settings (default: auto).
* @param maxCacheBytes: default 100 * 1024 * 1024, Set these limits higher if you get a packet queue overflow warning,
*	and you think normal playback would be possible with a larger packet queue.
* @param maxCacheSeconds: default 30, The main purpose of this option is to limit the readhead for local playback,
*	since a large readahead value is not overly useful in this case.
* @return
* - 0: Success.
* - > 0: Failure.
* @note If maxCacheBytes and maxCacheSeconds less than 0, ignore this value, use the default.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetCacheConfig(PLVPlayerPtr player, bool enable, int maxCacheBytes, int maxCacheSeconds);
/** 
* @brief get the video cache config info.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param[out] maxCacheBytes: cache bytes.
* @param[out] maxCacheSeconds: cache seconds.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetCacheConfig(PLVPlayerPtr player, int* maxCacheBytes, int* maxCacheSeconds);
/** 
* @brief set the video info before play.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param vid: video id.
* @param path: if play local video, mute set it, use UTF8.
* @param rate: the video rate. @see VIDEO_RATE_TYPE.
* @return
* - 0: Success.
* - > 0: Failure.
* @attention path must use UTF8.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetVideo(PLVPlayerPtr player, const char* vid, const char* path, int rate);
/**
* @brief play the video.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param token: play encryption video mute use it.
* @param seekMillisecond: play in time pos.
* @param sync: use sync or asynchronous.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerPlay(PLVPlayerPtr player, const char* token, int seekMillisecond, bool sync);
/**
* @brief play local video, must set the video path.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param seekMillisecond: play in time pos.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerPlayLocal(PLVPlayerPtr player, int seekMillisecond);
/**
* @brief load local video, no play, must set the video path.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param seekMillisecond: play in time pos.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerLoadLocal(PLVPlayerPtr player, int seekMillisecond);
/**
* @brief pause the video.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param pause: play state.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerPause(PLVPlayerPtr player, bool pause);
/**
* @brief stop the video.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerStop(PLVPlayerPtr player);
/**
* @brief set the video mute state.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param mute: play state.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetMute(PLVPlayerPtr player, bool mute);
/**
* @brief seek the play pos.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param millisecond: set time pos.
* @param exactSeek: for seek type, false will fast seek, or slow seek.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetSeek(PLVPlayerPtr player, int millisecond, bool exactSeek);
/**
* @brief stop in end frame.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSeekToEnd(PLVPlayerPtr player);
/**
* @brief set the video volume.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param volume: <0-1000> if 0 will mute.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetVolume(PLVPlayerPtr player, int volume);
/**
* @brief set the video volume gain.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param volume: <100-1000> if big 1000 will return failure.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetVolumeMax(PLVPlayerPtr player, int volume);
/**
* @brief screenshot the play video picture.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param filename: png/jpg file, use UTF8.
* @return
* - 0: Success.
* - > 0: Failure.
* @attention path must use UTF8
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerScreenshot(PLVPlayerPtr player, const char* filename);
/**
* @brief Slow down or speed up playback by the factor given as parameter.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param speed: <0.01 - 100> 1.0 normal.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetSpeed(PLVPlayerPtr player, double speed);
/**
* @brief the video mute state.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - true: mute state.
* - false: unmute state.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVPlayerIsMute(PLVPlayerPtr player);
/**
* @brief the video have load or idle.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - true: the player have load state.
* - false: the player is idle state.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVPlayerIsValid(PLVPlayerPtr player);
/**
* @brief the video play state or pause.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - true: the video is pause state.
* - false: unpause or is idle state.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVPlayerIsPause(PLVPlayerPtr player);
/**
* @brief the video playing or idle.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - true: the video is playing state.
* - false: pause or is idle state.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVPlayerIsPlaying(PLVPlayerPtr player);
/**
* @brief the video have load or idle.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - true: the video is loaded state.
* - false: unload or is idle state.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVPlayerIsLoaded(PLVPlayerPtr player);
/**
* @brief the video is loading or idle.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - true: the video is loading state.
* - false: unload or is idle state.
*/
PLAYERSDK_API bool PLAYERSDK_CALL PLVPlayerIsLoading(PLVPlayerPtr player);
/**
* @brief the video have rate count.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: the video is unload or no rate.
* - >0: have rate count.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetRateCount(PLVPlayerPtr player);
/**
* @brief the video current rate.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: the video is VIDEO_RATE_AUTO.
* - >0: the video current real rate.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetCurrentRate(PLVPlayerPtr player);
/**
* @brief the video volume.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: error.
* - >0: the video current volume.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetVolume(PLVPlayerPtr player);
/**
* @brief the video duration.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - 0: no load video.
* - >0: the video duration.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetDuration(PLVPlayerPtr player);
/**
* @brief the video speed.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - double: the video speed.
*/
PLAYERSDK_API double PLAYERSDK_CALL PLVPlayerGetSpeed(PLVPlayerPtr player);
/**
* @brief get audio device count.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @return
* - >=0 audio device count.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetAudioDeviceCount(PLVPlayerPtr player);
/**
* @brief the audio device info.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param index: audio device index.
* @param[out] deviceId: audio device id.
* @param[out] deviceName: audio device name.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetAudioDeviceInfo(PLVPlayerPtr player, 
	int index, char deviceId[PLV_MAX_DEVICE_ID_LENGTH], char deviceName[PLV_MAX_DEVICE_ID_LENGTH]);
/**
* @brief the current audio device id.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param[out] deviceId: audio device id.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerGetCurrentAudioDevice(PLVPlayerPtr player, char deviceId[PLV_MAX_DEVICE_ID_LENGTH]);
/**
* @brief set the current audio device.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @param deviceId: audio device id.
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerSetCurrentAudioDevice(PLVPlayerPtr player, const char deviceId[PLV_MAX_DEVICE_ID_LENGTH]);
/**
* @brief reload the audio device.
* @param player: the player object, for PLVPlayerCreate return. @see PLVPlayerCreate.
* @note If there is no original audio device, it needs to reload the audio after new insertion!
* @return
* - 0: Success.
* - > 0: Failure.
*/
PLAYERSDK_API int PLAYERSDK_CALL PLVPlayerReloadAudio(PLVPlayerPtr player);
/**
* @}
*/

