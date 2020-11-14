#pragma once

#include <QMap>
#include <QString>

namespace hackathon
{
  namespace query
  {
    const static QString dropTable = "drop table %1";

    const static QString getMaxIndex =
      "select coalesce(max(id), 0) as maxid from %1";

    const static QString tableExist =
      "select count(*)"
      "  from sqlite_master"
      " where type = 'table'"
      "   and lower(name) = lower(:name)";

    namespace spatial
    {
      // spatial table (r*tree)
      const static QString tableName = "point_tree";

      const static QString createQuery =
        "create virtual table point_tree using rtree ("
        " id,"
        " minx,"
        " maxx,"
        " miny,"
        " maxy,"
        " minz,"
        " maxz,"
        " +x real,"
        " +y real,"
        " +z real,"
        " +classification integer,"
        " +intensity integer,"
        " +gps_time real,"
        " +point_source_id integer,"
        " +color_hex_argb text,"
        " +color_a integer,"
        " +color_r integer,"
        " +color_g integer,"
        " +color_b integer,"
        ")";

      const static QString insertQuery =
        "insert into point_tree"
        " (id, minx, maxx, miny, maxy, minz, maxz,"
        "  x, y, z, classification, intensity,"
        "  gps_time, point_source_id, color_hex_argb,"
        "  color_a, color_r, color_g, color_b)"
        "values"
        " (:id, :minx, :maxx, :miny, :maxy, :minz, :maxz,"
        "  :x, :y, :z, :classification, :intensity,"
        "  :gpstime, :pointsourceid, :colorargb,"
        "  :colora, :colorr, :colorg, :colorb)";
    } // namespace spatial

    namespace classification
    {
    // classification code tables
    const static QString tableName = "classification";

    const static QString createQuery =
      "create table classification ("
      " id integer primary key,"
      " description text not null"
      ")";

    const static QString insertQuery =
      "insert into classification"
      " (id, description)"
      "values"
      " (:id, :description)";

    const static QMap<int, QString> tableData =
    {
      { 1, "unclassified" },
      { 2, "ground" },
      { 6, "buildings" },
      { 9, "water" },
      { 26, "kunstwerk" }
    };
    } // namespace classification
  } // namespace query
} // namespace hackathon
