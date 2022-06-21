#include "configure.h"

#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>

class Configure::ConfigurePrivate {
public:
	QSettings* settings = nullptr;
	QSettings* defaultSettings = nullptr;
	QString configFile;
};

Configure::Configure(void) :
	d_ptr(new ConfigurePrivate)
{
	d_ptr->defaultSettings = new QSettings("configure");
}

Configure::~Configure(void)
{
	Close();
	delete d_ptr->defaultSettings;
	delete d_ptr;
}

bool Configure::Open(const QString &fileName)
{
	if (fileName.isEmpty()) {
		return false;
	}
	d_ptr->configFile = fileName;
	if (d_ptr->settings) {
		Close();
	}
	d_ptr->settings = new QSettings(fileName, QSettings::IniFormat);
	return true;
}

void Configure::Close()
{
	if (d_ptr->settings) {
		delete d_ptr->settings;
	}
	d_ptr->settings = nullptr;
}

QString Configure::GetFileName(void) const
{
	return d_ptr->configFile;
}

void Configure::Clear(void)
{
	if (d_ptr->settings) {
		d_ptr->settings->clear();
	}
	if (d_ptr->defaultSettings) {
		d_ptr->defaultSettings->clear();
	}
}

bool Configure::IsOpen(void) const
{
	return d_ptr->settings ? true : false;
}

bool Configure::Save(void)
{
	if (!d_ptr->settings) {
		return false;
	}
	d_ptr->settings->sync();
	return true;
}

bool Configure::IsExist(const QString &section, const QString &name) const
{
	if (!d_ptr->settings) {
		return false;
	}
	d_ptr->settings->beginGroup(section);
	bool result = d_ptr->settings->contains(name);
	d_ptr->settings->endGroup();
	
	return result;
}

QVariant Configure::Get(const QString &section, 
	const QString &name, const QVariant &defaultValue/* = QVariant()*/) const
{
	if (!IsExist(section, name)) {
		return GetDefault(section, name, defaultValue);
	}
	d_ptr->settings->beginGroup(section);
	auto value = d_ptr->settings->value(name, defaultValue);
	d_ptr->settings->endGroup();
	return value;
}
void Configure::Set(const QString &section, const QString &name, const QVariant &value)
{
	if (!d_ptr->settings) {
		return;
	}
	d_ptr->settings->beginGroup(section);
	d_ptr->settings->setValue(name, value);
	d_ptr->settings->endGroup();
}

void Configure::Remove(const QString &section)
{
	if (!d_ptr->settings) {
		return;
	}
	d_ptr->settings->beginGroup(section);
	auto keys = d_ptr->settings->allKeys();
	for (auto & it : keys) {
		d_ptr->settings->remove(it);
	}
	d_ptr->settings->endGroup();
}

void Configure::Remove(const QString &section, const QString &name)
{
	if (!d_ptr->settings) {
		return;
	}
	d_ptr->settings->beginGroup(section);
	d_ptr->settings->remove(name);
	d_ptr->settings->endGroup();
}


bool Configure::IsExistDefault(const QString &section, const QString &name) const
{
	if (!d_ptr->defaultSettings) {
		return false;
	}
	d_ptr->defaultSettings->beginGroup(section);
	bool result = d_ptr->defaultSettings->contains(name);
	d_ptr->defaultSettings->endGroup();

	return result;
}

QVariant Configure::GetDefault(const QString &section, 
	const QString &name, const QVariant &defaultValue/* = QVariant()*/) const
{
	if (!d_ptr->defaultSettings) {
		return defaultValue;
	}
	d_ptr->defaultSettings->beginGroup(section);
	auto value = d_ptr->defaultSettings->value(name, defaultValue);
	d_ptr->defaultSettings->endGroup();
	return value;
}
void Configure::SetDefault(const QString &section, const QString &name, const QVariant &value)
{
	if (!d_ptr->defaultSettings) {
		return;
	}
	d_ptr->defaultSettings->beginGroup(section);
	d_ptr->defaultSettings->setValue(name, value);
	d_ptr->defaultSettings->endGroup();
}