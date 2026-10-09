#pragma once

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cstddef>
#include <istream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Echohearts::EcoKinIntake {

struct Record {
    std::size_t csvLine{0};
    std::string name;
    std::string dexId;
    std::string status;
    std::string reviewFlags;
    std::string suggestedMergeTarget;
    std::string formBaseCandidate;
    std::string proposalGate;
    std::string canonicalDisposition;
    std::string idStatus;
    int sourceRow{0};
    std::array<int, 4> originalStats{};
    std::array<int, 4> proposedStats{};
    int proposalDelta{0};
};

struct ValidationIssue {
    std::size_t csvLine{0};
    std::string message;
};

struct ValidationOptions {
    bool requireCanonicalCounts{false};
};

struct ValidationReport {
    std::size_t recordCount{0};
    std::size_t protectedIdCount{0};
    std::size_t unnumberedCount{0};
    std::size_t proposedChangeCount{0};
    std::size_t archiveOnlyCount{0};
    std::map<std::string, std::size_t> statusCounts;
    std::vector<ValidationIssue> issues;

    bool Ok() const noexcept { return issues.empty(); }
};

namespace Detail {

struct CsvRow {
    std::size_t line{1};
    std::vector<std::string> fields;
};

inline std::vector<CsvRow> ReadCsv(std::istream& input) {
    std::vector<CsvRow> rows;
    CsvRow row;
    std::string field;
    bool quoted = false;
    bool closedQuote = false;
    bool rowStarted = false;
    std::size_t line = 1;
    row.line = line;

    const auto finishField = [&]() {
        row.fields.push_back(field);
        field.clear();
        closedQuote = false;
    };
    const auto finishRow = [&]() {
        finishField();
        rows.push_back(std::move(row));
        row = CsvRow{};
        row.line = line + 1;
        rowStarted = false;
    };

    char ch = '\0';
    while (input.get(ch)) {
        if (quoted) {
            if (ch == '"') {
                if (input.peek() == '"') {
                    input.get(ch);
                    field += '"';
                } else {
                    quoted = false;
                    closedQuote = true;
                }
            } else {
                if (ch == '\n') ++line;
                field += ch;
            }
            continue;
        }

        if (closedQuote && ch != ',' && ch != '\r' && ch != '\n') {
            throw std::runtime_error("unexpected character after quoted CSV field at line " + std::to_string(line));
        }
        if (ch == '"') {
            if (!field.empty() || closedQuote) {
                throw std::runtime_error("unexpected quote in CSV field at line " + std::to_string(line));
            }
            quoted = true;
            rowStarted = true;
        } else if (ch == ',') {
            finishField();
            rowStarted = true;
        } else if (ch == '\n' || ch == '\r') {
            if (ch == '\r' && input.peek() == '\n') input.get(ch);
            if (rowStarted || !field.empty() || !row.fields.empty()) finishRow();
            ++line;
            row.line = line;
        } else {
            field += ch;
            rowStarted = true;
        }
    }

    if (quoted) {
        throw std::runtime_error("unterminated quoted CSV field at line " + std::to_string(row.line));
    }
    if (rowStarted || !field.empty() || !row.fields.empty()) finishRow();
    if (input.bad()) throw std::runtime_error("failed while reading CSV input");
    return rows;
}

inline std::string Trim(std::string value) {
    const auto notSpace = [](unsigned char ch) { return std::isspace(ch) == 0; };
    const auto first = std::find_if(value.begin(), value.end(), notSpace);
    const auto last = std::find_if(value.rbegin(), value.rend(), notSpace).base();
    if (first >= last) return {};
    return std::string(first, last);
}

inline std::string FoldAscii(std::string value) {
    for (char& ch : value) {
        const auto byte = static_cast<unsigned char>(ch);
        if (byte < 0x80) ch = static_cast<char>(std::tolower(byte));
    }
    return value;
}

inline int Integer(const std::string& value, const std::string& column, std::size_t line) {
    int result = 0;
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto parsed = std::from_chars(begin, end, result);
    if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != end) {
        throw std::runtime_error("invalid integer in " + column + " at line " + std::to_string(line));
    }
    return result;
}

inline std::string ReferenceName(std::string value) {
    const auto annotation = value.find(" (");
    if (annotation != std::string::npos) value.resize(annotation);
    return FoldAscii(Trim(std::move(value)));
}

} // namespace Detail

