#pragma once

#include <QByteArray>
#include <QString>

class QTextDocument;

namespace RtfCodec {
inline constexpr qsizetype maxFileSize = 8 * 1024 * 1024;
bool available();
bool decode(const QByteArray &data, QTextDocument &document, QString &error);
bool encode(const QTextDocument &document, QByteArray &data, QString &error);
}
