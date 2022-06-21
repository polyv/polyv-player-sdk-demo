using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace polyv_player_demo_csharp
{
    class PlvPlayerSDK
    {
        #region SDK 类型定义
        /** 设备相关的最大字符串大小. */
        public const int PLV_MAX_DEVICE_ID_LENGTH = 512;

        /** the video rate. */
        public enum VIDEO_RATE_TYPE
        {
            /** 自动切换清晰度类型. */
            VIDEO_RATE_AUTO = 0,///< 只在在线播放中使用, 不能用在下载接口. 

            VIDEO_RATE_SD,///< 标清.
            VIDEO_RATE_HD,///< 高清.
            VIDEO_RATE_BD,///< 超清.
        };

        /** Output log filter level. */
        public enum LOG_FILTER_TYPE
        {
            /** 不输出任何日志类型. */
            LOG_FILTER_OFF = 0,

            /** 输出所有日志信息.
             * 如果要获取最完整的日志文件, 请将日志过滤设置为此类型, 但日志文件会增大.
             * 调试信息.
             */
            LOG_FILTER_DEBUG,
            /** 输出所有 FATAL, ERROR, WARNING, INFO 的日志信息.
             * 建议将日志过滤器设置为此级别.
             * 一般信息.
             * @note 默认值
             */
            LOG_FILTER_INFO,
            /** 输出所有 FATAL, ERROR, WARNING 的日志信息.
            * 警告信息.
             */
            LOG_FILTER_WARN,
            /** 输出所有 FATAL, ERROR 的日志信息.
            * 错误信息.
            */
            LOG_FILTER_ERROR,
            /** 输出所有 FATAL 的日志信息.
            * 致命信息.
            */
            LOG_FILTER_FATAL,
        };

        /** HDMI Device Changed type*/
        public enum HDMI_DEVICE_TYPE
        {
            /** 无 HDMI 设备. */
            HDMI_DEVICE_NONE = 0,
            /** HDMI 插入使用中. */
            HDMI_DEVICE_USE,
            /** HDMI 拔出停止使用. */
            HDMI_DEVICE_UNUSE,
        };

        /** Video output drivers are interfaces to different video output facilities. 
        *	If there is no video problem, it is generally not recommended to use.
        */
        public enum VIDEO_OUTPUT_DEVICE
        {
            /** 系统默认类型. 
	        * @note 默认值
	        */
            VIDEO_OUTPUT_NONE = 0,

            /** 基于着色器的GPU渲染器.
            * 通用、可定制的GPU加速视频输出驱动程序. 
            * 它支持扩展缩放方法、抖动、颜色管理、自定义着色器、HDR等.
            */
            VIDEO_OUTPUT_GPU,

            /** Direct3D 9渲染器
            * 使用Direct3D接口的视频输出驱动程序.
            * 此驱动程序用于与未提供适当OpenGL驱动程序的系统兼容, 并且ANGLE的性能不好.
            * @note 只能应用于windows系统
            */
            VIDEO_OUTPUT_DIRECT3D,
        };

        /** first http request. */
        public enum SDK_HTTP_REQUEST
        {
            /** 将首先使用 http 协议请求数据, 如果失败, 请再次使用 https 协议*/
            FIRST_HTTP_REQUEST = 0,
            /** 将首先使用 https 协议请求数据, 如果失败, 请再次使用 http 协议.
            * @note 默认值
            */
            FIRST_HTTPS_REQUEST,
            /** 一直使用 http 协议. */
            ONLY_HTTP_REQUEST,
            /** 一直使用 https 协议. */
            ONLY_HTTPS_REQUEST,
        };

        /** the error code. */
        public enum SDK_ERROR_TYPE
        {
            E_NO_ERR = 0,///< 成功.
            E_NO_INIT,///< SDK 未初始化.
            E_NO_FILE,///< 没有此文件.
            E_NO_RATE,///< 没有此视频清晰度.
            E_NO_SPACE,///< 硬盘没有剩余空间.
            E_NO_SUPPORT,///< SDK 不支持此接口.
            E_VID_ERR,///< 视频 id 不合法.
            E_KEY_ERR,///< 下载 key 文件失败或者 key 文件不合法.
            E_ZIP_ERR,///< 下载 zip 包失败或者 zip 包解压失败.
            E_M3U8_ERR,///< 下载 m3u8 文件失败或者 m3u8 文件不合法.
            E_HTTP_ERR,///< 网络请求失败, 请检查网络.
            E_PATH_ERR,///< 文件路径不合法.
            E_PARAM_ERR,///< 参数传递不合法.
            E_DELETE_ERR,///< 删除文件失败.
            E_INVOKE_ERR,///< 调用接口失败.
            E_NETWORK_ERR,///< 网络错误.
            E_DOWNLOAD_ERR,///< 下载失败, 可能出现打开文件或者路径失败.
            E_DOWNLOADING,///< 此文件正在下载中.
            E_ABORT_DOWNLOAD,///< 用户主动中断下载.
            E_DELETE_VIDEO,///< 用户主动取消下载并删除文件.
            E_MEDIA_UNINIT,///< 未调用视频的初始化接口(setvideo).
            E_MEDIA_UNLOAD,///< 视频未加载.
            E_MEDIA_LOADING,///< 视频正在加载中.
            E_TSFILESIZE,///< ts 文件为空.
            E_OUTFLOW, ///< 流量超额.
            E_OSDFONT_ERR,///< 跑马灯没有字体文件.
            E_FILE_INCOMPLETE,///< 视频文件不完整.
            E_OBJECT_NULL,///< 对象为空.
            E_NO_AUDIO_DEVICE,///< 无扬声器设备.

            E_ERROR_MAX,///< 超过此值将出现核心代码错误.
        };

        /** the player media state. */
        public enum PLAYER_MEDIA_STATE
        {
            MEDIA_STATE_LOADING = 0,///< 加载中.
            MEDIA_STATE_LOADED,///< 加载完成.
            MEDIA_STATE_PLAY,///< 播放中.
            MEDIA_STATE_PAUSE,///< 暂停中.
            MEDIA_STATE_BEGIN_CACHE,///< 开始缓存.
            MEDIA_STATE_END_CACHE,///< 停止缓存.
            MEDIA_STATE_BEGIN_SEEKING,///< 开始跳转.
            MEDIA_STATE_END_SEEKING,///< 停止跳转.
            MEDIA_STATE_FAIL,///< 播放失败.
            MEDIA_STATE_END,///< 播放结束.
        };

        /** the player media property. */
        public enum PLAYER_MEDIA_PROPERTY
        {
            MEDIA_PROPERTY_DURATION = 0,///< 播放时长.
            MEDIA_PROPERTY_POSTION,///< 播放进度. @attention 不使用, 请使用播放进度回调.
            MEDIA_PROPERTY_HWDEC,///< 硬件编码.
            MEDIA_PROPERTY_VIDEO_CODEC,///< 视频编码.
            MEDIA_PROPERTY_VIDEO_BITRATE,///< 视频码率.
            MEDIA_PROPERTY_VIDEO_FPS,///< 视频帧率.
            MEDIA_PROPERTY_VIDEO_WIDTH,///< 视频宽度.
            MEDIA_PROPERTY_VIDEO_HEIGHT,///< 视频高度.
            MEDIA_PROPERTY_AUDIO_CODEC,///< 音频编码.
            MEDIA_PROPERTY_AUDIO_BITRATE,///< 音频码率.
            MEDIA_PROPERTY_CACHE_SPEED,///< 缓存速度, MB/S.
            MEDIA_PROPERTY_CACHE_PROGRESS, ///< 缓存进度, 100%.
            MEDIA_PROPERTY_CACHE_TIME,///< 缓存时间, 单位秒，整型.
        };

        /** the player media data format. */
        public enum PLAYER_MEDIA_FORMAT
        {
            MEDIA_FORMAT_INT64 = 0,///< 整型.
            MEDIA_FORMAT_DOUBLE,///< 浮动型.
            MEDIA_FORMAT_STRING,///< 字符串.
        };

        /** OSD display type. */
        public enum OSD_DISPLAY_TYPE
        {
            OSD_DISPALY_ROLL = 0,///< 从右到左滚动. @note 默认值
            OSD_DISPALY_BLINK,///< 随机位置闪烁.
        };

        /** OSD info setting. */
        [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct OSDConfigInfo
        {
            public string text;///< 跑马灯内容, 必须使用 utf8.
            public int textSize;///< 字体大小, [1, 255], 默认值 55.
            public string textColor;///< 字体颜色值 RGB, 默认值 #000000.
            public int textAlpha;///< 字体透明值, [0, 255], 默认值 255.
            public bool border;///< 是否描边, 默认值 false.
            public string borderColor;///< 描边颜色值 RGB.
            public int borderAlpha;///< 描边透明值, [0, 255], 默认值 255.
            public int borderWidth;///< 描边大小, [1, 255], 默认值 1.
            public OSD_DISPLAY_TYPE animationEffect;///< 跑马灯显示动效类型, 默认值 OSD_DISPALY_ROLL.@see OSD_DISPLAY_TYPE.

            /** @brief 单次跑马灯显示的时长, 单位: 秒. 默认值 5.
            * @param OSD_DISPALY_ROLL: 表示单次滚动的时长(从开始滚入到完全滚出)
            * @param OSD_DISPALY_BLINK: 表示从开始显示到完全消失所需的时长
            */
            public int displayDuration;

            /** @brief 两次跑马灯显示的间隔时长, 单位: 秒. 默认值 1.
            * @param OSD_DISPALY_ROLL: 表示两次滚动的间隔时长(从完全滚出到下一次滚入的间隔)
            * @param OSD_DISPALY_BLINK: 表示两次闪烁的间隔时长(从完全消失到下一次出现的间隔)
            */
            public int displayInterval;
            /**	@brief 跑马灯文字从显示到消失的渐变动画的时长, 单位: 秒. 默认值 1.
            * 当值为0时, 不显示渐变效果.
            * 当 fadeDuration < displayDuration 时, 循环渐变效果.
            * 对 OSD_DISPALY_ROLL 和 OSD_DISPALY_BLINK 都生效.
            */
            public int fadeDuration;
        }

        /** logo text setting. */
        [StructLayoutAttribute(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct LogoTextInfo
        {
            public string text;///< LOGO 内容信息, 必须使用 utf8.
            public string textFontName;///< 字体名称, 可以为空, 必须使用系统自带字体.
            public int textSize;///< 字体大小, [1, 255], 默认值 16.
            public string textColor;///< 字体颜色值 ARGB, 默认值 #FFFFFFFF.
            public int borderWidth;///< 描边大小, [0, 255], 如果为 0 值时描边失效, 默认值 1.
            public string borderColor;///< 描边颜色值 ARGB, 默认值 #FF000000.	
            public int alignX;///< [-1 0 1] 水平位置 左中右, 默认值 1.
            public int alignY;///< [-1 0 1] 垂直位置 上中下, 默认值 -1.
        }
        #endregion

        #region SDK 主要控制 API
        //main api
        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr PLVGetSdkVersion();

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr PLVGetSdkErrorDescription(int err);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVCheckFileComplete(string vid, string videoFilePath, int rate);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkLogFile(string filename);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkLogLevel(LOG_FILTER_TYPE level);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkHttpRequest(SDK_HTTP_REQUEST type);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkCacertFile(string fileName);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkHwdecEnable(bool enable);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkKeepLastFrame(bool enable);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkVideoOutputDevice(VIDEO_OUTPUT_DEVICE type, string context = null);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetSdkViewerInfo(string viewerId, string viewerName, string viewerAvatar,
            string viewerExtraInfo1 = null, string viewerExtraInfo2 = null, string viewerExtraInfo3 = null);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVInitSdkLibrary(string userId, string secretKey, string readToken);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void PLVReleaseSdkLibrary();
        #endregion

        #region  SDK 软件防录制及 HDMI 设备变动通知
        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetPreventRecord(IntPtr window, bool enable);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVGetPreventRecord(IntPtr window, ref bool enable);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnPluginInjectHandler(IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetPluginInjectHandler(bool enable, OnPluginInjectHandler handler, IntPtr data);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnHDMIDeviceChangedHandler(HDMI_DEVICE_TYPE type, string device, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVSetHDMIDeviceChangedHandler(bool enable, OnHDMIDeviceChangedHandler handler, IntPtr data);
        #endregion

        #region SDK 下载 API
        //download api
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnDownloadErrorHandler(string vid, int code, IntPtr data);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnDownloadProgressHandler(string vid, long receivedBytes, long totalBytes, IntPtr data);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnDownloadResultHandler(string vid, int rate, int code, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr PLVDownloadCreate();

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void PLVDownloadDestroy(IntPtr download);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadSetErrorHandler(IntPtr download, OnDownloadErrorHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadSetProgressHandler(IntPtr download, OnDownloadProgressHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadSetResultHandler(IntPtr download, OnDownloadResultHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadResetHandler(IntPtr download);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadSetVideo(IntPtr download, string vid, string path, int rate);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadStart(IntPtr download, bool autoDownRate);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadPause(IntPtr download);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadStop(IntPtr download);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVDownloadDelete(IntPtr download);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVDownloadIsDownloading(IntPtr download);
        #endregion

        #region SDK 播放 API
        //player api
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnPlayerStateHandler(string vid, int state, IntPtr data);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnPlayerPropertyHandler(string vid, int property, int format, IntPtr value, IntPtr data);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnPlayerRateChangeHandler(string vid, int inputRate, int realRate, IntPtr data);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnPlayerProgressHandler(string vid, int millisecond, IntPtr data);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void OnPlayerAudioDeviceHandler(string vid, int audioDeviceCount, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr PLVPlayerCreate(IntPtr window);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void PLVPlayerDestroy(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetStateHandler(IntPtr player, OnPlayerStateHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetPropertyHandler(IntPtr player, OnPlayerPropertyHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetRateChangeHandler(IntPtr player, OnPlayerRateChangeHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetProgressHandler(IntPtr player, OnPlayerProgressHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetAudioDeviceHandler(IntPtr player, OnPlayerAudioDeviceHandler handler, IntPtr data);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerResetHandler(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetOSDConfig(IntPtr player, bool enable, OSDConfigInfo config);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetOSDConfig(IntPtr player, ref OSDConfigInfo config);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetLogoText(IntPtr player, bool enable, LogoTextInfo config);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetLogoText(IntPtr player, ref LogoTextInfo config);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetCacheConfig(IntPtr player, bool enable, int maxCacheBytes, int maxCacheSeconds);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetCacheConfig(IntPtr player, ref int maxCacheBytes, ref int maxCacheSeconds);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetVideo(IntPtr player, string vid, string path, int rate);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerPlay(IntPtr player, string token, int seekMillisecond, bool sync);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerPlayLocal(IntPtr player, int seekMillisecond);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerLoadLocal(IntPtr player, int seekMillisecond);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerPause(IntPtr player, bool pause);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerStop(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetMute(IntPtr player, bool mute);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetSeek(IntPtr player, int millisecond, bool exactSeek);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSeekToEnd(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetVolume(IntPtr player, int volume);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetVolumeMax(IntPtr player, int volume);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerScreenshot(IntPtr player, string filename);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetSpeed(IntPtr player, double speed);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVPlayerIsMute(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVPlayerIsValid(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVPlayerIsPause(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVPlayerIsPlaying(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVPlayerIsLoaded(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool PLVPlayerIsLoading(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetRateCount(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetCurrentRate(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetVolume(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetDuration(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern double PLVPlayerGetSpeed(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetAudioDeviceCount(IntPtr player);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetAudioDeviceInfo(IntPtr player, int index, ref string deviceId, ref string deviceName);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerGetCurrentAudioDevice(IntPtr player, ref IntPtr deviceId);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerSetCurrentAudioDevice(IntPtr player, string deviceId);

        [DllImport(@".\..\..\..\..\..\..\plv-player-sdk\windows\lib\x64\plv-player-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PLVPlayerReloadAudio(IntPtr player);
        #endregion
    }
}
