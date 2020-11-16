#include "hackathonwriter.h"
#include <pdal/PointView.hpp>
#include <pdal/pdal_features.hpp>
#include <pdal/util/FileUtils.hpp>
#include <pdal/util/ProgramArgs.hpp>
#include <QDir>
#include <QSqlQuery>
#include "sqlquery.h"

namespace pdal
{
  static PluginInfo const pluginInfo
  {
    "writers.hackathontoo",
    "HackathonToo Writer",
    "http://path/to/documentation"
  };

  CREATE_SHARED_STAGE(HackathonWriter, pluginInfo);

  char *args = "pdal";
  int argc = 1;

  HackathonWriter::HackathonWriter()
    : m_settings()
    , m_cache()
    , m_currentCacheSize(0)
    , m_storage()
    , m_currentId(0)
    , m_app(argc, &args)
    , m_imageMap()
  {
  }

  HackathonWriter::~HackathonWriter()
  {
  }

  std::string HackathonWriter::getName() const
  {
    log()->get(LogLevel::Debug1) << "getName() was called\n";

    return pluginInfo.name;
  }

  void HackathonWriter::addArgs(ProgramArgs &args)
  {
    log()->get(LogLevel::Debug1) << "addArgs() was called\n";

    // connection to sqlite database
    args.add("connection", "SQL connection string", m_settings.connectionString).setPositional();
    args.addSynonym("connection", "filename");

    // overwrite flag
    args.add("overwrite", "Whether existing data should be overwritten (default is false)", m_settings.overwrite);

    // cache size
    args.add("cachesize", "Cache size in points", m_settings.cacheSize);

    // image path
    args.add("imagePath", "Path to folder with arial images (png)", m_settings.imagePath);
  }

  void HackathonWriter::initialize()
  {
    log()->get(LogLevel::Debug1) << "initialize() was called\n";

    if (!loadImages(QString::fromStdString(m_settings.imagePath)))
    {
      throwError("Cannot load images");
    }

    log()->get(LogLevel::Debug) << "Connection: '" << m_settings.connectionString << "'" << std::endl;

    // initialize storage
    m_storage = std::make_unique<hackathon::Storage>(log(), QString::fromStdString(m_settings.connectionString));

    if (!m_storage->connect())
    {
      throwError("Unable to connect to database");
    }

    log()->get(LogLevel::Debug) << "Connected to database\n";
  }

