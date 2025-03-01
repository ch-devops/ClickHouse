#pragma once

#include <Storages/StorageURL.h>
#include <BridgeHelper/XDBCBridgeHelper.h>

namespace Poco
{
class Logger;
}

namespace DB
{

/** Implements storage in the XDBC database.
  * Use ENGINE = xdbc(connection_string, table_name)
  * Example ENGINE = odbc('dsn=test', table)
  * Read only.
  */
class StorageXDBC : public IStorageURLBase
{
public:
    Pipe read(
        const Names & column_names,
        const StorageSnapshotPtr & storage_snapshot,
        SelectQueryInfo & query_info,
        ContextPtr context,
        QueryProcessingStage::Enum processed_stage,
        size_t max_block_size,
        unsigned num_streams) override;

    StorageXDBC(
        const StorageID & table_id_,
        const std::string & remote_database_name,
        const std::string & remote_table_name,
        const ColumnsDescription & columns_,
        const String & comment,
        ContextPtr context_,
        BridgeHelperPtr bridge_helper_);

    SinkToStoragePtr write(const ASTPtr & query, const StorageMetadataPtr & /*metadata_snapshot*/, ContextPtr context) override;

    std::string getName() const override;
private:
    BridgeHelperPtr bridge_helper;
    std::string remote_database_name;
    std::string remote_table_name;

    Poco::Logger * log;

    std::string getReadMethod() const override;

    std::vector<std::pair<std::string, std::string>> getReadURIParams(
        const Names & column_names,
        const StorageSnapshotPtr & storage_snapshot,
        const SelectQueryInfo & query_info,
        ContextPtr context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size) const override;

    std::function<void(std::ostream &)> getReadPOSTDataCallback(
        const Names & column_names,
        const ColumnsDescription & columns_description,
        const SelectQueryInfo & query_info,
        ContextPtr context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size) const override;

    Block getHeaderBlock(const Names & column_names, const StorageSnapshotPtr & storage_snapshot) const override;

    bool isColumnOriented() const override;
};

}
