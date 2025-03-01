#pragma once

#include <Parsers/IAST.h>
#include <Parsers/ASTQueryWithOnCluster.h>


namespace DB
{
using Strings = std::vector<String>;
using DatabaseAndTableName = std::pair<String, String>;


/** BACKUP { TABLE [db.]table_name [AS [db.]table_name_in_backup] [PARTITION[S] partition_expr [,...]] |
  *          DICTIONARY [db.]dictionary_name [AS [db.]dictionary_name_in_backup] |
  *          TEMPORARY TABLE table_name [AS table_name_in_backup] |
  *          ALL TEMPORARY TABLES [EXCEPT ...] |
  *          DATABASE database_name [EXCEPT ...] [AS database_name_in_backup] |
  *          ALL DATABASES [EXCEPT ...] } [,...]
  *        [ON CLUSTER 'cluster_name']
  *        TO { File('path/') |
  *             Disk('disk_name', 'path/')
  *        [SETTINGS base_backup = {File(...) | Disk(...)}]
  *
  * RESTORE { TABLE [db.]table_name_in_backup [INTO [db.]table_name] [PARTITION[S] partition_expr [,...]] |
  *           DICTIONARY [db.]dictionary_name_in_backup [INTO [db.]dictionary_name] |
  *           TEMPORARY TABLE table_name_in_backup [INTO table_name] |
  *           ALL TEMPORARY TABLES [EXCEPT ...] |
  *           DATABASE database_name_in_backup [EXCEPT ...] [INTO database_name] |
  *           ALL DATABASES [EXCEPT ...] } [,...]
  *         [ON CLUSTER 'cluster_name']
  *         FROM {File(...) | Disk(...)}
  *
  * Notes:
  * RESTORE doesn't drop any data, it either creates a table or appends an existing table with restored data.
  * This behaviour can cause data duplication.
  * If appending isn't possible because the existing table has incompatible format then RESTORE will throw an exception.
  *
  * The "UNDER NAME" clause is useful to backup or restore under another name.
  * For the BACKUP command this clause allows to set the name which an object will have inside the backup.
  * And for the RESTORE command this clause allows to set the name which an object will have after RESTORE has finished.
  *
  * "ALL DATABASES" means all databases except the system database and the internal database containing temporary tables.
  * "EVERYTHING" works exactly as "ALL DATABASES, ALL TEMPORARY TABLES"
  *
  * The "WITH BASE" clause allows to set a base backup. Only differences made after the base backup will be
  * included in a newly created backup, so this option allows to make an incremental backup.
  */
class ASTBackupQuery : public IAST, public ASTQueryWithOnCluster
{
public:
    enum Kind
    {
        BACKUP,
        RESTORE,
    };
    Kind kind = Kind::BACKUP;

    enum ElementType
    {
        TABLE,
        DATABASE,
        ALL_DATABASES,
    };

    struct Element
    {
        ElementType type;
        DatabaseAndTableName name;
        DatabaseAndTableName new_name;
        bool name_is_in_temp_db = false;
        bool new_name_is_in_temp_db = false;
        ASTs partitions;
        std::set<String> except_list;

        void setDatabase(const String & new_database);
    };

    using Elements = std::vector<Element>;
    static void setDatabase(Elements & elements, const String & new_database);
    void setDatabase(const String & new_database) { setDatabase(elements, new_database); }

    Elements elements;

    ASTPtr backup_name;

    ASTPtr settings;

    /// Base backup. Only differences made after the base backup will be included in a newly created backup,
    /// so this setting allows to make an incremental backup.
    ASTPtr base_backup_name;

    /// List of cluster's hosts' IDs if this is a BACKUP/RESTORE ON CLUSTER command.
    ASTPtr cluster_host_ids;

    String getID(char) const override;
    ASTPtr clone() const override;
    void formatImpl(const FormatSettings & format, FormatState &, FormatStateStacked) const override;
    ASTPtr getRewrittenASTWithoutOnCluster(const WithoutOnClusterASTRewriteParams &) const override;
};
}
