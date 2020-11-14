#pragma once

#include <pdal/Streamable.hpp>
#include <pdal/Writer.hpp>
#include <QColor>
#include <QCoreApplication>
#include <QHash>
#include <QImage>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include "cache.h"
#include "settings.h"
#include "storage.h"

namespace pdal
{
  /// Class to write point data to an SQLite database.
  class PDAL_DLL HackathonWriter : public Writer, public Streamable
  {
  private:
    hackathon::Settings m_settings;
    hackathon::Cache m_cache;
    int32_t m_currentCacheSize;
    std::unique_ptr<hackathon::Storage> m_storage;
    int64_t m_currentId;
    QCoreApplication m_app;
    QHash<QString, QImage> m_imageMap;

  public:
    /// Constructor.
    HackathonWriter();

    /// Destructor.
    ~HackathonWriter();

    /// Copy constructor, disabled.
    ///
    /// \param[in] that Object to copy.
    HackathonWriter(const HackathonWriter &that) = delete;

    /// Move constructor, disabled.
    ///
    /// \param[in] that Object to copy move.
    HackathonWriter(HackathonWriter &&that) = delete;

    /// Assignment operator, disabled.
    ///
    /// \param[in] that Object to assign.
    HackathonWriter &operator=(const HackathonWriter &that) = delete;

    /// Move assignment operator, disabled.
    ///
    /// \param[in] that Object to move assign.
    HackathonWriter& operator=(HackathonWriter &&that) = delete;

    /// Returns the name of the writer.
    ///
    /// \return Name of writer
    std::string getName() const;

  private:
    virtual void addArgs(ProgramArgs &args);
    virtual void initialize();
    virtual void ready(PointTableRef table);
    virtual void write(const PointViewPtr view);
    virtual bool processOne(PointRef &point);
    virtual void done(PointTableRef table);

    bool flushCache();

    bool loadImages(const QString &path);

    QColor findColor(double x, double y);
  };
} // namespace pdal
