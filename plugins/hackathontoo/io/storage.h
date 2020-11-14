#pragma once

#include <memory>
#include <optional>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <pdal/Log.hpp>
#include "cache.h"

namespace hackathon
{
  /// Class to manage access to an SQLite database.
  class Storage
  {
  private:
    pdal::LogPtr m_log;
    QString m_connectionString;
    QString m_connectionName;
    std::unique_ptr<QSqlDatabase> m_db;

  public:
    /// Constructor.
    ///
    /// \param[in] log PDAL logger.
    /// \param[in] connectionString Connection string for SQLite database.
    /// \param[in] connectionName Name for database connection, default is
    /// empty name.
    Storage(pdal::LogPtr log,
            const QString &connectionString,
            const QString &connectionName = QString());

    /// Destructor.
    ~Storage();

    /// Copy constructor, disabled.
    ///
    /// \param[in] that Object to copy.
    Storage(const Storage &that) = delete;

    /// Move constructor, disabled.
    ///
    /// \param[in] that Object to copy move.
    Storage(Storage &&that) = delete;

    /// Assignment operator, disabled.
    ///
    /// \param[in] that Object to assign.
    Storage &operator=(const Storage &that) = delete;

    /// Move assignment operator, disabled.
    ///
    /// \param[in] that Object to move assign.
    Storage& operator=(Storage &&that) = delete;

    /// Opens connection to database.
    ///
    /// \param[in] reconnect True to close database connection and reconnect.
    /// \return True if successful, false on errors.
    bool connect(bool reconnect = false);

    /// Indicates whether there is still a connection to the database. A known
    /// query (select 1 from dual) is executed to test the connection.
    ///
    /// \return True if connected to database, false if connection is lost.
    bool isConnected() const;

    /// Starts a transaction on the database.
    ///
    /// \return True is successful, false on errors.
    bool beginTransaction();

    /// Rolls back a transaction on the database.
    ///
    /// \return True is successful, false on errors.
    bool rollbackTransaction();

    /// Commits a transaction on the database. Performs a rollback if the
    /// commit fails.
    ///
    /// \return True is successful, false on errors.
    bool commitTransaction();

    /// Indicates whether the table with the given name exists.
    ///
    /// \param[in] tableName Name of table to check
    /// \param[out] exists True if table exists, false if table does not exist.
    /// \return True if successful, false on errors
    bool doesTableExist(const QString &tableName, bool &exists);

    /// Creates a table using the given name and create query. Does not
    /// perform a check to see if the table already exists.
    ///
    /// \param[in] tableName Name of table, used for logging only.
    /// \param]in] createQuery SQL query to create table.
    /// \return True if successful, false on error.
    bool createTable(const QString &tableName,
                     const QString &createQuery);

    /// Deletes the table with the given name. Does not perform a check to
    /// see if the table exists.
    ///
    /// \param[in] tableName Name of table to delete.
    /// \return True if successful, false on error.
    bool deleteTable(const QString &tableName);

    /// Fills the classification table with data. Does not perform a check to
    /// see if the table exists
    ///
    /// \return True if successful, false on error.
    bool fillClassificationTable();

    /// Create an index using the given name and create query. Does not
    /// perform a check to see if the index already exists.
    ///
    /// \param[in] indexName Name of index, used for logging only.
    /// \param[in] createQuery SQL query to create index.
    /// \return True if successful, false on error/
    bool createIndex(const QString &indexName,
                     const QString &createQuery);

    /// Returns the maximum value of the column 'id' for the given table. Does
    /// not perform a check to see if the table exists.
    ///
    /// \param[in] tableName Name of table to get maximum id of.
    /// \param[out] maxId Maximum id.
    /// \return True if successful, false on error.
    bool getMaxDataId(const QString &tableName, long long &maxId);

    /// Writes cache to database.
    ///
    /// \param[in] cache Cache to write.
    /// \return True if succesful, false on error.
    bool writeCache(const Cache &cache);

  protected:
    /// Logs the given SQL error if it is valid.
    ///
    /// \param[in] message Error message, the SQL error information will be
    /// appended to this message.
    /// \param[in] sqlError SQL error to log.
    void logSqlError(const QString &message,
                     const QSqlError &sqlError) const;
  };
} // namespace hackathon
