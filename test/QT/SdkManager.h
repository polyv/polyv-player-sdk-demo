#pragma once
#include <QObject>

#include "plv-player-core.h"

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
	void SetViewer(const QString& viewerId, const QString& viewerName, const QString& viewerAvatar);

	void SetHwdecEnable(void);
	void SetKeepLastFrame(void);

	Account GetAccount(void) const {
		return account;
	}
	SDK_HTTP_REQUEST GetRequestType(void) const {
		return httpRequest;
	}

signals:
	void SignalInitResult(bool result, const QString& msg);
protected:
	SdkManager();
	~SdkManager();

	
private:
	static SdkManager* manager;
	Account account;

	SDK_HTTP_REQUEST httpRequest = FIRST_HTTPS_REQUEST;
	
};