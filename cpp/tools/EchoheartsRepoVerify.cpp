#include <sqlite3.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct SqliteCloser {
    void operator()(sqlite3* db) const noexcept {
        if (db != nullptr) sqlite3_close(db);
    }
};
using DbPtr = std::unique_ptr<sqlite3, SqliteCloser>;

struct CheckResult {
    bool ok{true};
    std::string message;
};

std::string QuoteIdentifier(const std::string& value) {
    std::string out = "\"";
    for (char ch : value) {
        if (ch == '"') out += '"';
        out += ch;
    }
    out += '"';
    return out;
}

std::vector<std::vector<std::string>> Query(sqlite3* db, const std::string& sql) {
    sqlite3_stmt* rawStmt = nullptr;
    const int prepareRc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
    if (prepareRc != SQLITE_OK) {
        throw std::runtime_error("prepare failed: " + std::string(sqlite3_errmsg(db)) + " | SQL: " + sql);
    }
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt(rawStmt, sqlite3_finalize);

    std::vector<std::vector<std::string>> rows;
    for (;;) {
        const int rc = sqlite3_step(stmt.get());
        if (rc == SQLITE_DONE) break;
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("step failed: " + std::string(sqlite3_errmsg(db)) + " | SQL: " + sql);
        }
        std::vector<std::string> row;
        const int columns = sqlite3_column_count(stmt.get());
        for (int i = 0; i < columns; ++i) {
            const unsigned char* text = sqlite3_column_text(stmt.get(), i);
            row.emplace_back(text == nullptr ? "" : reinterpret_cast<const char*>(text));
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

void Execute(sqlite3* db, const std::string& sql) {
    char* error = nullptr;
    const int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error);
    if (rc != SQLITE_OK) {
        std::string message = error == nullptr ? sqlite3_errmsg(db) : error;
        sqlite3_free(error);
        throw std::runtime_error("exec failed: " + message + " | SQL: " + sql);
    }
}

CheckResult CheckSingleOk(sqlite3* db, const std::string& sql, const std::string& label) {
    const auto rows = Query(db, sql);
    if (rows.size() == 1 && rows.front().size() == 1 && rows.front().front() == "ok") {
        return {true, label + ": ok"};
    }
    std::ostringstream oss;
    oss << label << ": FAILED";
    for (const auto& row : rows) {
        if (!row.empty()) oss << " | " << row.front();
    }
    return {false, oss.str()};
}

CheckResult CheckForeignKeys(sqlite3* db) {
    const auto rows = Query(db, "PRAGMA foreign_key_check;");
    if (rows.empty()) return {true, "foreign_key_check: ok"};
    std::ostringstream oss;
    oss << "foreign_key_check: FAILED with " << rows.size() << " violation(s)";
    for (const auto& row : rows) {
        if (row.size() >= 3) {
            oss << " | table=" << row[0] << " rowid=" << row[1] << " parent=" << row[2];
        }
    }
    return {false, oss.str()};
}

std::vector<std::string> UserTables(sqlite3* db) {
    const auto rows = Query(db,
        "SELECT name FROM sqlite_schema WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name;");
    std::vector<std::string> tables;
    for (const auto& row : rows) {
        if (!row.empty()) tables.push_back(row.front());
    }
    return tables;
}

CheckResult CheckSchemaReadable(sqlite3* db) {
    const auto rows = Query(db,
        "SELECT type,name,tbl_name,COALESCE(sql,'') FROM sqlite_schema ORDER BY type,name;");
    if (rows.empty()) return {false, "schema scan: FAILED, sqlite_schema is empty"};
    return {true, "schema scan: ok (" + std::to_string(rows.size()) + " objects)"};
}

CheckResult CheckEveryTableReadable(sqlite3* db) {
    const auto tables = UserTables(db);
    for (const auto& table : tables) {
        Query(db, "SELECT COUNT(*) FROM " + QuoteIdentifier(table) + ";");
    }
    return {true, "table read scan: ok (" + std::to_string(tables.size()) + " user tables)"};
}

CheckResult CheckMetadata(sqlite3* db) {
    const auto pageCount = Query(db, "PRAGMA page_count;");
    const auto freeList = Query(db, "PRAGMA freelist_count;");
    const auto userVersion = Query(db, "PRAGMA user_version;");
    const auto applicationId = Query(db, "PRAGMA application_id;");
    if (pageCount.empty() || freeList.empty() || userVersion.empty() || applicationId.empty()) {
        return {false, "database metadata: FAILED"};
    }
    return {true, "database metadata: ok"};
}

CheckResult CheckEveryTablePreparable(sqlite3* db) {
    const auto tables = UserTables(db);
    for (const auto& table : tables) {
        sqlite3_stmt* rawStmt = nullptr;
        const std::string sql = "SELECT * FROM " + QuoteIdentifier(table) + " LIMIT 0;";
        const int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
        if (rawStmt != nullptr) sqlite3_finalize(rawStmt);
        if (rc != SQLITE_OK) {
            return {false, "prepare scan: FAILED for table " + table + ": " + sqlite3_errmsg(db)};
        }
    }
    return {true, "prepare scan: ok (" + std::to_string(tables.size()) + " user tables)"};
}

void PrintUsage() {
    std::cout
        << "Echohearts repository/database verifier\n\n"
        << "Usage:\n"
        << "  echo_verify --database <path.db> [--level 1..10]\n\n"
        << "Default level: 5\n"
        << "Level 1  Open database read-only and fetch SQLite version\n"
        << "Level 2  Scan sqlite_schema\n"
        << "Level 3  PRAGMA quick_check\n"
        << "Level 4  PRAGMA integrity_check\n"
        << "Level 5  PRAGMA foreign_key_check\n"
        << "Level 6  Read every user table\n"
        << "Level 7  Validate database metadata pragmas\n"
        << "Level 8  Enable cell_size_check and rerun integrity_check\n"
        << "Level 9  Prepare a read query for every user table\n"
        << "Level 10 Repeat full integrity + FK + table-read verification\n";
}

} // namespace

