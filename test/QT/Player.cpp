#include "Player.h"
#include "SdkManager.h"
#include <QDebug>
#include <log/log.h>


PlayerOSDConfig Player::osdConfig;
PlayerLogoConfig Player::logoConfig;
PlayerCacheConfig Player::cacheConfig;

Player::Player(void* window, QObject* parent/* = nullptr*/)
	: QObject(parent)
{
	player = PLVPlayerCreate(window);
	PLVPlayerSetStateHandler(player, [](const char* vid, int state, void* data) {
		Player* obj = (Player*)data;
		QMetaObject::invokeMethod(obj, "OnState", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(int, state));
	}, this);
	PLVPlayerSetPropertyHandler(player, [](const char* vid, int property, int format, const char* value, void* data) {
		Player* obj = (Player*)data;
		QMetaObject::invokeMethod(obj, "OnProperty", Qt::QueuedConnection, 
			Q_ARG(QString, vid), Q_ARG(int, property), Q_ARG(int, format), Q_ARG(QString, value));
	}, this);
	PLVPlayerSetRateChangeHandler(player, [](const char* vid, int inputBitRate, int realBitRate, void* data) {
		Player* obj = (Player*)data;
		QMetaObject::invokeMethod(obj, "OnRateChange", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(int, inputBitRate), Q_ARG(int, realBitRate));
	}, this);
	PLVPlayerSetProgressHandler(player, [](const char* vid, int millisecond, void* data) {
		Player* obj = (Player*)data;
		QMetaObject::invokeMethod(obj, "OnProgress", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(int, millisecond));
	}, this);
	PLVPlayerSetAudioDeviceHandler(player, [](const char* vid, int audioDeviceCount, void* data) {
		Player* obj = (Player*)data;
		QMetaObject::invokeMethod(obj, "OnAudioDeviceChange", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(int, audioDeviceCount));
	}, this);

	UpdateOSDConfig();
	UpdateLogoConfig();
	UpdateCacheConfig();
}
Player::~Player(void)
{
	PLVPlayerDestroy(player);
	qInfo() << "the player destory.";
}

PlayerOSDConfig& Player::GetOSDConfig()
{
	return osdConfig;
}

PlayerLogoConfig& Player::GetLogoConfig()
{
	return logoConfig;
}

PlayerCacheConfig& Player::GetCacheConfig()
{
	return cacheConfig;
}

