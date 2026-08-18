#include "bible-engine.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QXmlStreamReader>

namespace wg {

static const QStringList kBookAliases = {
  "Génesis|Genesis|Gen|Gn", "Éxodo|Exodo|Ex|Éx", "Levítico|Levitico|Lev|Lv", "Números|Numeros|Num|Nm",
  "Deuteronomio|Deut|Dt", "Josué|Josue|Jos", "Jueces|Jue|Jc", "Rut|Rt", "1 Samuel|1Sam|1 Sam|1S",
  "2 Samuel|2Sam|2 Sam|2S", "1 Reyes|1Re|1 Re|1R", "2 Reyes|2Re|2 Re|2R", "1 Crónicas|1 Cronicas|1Cr|1 Cr",
  "2 Crónicas|2 Cronicas|2Cr|2 Cr", "Esdras|Esd|Ezr", "Nehemías|Nehemias|Neh", "Ester|Est", "Job",
  "Salmos|Salmo|Sal|Sl", "Proverbios|Prov|Pr", "Eclesiastés|Eclesiastes|Ecl", "Cantares|Cantar de los Cantares|Cant|Cnt",
  "Isaías|Isaias|Isa|Is", "Jeremías|Jeremias|Jer|Jr", "Lamentaciones|Lam|Lm", "Ezequiel|Eze|Ez",
  "Daniel|Dan|Dn", "Oseas|Os", "Joel|Jl", "Amós|Amos|Am", "Abdías|Abdias|Abd", "Jonás|Jonas|Jon",
  "Miqueas|Miq|Mi", "Nahúm|Nahum|Nah", "Habacuc|Hab", "Sofonías|Sofonias|Sof", "Hageo|Hag|Hg",
  "Zacarías|Zacarias|Zac", "Malaquías|Malaquias|Mal", "Mateo|Mat|Mt", "Marcos|Mar|Mc", "Lucas|Luc|Lc",
  "Juan|Jn|Jua", "Hechos|Hch|Hech", "Romanos|Rom|Ro", "1 Corintios|1Cor|1 Cor|1Co", "2 Corintios|2Cor|2 Cor|2Co",
  "Gálatas|Galatas|Gal|Ga", "Efesios|Efe|Ef", "Filipenses|Fil|Flp", "Colosenses|Col", "1 Tesalonicenses|1Tes|1 Tes|1Ts",
  "2 Tesalonicenses|2Tes|2 Tes|2Ts", "1 Timoteo|1Tim|1 Tim|1Ti", "2 Timoteo|2Tim|2 Tim|2Ti", "Tito|Tit",
  "Filemón|Filemon|Flm", "Hebreos|Heb", "Santiago|Stg|Sant", "1 Pedro|1Pe|1 Pe", "2 Pedro|2Pe|2 Pe",
  "1 Juan|1Jn|1 Jn", "2 Juan|2Jn|2 Jn", "3 Juan|3Jn|3 Jn", "Judas|Jud", "Apocalipsis|Apoc|Apo|Ap"
};

BibleEngine::BibleEngine()
{
  buildAliases();
}

QString BibleEngine::normalize(const QString &value)
{
  QString out;
  const QString decomposed = value.trimmed().toLower().normalized(QString::NormalizationForm_D);
  for (const QChar c : decomposed) {
    if (c.category() != QChar::Mark_NonSpacing)
      out += c;
  }
  out.replace(QRegularExpression("\\s+"), " ");
  return out.trimmed();
}

void BibleEngine::buildAliases()
{
  aliases_.clear();
  for (int i = 0; i < kBookAliases.size(); ++i) {
    const auto aliases = kBookAliases[i].split('|');
    for (const QString &alias : aliases)
      aliases_.insert(normalize(alias), i);
  }
}

QString BibleEngine::installedPath() const
{
  const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/worship-graphics/bibles";
  return base + "/RVR1960.xml";
}

bool BibleEngine::installFromXml(const QString &sourcePath, QString *error)
{
  if (sourcePath.isEmpty()) {
    if (error) *error = "No se seleccionó ningún archivo XML.";
    return false;
  }

  const QString target = installedPath();
  QDir().mkpath(QFileInfo(target).absolutePath());
  if (QFile::exists(target)) QFile::remove(target);
  if (!QFile::copy(sourcePath, target)) {
    if (error) *error = "No se pudo copiar la Biblia al almacenamiento local de Worship Graphics.";
    return false;
  }

  QSettings settings("Worship Graphics", "Worship Graphics");
  settings.setValue("bible/rvr1960Path", target);
  return loadXml(target, error);
}

bool BibleEngine::loadInstalled(QString *error)
{
  QSettings settings("Worship Graphics", "Worship Graphics");
  QString path = settings.value("bible/rvr1960Path").toString();
  if (path.isEmpty()) path = installedPath();
  if (!QFile::exists(path)) {
    if (error) *error = "Aún no hay una Biblia RVR1960 instalada localmente.";
    return false;
  }
  return loadXml(path, error);
}

bool BibleEngine::loadXml(const QString &path, QString *error)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (error) *error = "No se pudo abrir el archivo XML.";
    return false;
  }

  QVector<Book> parsed(kBookAliases.size());
  for (int i = 0; i < parsed.size(); ++i)
    parsed[i].name = kBookAliases[i].section('|', 0, 0);

  QXmlStreamReader xml(&file);
  int currentBook = -1;
  int currentChapter = -1;
  QString translation;

  while (!xml.atEnd()) {
    xml.readNext();
    if (!xml.isStartElement()) continue;

    if (xml.name() == QStringLiteral("bible")) {
      translation = xml.attributes().value("translation").toString();
    } else if (xml.name() == QStringLiteral("book")) {
      const int number = xml.attributes().value("number").toInt();
      currentBook = number - 1;
      currentChapter = -1;
    } else if (xml.name() == QStringLiteral("chapter") && currentBook >= 0 && currentBook < parsed.size()) {
      const int number = xml.attributes().value("number").toInt();
      currentChapter = number - 1;
      auto &chapters = parsed[currentBook].chapters;
      if (chapters.size() <= currentChapter) chapters.resize(currentChapter + 1);
    } else if (xml.name() == QStringLiteral("verse") && currentBook >= 0 && currentChapter >= 0) {
      const int number = xml.attributes().value("number").toInt();
      auto &verses = parsed[currentBook].chapters[currentChapter];
      if (verses.size() < number) verses.resize(number);
      verses[number - 1] = xml.readElementText(QXmlStreamReader::IncludeChildElements).trimmed();
    }
  }

  if (xml.hasError()) {
    if (error) *error = "XML inválido: " + xml.errorString();
    return false;
  }

  bool anyVerse = false;
  for (const auto &book : parsed) {
    for (const auto &chapter : book.chapters) {
      if (!chapter.isEmpty()) { anyVerse = true; break; }
    }
    if (anyVerse) break;
  }
  if (!anyVerse) {
    if (error) *error = "El XML no contiene versículos reconocibles.";
    return false;
  }

  books_ = std::move(parsed);
  translationName_ = translation.isEmpty() ? "RVR1960" : translation;
  loaded_ = true;
  return true;
}

