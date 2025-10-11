# plv-player-sdk

SDK 的版本变更说明

## Version 2.9.0
变更与修复
1. 修复VRM13视频在无缓存时无法精确seek的问题
2. 修复因网络重置原因导致播放跳帧或播放结束的问题
3. 修复下载视频时,因token有效性导致暂停后恢复下载失败的问题
4. 去掉部分无用的依赖库，降低SDK文件大小

## Version 2.8.1
变更与修复
1. Electron下硬件防录制功能不再默认启用，需要调用接口打开
2. 点播新增PLVPlayerSetSeekCacheEnable接口，用于控制是否基于缓存进行seek

## Version 2.8.0
新特性
1. 支持MAC原生ARM64架构SDK

## Version 2.7.1
变更与修复
1. 修复播放转播频道直播画面时,播放启动速度慢的问题

## Version 2.7.0
新特性
1. 支持频道直播拉流功能,针对直播播放器有一套新增的接口
2. 支持虚拟机环境检测,目前仅支持Windows

变更与修复
1. C++接口标记为过时
    1. PLVMigrateLocalVideoKeyFile 建议使用新的token接口
    2. PLVPlayerSetAudioDeviceHandler 建议使用PLVGetAudioDeviceCount获取音频设备列表
    3. PLVPlayerGetAudioDeviceCount 建议使用PLVGetAudioDeviceCount获取音频设备列表
    4. PLVPlayerGetAudioDeviceInfo 建议使用PLVGetAudioDeviceInfo获取音频设备信息
2. 修复VRM13视频在同步播放进度后,无法seek到0-10秒的问题

## Version 2.6.3
变更与修复  
1. 修复弱网情况下，播放器停止时间过长的问题

## Version 2.6.2
变更与修复  
1. 记忆进度在视频最后几秒时,再次播放现在会定位到最前
2. Elctron-SDK-NAPI版本8,构建node-18.20.3
3. 修复播放VRM12、VRM13在线播放缓冲过低的问题
4. 修复中文路径下证书获取失败的问题

## Version 2.6.1
变更与修复
1. 相关安全性提升：证书、日志、seed参数防护

## Version 2.6.0

新特性
1. C++接口支持自定义播放seed，仅针对VRM12|VRM13的在线播放
2. C++接口新增：PLVPlayerSetAudioPlayErrorHandler,用于音频播放失败时回调通知
3. Electron新增：回调事件PlayerAudioPlayError,用于音频播放失败时回调通知

变更与修复
1. C++接口标记为过时
    1. PLVAccountInfo中的secretKey与appSecret不建议传入SDK
    2. PLVSetSdkHttpRequest 不建议调用
    3. PLVRequestVideoInfo 建议在服务器上请求视频信息
    4. PLVCancelRequestVideoInfo 建议在服务器上请求视频信息
    5. PLVPlayerReloadAudioDevice 暂无需调用
2. C++接口参数变更
    1. PLVDownloadStart时，加密视频必须提供token
3. Electron以下接口被移除
    1. setSdkHttpRequest sdk内部始终以https方式请求
    2. requestVideoInfo 建议在服务器上请求视频信息
    3. cancelRequestVideoInfo 建议在服务器上请求视频信息
    4. playerReloadAudioDevice 暂无需主动重载
4. Electron以下接口参数变更
    1. initSdkLibrary中，不再处理secretKey与appSecret参数,不要再传入
    2. downloadStart中，加密视频必须提供token
5. 音频变更:WIN32下不再尝试使用openal播放音频
6. 修复MAC上结束播放后重新开启播放有一定概率死锁的问题

## Version 2.5.3
变更与修复
1. 修复未释放JS事件相关的问题

## Version 2.5.2
变更与修复
1. 修复Windows下播放时，音频反复禁用与启用可能导致崩溃的问题
2. 修复Windows-Electron下，硬件防录制无回调通知的问题
3. VRM12、13水印样式优化

## Version 2.5.1
变更与修复
1. 使用2.3.1及以下版本下载的离线加密视频，经过2.4.0及以上版本迁移概率性失败的问题
2. MAC下主动停止播放未发生停止回调
3. VRM12|13会自带视频水印，无法使用接口去除
4. Electron-SDK下硬件防录制回调类型错误的问题
5. Electron-SDK清晰度切换不准确问题

## Version 2.5.0

接口变更
1. PLVInitSdkLibrary：增加子账户设置
2. PLVSetSdkLocalRememberPlay：本地续播接口
3. PLVVideoRequestInfo：视频信息部分字段改为64位

新特性
1. 支持ECDN在线播放
2. 支持跨端续播
3. 支持使用子账户
4. 支持Electron调用

变更与修复
1. 文件大小显示不准确
2. M1上全屏播放有一定概率崩溃
3. Windows上使用WASAPI播放音频失败时尝试使用SDL
4. VRM13会自带视频水印，无法使用接口去除

## Version 2.4.0

### 迁移说明

1. 兼容性说明：
   * 使用此版本下载的离线视频，不能使用低于此版本的 SDK 进行播放；
   * 不能直接播放低版本的离线视频，必须调用此版本的迁移接口进行迁移后才能正常播放；
2. 迁移说明：
   * 使用此版本时，所有低版本的离线视频都必须迁移；
   * 迁移过程不可逆，也就是迁移后不能使用低版本 SDK 播放；
   * 迁移接口里的 secretKey 对应下载离线视频时的 secretKey；

变更项：

