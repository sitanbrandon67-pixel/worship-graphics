#pragma once

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

namespace wg {

struct BiblePassage {
  bool valid = false;
  QString reference;
  QString text;
  int bookIndex = -1;
  int chapter = 0;
  int verseStart = 0;
  int verseEnd = 0;
};

class BibleEngine {
public:
  BibleEngine();

  bool installFromXml(const QString &sourcePath, QString *error = nullptr);
  bool loadInstalled(QString *error = nullptr);
  bool loadXml(const QString &path, QString *error = nullptr);

  bool isLoaded() const { return loaded_; }
  QString translationName() const { return translationName_; }
  QString installedPath() const;

  QStringList bookNames() const;
  QStringList bookSuggestions(const QString &query, int limit = 8) const;
  QString completeReference(const QString &query) const;
  int chapterCount(int bookIndex) const;
  int verseCount(int bookIndex, int chapter) const;

  BiblePassage search(const QString &query) const;
  BiblePassage passage(int bookIndex, int chapter, int verseStart = 0, int verseEnd = 0) const;
  BiblePassage adjacent(const BiblePassage &current, int delta) const;

private:
  struct Book {
    QString name;
    QVector<QVector<QString>> chapters;
  };

  static QString normalize(const QString &value);
  static QString canonicalBookName(int index);
  static QString bookPartOf(const QString &query);
  void buildAliases();
  int findBook(const QString &name) const;

  QVector<Book> books_;
  QHash<QString, int> aliases_;
  QString translationName_;
  bool loaded_ = false;
};

} // namespace wg
