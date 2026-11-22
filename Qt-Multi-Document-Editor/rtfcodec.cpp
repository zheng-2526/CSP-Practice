#include "rtfcodec.h"

#ifdef Q_OS_WIN
#include <QColor>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <richedit.h>
#include <cstring>
#include <string>
#include <vector>

namespace {

HMODULE richEditLibrary() {
    static HMODULE library = LoadLibraryExW(L"Msftedit.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    return library;
}

struct RichEdit {
    // 隐藏控件仅负责文件转换，用户仍在 Qt 的编辑区输入。
    HWND hwnd = richEditLibrary() ? CreateWindowExW(0, MSFTEDIT_CLASS, L"",
        WS_POPUP | ES_MULTILINE, 0, 0, 0, 0, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr)
        : nullptr;

    ~RichEdit() {
        if(hwnd) DestroyWindow(hwnd);
    }
};

struct Input {
    const QByteArray &data;
    qsizetype offset = 0;
};

DWORD CALLBACK readRtf(DWORD_PTR cookie, LPBYTE buffer, LONG capacity, LONG *size) {
    auto &input = *reinterpret_cast<Input *>(cookie);
    *size = static_cast<LONG>(qMin<qsizetype>(capacity, input.data.size() - input.offset));
    if(*size > 0) std::memcpy(buffer, input.data.constData() + input.offset, *size);
    input.offset += *size;
    return 0;
}

DWORD CALLBACK writeRtf(DWORD_PTR cookie, LPBYTE buffer, LONG size, LONG *written) {
    auto &data = *reinterpret_cast<QByteArray *>(cookie);
    data.append(reinterpret_cast<const char *>(buffer), size);
    *written = size;
    return 0;
}

void selectRange(HWND hwnd, int l, int r) {
    CHARRANGE range{l, r};
    SendMessageW(hwnd, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&range));
}

QTextCharFormat charFormatAt(HWND hwnd, int i, int n) {
    selectRange(hwnd, i, i + n);
    CHARFORMAT2W native{};
    native.cbSize = sizeof(native);
    SendMessageW(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&native));

    QTextCharFormat format;
    if(native.szFaceName[0]) format.setFontFamilies({QString::fromWCharArray(native.szFaceName)});
    if(native.yHeight > 0) format.setFontPointSize(native.yHeight / 20.0);
    format.setFontWeight(native.dwEffects & CFE_BOLD ? QFont::Bold : QFont::Normal);
    format.setFontItalic(native.dwEffects & CFE_ITALIC);
    format.setFontUnderline(native.dwEffects & CFE_UNDERLINE);
    format.setFontStrikeOut(native.dwEffects & CFE_STRIKEOUT);
    if(!(native.dwEffects & CFE_AUTOCOLOR))
        format.setForeground(QColor(GetRValue(native.crTextColor),
                                    GetGValue(native.crTextColor), GetBValue(native.crTextColor)));
    if(!(native.dwEffects & CFE_AUTOBACKCOLOR))
        format.setBackground(QColor(GetRValue(native.crBackColor),
                                    GetGValue(native.crBackColor), GetBValue(native.crBackColor)));
    return format;
}

Qt::Alignment alignmentAt(HWND hwnd, int i) {
    selectRange(hwnd, i, i);
    PARAFORMAT native{};
    native.cbSize = sizeof(native);
    SendMessageW(hwnd, EM_GETPARAFORMAT, 0, reinterpret_cast<LPARAM>(&native));
    if(native.wAlignment == PFA_CENTER) return Qt::AlignHCenter;
    if(native.wAlignment == PFA_RIGHT) return Qt::AlignRight;
    if(native.wAlignment == PFA_JUSTIFY) return Qt::AlignJustify;
    return Qt::AlignLeft;
}