  void HackathonWriter::ready(PointTableRef table)
  {
    log()->get(LogLevel::Debug1) << "ready() was called\n";

    const QString spatialTable = hackathon::query::spatial::tableName;
    const QString classificationTable = hackathon::query::classification::tableName;

    // check if tables exist
    bool hasSpatialTable = false;
    bool hasClassificationTable = false;

    if (!m_storage->doesTableExist(spatialTable, hasSpatialTable))
    {
      throwError(QString("Unable to determine if table '%1' exists").arg(spatialTable).toStdString());
    }

    if (!m_storage->doesTableExist(classificationTable, hasClassificationTable))
    {
      throwError(QString("Unable to determine if table '%1' exists").arg(classificationTable).toStdString());
    }

    log()->get(LogLevel::Debug) << "hasSpatialTable '" << hasSpatialTable << "'\n";
    log()->get(LogLevel::Debug) << "hasClassificationTable '" << hasClassificationTable << "'\n";

    if (m_settings.overwrite)
    {
      // overwrite requested, delete tables
      if (hasSpatialTable)
      {
        if (!m_storage->deleteTable(spatialTable))
        {
          throwError(QString("Cannot delete table '%1'").arg(spatialTable).toStdString());
        }

        hasSpatialTable = false;
      }

      if (hasClassificationTable)
      {
        if (!m_storage->deleteTable(classificationTable))
        {
          throwError(QString("Cannot delete table '%1'").arg(classificationTable).toStdString());
        }

        hasClassificationTable = false;
      }
    }

    // create tables
    if (!hasSpatialTable)
    {
      if (!m_storage->createTable(spatialTable, hackathon::query::spatial::createQuery))
      {
        throwError(QString("Cannot create table '%1'").arg(spatialTable).toStdString());
      }
    }

    if (!hasClassificationTable)
    {
      if (!m_storage->createTable(classificationTable, hackathon::query::classification::createQuery))
      {
        throwError(QString("Cannot create table '%1'").arg(classificationTable).toStdString());
      }

      if (!m_storage->fillClassificationTable())
      {
        throwError(QString("Cannot fill table '%1'").arg(classificationTable).toStdString());
      }
    }

    // get max id
    long long spatialMaxId = 0;

    if (!m_storage->getMaxDataId(spatialTable, spatialMaxId))
    {
      throwError(QString("Unable to determine maximum id for table '%1'").arg(spatialTable).toStdString());
    }

    m_currentId = spatialMaxId;

    // clear cache
    m_cache.clear();
    m_currentCacheSize = 0;

    // HACK: reconnect to storage to prevent following messages:
    //  SQLite code: 17 msg: 'statement aborts at 7: [SELECT nodeno FROM
    //  'main'.'point_tree_rowid' WHERE rowid = ?1] database schema has changed'
    //  SQLite code: 17 msg: 'statement aborts at 12: [INSERT OR REPLACE INTO
    //  'main'.'point_tree_rowid' VALUES(?1, ?2)] database schema has changed'
    //  SQLite code: 17 msg: 'statement aborts at 12: [INSERT OR REPLACE INTO
    //  'main'.'point_tree_node' VALUES(?1, ?2)] database schema has changed'
    //  Inserted 0 points
    //  SQLite code: 17 msg: 'statement aborts at 12: [INSERT OR REPLACE INTO
    //  'main'.'point_tree_parent' VALUES(?1, ?2)] database schema has changed'
    if (!m_storage->connect(true))
    {
      throwError("Unable to connect to database");
    }
  }

  void HackathonWriter::write(const PointViewPtr view)
  {
    log()->get(LogLevel::Debug1) << "write() was called\n";

    PointRef point(*view, 0);

    for (PointId idx = 0; idx < view->size(); ++idx)
    {
      point.setPointId(idx);
      processOne(point);
    }
  }

  bool HackathonWriter::processOne(PointRef &point)
  {
    // get fields
    const double x = point.getFieldAs<double>(Dimension::Id::X);
    const double y = point.getFieldAs<double>(Dimension::Id::Y);
    const double z = point.getFieldAs<double>(Dimension::Id::Z);
    const int classification = point.getFieldAs<int>(Dimension::Id::Classification);
    const int intensity = point.getFieldAs<int>(Dimension::Id::Intensity);
    const double gpsTime = point.getFieldAs<double>(Dimension::Id::GpsTime);
    const int64_t pointSourceId = point.getFieldAs<int64_t>(Dimension::Id::PointSourceId);

    // get color
    const QColor color = findColor(x, y);

    // add to cache
    m_cache.id.append(++m_currentId);
    m_cache.minX.append(x - 0.01);
    m_cache.maxX.append(x + 0.01);
    m_cache.minY.append(y - 0.01);
    m_cache.maxY.append(y + 0.01);
    m_cache.minZ.append(z - 0.01);
    m_cache.maxZ.append(z + 0.01);
    m_cache.x.append(x);
    m_cache.y.append(y);
    m_cache.z.append(z);
    m_cache.classification.append(classification);
    m_cache.intensity.append(intensity);
    m_cache.gpsTime.append(gpsTime);
    m_cache.pointSourceId.append(pointSourceId);
    m_cache.colorArgb.append(color.name());
    m_cache.colorA.append(color.alpha());
    m_cache.colorR.append(color.red());
    m_cache.colorG.append(color.green());
    m_cache.colorB.append(color.blue());

    if (++m_currentCacheSize == m_settings.cacheSize)
    {
      // flush cache
      if (!flushCache())
      {
        throwError("Unable to flush points to database");
      }
    }

    return true;
  }

