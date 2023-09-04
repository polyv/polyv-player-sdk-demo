#pragma once
#include <QObject>
#include <QList>
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

		QString viewerId;
		QString viewerName;
		QString viewerAvatar;
	};

public:
	void Init(const QString& userId, const QString secretKey);
	void Release();
	void SetViewer(const QString& viewerId, const QString& viewerName, const QString& viewerAvatar);

	QString GetErrorDescription(int code);
	bool CheckFileComplete(const QString& vid, const QString& path, int rate);
	bool DeleteLocalVideoFile(const QString& vid, const QString& path, int rate);
	bool MigrateLocalVideoKeyFile(const QString& keyFile, const QString& secretKey);

	void SetLogPath();
	void SetLogLevel();
	void SetLogCallback();
	void SetHwdecEnable();
	void SetKeepLastFrame();
	void SetVideoOutputDevice();
	void SetHttpRequest();
	void SetRetryCount();
	void SetSoftwareRecording();
	void SetHardwareRecording();

	Account GetAccount(void) const {
		return account;
	}
	
	struct ValueItem {
		int value;
		QString name;
		bool defValue;
	};
	QList<ValueItem> GetLogItems() const;
	QList<ValueItem> GetHttpItems() const;
	QList<ValueItem> GetOutputItems() const;
	QList<ValueItem> GetRateItems() const;
	QStringList GetOutputContexts() const;

Q_SIGNALS:
	void SignalInitResult(bool result, const QString& msg);

private:
	static SdkManager* manager;
	Account account;

protected:
	SdkManager();
	~SdkManager();
};