inline std::vector<Record> Parse(std::istream& input) {
    const auto rows = Detail::ReadCsv(input);
    if (rows.empty()) throw std::runtime_error("CSV is empty");

    auto headers = rows.front().fields;
    if (!headers.empty() && headers.front().size() >= 3 &&
        static_cast<unsigned char>(headers.front()[0]) == 0xEF &&
        static_cast<unsigned char>(headers.front()[1]) == 0xBB &&
        static_cast<unsigned char>(headers.front()[2]) == 0xBF) {
        headers.front().erase(0, 3);
    }

    std::unordered_map<std::string, std::size_t> columns;
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (!columns.emplace(headers[i], i).second) {
            throw std::runtime_error("duplicate CSV column: " + headers[i]);
        }
    }

    const std::vector<std::string> requiredColumns = {
        "Name", "DexID", "Status", "Vibrance", "Density", "Harmony", "Purity",
        "SourceRow", "ReviewFlags", "SuggestedMergeTarget", "FormBaseCandidate",
        "CanonicalDisposition", "IDStatus", "ProposedVibrance", "ProposedDensity",
        "ProposedHarmony", "ProposedPurity", "ProposalDelta", "ProposalGate"
    };
    for (const auto& name : requiredColumns) {
        if (columns.find(name) == columns.end()) {
            throw std::runtime_error("missing required CSV column: " + name);
        }
    }

    const auto cell = [&](const Detail::CsvRow& row, const std::string& name) -> const std::string& {
        const auto index = columns.at(name);
        if (row.fields.size() != headers.size()) {
            throw std::runtime_error("CSV column count mismatch at line " + std::to_string(row.line));
        }
        return row.fields[index];
    };

    std::vector<Record> records;
    records.reserve(rows.size() - 1);
    for (std::size_t i = 1; i < rows.size(); ++i) {
        const auto& row = rows[i];
        Record record;
        record.csvLine = row.line;
        record.name = Detail::Trim(cell(row, "Name"));
        record.dexId = Detail::Trim(cell(row, "DexID"));
        record.status = Detail::Trim(cell(row, "Status"));
        record.reviewFlags = Detail::Trim(cell(row, "ReviewFlags"));
        record.suggestedMergeTarget = Detail::Trim(cell(row, "SuggestedMergeTarget"));
        record.formBaseCandidate = Detail::Trim(cell(row, "FormBaseCandidate"));
        record.proposalGate = Detail::Trim(cell(row, "ProposalGate"));
        record.canonicalDisposition = Detail::Trim(cell(row, "CanonicalDisposition"));
        record.idStatus = Detail::Trim(cell(row, "IDStatus"));
        record.sourceRow = Detail::Integer(cell(row, "SourceRow"), "SourceRow", row.line);
        const std::array<std::string, 4> statNames{"Vibrance", "Density", "Harmony", "Purity"};
        const std::array<std::string, 4> proposedNames{
            "ProposedVibrance", "ProposedDensity", "ProposedHarmony", "ProposedPurity"
        };
        for (std::size_t stat = 0; stat < record.originalStats.size(); ++stat) {
            record.originalStats[stat] = Detail::Integer(cell(row, statNames[stat]), statNames[stat], row.line);
            record.proposedStats[stat] = Detail::Integer(cell(row, proposedNames[stat]), proposedNames[stat], row.line);
        }
        record.proposalDelta = Detail::Integer(cell(row, "ProposalDelta"), "ProposalDelta", row.line);
        records.push_back(std::move(record));
    }
    return records;
}

