#include "storage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include "sqlquery.h"

namespace hackathon
{
  Storage::Storage(pdal::LogPtr log, const QString &connectionString, const QString &connectionName)
    : m_log(log)
    , m_connectionString(connectionString)
    , m_connectionName(connectionName)
    , m_db(nullptr)
  {
    if (!m_connectionName.isEmpty())
    {
      // try to get named connection
      QSqlDatabase db = QSqlDatabase::database(m_connectionName);

      if (!db.isValid())
      {
        // not valid, add
        db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
      }

      m_db = std::make_unique<QSqlDatabase>(db);
    }
    else
    {
      // nameless connection
      m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE"));
    }
  }

  Storage::~Storage()
  {
    // close connection
    if (m_db)
    {
      if (m_db->isOpen())
      {
        m_db->close();
      }

      m_db = nullptr;
    }

    // remove named connection from list
    if (!m_connectionName.isEmpty())
    {
      QSqlDatabase::removeDatabase(m_connectionName);
    }
  }

  bool Storage::connect(bool reconnect)
  {
    if (m_db->isOpen())
    {
      if (reconnect)
      {
        // close
        m_db->close();
      }
      else
      {
        // database already open
        return true;
      }
    }

    // set database name to connection string
    m_db->setDatabaseName(m_connectionString);

    if (!m_db->open())
    {
      // log error information if different from previous
      QSqlError connectError = m_db->lastError();

      logSqlError(QString("Cannot open \"%1\" connection to database, will retry").arg(m_connectionName),
        connectError);

      return false;
    }

    m_log->get(pdal::LogLevel::Info) << m_connectionName.toStdString() << " connected\n";

    return true;
  }

  bool Storage::isConnected() const
  {
    if (!m_db->isOpen())
    {
      // not open
      return false;
    }

    // prepare query
    QSqlQuery query("select 1 from dual", *m_db);

    if (!query.exec())
    {
      // not connected
      query.finish();

      return false;
    }

    // connected
    query.finish();

    return true;
  }

  bool Storage::beginTransaction()
  {
    m_log->get(pdal::LogLevel::Debug1) << "beginTransaction was called\n";

    // start database transaction
    if (!m_db->transaction())
    {
      m_log->get(pdal::LogLevel::Error) << "Cannot start database transaction. "
        << m_db->lastError().databaseText().toStdString() << std::endl;

      return false;
    }

    return true;
  }

  bool Storage::rollbackTransaction()
  {
    m_log->get(pdal::LogLevel::Debug1) << "rollbackTransaction was called\n";

    // rollback database transaction
    if (!m_db->rollback())
    {
      m_log->get(pdal::LogLevel::Error) << "Cannot rollback database transaction. "
        << m_db->lastError().databaseText().toStdString() << std::endl;

      return false;
    }

    return true;
  }

  bool Storage::commitTransaction()
  {
    m_log->get(pdal::LogLevel::Debug1) << "commitTransaction was called\n";

    // commit database transaction
    if (!m_db->commit())
    {
      // rollback
      m_log->get(pdal::LogLevel::Error) << "Cannot commit in database. "
        << m_db->lastError().databaseText().toStdString() << std::endl;

      m_db->rollback();

      return false;
    }

    return true;
  }

  bool Storage::doesTableExist(const QString &tableName, bool &exists)
  {
    m_log->get(pdal::LogLevel::Debug1) << "doesTableExist() was called\n";

    exists = false;

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(query::tableExist);

    // set name
    query.bindValue(":name", tableName);

    // execute query
    if (!query.exec())
    {
      // log error information
      logSqlError(QString("Cannot determine if table '%1' exists").arg(tableName), query.lastError());

      query.finish();

      return false;
    }

    // read records
    if (query.next())
    {
      bool ok = false;

      const int count = query.value(0).toInt(&ok);

      if (!ok)
      {
        // value is not a valid number
        m_log->get(pdal::LogLevel::Error) << "Value '" << query.value(0).toString().toStdString()
          << "' for count is not a valid number\n";

        query.finish();

        return false;
      }

      query.finish();
      
      // set output
      exists = (count > 0);

      return true;
    }

    // no record, report as error
    query.finish();

    return false;
  }
  
  bool Storage::createTable(const QString &tableName, const QString &createQuery)
  {
    m_log->get(pdal::LogLevel::Debug1) << "createTable() was called\n";

    m_log->get(pdal::LogLevel::Debug) << "Creating table '" << tableName.toStdString() << "'\n";

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(createQuery);

    // execute query
    if (!query.exec())
    {
      // log error information
      logSqlError(QString("Cannot create table '%1'").arg(tableName), query.lastError());

      query.finish();

      return false;
    }

    query.finish();

    m_log->get(pdal::LogLevel::Debug) << "Created table '" << tableName.toStdString() << "'\n";

    return true;
  }

  bool Storage::deleteTable(const QString &tableName)
  {
    m_log->get(pdal::LogLevel::Debug1) << "deleteTable() was called\n";

    m_log->get(pdal::LogLevel::Debug) << "Dropping table '" << tableName.toStdString() << "'\n";

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(query::dropTable.arg(tableName));

    // execute query
    if (!query.exec())
    {
      // log error information
      logSqlError(QString("Cannot drop table '%1'").arg(tableName), query.lastError());

      query.finish();

      return false;
    }

    query.finish();

    m_log->get(pdal::LogLevel::Debug) << "Dropped table '" << tableName.toStdString() << "'\n";

    return true;
  }

