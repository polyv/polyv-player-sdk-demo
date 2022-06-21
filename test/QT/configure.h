#pragma once

#include <QVariant>
class QString;
class QVariant;
class QSettings;
class Configure{
public:
    explicit Configure(void);
	~Configure(void);
public:
	bool Open(const QString &fileName);
	void Close(void);

	QString GetFileName(void) const;
	
	void Clear(void);
	bool IsOpen(void) const;
	bool Save(void);

	bool IsExist(const QString &section, const QString &name) const;

	QVariant Get(const QString &section, 
		const QString &name, const QVariant &defaultValue = QVariant()) const;
    void Set(const QString &section, const QString &name, const QVariant &value);

	void Remove(const QString &section);
	void Remove(const QString &section, const QString &name);

	bool IsExistDefault(const QString &section, const QString &name) const;

	QVariant GetDefault(const QString &section, 
		const QString &name, const QVariant &defaultValue = QVariant()) const;
	void SetDefault(const QString &section, const QString &name, const QVariant &value);
private:
	class ConfigurePrivate;
	ConfigurePrivate* d_ptr;
    

private:  // emphasize the following members are private
	Configure(const Configure&) = delete;
	const Configure& operator=(const Configure&) = delete;
};