inline ValidationReport Validate(
    const std::vector<Record>& records,
    const ValidationOptions& options = {})
{
    constexpr std::size_t ExpectedRecords = 554;
    constexpr std::size_t ExpectedProtectedIds = 125;
    constexpr std::size_t ExpectedUnnumbered = 429;
    const std::unordered_set<std::string> knownStatuses = {
        "PENDING REVIEW", "PERMANENT DEX", "MERGE-DUPLICATE", "LEGACY-HISTORICAL",
        "RETIRED", "RENAME REQUIRED", "LOCKED CANON"
    };

    ValidationReport report;
    report.recordCount = records.size();
    std::unordered_set<std::string> names;
    std::unordered_set<std::string> ids;
    std::unordered_set<int> sourceRows;
    std::unordered_set<std::string> nameLookup;
    for (const auto& record : records) {
        const auto normalizedName = Detail::FoldAscii(Detail::Trim(record.name));
        if (!normalizedName.empty()) nameLookup.insert(normalizedName);
    }

    const auto issue = [&](const Record& record, std::string message) {
        report.issues.push_back({record.csvLine, std::move(message)});
    };

    for (const auto& record : records) {
        ++report.statusCounts[record.status];
        const auto name = Detail::FoldAscii(Detail::Trim(record.name));
        if (name.empty()) {
            issue(record, "name is required");
        } else if (!names.insert(name).second) {
            issue(record, "duplicate name: " + record.name);
        }
        if (knownStatuses.find(record.status) == knownStatuses.end()) {
            issue(record, "unknown status: " + record.status);
        }
        if (!sourceRows.insert(record.sourceRow).second || record.sourceRow <= 0) {
            issue(record, "SourceRow must be a unique positive provenance reference");
        }
        if (record.canonicalDisposition.empty()) {
            issue(record, "CanonicalDisposition is required");
        }
        if (record.reviewFlags.empty() &&
            (record.status == "MERGE-DUPLICATE" || record.status == "RENAME REQUIRED")) {
            issue(record, "duplicate or rename status requires a review flag");
        }

        const bool numbered = !record.dexId.empty();
        if (numbered) {
            ++report.protectedIdCount;
            if (record.dexId.size() != 7 || record.dexId.compare(0, 4, "DEX-") != 0 ||
                !std::all_of(record.dexId.begin() + 4, record.dexId.end(),
                             [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
                issue(record, "DexID must be a protected DEX-001..125 ID; visual-wave labels are not permanent IDs");
            } else {
                const int dexNumber = Detail::Integer(record.dexId.substr(4), "DexID", record.csvLine);
                if (dexNumber < 1 || dexNumber > 125) {
                    issue(record, "DexID is outside the protected DEX-001..125 range");
                }
            }
            if (!ids.insert(record.dexId).second) issue(record, "duplicate DexID: " + record.dexId);
            if (record.idStatus != "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED") {
                issue(record, "numbered entry has inconsistent IDStatus");
            }
        } else {
            ++report.unnumberedCount;
            if (record.idStatus != "NO PRODUCTION ID — DO NOT ASSIGN") {
                issue(record, "unnumbered candidate must remain without a production ID");
            }
        }

        int originalTotal = 0;
        int proposedTotal = 0;
        bool validStats = true;
        for (std::size_t stat = 0; stat < record.originalStats.size(); ++stat) {
            const int original = record.originalStats[stat];
            const int proposed = record.proposedStats[stat];
            if (original < 0 || original > 100 || proposed < 0 || proposed > 100) {
                validStats = false;
            }
            originalTotal += original;
            proposedTotal += proposed;
        }
        if (!validStats) issue(record, "original and proposed attributes must each be within 0..100");

        if (record.proposalGate == "NO NUMERIC CHANGE PROPOSED") {
            if (record.proposalDelta != 0 || record.proposedStats != record.originalStats) {
                issue(record, "no-change proposal gate conflicts with the imported attributes");
            }
        } else if (record.proposalGate == "DRAFT CAP SUGGESTION; NOT APPROVED") {
            ++report.proposedChangeCount;
            if (proposedTotal - originalTotal != record.proposalDelta || proposedTotal == originalTotal) {
                issue(record, "draft proposal delta does not match the separately retained attributes");
            }
        } else {
            issue(record, "unknown or missing ProposalGate: " + record.proposalGate);
        }

        if (record.status == "RETIRED" || record.status == "LEGACY-HISTORICAL" ||
            record.status == "MERGE-DUPLICATE" || record.status == "RENAME REQUIRED") {
            ++report.archiveOnlyCount;
        }
    }

    for (const auto& record : records) {
        for (const auto& reference : {record.suggestedMergeTarget, record.formBaseCandidate}) {
            if (reference.empty()) continue;
            const auto target = Detail::ReferenceName(reference);
            if (!target.empty() && nameLookup.find(target) == nameLookup.end()) {
                issue(record, "unresolved canonical name reference: " + reference);
            }
        }
    }

    if (options.requireCanonicalCounts) {
        if (records.size() != ExpectedRecords) {
            report.issues.push_back({0, "expected 554 intake records, found " + std::to_string(records.size())});
        }
        if (report.protectedIdCount != ExpectedProtectedIds) {
            report.issues.push_back({0, "expected 125 protected IDs, found " + std::to_string(report.protectedIdCount)});
        }
        if (report.unnumberedCount != ExpectedUnnumbered) {
            report.issues.push_back({0, "expected 429 unnumbered archive candidates, found " + std::to_string(report.unnumberedCount)});
        }
        for (int i = 1; i <= 125; ++i) {
            const auto expectedId = std::string("DEX-") + (i < 10 ? "00" : i < 100 ? "0" : "") + std::to_string(i);
            if (ids.find(expectedId) == ids.end()) {
                report.issues.push_back({0, "missing protected ID: " + expectedId});
            }
        }
        if (report.proposedChangeCount != 23) {
            report.issues.push_back({0, "expected 23 unapproved draft stat proposals, found " +
                                         std::to_string(report.proposedChangeCount)});
        }
    }
    return report;
}

} // namespace Echohearts::EcoKinIntake
