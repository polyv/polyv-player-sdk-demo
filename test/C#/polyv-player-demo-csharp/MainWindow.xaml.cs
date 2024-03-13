using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.IO;
using static polyv_player_demo_csharp.PlvPlayerSDK;

namespace polyv_player_demo_csharp
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        bool isControlshidden = false;
        LogWrite appLog = new LogWrite();
        //init
        bool inited = false;
        string viewerId = "yourViewerId";/*虽然可以不调用此接口，SDK依然正常运行，但是仍然建议使用者设置观看者信息，由用户自定义*/
        string viewerName = "yourViewerName";
        string videoPath = System.AppDomain.CurrentDomain.BaseDirectory + "video";
        List<VideoInfo> videoList = new List<VideoInfo>();
        
        //downloader
        PlvPlayerSDK.OnDownloadErrorHandler downloadErrorHandler = null;
        PlvPlayerSDK.OnDownloadProgressHandler downloadProgressHandler = null;
        PlvPlayerSDK.OnDownloadResultHandler downloadResultHandler = null;
        IntPtr downloader = IntPtr.Zero;
		//player
		IntPtr renderBuffer = Marshal.AllocHGlobal(1920 * 1088 * 4);
		PlvPlayerSDK.OnPlayerVideoFrameHandler playFrameLockHandler = null;
		PlvPlayerSDK.OnPlayerVideoFrameHandler playFrameUnlockHandler = null;
		PlvPlayerSDK.OnPlayerStateHandler playStateHandler = null;
        PlvPlayerSDK.OnPlayerPropertyHandler playPropertyHandler = null;
        PlvPlayerSDK.OnPlayerRateChangeHandler playRateChangeHandler = null;
        PlvPlayerSDK.OnPlayerProgressHandler playProgressHandler = null;
        PlvPlayerSDK.OnPlayerAudioDeviceHandler playAudioDeviceHandler = null;
        IntPtr player = IntPtr.Zero;
        string playToken = "";
        long duration = 0; /*视频时长：ms*/
        long currentPos = 0;/*当前位置：ms*/
        bool playSeeking = false;
        int audioDeviceCount = -1;

        #region 初始化
        public MainWindow()
        {
            InitializeComponent();
        }

        private string ConvertUtf8(string str)
        {
            byte[] utf8 = System.Text.Encoding.Convert(Encoding.Default, Encoding.UTF8, System.Text.Encoding.Default.GetBytes(str));
            return System.Text.Encoding.Default.GetString(utf8);
        }

        private void WindowLoaded(object sender, RoutedEventArgs e)
        {
            string logPath = System.AppDomain.CurrentDomain.BaseDirectory + "\\log中文";
            if (!Directory.Exists(logPath))
            {
                Directory.CreateDirectory(logPath);
            }
            appLog.SetPath(logPath);
            appLog.Debug("init application...");
            //初始化全局信息
            string version = Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkVersion());
            appLog.Debug("the sdk version:" + version);
            int ret = PlvPlayerSDK.PLVSetSdkLogFile(ConvertUtf8(logPath + "\\测试\\" + DateTime.Now.ToString("yyyy-MM-dd hh-mm-ss") + ".log"));
            ret = PlvPlayerSDK.PLVSetSdkLogLevel(PlvPlayerSDK.LOG_FILTER_TYPE.LOG_FILTER_DEBUG);
            ret = PlvPlayerSDK.PLVSetSdkHttpRequest(PlvPlayerSDK.SDK_HTTP_REQUEST.ONLY_HTTPS_REQUEST);
			ret = PlvPlayerSDK.PLVSetSdkCacertFile(@".\..\..\..\..\..\..\plv-player-sdk\windows\x64\lib\cacert.pem");
			/* 设置解码类型，全局有效
             * 注意：一般建议您使用软解码！可以大大降低解码花屏出错的概率！
             *      建议仅当CPU性能不足或GPU驱动等测试正常时，使用硬解码！
             */
			ret = PlvPlayerSDK.PLVSetSdkHwdecEnable(false);
        }

        private void WindowClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (inited)
            {
                appLog.Debug("release begin");
                try
                {
                    if (player != IntPtr.Zero)
                    {
                        PlvPlayerSDK.PLVPlayerDestroy(player);
						Marshal.FreeHGlobal(renderBuffer);
					}
                    if (downloader != IntPtr.Zero)
                    {
                        PlvPlayerSDK.PLVDownloadDestroy(downloader);
                    }
                    PlvPlayerSDK.PLVReleaseSdkLibrary();
                }
                catch(Exception exception)
                {
                    appLog.Debug("sdk release error! msg:" + exception.Message);
                }
                inited = false;
                appLog.Debug("release end");
            }
            appLog.Debug("release application...");
        }

        private void OnGetVideoList(object sender, RoutedEventArgs e)
        {
            if (userIdTextBox.Text == String.Empty || secretKeyTextBox.Text == String.Empty)
            {
                MessageBox.Show("用户信息值不可空！");
                return;
            }

            //get video info
            string errmsg = "";
            if (!PlayerUtilities.RequestVideoList(ref videoList, ref errmsg, userIdTextBox.Text, secretKeyTextBox.Text))
            {
                appLog.Debug("request video list error! " + errmsg); 
                MessageBox.Show("获取视频失败，错误信息：" + errmsg);
                return;
            }
            appLog.Debug("get video, count:" + videoList.Count);
            if (videoList.Count <= 0)
            {
                MessageBox.Show("无视频，请上传！");
                return;
            }
            if (videoList.Count > 50)
            {
                videoList.RemoveRange(50, videoList.Count - 50);
            }
            vidListComboBox.ItemsSource = videoList;
            vidListComboBox.SelectedIndex = 0;
            inited = true;

            try
            {
				//init sdk info
				PlvPlayerSDK.PLVAccountInfo accountInfo;
				accountInfo.userId = userIdTextBox.Text;
				accountInfo.secretKey = secretKeyTextBox.Text;
				accountInfo.appId = null;
				accountInfo.appSecret = null;
				accountInfo.subAccount = false;
				int ret = PlvPlayerSDK.PLVInitSdkLibrary(accountInfo);
                ret = PlvPlayerSDK.PLVSetSdkKeepLastFrame(true);
                ret = PlvPlayerSDK.PLVSetSdkViewerInfo(viewerId, viewerName, "");

                //init player and downloader
                if (downloader == IntPtr.Zero)
                {
                    downloadErrorHandler = new PlvPlayerSDK.OnDownloadErrorHandler(OnDownloadErrorHandler);
                    downloadProgressHandler = new PlvPlayerSDK.OnDownloadProgressHandler(OnDownloadProgressHandler);
                    downloadResultHandler = new PlvPlayerSDK.OnDownloadResultHandler(OnDownloadResultHandler);
                    downloader = PlvPlayerSDK.PLVDownloadCreate();
                    PlvPlayerSDK.PLVDownloadSetErrorHandler(downloader, downloadErrorHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVDownloadSetProgressHandler(downloader, downloadProgressHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVDownloadSetResultHandler(downloader, downloadResultHandler, IntPtr.Zero);
                }
                if (player == IntPtr.Zero)
                {
					playFrameLockHandler = new PlvPlayerSDK.OnPlayerVideoFrameHandler(OnPlayerFrameLockHandler);
					playFrameUnlockHandler = new PlvPlayerSDK.OnPlayerVideoFrameHandler(OnPlayerFrameUnlockHandler);
					playStateHandler = new PlvPlayerSDK.OnPlayerStateHandler(OnPlayerStateHandler);
                    playPropertyHandler = new PlvPlayerSDK.OnPlayerPropertyHandler(OnPlayerPropertyHandler);
                    playRateChangeHandler = new PlvPlayerSDK.OnPlayerRateChangeHandler(OnPlayerRateChangeHandler);
                    playProgressHandler = new PlvPlayerSDK.OnPlayerProgressHandler(OnPlayerProgressHandler);
                    playAudioDeviceHandler = new PlvPlayerSDK.OnPlayerAudioDeviceHandler(OnPlayerAudioDeviceHandler);
					player = PlvPlayerSDK.PLVPlayerCreate(playWindow.Handle);
					//player = PlvPlayerSDK.PLVPlayerCreate(IntPtr.Zero);
					PlvPlayerSDK.PLVPlayerSetVideoFrameHandler(player, playFrameLockHandler, playFrameUnlockHandler, IntPtr.Zero);
					PlvPlayerSDK.PLVPlayerSetStateHandler(player, playStateHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVPlayerSetPropertyHandler(player, playPropertyHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVPlayerSetRateChangeHandler(player, playRateChangeHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVPlayerSetProgressHandler(player, playProgressHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVPlayerSetAudioDeviceHandler(player, playAudioDeviceHandler, IntPtr.Zero);
                    PlvPlayerSDK.PLVPlayerSetVolumeMax(player, 200);

                    PlvPlayerSDK.PLVOsdConfigInfo osd;
                    osd.text = "test001";
                    osd.textSize = 45;
                    osd.textColor = "#FFFF0000";// red
					osd.borderSize = 1;
					osd.borderColor = "#FFFFFFFF";// white
					osd.animationEffect = PlvPlayerSDK.OSD_DISPLAY_TYPE.OSD_DISPALY_ROLL;
                    osd.displayDuration = 15;
                    osd.displayInterval = 1;
                    osd.fadeDuration = 0;
                    PlvPlayerSDK.PLVPlayerSetOSDConfig(player, true, osd);

                    PlvPlayerSDK.PLVLogoTextInfo logo;
					logo.text = "polyv";
					logo.textSize = 50;
					logo.textColor = "#FFFF0000";
					logo.borderSize = 1;
					logo.borderColor = "#0FFFFFFF";
                    logo.alignX = 1;
                    logo.alignY = -1;
                    PlvPlayerSDK.PLVPlayerSetLogoText(player, true, logo);

                }
            }
            catch (Exception exception)
            {
                appLog.Debug("init sdk error! " + exception.Message);
            }
        }

        private void OnVideoChanged(object sender, SelectionChangedEventArgs e)
        {
            try
            {
                vidTextBox.Text = vidListComboBox.SelectedValue.ToString();
            }
            catch (Exception exception)
            {
                appLog.Debug("OnVideoChanged error! " + exception.Message);
            }
        }
        #endregion

        #region 下载功能
        private void OnDownloadErrorHandler(string vid, int code, IntPtr data)
        {
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                downloadStatusLabel.Content = String.Format("下载失败，错误码：{0}", code);
                downloadProgressBar.Visibility = Visibility.Collapsed;
                downloadStartButton.IsEnabled = true;
                downloadPauseButton.IsEnabled = false;
                downloadStopButton.IsEnabled = false;
                downloadDeleteButton.IsEnabled = false;
                appLog.Debug(String.Format("下载失败，错误码：{0}", code));
            }));
        }

        private void OnDownloadProgressHandler(string vid, long receivedBytes, long totalBytes, IntPtr data)
        {
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                long percent = receivedBytes * 100 / totalBytes;
                downloadProgressBar.Visibility = Visibility.Visible;
                downloadProgressBar.Value = percent;
                downloadStatusLabel.Content = String.Format("正在下载：{0}%", percent);
            }));
        }

        private void OnDownloadResultHandler(string vid, int rate, int code, IntPtr data)
        {
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                string status = "";
                if (code == (int)PlvPlayerSDK.SDK_ERROR_TYPE.E_ABORT_OPERATION)
                {
                    status = string.Format("下载暂停或停止,代码：{0}", code);
                }
                else if (code == (int)PlvPlayerSDK.SDK_ERROR_TYPE.E_DOWNLOAD_ERR)
                {
                    status = string.Format("下载错误，代码:{0}", code);
                }
                else if (code == (int)PlvPlayerSDK.SDK_ERROR_TYPE.E_NO_ERR)
                {
                    status = string.Format("下载成功！");
                }
                else
                {
                    status = string.Format("其他错误，代码:{0}", code);
                }
                downloadStatusLabel.Content = status;
                downloadProgressBar.Visibility = Visibility.Collapsed;
                downloadStartButton.IsEnabled = true;
                downloadPauseButton.IsEnabled = false;
                downloadStopButton.IsEnabled = false;
                downloadDeleteButton.IsEnabled = true;
                appLog.Debug(status);
            }));
        }

        private void OnDownloadStart(object sender, RoutedEventArgs e)
        {
            if (!inited)
            {
                MessageBox.Show("请获取视频！");
                return;
            }
            PlvPlayerSDK.PLVDownloadSetVideo(downloader, vidTextBox.Text, ConvertUtf8(videoPath),
                                               Convert.ToInt32(downloadRateComboBox.SelectedIndex + 1));
            appLog.Debug("download start:vid:" + vidTextBox.Text +
                " path:" + videoPath +
                " rate:" + Convert.ToInt32(downloadRateComboBox.SelectedIndex + 1));
            //download
            try
            {
                int ret = PlvPlayerSDK.PLVDownloadStart(downloader, false);
                if (ret != 0)
                {
                    appLog.Debug("download start error! ret:" + ret);
                    MessageBox.Show("下载失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                    return;
                }
            }
            catch (Exception exception)
            {
                appLog.Debug("download start error! msg:" + exception.Message);
            }
            downloadStartButton.IsEnabled = false;
            downloadPauseButton.IsEnabled = true;
            downloadStopButton.IsEnabled = true;
            downloadDeleteButton.IsEnabled = false;
        }

        private void OnDownloadPause(object sender, RoutedEventArgs e)
        {
            try
            {
                int ret = PlvPlayerSDK.PLVDownloadPause(downloader, true);
                if (ret != 0)
                {
                    appLog.Debug("download pause error! ret:" + ret);
                    MessageBox.Show("暂停下载失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                    return;
                }
            }
            catch (Exception exception)
            {
                appLog.Debug("download pause error! msg:" + exception.Message);
            }
        }

        private void OnDownloadStop(object sender, RoutedEventArgs e)
        {
            try
            {
                int ret = PlvPlayerSDK.PLVDownloadStop(downloader);
                if (ret != 0)
                {
                    appLog.Debug("download stop error! ret:" + ret);
                    MessageBox.Show("停止下载失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                    return;
                }
            }
            catch (Exception exception)
            {
                appLog.Debug("download stop error! msg:" + exception.Message);
            }
            downloadProgressBar.Visibility = Visibility.Collapsed;
            downloadStartButton.IsEnabled = true;
            downloadPauseButton.IsEnabled = false;
            downloadStopButton.IsEnabled = false;
            downloadDeleteButton.IsEnabled = true;
        }

        private void OnDownloadDelete(object sender, RoutedEventArgs e)
        {
            try
            {
                int ret = PlvPlayerSDK.PLVDownloadDelete(downloader);
                if (ret != 0)
                {
                    appLog.Debug("download delete error! ret:" + ret);
                    MessageBox.Show("删除下载失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                    return;
                }
            }
            catch (Exception exception)
            {
                appLog.Debug("download delete error! msg:" + exception.Message);
            }
            downloadDeleteButton.IsEnabled = false;
        }
        #endregion

        #region 播放功能

		private bool OnPlayerFrameLockHandler(string vid, ref PLVVideoFrame frame, IntPtr data)
		{
			frame.data0 = renderBuffer;
			return true;
		}

		private bool OnPlayerFrameUnlockHandler(string vid, ref PLVVideoFrame frame, IntPtr data)
		{
			return true;
		}
		private void OnPlayerStateHandler(string vid, int state, IntPtr data)
        {
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                PlvPlayerSDK.PLAYER_MEDIA_STATE s = (PlvPlayerSDK.PLAYER_MEDIA_STATE)state;
                switch (s)
                {
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_LOADING:
                        playStatusLabel.Content = "加载中";
                        playStartButton.IsEnabled = false;
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_LOADED:
                        playStatusLabel.Content = "加载完成";
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_PLAY:
                        string rate = "自动";
                        if (PlvPlayerSDK.PLVPlayerGetCurrentRate(player) == (int)PlvPlayerSDK.VIDEO_RATE_TYPE.VIDEO_RATE_LD) { rate = "流畅"; }
                        else if (PlvPlayerSDK.PLVPlayerGetCurrentRate(player) == (int)PlvPlayerSDK.VIDEO_RATE_TYPE.VIDEO_RATE_SD) { rate = "高清"; }
                        else if (PlvPlayerSDK.PLVPlayerGetCurrentRate(player) == (int)PlvPlayerSDK.VIDEO_RATE_TYPE.VIDEO_RATE_HD) { rate = "超清"; }
                        playStatusLabel.Content = "播放" + " - " + rate;
                        playPauseButton.IsEnabled = true;
                        playResumeButton.IsEnabled = true;
                        playStopButton.IsEnabled = true;
                        volumeLabel.Content = PlvPlayerSDK.PLVPlayerGetVolume(player);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_PAUSE:
                        playStatusLabel.Content = "暂停";
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_BEGIN_CACHE:
                        playStatusLabel.Content = "开始缓存";
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_END_CACHE:
                        playStatusLabel.Content = "缓存完毕";
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_BEGIN_SEEKING:
                        playStatusLabel.Content = "定位中";
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_END_SEEKING:
                        playStatusLabel.Content = "定位完成";
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_FAIL:
                        playStatusLabel.Content = "失败";
                        playStartButton.IsEnabled = true;
                        playPauseButton.IsEnabled = false;
                        playResumeButton.IsEnabled = false;
                        playStopButton.IsEnabled = false;
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_STATE.MEDIA_STATE_END:
                        playStatusLabel.Content = "结束";
                        playStartButton.IsEnabled = true;
                        playPauseButton.IsEnabled = false;
                        playResumeButton.IsEnabled = false;
                        playStopButton.IsEnabled = false;
                        playSlider.Value = 0;
                        if (continuePlayCheck.IsChecked == true && vidListComboBox.SelectedIndex < videoList.Count - 1)
                        {
                            vidListComboBox.SelectedIndex += 1;
                            OnPlayStart(playStartButton, new RoutedEventArgs());
                        }
                        break;
                    default:break;
                }
                appLog.Debug("OnPlayerStateHandler，vid:" + videoId + " " + playStatusLabel.Content.ToString());
            }));
        }

        private void OnPlayerPropertyHandler(string vid, int property, int format, IntPtr value, IntPtr data)
        {
            //注意：由于采用异步委托的方式，指针指向的内容在委托方法中不可访问！
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            string propertyValue = Marshal.PtrToStringAnsi(value);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                string logMsg = "";
                PlvPlayerSDK.PLAYER_MEDIA_PROPERTY p = (PlvPlayerSDK.PLAYER_MEDIA_PROPERTY)property;
                switch (p)
                {
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_DURATION:
                        duration = Convert.ToInt64(propertyValue);
                        logMsg = string.Format("duration:{0}", duration);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_HWDEC:
                        string hwdec = propertyValue;
                        logMsg = string.Format("hwdec:{0}", hwdec);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_VIDEO_CODEC:
                        string videoCodec = propertyValue;
                        logMsg = string.Format("videoCodec:{0}", videoCodec);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_VIDEO_BITRATE:
                        long videoBitRate = Convert.ToInt64(propertyValue);
                        logMsg = string.Format("videoBitRate:{0}", videoBitRate);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_VIDEO_FPS:
                        double videoFps = Convert.ToDouble(propertyValue);
                        logMsg = string.Format("videoFps:{0}", videoFps);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_VIDEO_WIDTH:
                        long videoWidth = Convert.ToInt64(propertyValue);
                        logMsg = string.Format("videoWidth:{0}", videoWidth);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_VIDEO_HEIGHT:
                        long videoHeight = Convert.ToInt64(propertyValue);
                        logMsg = string.Format("videoHeight:{0}", videoHeight);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_AUDIO_CODEC:
                        string audioCodec = propertyValue;
                        logMsg = string.Format("audioCodec:{0}", audioCodec);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_AUDIO_BITRATE:
                        long audioBitrate = Convert.ToInt64(propertyValue);
                        logMsg = string.Format("audioBitrate:{0}", audioBitrate);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_CACHE_SPEED:
                        string cacheSpeed = propertyValue;
                        logMsg = string.Format("cacheSpeed:{0}", cacheSpeed);
                        break;
                    case PlvPlayerSDK.PLAYER_MEDIA_PROPERTY.MEDIA_PROPERTY_CACHE_PROGRESS:
                        string cacheProgress = propertyValue;
                        logMsg = string.Format("cacheProgress:{0}", cacheProgress);
                        break;
                    default: break;
                }
                appLog.Debug("OnPlayerPropertyHandler, vid:" + videoId + " " + logMsg);
            }));
        }

        private void OnPlayerRateChangeHandler(string vid, int inputRate, int realRate, IntPtr data)
        {
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            { 
                if (inputRate != realRate)
                {
//                     string rate = "自动";
//                     if (realRate == (int)PlvPlayerSDK.VIDEO_RATE_TYPE.VIDEO_RATE_LD) { rate = "流畅"; }
//                     else if (realRate == (int)PlvPlayerSDK.VIDEO_RATE_TYPE.VIDEO_RATE_SD) { rate = "高清"; }
//                     else if (realRate == (int)PlvPlayerSDK.VIDEO_RATE_TYPE.VIDEO_RATE_HD) { rate = "超清"; }
//                     MessageBox.Show("播放清晰度:" + rate);
                }
                appLog.Debug("OnPlayerRateChangeHandler, vid:" + videoId + " input:" + inputRate + " real:" + realRate);
            }));
        }

        private void OnPlayerProgressHandler(string vid, int millisecond, IntPtr data)
        {
            string videoId = vid;// Marshal.PtrToStringAnsi(vid);
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                currentPos = millisecond;
                if (!playSeeking && duration != 0)
                {
                    playSlider.Value = ((double)millisecond / duration) * 100;
                }
                playTimeLabel.Content = PlayerUtilities.millSecond2Time(millisecond) + " / " + PlayerUtilities.millSecond2Time(duration);
            }));
        }

        private void OnPlayerAudioDeviceHandler(int count, IntPtr data)
        {
            App.Current.Dispatcher.BeginInvoke((Action)(() =>
            {
                if (count > 0)
                {
                    if (audioDeviceCount == 0)
                    {
                        PlvPlayerSDK.PLVPlayerReloadAudio(player);
                    }
                }
                audioDeviceCount = count;
                appLog.Debug("OnPlayerAudioDeviceHandler,count:" + count);
            }));
        }

        private void OnPlayStart(object sender, RoutedEventArgs e)
        {
            if (!inited)
            {
                MessageBox.Show("请获取视频！");
                return;
            }

            try
            {
                //播放
                playToken = "";
                duration = 0;
                playSlider.Value = 0;
                playSpeedComboBox.SelectedIndex = 1;
                int ret = PlvPlayerSDK.PLVPlayerSetInfo(player, vidTextBox.Text, ConvertUtf8(videoPath),
                    Convert.ToInt32(playRateComboBox.SelectedIndex));
                appLog.Debug("play start:vid:" + vidTextBox.Text +
                " path:" + videoPath +
                " rate:" + Convert.ToInt32(playRateComboBox.SelectedIndex));
                if (localPlayCheck.IsChecked == true)
                {
                    appLog.Debug("local play");
                    if (preloadPlayCheck.IsChecked == true)
                    {
                        appLog.Debug("preload play");
                        ret = PlvPlayerSDK.PLVPlayerLoadLocal(player, 0, true);
                    }
                    else
                    {
                        ret = PlvPlayerSDK.PLVPlayerPlayLocal(player, 0, true);
                    }
                }
                else
                {
                    //获取token
                    if (!PlayerUtilities.GetToken(ref playToken, userIdTextBox.Text, secretKeyTextBox.Text, vidTextBox.Text, viewerId, viewerName))
                    {
                        appLog.Debug("get token error! " + playToken);
                        MessageBox.Show("获取token失败！错误信息:" + playToken);
                        return;
                    }
                    //在线播放
                    ret = PlvPlayerSDK.PLVPlayerPlay(player, playToken, 0, true, false, true);
                }
                if (ret != 0)
                {
                    appLog.Debug("play error! ret:" + ret);
                    MessageBox.Show("播放失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                    return;
                }
            }
            catch(Exception exception)
            {
                appLog.Debug("play error! msg:" + exception.Message);
            }
            playStartButton.IsEnabled = false;
            playPauseButton.IsEnabled = true;
            playResumeButton.IsEnabled = true;
            playStopButton.IsEnabled = true;
        }

        private void OnPlayPause(object sender, RoutedEventArgs e)
        {
            try
            {
                int ret = PlvPlayerSDK.PLVPlayerPause(player, true);
                if (ret != 0)
                {
                    appLog.Debug("play pause error! ret:" + ret);
                    MessageBox.Show("暂停播放失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                }
            }
            catch(Exception exception)
            {
                appLog.Debug("play pause error! " + exception.Message);
            }
        }

        private void OnPlayResume(object sender, RoutedEventArgs e)
        {
            try
            {
                int ret = PlvPlayerSDK.PLVPlayerPause(player, false);
                if (ret != 0)
                {
                    appLog.Debug("play resume error! ret:" + ret);
                    MessageBox.Show("继续播放失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                }
            }
            catch (Exception exception)
            {
                appLog.Debug("play resume error! " + exception.Message);
            }
        }

        private void OnPlayStop(object sender, RoutedEventArgs e)
        {
            try
            {
                continuePlayCheck.IsChecked = false;
                int ret = PlvPlayerSDK.PLVPlayerStop(player);
                if (ret != 0)
                {
                    appLog.Debug("play stop error! ret:" + ret);
                    MessageBox.Show("停止播放失败！错误信息:" + Marshal.PtrToStringAnsi(PlvPlayerSDK.PLVGetSdkErrorDescription(ret)));
                    return;
                }
            }
            catch (Exception exception)
            {
                appLog.Debug("play stop error! " + exception.Message);
            }
            playStartButton.IsEnabled = true;
            playPauseButton.IsEnabled = false;
            playResumeButton.IsEnabled = false;
            playStopButton.IsEnabled = false;
        }

        private void OnPlaySeekStart(object sender, MouseEventArgs e)
        {
            if (inited)
            {
                playSeeking = true;
            }
        }

        private void OnPlaySeekStop(object sender, MouseEventArgs e)
        {
            if (inited)
            {
                double pos = playSlider.Value * duration / 100;
                PlvPlayerSDK.PLVPlayerSetSeek(player, (int)pos);
                playSeeking = false;
            }
        }

        private void OnVolumeChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (inited)
            {
                PlvPlayerSDK.PLVPlayerSetVolume(player, (int)volumeSlider.Value);
                volumeLabel.Content = PlvPlayerSDK.PLVPlayerGetVolume(player);
            }
        }

        private void OnPlaySpeedChanged(object sender, SelectionChangedEventArgs e)
        {
            if (inited)
            {
                ComboBoxItem item = playSpeedComboBox.SelectedItem as ComboBoxItem;
                PlvPlayerSDK.PLVPlayerSetSpeed(player, Convert.ToDouble(Convert.ToDouble(item.Content)));
            }
        }

        private void OnPlayRateChanged(object sender, SelectionChangedEventArgs e)
        {
            if (inited)
            {
                try
                {
                    if (PlvPlayerSDK.PLVPlayerIsLoaded(player))
                    {
                        continuePlayCheck.IsChecked = false;
                        long pos = currentPos;
                        PlvPlayerSDK.PLVPlayerStop(player);
                        int ret = PlvPlayerSDK.PLVPlayerSetInfo(player, vidTextBox.Text, ConvertUtf8(videoPath),
                            Convert.ToInt32(playRateComboBox.SelectedIndex));
                        if (localPlayCheck.IsChecked == false && playToken.Length > 0)
                        {
                            PlvPlayerSDK.PLVPlayerPlay(player, playToken, (int)pos, true, false, true);
                        }
                        else
                        {
							PlvPlayerSDK.PLVPlayerPlayLocal(player, (int)pos, true);
                        }
                    }
                }
                catch (Exception exception)
                {
                    appLog.Debug("rate changed! replay error! " + exception.Message);
                }
            }
        }

        private void OnPlayControlHide(object sender, RoutedEventArgs e)
        {
            if (!isControlshidden)
            {
                viewbox1.Margin = new Thickness(0, 0, 0, 20);
                HiddenButton.Content = "显示";
                isControlshidden = true;
            }
            else
            {
                viewbox1.Margin = new Thickness(0, 0, 0, 178);
                HiddenButton.Content = "隐藏";
                isControlshidden = false;
            }
        }
        #endregion


    }
}