QStringList BibleEngine::bookNames() const
{
  QStringList result;
  for (const auto &book : books_) result << book.name;
  return result;
}

int BibleEngine::chapterCount(int bookIndex) const
{
  if (!loaded_ || bookIndex < 0 || bookIndex >= books_.size()) return 0;
  return books_[bookIndex].chapters.size();
}

int BibleEngine::verseCount(int bookIndex, int chapter) const
{
  if (!loaded_ || bookIndex < 0 || bookIndex >= books_.size()) return 0;
  if (chapter <= 0 || chapter > books_[bookIndex].chapters.size()) return 0;
  return books_[bookIndex].chapters[chapter - 1].size();
}

int BibleEngine::findBook(const QString &name) const
{
  return aliases_.value(normalize(name), -1);
}

BiblePassage BibleEngine::passage(int bookIndex, int chapter, int verseStart, int verseEnd) const
{
  BiblePassage result;
  if (!loaded_ || bookIndex < 0 || bookIndex >= books_.size()) return result;
  if (chapter <= 0 || chapter > books_[bookIndex].chapters.size()) return result;

  const auto &verses = books_[bookIndex].chapters[chapter - 1];
  if (verses.isEmpty()) return result;

  if (verseStart <= 0) {
    verseStart = 1;
    verseEnd = verses.size();
  } else if (verseEnd <= 0) {
    verseEnd = verseStart;
  }

  verseStart = qBound(1, verseStart, verses.size());
  verseEnd = qBound(verseStart, verseEnd, verses.size());

  QStringList pieces;
  for (int v = verseStart; v <= verseEnd; ++v) {
    const QString text = verses[v - 1].trimmed();
    if (text.isEmpty()) continue;
    if (verseStart == verseEnd)
      pieces << text;
    else
      pieces << QString::number(v) + "  " + text;
  }
  if (pieces.isEmpty()) return result;

  result.valid = true;
  result.bookIndex = bookIndex;
  result.chapter = chapter;
  result.verseStart = verseStart;
  result.verseEnd = verseEnd;
  result.text = pieces.join("  ");
  result.reference = books_[bookIndex].name + " " + QString::number(chapter);
  if (!(verseStart == 1 && verseEnd == verses.size())) {
    result.reference += ":" + QString::number(verseStart);
    if (verseEnd != verseStart) result.reference += "-" + QString::number(verseEnd);
  }
  return result;
}

BiblePassage BibleEngine::search(const QString &query) const
{
  if (!loaded_) return {};
  const QRegularExpression re(R"(^\s*(.+?)\s+(\d+)(?:\s*[: ]\s*(\d+)(?:\s*-\s*(\d+))?)?\s*$)");
  const auto match = re.match(query.trimmed());
  if (!match.hasMatch()) return {};

  const int book = findBook(match.captured(1));
  const int chapter = match.captured(2).toInt();
  const int start = match.captured(3).isEmpty() ? 0 : match.captured(3).toInt();
  const int end = match.captured(4).isEmpty() ? start : match.captured(4).toInt();
  return passage(book, chapter, start, end);
}

BiblePassage BibleEngine::adjacent(const BiblePassage &current, int delta) const
{
  if (!current.valid || delta == 0) return current;
  int book = current.bookIndex;
  int chapter = current.chapter;
  int verse = delta > 0 ? current.verseEnd + 1 : current.verseStart - 1;

  if (delta > 0 && verse > verseCount(book, chapter)) {
    ++chapter; verse = 1;
    if (chapter > chapterCount(book)) {
      ++book; chapter = 1; verse = 1;
      if (book >= books_.size()) return current;
    }
  } else if (delta < 0 && verse < 1) {
    --chapter;
    if (chapter < 1) {
      --book;
      if (book < 0) return current;
      chapter = chapterCount(book);
    }
    verse = verseCount(book, chapter);
  }
  return passage(book, chapter, verse, verse);
}

} // namespace wg