int Player::SetVideo(const QString& vid, const QString& path, int rate)
{
	return PLVPlayerSetVideo(player, vid.toStdString().c_str(), path.toStdString().c_str(), rate);
}
int Player::Reset()
{
	return PLVPlayerResetHandler(player);
}
void Player::UpdateOSDConfig()
{
	if (!osdConfig.enable) {
		PLVPlayerSetOSDConfig(player, osdConfig.enable, nullptr);
		return;
	}
	OSDConfigInfo config;
	config.animationEffect = (OSD_DISPLAY_TYPE)osdConfig.animationEffect;
	std::string borderColor = osdConfig.borderColor.toStdString();
	config.borderColor = borderColor.c_str();
	config.borderSize = osdConfig.borderSize;
	config.displayDuration = osdConfig.displayDuration;
	config.displayInterval = osdConfig.displayInterval;
	config.fadeDuration = osdConfig.fadeDuration;
	std::string text = osdConfig.text.toStdString();
	config.text = text.c_str();
	std::string textColor = osdConfig.textColor.toStdString();
	config.textColor = textColor.c_str();
	config.textSize = osdConfig.textSize;
	PLVPlayerSetOSDConfig(player, osdConfig.enable, &config);
}
void Player::UpdateLogoConfig()
{
	if (!logoConfig.enable) {
		PLVPlayerSetLogoText(player, logoConfig.enable, nullptr);
		return;
	}
	LogoTextInfo config;
	std::string text = logoConfig.text.toStdString();
	config.text = text.c_str();
	std::string textFontName = logoConfig.textFontName.toStdString();
	config.textFontName = textFontName.c_str();
	config.textSize = logoConfig.textSize;
	std::string textColor = logoConfig.textColor.toStdString();
	config.textColor = textColor.c_str();
	config.borderSize = logoConfig.borderSize;
	std::string borderColor = logoConfig.borderColor.toStdString();
	config.borderColor = borderColor.c_str();
	config.alignX = logoConfig.alignX;
	config.alignY = logoConfig.alignY;

	PLVPlayerSetLogoText(player, logoConfig.enable, &config);
}
void Player::UpdateCacheConfig()
{
	PLVPlayerSetCacheConfig(player, cacheConfig.enable, cacheConfig.maxCacheBytes, cacheConfig.maxCacheSeconds);
}
int Player::Play(const QString& token, int seekMillisecond, bool sync)
{
	return PLVPlayerPlay(player, token.toStdString().c_str(), seekMillisecond, sync);
}
int Player::PlayLocal(int seekMillisecond)
{
	return PLVPlayerPlayLocal(player, seekMillisecond);
}
int Player::LoadLocal(int seekMillisecond)
{
	return PLVPlayerLoadLocal(player, seekMillisecond);
}
int Player::Pause(bool pause)
{
	return PLVPlayerPause(player, pause);
}
int Player::Stop()
{
	return PLVPlayerStop(player);
}
int Player::SetMute(bool mute)
{
	return PLVPlayerSetMute(player, mute);
}
int Player::SetSeek(int millisecond)
{
	return PLVPlayerSetSeek(player, millisecond);
}
int Player::SeekToEnd()
{
	return PLVPlayerSeekToEnd(player);
}
int Player::SetVolume(int volume)
{
	return PLVPlayerSetVolume(player, volume);
}
int Player::GetVolume()
{
	return PLVPlayerGetVolume(player);
}
int Player::SetVolumeMax(int volume)
{
	return PLVPlayerSetVolumeMax(player, volume);
}
int Player::Screenshot(const QString& fileName)
{
	return PLVPlayerScreenshot(player, fileName.toStdString().c_str());
}
int Player::SetSpeed(double speed)
{
	return PLVPlayerSetSpeed(player, speed);
}
double Player::GetSpeed()
{
	return PLVPlayerGetSpeed(player);
}
int Player::GetDuration()
{
	return PLVPlayerGetDuration(player);
}
int Player::GetRateCount()
{
	return PLVPlayerGetRateCount(player);
}
int Player::GetCurrentRate()
{
	return PLVPlayerGetCurrentRate(player);
}
bool Player::IsMute()
{
	return PLVPlayerIsMute(player);
}
bool Player::IsValid()
{
	return PLVPlayerIsValid(player);
}
bool Player::IsPause()
{
	return PLVPlayerIsPause(player);
}
bool Player::IsPlaying()
{
	return PLVPlayerIsPlaying(player);
}
bool Player::IsLoaded()
{
	return PLVPlayerIsLoaded(player);
}
bool Player::IsLoading()
{
	return PLVPlayerIsLoading(player);
}

int Player::GetAudioDeviceCount()
{
	return PLVPlayerGetAudioDeviceCount(player);
}
int Player::GetAudioDeviceInfo(int index, QString& deviceId, QString& deviceName)
{
	char id[PLV_MAX_DEVICE_ID_LENGTH] = { 0 };
	char name[PLV_MAX_DEVICE_ID_LENGTH] = { 0 };
	int ret = PLVPlayerGetAudioDeviceInfo(player, index, id, name);
	deviceId = id;
	deviceName = name;
	return ret;
}
int Player::GetCurrentAudioDevice(QString& deviceId)
{
	char id[PLV_MAX_DEVICE_ID_LENGTH] = { 0 };
	int ret = PLVPlayerGetCurrentAudioDevice(player, id);
	deviceId = id;
	return ret;
}
int Player::SetCurrentAudioDevice(const QString& deviceId)
{
	return PLVPlayerSetCurrentAudioDevice(player, deviceId.toStdString().c_str());
}
int Player::ReloadAudio()
{
	return PLVPlayerReloadAudio(player);
}

void Player::OnState(QString vid, int state)
{
	(void)vid;
	emit SignalState(state);
}
void Player::OnProperty(QString vid, int property, int format, QString value)
{
	(void)vid;
	emit SignalProperty(property, format, value);
}
void Player::OnRateChange(QString vid, int inputBitRate, int realBitRate)
{
	(void)vid;
	emit SignalRateChange(inputBitRate, realBitRate);
}
void Player::OnProgress(QString vid, int millisecond)
{
	(void)vid;
	emit SignalProgress(millisecond);
}
void Player::OnAudioDeviceChange(QString vid, int audioDeviceCount)
{
	(void)vid;
	if (audioDeviceCount > 0) {
		ReloadAudio();
	}
	emit SignalAudioDeviceChange(audioDeviceCount);
}