1. 接口及参数类型变更：
   1. plv-player-def.h 变更：
        1. VIDEO_RATE_TYPE；
        2. HDMI_DEVICE_TYPE；
        3. SDK_ERROR_TYPE；
        4. VIDEO_OUTPUT_DEVICE 增加 win 平台限定；
        5. OSDConfigInfo 修改为 PLVOsdConfigInfo，防止重名；
        6. LogoTextInfo 修改为 PLVLogoTextInfo，并去掉 textFontName 参数，防止重名；
        7. 增加类型 VIDEO_REQUEST_TYPE/SOFTWARE_RECORDING_NOTIFY_TYPE；
        8. 增加数据结构 PLVVideoRequestParam/PLVVideoRequestInfo/PLVVideoRequestPageInfo；
   2. plv-player-core.h 变更：
        1. 防录制接口全部更换，明确区分软件与硬件防录制，详见接口；
        2. 把下载单独抽取来一个文件 plv-player-download.h：
            1. PLVDownloadSetVideo 变更为 PLVDownloadSetInfo；
            2. PLVDownloadPause 增加参数 pause 决定是否暂停或者恢复；
        3. 播放接口变量：
            1. OnPlayerAudioDeviceHandler 回调去掉 vid 的回调；
            2. PLVPlayerSetVideo 变更为 PLVPlayerSetInfo；
            3. PLVPlayerGetRateCount 变更为 PLVPlayerGetCurrentRateCount；
            4. PLVPlayerReloadAudio 变更为 PLVPlayerReloadAudioDevice；
            5. PLVPlayerPlay 增加参数 autoDownRate 用于清晰度自动降级；
            6. PLVPlayerPlay 增加参数 playWithToken 用于判断是否播放时带上token；
            7. PLVPlayerPlayLocal 增加参数 autoDownRate 用于清晰度自动降级；
            8. PLVPlayerLoadLocal 增加参数 autoDownRate 用于清晰度自动降级；
        4. 全局接口：
            1. 新增 PLVSetSdkLogMessageCallback 用于将日志信息回调给上层处理；
            2. 修改 PLVInitSdkLibrary 去掉 readToken 参数；
            3. 新增 PLVRequestVideoInfo 用于请求视频信息；
            4. 新增 PLVCancelRequestVideoInfo 用于中断请求视频信息；
            5. 新增 PLVMigrateLocalVideoKeyFile 用于迁移已经下载的离线视频到新的key方案上；
            6. 新增 PLVDeleteLocalVideoFile 用于删除离线视频；

新特性：

1. 完善 mac 下防录制功能；
2. 增加设置日志信息回调给上层；
3. 增加请求视频信息接口；
4. 增加播放溯源视频参数，playWithToken为true；
5. 增加全局删除视频接口；
6. 增加迁移离线视频到新的加密方式上的接口；

修复问题：

* 【修复】下载暂停时线程退出问题；
* 【修复】跑马灯在播放中支持修改；
* 【修复】部分vrm13拖动进度时出现音画不同步问题；
* 【修复】处理videojson时，重试的处理逻辑；
* 【优化】vrm12/vrm13没有传viewerId，返回错误码；
* 【优化】指定VIDEO_RATE_AUTO时，使用videojson里默认清晰度，不存在时逐级切换，而不是直接使用最低清晰度播放；
* 【优化】当视频为源文件时，下载会判断是否MP4文件，如果是则下载，并且文件指定清晰度为LD，为m3u8时返回E_NO_SUPPORT；
* 【优化】优化下载解压失败时处理逻辑，3次解压失败时会删除视频重新下载解压；

## Version 2.3.1

修复问题：

* 【修复】播放vrm13离线视频，路径中包含中文时播放无画面的问题；

## Version 2.3.0

新特性：

1. 增加接口，支持设置重试下载的次数；

修复问题：

* 【修改】跑马灯渐变功能调整，支持淡入淡出；
* 【优化】优化网络请求耗时时间；
* 【优化】日志输出增加vid打印，增加Debug级别的内核日志输出；
* 【优化】在开始下载前会检测磁盘空间，如果小于100M会返回错误码，下载中失败会判断剩余空间是否小于10M，如果是回调返回错误码；

## Version 2.2.1

修复问题：

* 【修复】某些机器下GDI渲染模糊问题；
* 【修改】创建播放器对象时如果传入窗口句柄为空时返回对象为空；
* 【修改】MAC最低支持版本10.15；

## Version 2.2.0

新特性：

1. 支持VRM12播放；
2. 支持VRM13播放；
3. 减少MAC版本依赖库；
4. 支持设置Logo文字；
5. 统一跑马灯与Logo文字颜色格式；
6. 修改播放时跳转指定时间，只有跳转完成后才收到加载完成回调通知；

修复问题：

* 【功能】增加GDI渲染，解决部分机器驱动问题导致播放卡顿问题；
* 【功能】增加设置证书路径接口，解决https请求报错问题；
* 【功能】修复路径字符问题，统一UTF8字符；
* 【修改】默认关闭硬件加速，在VRM13下硬件加速可能造成花屏；

## Version 2.1.0

新特性：

1. 支持VRM12播放；
2. 支持MAC版本；
3. 支持设置Logo文字；

## Version 2.0.0

新特性：

1. 减少依赖库问题；
2. 增加win防录制接口回调；

## Version 1.2.0

新特性：

1. 增加播放缓冲进度回调；

修复问题：

* 【功能】VLog计算错误问题；

## Version 1.1.0

新特性：

1. 更新为C++实现；

修复问题：

* 【功能】某个显卡播放视频卡住问题；
