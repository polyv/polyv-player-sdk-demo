#pragma once
#include <QObject>
#include "plv-player-core.h"

#include "Downloader.h"
#include "Player.h"

class SdkManager : public QObject {
	Q_OBJECT
public:
	static SdkManager *GetManager();
	static void CloseManager();

	struct Account {
		QString userId;
		QString secretKey;
		QString readToken;

		QString viewerId;
		QString viewerName;
		QString viewerAvatar;
	};

public:
	void Init(const QString& userId, const QString secretKey, const QString& readToken);
	void Release();
	void SetViewer(const QString& viewerId, const QString& viewerName, const QString& viewerAvatar);

	QString GetErrorDescription(int code);
	void SetHwdecEnable(void);
	void SetKeepLastFrame(void);
	void SetVideoOutputDevice(VIDEO_OUTPUT_DEVICE type, const QString& context = QString());
	bool CheckFileComplete(const QString& vid, const QString& path, int rate);

	Account GetAccount(void) const {
		return account;
	}
	SDK_HTTP_REQUEST GetRequestType(void) const {
		return httpRequest;
	}

Q_SIGNALS:
	void SignalInitResult(bool result, const QString& msg);


#ifdef _WIN32
public:
	void SetSoftwareRecord(void* window, bool enable);
	bool GetSoftwareRecord(void* window);
	void SetHdmiRecord(bool enable);
#endif

signals:
	void SignalPluginInject();
	void SignalHDMIDeviceChanged(int type, QString device);

private slots:
	void OnPluginInject();
	void OnHDMIDeviceChanged(int type, QString device);


private:
	static SdkManager* manager;
	Account account;

	SDK_HTTP_REQUEST httpRequest = FIRST_HTTPS_REQUEST;


protected:
	SdkManager();
	~SdkManager();
};