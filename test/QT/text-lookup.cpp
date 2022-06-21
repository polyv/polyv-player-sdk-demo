#include "text-lookup.h"

#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>

#include "qt-helper.h"

TextLookup::TextLookup(void)
{

}

TextLookup::~TextLookup(void)
{
	if (settings) {
		settings->deleteLater();
	}
}

bool TextLookup::Append(const QString &filePath)
{
	if (!settings) {
		settings = new QSettings("organization");
		settings->setDefaultFormat(QSettings::IniFormat);
	}
	QSettings ini(filePath, QSettings::IniFormat);
	auto allKeys = ini.allKeys();
	for (auto& it : allKeys) {
		//qDebug() << it << "," << ini.value(it).toString();
		settings->setValue(it, QT_TO_UTF8(ini.value(it).toString()));
	}
	//if (allKeys.empty()) {
	//	QFile file(filePath);
	//	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
	//		return false;
	//	}	
	//	QByteArray line;
	//	QList<QByteArray> array;
	//	while (!file.atEnd()){
	//		line = file.readLine();
	//		array = line.split('=');
	//		if (2 == array.size()) {
	//			settings->setValue(QT_UTF8(array.at(0)), QT_UTF8(array.at(1)));
	//			//qDebug() << QT_UTF8(array.at(0)) << "," << QT_UTF8(array.at(1));
	//		}
	//	}
	//	file.close();
	//}
    return true;
}

void TextLookup::Clear(void)
{
	if (settings) {
		settings->clear();
	}
}

QString TextLookup::Get(const QString &name) const
{
	if (settings && settings->contains(name)) {
		return settings->value(name, name).toString();
	}
	return QString();
}

void TextLookup::Set(const QString &name, const QString &value)
{
	if (settings) {
		settings->setValue(name, value);
	}
}

QString TextLookup::GetExist(const QString &name) const
{
	if (name.isEmpty()) {
		return name;
	}
	if (settings && settings->contains(name)) {
		return settings->value(name, name).toString();
	}
	return name;
}

bool TextLookup::IsExist(const QString& name) const
{
	return settings ? settings->contains(name) : false;
}