bool setCharFormat(HWND hwnd, QTextCharFormat format, const QFont &defaultFont) {
    format.setFont(format.font().resolve(defaultFont));
    QFont font = format.font();
    CHARFORMAT2W native{};
    native.cbSize = sizeof(native);
    native.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE |
                    CFM_STRIKEOUT | CFM_COLOR | CFM_BACKCOLOR;
    native.yHeight = qRound((font.pointSizeF() > 0 ? font.pointSizeF() : 12.0) * 20);
    std::wstring name = font.family().left(LF_FACESIZE - 1).toStdWString();
    name.copy(native.szFaceName, name.size());
    if(font.bold()) native.dwEffects |= CFE_BOLD;
    if(font.italic()) native.dwEffects |= CFE_ITALIC;
    if(font.underline()) native.dwEffects |= CFE_UNDERLINE;
    if(font.strikeOut()) native.dwEffects |= CFE_STRIKEOUT;
    if(format.foreground().style() == Qt::NoBrush) native.dwEffects |= CFE_AUTOCOLOR;
    else {
        QColor color = format.foreground().color();
        native.crTextColor = RGB(color.red(), color.green(), color.blue());
    }
    if(format.background().style() == Qt::NoBrush) native.dwEffects |= CFE_AUTOBACKCOLOR;
    else {
        QColor color = format.background().color();
        native.crBackColor = RGB(color.red(), color.green(), color.blue());
    }
    return SendMessageW(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&native)) != 0;
}

bool setAlignment(HWND hwnd, Qt::Alignment alignment) {
    PARAFORMAT native{};
    native.cbSize = sizeof(native);
    native.dwMask = PFM_ALIGNMENT;
    if(alignment & Qt::AlignHCenter) native.wAlignment = PFA_CENTER;
    else if(alignment & Qt::AlignRight) native.wAlignment = PFA_RIGHT;
    else if(alignment & Qt::AlignJustify) native.wAlignment = PFA_JUSTIFY;
    else native.wAlignment = PFA_LEFT;
    return SendMessageW(hwnd, EM_SETPARAFORMAT, 0, reinterpret_cast<LPARAM>(&native)) != 0;
}

}
#endif

bool RtfCodec::available() {
#ifdef Q_OS_WIN
    return richEditLibrary() != nullptr;
#else
    return false;
#endif
}

bool RtfCodec::decode(const QByteArray &data, QTextDocument &document, QString &error) {
#ifdef Q_OS_WIN
    QByteArray content = data;
    // Rich Edit 有时会在 RTF 文本之后附加 C 字符串的终止符。
    while(content.endsWith('\0')) content.chop(1);
    content = content.trimmed();
    if(!content.startsWith("{\\rtf1") || !content.endsWith('}')) {
        error = QStringLiteral("文件不是有效的 RTF 文档。");
        return false;
    }

    // 跳过转义的反斜线。遇到超出编辑器范围的结构则保留原文件，避免丢失内容。
    static const QRegularExpression unsupported(
        QStringLiteral(R"(\\\\|\\(pict|object|objdata|bin|trowd|intbl|header[lrf]?|footer[lrf]?|footnote)(?![a-z]))"),
        QRegularExpression::CaseInsensitiveOption);
    auto matches = unsupported.globalMatch(QString::fromLatin1(content));
    while(matches.hasNext()) {
        if(!matches.next().captured(1).isEmpty()) {
            error = QStringLiteral("当前 RTF 功能只支持文字和基础格式，无法导入图片、表格或嵌入对象等内容。");
            return false;
        }
    }
    int depth = 0;
    for(qsizetype i = 0; i < content.size(); i++) {
        char c = content[i];
        // 转义后的花括号只是正文，不参与 RTF 分组。
        if(c == '\\') i++;
        else if(c == '{') depth++;
        else if(c == '}' && --depth == 0) {
            if(i != content.size() - 1) depth = -1;
            break;
        }
    }
    if(depth != 0) {
        error = QStringLiteral("RTF 文件的花括号分组不完整。");
        return false;
    }

    RichEdit editor;
    if(editor.hwnd) {
        Input input{content};
        EDITSTREAM stream{reinterpret_cast<DWORD_PTR>(&input), 0, readRtf};
        SendMessageW(editor.hwnd, EM_STREAMIN, SF_RTF, reinterpret_cast<LPARAM>(&stream));
        if(stream.dwError) {
            error = QStringLiteral("RTF 读取失败，请检查文件内容。");
            return false;
        }

        // 不扩展成 CRLF，让 Rich Edit 和 Qt 共用 UTF-16 字符位置。
        constexpr UINT unicode = 1200;
        GETTEXTLENGTHEX length{GTL_PRECISE | GTL_NUMCHARS, unicode};
        int n = static_cast<int>(SendMessageW(editor.hwnd, EM_GETTEXTLENGTHEX,
                                              reinterpret_cast<WPARAM>(&length), 0));
        if(n < 0 || n > 250000) {
            error = QStringLiteral("RTF 文档超过 25 万字符的编辑上限。");
            return false;
        }
        std::vector<wchar_t> buffer(n + 1);
        GETTEXTEX getText{static_cast<DWORD>(buffer.size() * sizeof(wchar_t)), GT_RAWTEXT,
                          unicode, nullptr, nullptr};
        n = static_cast<int>(SendMessageW(editor.hwnd, EM_GETTEXTEX,
            reinterpret_cast<WPARAM>(&getText), reinterpret_cast<LPARAM>(buffer.data())));
        QString text = QString::fromWCharArray(buffer.data(), n);
        document.setPlainText(text.replace('\r', '\n').replace(QChar(0x0b), QChar::LineSeparator));
        QTextCursor cursor(&document);
        for(QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
            int l = block.position();
            int end = l + block.length() - 1;
            cursor.setPosition(l);
            QTextBlockFormat paragraph;
            paragraph.setAlignment(alignmentAt(editor.hwnd, l));
            cursor.setBlockFormat(paragraph);
            // 合并连续同格式的字，减少 Qt 文档中的格式片段。
            while(l < end) {
                int r = l + (text[l].isHighSurrogate() && l + 1 < end ? 2 : 1);
                QTextCharFormat format = charFormatAt(editor.hwnd, l, r - l);
                while(r < end) {
                    int count = text[r].isHighSurrogate() && r + 1 < end ? 2 : 1;
                    if(charFormatAt(editor.hwnd, r, count) != format) break;
                    r += count;
                }
                cursor.setPosition(l);
                cursor.setPosition(r, QTextCursor::KeepAnchor);
                cursor.setCharFormat(format);
                l = r;
            }
        }
        return true;
    }
#endif
    error = QStringLiteral("此电脑上的 Windows Rich Edit 不可用。");
    return false;
}

