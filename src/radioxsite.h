#ifndef RADIOXSITE_H
#define RADIOXSITE_H

#include <QString>

// ---------------------------------------------------------------------------
// Radio X site-specific constants
//
// All URL endpoints and path templates for radiox.de live here.
// When the Radio X website changes its URL structure, update this file.
// ---------------------------------------------------------------------------

namespace RadioXSite {

const QString kBaseUrl       = QStringLiteral("https://www.radiox.de");
const QString kLivestreamUrl = QStringLiteral("http://stream.radiox.de:8000/live");

// Endpoint path templates (use .arg(param) for placeholders)
const QString kPathProgramWeek = QStringLiteral("/plus7/ajax/program_week");
const QString kPathRecordings  = QStringLiteral("/plus7/ajax/plus7_content_all");
const QString kPathSendetipps  = QStringLiteral("/programm/sendetipps");
const QString kPathShowDetail  = QStringLiteral("/sendungen/%1");
const QString kPathPlayerPage  = QStringLiteral("/plus7/ajax/player/%1");

// Regex patterns for extracting audio URLs from player pages
const QString kReAudioSrc  = QStringLiteral("<(?:audio|source)[^>]+src=[\"']([^\"']+)[\"']");
const QString kReAudioFile = QStringLiteral("(https?://[^\\s\"'<>]+\\.(?:mp3|ogg|m4a|aac|opus|oga))");

} // namespace RadioXSite

#endif // RADIOXSITE_H