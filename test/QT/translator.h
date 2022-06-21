#pragma once
#include <QTranslator>

#include "text-lookup.h"

class Translator : public QTranslator {
	Q_OBJECT

public:
	static TextLookup* GetTranslator(void);

	static QString Translate(const QString& name);
public:
	bool isEmpty() const override {
		return false; 
	}
	QString translate(const char *context, const char *sourceText,
		const char *disambiguation, int n) const override;

private:
	static TextLookup textLookup;
};

#define QTStr(name) Translator::Translate(name)