bool RtfCodec::encode(const QTextDocument &document, QByteArray &data, QString &error) {
#ifdef Q_OS_WIN
    QString text = document.toRawText();
    if(text.contains(QChar::ObjectReplacementCharacter) ||
       !document.rootFrame()->childFrames().isEmpty()) {
        error = QStringLiteral("当前 RTF 功能只支持文字和基础格式，含图片、表格的文档请保存为 HTML。");
        return false;
    }
    if(text.size() > 250000) {
        error = QStringLiteral("RTF 文档超过 25 万字符的编辑上限。");
        return false;
    }
    RichEdit editor;
    if(editor.hwnd) {
        text.replace(QChar::ParagraphSeparator, '\r').replace(QChar::LineSeparator, QChar(0x0b));
        std::wstring nativeText = text.toStdWString();
        SETTEXTEX setText{ST_DEFAULT, 1200};
        SendMessageW(editor.hwnd, EM_SETTEXTEX, reinterpret_cast<WPARAM>(&setText),
                     reinterpret_cast<LPARAM>(nativeText.c_str()));
        GETTEXTLENGTHEX length{GTL_PRECISE | GTL_NUMCHARS, 1200};
        auto n = SendMessageW(editor.hwnd, EM_GETTEXTLENGTHEX,
                              reinterpret_cast<WPARAM>(&length), 0);
        bool ok = n == text.size();
        for(QTextBlock block = document.begin(); ok && block.isValid(); block = block.next()) {
            selectRange(editor.hwnd, block.position(), block.position());
            ok = setAlignment(editor.hwnd, block.blockFormat().alignment());
            for(auto i = block.begin(); ok && !i.atEnd(); ++i) {
                auto fragment = i.fragment();
                if(!fragment.isValid()) continue;
                selectRange(editor.hwnd, fragment.position(), fragment.position() + fragment.length());
                ok = setCharFormat(editor.hwnd, fragment.charFormat(), document.defaultFont());
            }
        }
        QByteArray output;
        EDITSTREAM stream{reinterpret_cast<DWORD_PTR>(&output), 0, writeRtf};
        if(ok) SendMessageW(editor.hwnd, EM_STREAMOUT, SF_RTFNOOBJS, reinterpret_cast<LPARAM>(&stream));
        while(output.endsWith('\0')) output.chop(1);
        if(output.size() > maxFileSize) {
            error = QStringLiteral("转换后的 RTF 文件超过 8 MB。");
            return false;
        }
        if(ok && !stream.dwError && output.startsWith("{\\rtf1")) {
            data = output;
            return true;
        }
        error = QStringLiteral("无法转换为 RTF，原文件未写入。");
        return false;
    }
#endif
    error = QStringLiteral("此电脑上的 Windows Rich Edit 不可用。");
    return false;
}
