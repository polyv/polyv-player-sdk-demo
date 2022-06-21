#include "translator.h"

TextLookup Translator::textLookup;

TextLookup* Translator::GetTranslator()
{
	return &textLookup;
}

QString Translator::translate(const char *context, const char *sourceText,
	const char *disambiguation, int n) const
{
	if (!textLookup.IsExist(sourceText)) {
		return QTranslator::translate(context, sourceText, disambiguation, n);
	}
	return Translate(sourceText);
}

QString Translator::Translate(const QString& name)
{
	return textLookup.GetExist(name);
}