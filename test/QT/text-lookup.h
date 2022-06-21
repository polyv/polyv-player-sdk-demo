#pragma once

class QString;
class QSettings;
class TextLookup
{
public:
    explicit TextLookup(void);
	~TextLookup(void);
public:
	bool Append(const QString &filePath);
	void Clear(void);

public:
	// if no exist return name
    QString Get(const QString &name) const;
    void Set(const QString &name, const QString &value);

	// if no exist return empty
	QString GetExist(const QString &name) const;
	bool IsExist(const QString& name) const;
private:
    QSettings *settings = nullptr;

private:  // emphasize the following members are private
	TextLookup(const TextLookup&) = delete;
	const TextLookup& operator=(const TextLookup&) = delete;
};