  void HackathonWriter::done(PointTableRef table)
  {
    log()->get(LogLevel::Debug1) << "done() was called\n";

    if (!flushCache())
    {
      throwError("Unable to flush points to database");
    }
  }

  bool HackathonWriter::flushCache()
  {
    // write cache to database
    if (!m_storage->writeCache(m_cache))
    {
      return false;
    }

    // clear cache
    m_cache.clear();
    m_currentCacheSize = 0;

    return true;
  }

  bool HackathonWriter::loadImages(const QString &path)
  {
    log()->get(LogLevel::Debug1) << "loadImages() was called\n";

    m_imageMap.clear();

    // get folder info
    QDir imageDir(path, "*.png", QDir::Name | QDir::IgnoreCase, QDir::Files);
    
    if (!imageDir.exists())
    {
      log()->get(LogLevel::Debug1) << "Image folder '" << path.toStdString() << "' does not exist\n";

      return false;
    }

    // get file entries
    QFileInfoList fileInfo = imageDir.entryInfoList();

    for (auto iter = fileInfo.cbegin(); iter != fileInfo.cend(); ++iter)
    {
      log()->get(LogLevel::Debug1) << "Loading image file '" << iter->canonicalFilePath().toStdString() << "'\n";

      // use base of file name as key
      const QString key = iter->baseName();

      // load image
      QImage image;

      if (!image.load(iter->canonicalFilePath()))
      {
        // image loading failed, skip
        log()->get(LogLevel::Error) << "Cannot load image\n";

        continue;
      }

      // add to hash
      log()->get(LogLevel::Debug1) << "Storing image file with key '" << key.toStdString() << "'\n";

      m_imageMap[key] = image;
    }

    return true;
  }

  QColor HackathonWriter::findColor(double x, double y)
  {
    log()->get(LogLevel::Debug2) << "findColor() was called\n";
    log()->get(LogLevel::Debug2) << " - coordinates (" << x << ", " << y << ")\n";

    const double tileX = (x - 94000.0) / 250.0;
    const double tileY = (y - 399000.0) / 250.0;

    log()->get(LogLevel::Debug2) << " - tile (" << tileX << ", " << tileY << ")\n";

    const unsigned int imageX = static_cast<unsigned int>(tileX) * 250 + 94000;
    const unsigned int imageY = static_cast<unsigned int>(tileY) * 250 + 399000;

    const QString key = QString("%1_%2").arg(imageX).arg(imageY);

    log()->get(LogLevel::Debug2) << "- image key: " << key.toStdString() << "\n";

    const double fracX = tileX - static_cast<double>(static_cast<unsigned int>(tileX));
    const double fracY = tileY - static_cast<double>(static_cast<unsigned int>(tileY));

    log()->get(LogLevel::Debug2) << " - frac (" << fracX << ", " << fracY << ")\n";

    const unsigned int pixelX = static_cast<unsigned int>(fracX * 1000);
    const unsigned int pixelY = 999 - static_cast<unsigned int>(fracY * 1000);

    log()->get(LogLevel::Debug2) << "- pixel (" << pixelX << ", " << pixelY << ")\n";

    auto foundImage = m_imageMap.constFind(key);

    if (foundImage != m_imageMap.cend())
    {
      log()->get(LogLevel::Debug2) << "Found image, getting pixel\n";

      const  QColor color = m_imageMap[key].pixelColor(pixelX, pixelY);
      log()->get(LogLevel::Debug2) << "- color is '" << color.name().toStdString() << "'\n";

      return color;
    }

    log()->get(LogLevel::Warning) << "Image not found for coordinates (" << x << ", " << y<< "), defaulting to black\n";

    // default is black
    return QColor("black");
  }
} // namespace pdal