int main(int argc, char** argv) {
    std::string databasePath;
    int level = 5;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return EXIT_SUCCESS;
        }
        if (arg == "--database" && i + 1 < argc) {
            databasePath = argv[++i];
            continue;
        }
        if (arg == "--level" && i + 1 < argc) {
            level = std::stoi(argv[++i]);
            continue;
        }
        std::cerr << "Unknown or incomplete argument: " << arg << "\n";
        PrintUsage();
        return EXIT_FAILURE;
    }

    if (databasePath.empty()) {
        std::cerr << "--database is required\n";
        PrintUsage();
        return EXIT_FAILURE;
    }
    if (level < 1 || level > 10) {
        std::cerr << "--level must be between 1 and 10\n";
        return EXIT_FAILURE;
    }

    sqlite3* rawDb = nullptr;
    const int openRc = sqlite3_open_v2(databasePath.c_str(), &rawDb, SQLITE_OPEN_READONLY, nullptr);
    DbPtr db(rawDb);
    if (openRc != SQLITE_OK || db == nullptr) {
        std::cerr << "Unable to open database read-only: "
                  << (rawDb == nullptr ? "unknown SQLite error" : sqlite3_errmsg(rawDb)) << "\n";
        return EXIT_FAILURE;
    }

    try {
        Execute(db.get(), "PRAGMA foreign_keys=ON;");
        std::vector<CheckResult> results;

        const auto version = Query(db.get(), "SELECT sqlite_version();");
        results.push_back({!version.empty(), version.empty() ? "SQLite fetch: FAILED" : "SQLite fetch: ok (" + version.front().front() + ")"});

        if (level >= 2) results.push_back(CheckSchemaReadable(db.get()));
        if (level >= 3) results.push_back(CheckSingleOk(db.get(), "PRAGMA quick_check;", "quick_check"));
        if (level >= 4) results.push_back(CheckSingleOk(db.get(), "PRAGMA integrity_check;", "integrity_check"));
        if (level >= 5) results.push_back(CheckForeignKeys(db.get()));
        if (level >= 6) results.push_back(CheckEveryTableReadable(db.get()));
        if (level >= 7) results.push_back(CheckMetadata(db.get()));
        if (level >= 8) {
            Execute(db.get(), "PRAGMA cell_size_check=ON;");
            results.push_back(CheckSingleOk(db.get(), "PRAGMA integrity_check;", "cell_size integrity_check"));
        }
        if (level >= 9) results.push_back(CheckEveryTablePreparable(db.get()));
        if (level >= 10) {
            results.push_back(CheckSingleOk(db.get(), "PRAGMA integrity_check;", "final integrity_check"));
            results.push_back(CheckForeignKeys(db.get()));
            results.push_back(CheckEveryTableReadable(db.get()));
        }

        bool allOk = true;
        std::cout << "Echohearts verify level " << level << "\n";
        for (const auto& result : results) {
            std::cout << (result.ok ? "[PASS] " : "[FAIL] ") << result.message << "\n";
            allOk = allOk && result.ok;
        }
        std::cout << (allOk ? "VERIFICATION PASSED\n" : "VERIFICATION FAILED\n");
        return allOk ? EXIT_SUCCESS : EXIT_FAILURE;
    } catch (const std::exception& ex) {
        std::cerr << "Verifier exception: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