  bool Storage::fillClassificationTable()
  {
    m_log->get(pdal::LogLevel::Debug1) << "fillClassificationTable() was called\n";

    const QString tableName = query::classification::tableName;

    m_log->get(pdal::LogLevel::Debug) << "Filling table '" << tableName.toStdString() << "'\n";

    if (!beginTransaction())
    {
      // error
      return false;
    }

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(query::classification::insertQuery);

    // prepare data in lists
    QVariantList keys;
    QVariantList descriptions;

    for (auto iter = query::classification::tableData.constKeyValueBegin();
      iter != query::classification::tableData.constKeyValueEnd(); ++iter)
    {
      keys << (*iter).first;
      descriptions << (*iter).second;
    }

    // bind lists to query
    query.addBindValue(keys);
    query.addBindValue(descriptions);

    // batch insert in table
    if (!query.execBatch())
    {
      // log error information
      logSqlError(QString("Cannot insert data into table '%1'").arg(tableName),
        query.lastError());

      query.finish();

      // write error, rollback
      rollbackTransaction();

      return false;
    }

    query.finish();

    if (!commitTransaction())
    {
      // commit error, rollback was performed
      return false;
    }

    m_log->get(pdal::LogLevel::Debug) << "Inserted data into table '" << tableName.toStdString() << "'\n";

    return true;
  }

  bool Storage::createIndex(const QString &indexName, const QString &createQuery)
  {
    m_log->get(pdal::LogLevel::Debug1) << "createIndex() was called\n";

    m_log->get(pdal::LogLevel::Debug) << "Creating index '" << indexName.toStdString() << "'\n";

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(createQuery);

    // execute query
    if (!query.exec())
    {
      // log error information
      logSqlError(QString("Cannot create index '%1'").arg(indexName), query.lastError());

      query.finish();

      return false;
    }

    query.finish();

    m_log->get(pdal::LogLevel::Debug) << "Created index '" << indexName.toStdString() << "'\n";

    return true;
  }

  bool Storage::getMaxDataId(const QString &tableName, long long &maxId)
  {
    m_log->get(pdal::LogLevel::Debug1) << "getMaxDataId() was called\n";

    maxId = 0;

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(query::getMaxIndex.arg(tableName));

    // execute query
    if (!query.exec())
    {
      // log error information
      logSqlError(QString("Cannot get maximum id for table '%1'").arg(tableName), query.lastError());

      query.finish();

      return false;
    }

    // read records
    if (query.next())
    {
      bool ok = false;

      maxId = query.value(0).toLongLong(&ok);

      if (!ok)
      {
        // value is not a valid number
        m_log->get(pdal::LogLevel::Error) << "Value '" << query.value(0).toString().toStdString()
          << "' for maximum id is not a valid number\n";

        query.finish();

        return false;
      }

      query.finish();

      return true;
    }

    // no record, report as error
    query.finish();

    return false;
  }
  
  bool Storage::writeCache(const Cache &cache)
  {
    m_log->get(pdal::LogLevel::Debug1) << "writeCache() was called\n";

    const QString tableName = query::spatial::tableName;

    m_log->get(pdal::LogLevel::Debug) << "Inserting " << cache.id.size() << " points into table '"
      << tableName.toStdString() << "'\n";

    if (!beginTransaction())
    {
      // error
      return false;
    }

    // prepare query
    QSqlQuery query(*m_db);

    query.prepare(query::spatial::insertQuery);

    // bind lists to query
    query.addBindValue(cache.id);
    query.addBindValue(cache.minX);
    query.addBindValue(cache.maxX);
    query.addBindValue(cache.minY);
    query.addBindValue(cache.maxY);
    query.addBindValue(cache.minZ);
    query.addBindValue(cache.maxZ);
    query.addBindValue(cache.x);
    query.addBindValue(cache.y);
    query.addBindValue(cache.z);
    query.addBindValue(cache.classification);
    query.addBindValue(cache.intensity);
    query.addBindValue(cache.gpsTime);
    query.addBindValue(cache.pointId);
    query.addBindValue(cache.pointSourceId);

    // batch insert in table
    if (!query.execBatch())
    {
      // log error information
      logSqlError(QString("Cannot insert data into table '%1'").arg(tableName),
        query.lastError());

      query.finish();

      // write error, rollback
      rollbackTransaction();

      return false;
    }

    query.finish();

    if (!commitTransaction())
    {
      // commit error, rollback was performed
      return false;
    }

    m_log->get(pdal::LogLevel::Debug) << "Inserted data into table '" << tableName.toStdString() << "'\n";

    return true;
  }

  void Storage::logSqlError(const QString &message, const QSqlError &sqlError) const
  {
    if ((sqlError.isValid()) && (sqlError.type() != QSqlError::NoError))
    {
      QString errorMsg = QString("%1. %2 (number %3)")
                         .arg(message)
                         .arg(sqlError.text())
                         .arg(sqlError.number());

      m_log->get(pdal::LogLevel::Error) << errorMsg.toStdString() << std::endl;
    }
  }
} // namespace hackathon
