#pragma once

#include <QObject>
#include <plv-player-core.h>


struct PlayerOSDConfig {
	QString text;
	int textSize = 55;
	QString textColor = "#FF000000";
	int borderSize = 1;
	QString borderColor = "#FFFFFFFF";
	int animationEffect = 0;
	int displayDuration = 5;
	int displayInterval = 1;
	int fadeDuration = 1;

	bool enable = true;
};

struct PlayerLogoConfig {
	QString text;
	QString textFontName;
	int textSize = 16;
	QString textColor = "#FF000000";
	int borderSize = 1;
	QString borderColor = "#FFFFFFFF";
	int alignX = 1;
	int alignY = -1;

	bool enable = true;
};

struct PlayerCacheConfig {
	bool enable = true;
	int maxCacheBytes = -1;
	int maxCacheSeconds = -1;
};

//////////////////////////////////////////////////
class Player : public QObject {
	Q_OBJECT
public:
	explicit Player(void* window,  QObject* parent = nullptr);
	~Player(void);

public:
	static PlayerOSDConfig& GetOSDConfig();
	static PlayerLogoConfig& GetLogoConfig();
	static PlayerCacheConfig& GetCacheConfig();
public:
	int SetVideo(const QString& vid, const QString& path, int rate);
	int Reset();
	void UpdateOSDConfig();
	void UpdateLogoConfig();
	void UpdateCacheConfig();
	int Play(const QString& token, int seekMillisecond, bool sync);
	int PlayLocal(int seekMillisecond);
	int LoadLocal(int seekMillisecond);
	int Pause(bool pause);
	int Stop();
	int SetMute(bool mute);
	int SetSeek(int millisecond);
	int SeekToEnd();
	int SetVolume(int volume);
	int GetVolume();
	int SetVolumeMax(int volume);
	int Screenshot(const QString& fileName);
	int SetSpeed(double speed);
	double GetSpeed();
	int GetDuration();
	int GetRateCount();
	int GetCurrentRate();
	bool IsMute();
	bool IsValid();
	bool IsPause();
	bool IsPlaying();
	bool IsLoaded();
	bool IsLoading();

	// audio api
	int GetAudioDeviceCount();
	int GetAudioDeviceInfo(int index, QString& deviceId, QString& deviceName);
	int GetCurrentAudioDevice(QString& deviceId);
	int SetCurrentAudioDevice(const QString& deviceId);
	int ReloadAudio();

signals:
	void SignalState(int state);
	void SignalProgress(int millisecond);
	void SignalProperty(int property, int format, QString value);
	void SignalRateChange(int inputBitRate, int realBitRate);
	void SignalAudioDeviceChange(int audioDeviceCount);
private slots:
	void OnState(QString vid, int state);
	void OnProgress(QString vid, int millisecond);
	void OnProperty(QString vid, int property, int format, QString value);
	void OnRateChange(QString vid, int inputBitRate, int realBitRate);
    void OnAudioDeviceChange(QString vid, int audioDeviceCount);

private:
	PLVPlayerPtr player = nullptr;

	static PlayerOSDConfig osdConfig;
	static PlayerLogoConfig logoConfig;
	static PlayerCacheConfig cacheConfig;

private:
	Player(const Player&) = delete;
	Player(Player&&) = delete;
	Player& operator=(const Player&) = delete;
	Player& operator=(Player&&) = delete;
};