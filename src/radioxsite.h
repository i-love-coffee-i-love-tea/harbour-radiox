#ifndef RADIOXSITE_H
#define RADIOXSITE_H

#include <QString>

// ---------------------------------------------------------------------------
// Radio X site-specific constants
//
// All URL endpoints and path templates for radiox.de live here.
// When the Radio X website changes its URL structure, update this file.
//
// Inline getter functions returning const QString& avoid ODR violations
// while keeping the .arg() convenience at call sites.
// ---------------------------------------------------------------------------

namespace RadioXSite {

inline const QString& kBaseUrl()
{
    static const QString s(QStringLiteral("https://www.radiox.de"));
    return s;
}

inline const QString& kLivestreamUrl()
{
    static const QString s(QStringLiteral("http://stream.radiox.de:8000/live"));
    return s;
}

// Endpoint path templates (use .arg(param) for placeholders)
inline const QString& kPathProgramWeek()
{
    static const QString s(QStringLiteral("/plus7/ajax/program_week"));
    return s;
}

inline const QString& kPathRecordings()
{
    static const QString s(QStringLiteral("/plus7/ajax/plus7_content_all"));
    return s;
}

inline const QString& kPathSendetipps()
{
    static const QString s(QStringLiteral("/programm/sendetipps"));
    return s;
}

inline const QString& kPathShowDetail()
{
    static const QString s(QStringLiteral("/sendungen/%1"));
    return s;
}

inline const QString& kPathPlayerPage()
{
    static const QString s(QStringLiteral("/plus7/ajax/player/%1"));
    return s;
}

// Regex patterns for extracting audio URLs from player pages
constexpr const char* kReAudioSrc  = "<(?:audio|source)[^>]+src=[\"']([^\"']+)[\"']";
constexpr const char* kReAudioFile = "(https?://[^\\s\"'<>]+\\.(?:mp3|ogg|m4a|aac|opus|oga))";

} // namespace RadioXSite

#endif // RADIOXSITE_H