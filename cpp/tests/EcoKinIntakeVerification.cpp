#include "EchoheartsEcoKinIntake.hpp"

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Echohearts::EcoKinIntake;

namespace {

void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

const std::string Header =
    "Name,DexID,Status,Vibrance,Density,Harmony,Purity,SourceRow,ReviewFlags,"
    "SuggestedMergeTarget,FormBaseCandidate,CanonicalDisposition,IDStatus,"
    "ProposedVibrance,ProposedDensity,ProposedHarmony,ProposedPurity,ProposalDelta,ProposalGate\n";

std::string Row(const std::string& name, const std::string& id, const std::string& status,
                int sourceRow, const std::string& idStatus, const std::string& flag = {},
                const std::string& mergeTarget = {}, const std::string& formBase = {},
                const std::array<int, 4>& original = {50, 50, 50, 50},
                const std::array<int, 4>& proposed = {50, 50, 50, 50},
                int delta = 0, const std::string& gate = "NO NUMERIC CHANGE PROPOSED") {
    std::ostringstream row;
    row << name << ',' << id << ',' << status << ','
        << original[0] << ',' << original[1] << ',' << original[2] << ',' << original[3] << ','
        << sourceRow << ',' << flag << ',' << mergeTarget << ',' << formBase
        << ",INTAKE; CANON PROMOTION REQUIRED," << idStatus << ','
        << proposed[0] << ',' << proposed[1] << ',' << proposed[2] << ',' << proposed[3] << ','
        << delta << ',' << gate << '\n';
    return row.str();
}

std::vector<Record> ParseText(const std::string& csv) {
    std::istringstream input(csv);
    return Parse(input);
}

std::string SmallIntake() {
    return Header +
        Row("Nature", "DEX-002", "LOCKED CANON", 12,
            "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED") +
        Row("Lanternwolf", "", "PENDING REVIEW", 43,
            "NO PRODUCTION ID — DO NOT ASSIGN");
}

} // namespace

int main() {
    try {
        {
            auto records = ParseText(SmallIntake());
            const auto report = Validate(records);
            Require(report.Ok(), "valid small intake fixture rejected");
            Require(report.protectedIdCount == 1 && report.unnumberedCount == 1,
                    "protected and unnumbered records were not distinguished");
            Require(report.proposedChangeCount == 0, "unexpected proposal count");
        }

        {
            auto records = ParseText(
                Header +
                Row("Nature", "DEX-002", "LOCKED CANON", 12,
                    "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED") +
                Row("Duplicate ID", "DEX-002", "PENDING REVIEW", 13,
                    "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED"));
            const auto report = Validate(records);
            Require(!report.Ok(), "duplicate protected ID was accepted");
        }

        {
            auto records = ParseText(SmallIntake());
            const auto report = Validate(records);
            Require(report.Ok(), "pending candidate fixture rejected");
            Require(records[1].dexId.empty() && records[1].status == "PENDING REVIEW",
                    "missing canonical approval promoted an intake candidate");
        }

        {
            auto records = ParseText(SmallIntake());
            records[1].canonicalDisposition = "CANON APPROVED";
            Require(!Validate(records).Ok(), "an unsupported canonical disposition was accepted");
        }

        {
            auto records = ParseText(
                Header + Row("Candidate", "", "PENDING REVIEW", 54,
                             "NO PRODUCTION ID — DO NOT ASSIGN", {}, {}, {},
                             {71, 83, 92, 64}, {65, 83, 92, 64}, -6,
                             "DRAFT CAP SUGGESTION; NOT APPROVED"));
            const auto report = Validate(records);
            Require(report.Ok(), "valid draft proposal fixture rejected");
            Require(records[0].originalStats == std::array<int, 4>{71, 83, 92, 64},
                    "imported attributes were not preserved");
            Require(records[0].proposedStats == std::array<int, 4>{65, 83, 92, 64},
                    "draft attributes were not retained separately");
        }

        {
            std::ostringstream csv;
            csv << Header;
            for (int id = 1; id <= 125; ++id) {
                const auto label = std::string("DEX-") + (id < 10 ? "00" : id < 100 ? "0" : "") + std::to_string(id);
                const std::string status = id == 2 ? "LOCKED CANON" : "PERMANENT DEX";
                const std::string name = id == 2 ? "Nature" : "Protected " + std::to_string(id);
                csv << Row(name, label, status, id + 1,
                           "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED");
            }
            for (int candidate = 1; candidate <= 429; ++candidate) {
                const auto original = std::array<int, 4>{50, 60, 70, 80};
                const auto proposed = candidate <= 23
                    ? std::array<int, 4>{45, 60, 70, 80}
                    : original;
                const auto gate = candidate <= 23
                    ? "DRAFT CAP SUGGESTION; NOT APPROVED"
                    : "NO NUMERIC CHANGE PROPOSED";
                csv << Row("Candidate " + std::to_string(candidate), "", "PENDING REVIEW",
                           126 + candidate, "NO PRODUCTION ID — DO NOT ASSIGN", {},
                           {}, {}, original, proposed, candidate <= 23 ? -5 : 0, gate);
            }
            const auto records = ParseText(csv.str());
            const auto report = Validate(records, {true});
            Require(report.Ok(), "554-record intake contract fixture rejected");
            Require(report.recordCount == 554 && report.protectedIdCount == 125 &&
                    report.unnumberedCount == 429, "canonical record totals mismatch");
            Require(report.proposedChangeCount == 23, "draft proposal count mismatch");
            for (std::size_t i = 125; i < records.size(); ++i) {
                Require(records[i].originalStats == std::array<int, 4>{50, 60, 70, 80},
                        "a draft stat proposal overwrote imported attributes");
            }
        }

        {
            auto records = ParseText(
                Header +
                Row("Lanternwolf", "EK-065", "PENDING REVIEW", 91,
                    "PROTECTED SLOT PRESENT; IDENTITY APPROVAL NOT INFERRED"));
            Require(!Validate(records).Ok(), "visual-wave label was accepted as a permanent Dex ID");
        }

        {
            auto records = ParseText(
                Header +
                Row("Same Name", "", "PENDING REVIEW", 96, "NO PRODUCTION ID — DO NOT ASSIGN") +
                Row("same name", "", "PENDING REVIEW", 97, "NO PRODUCTION ID — DO NOT ASSIGN"));
            Require(!Validate(records).Ok(), "duplicate canonical names were accepted");
        }

        {
            auto records = ParseText(
                Header +
                Row("Base Form", "", "PENDING REVIEW", 98, "NO PRODUCTION ID — DO NOT ASSIGN") +
                Row("Candidate Form", "", "PENDING REVIEW", 99, "NO PRODUCTION ID — DO NOT ASSIGN",
                    "POSSIBLE FORM / EVOLUTION RELATION", {}, "Base Form") +
                Row("Historical Alias", "", "MERGE-DUPLICATE", 100,
                    "NO PRODUCTION ID — DO NOT ASSIGN", "ARCHIVAL MERGE TARGET REVIEW",
                    "Base Form") +
                Row("Rename Candidate", "", "RENAME REQUIRED", 101,
                    "NO PRODUCTION ID — DO NOT ASSIGN", "RENAME/ORIGINALITY REVIEW"));
            const auto report = Validate(records);
            Require(report.Ok(), "form, duplicate, or rename review fixture rejected");
            Require(report.archiveOnlyCount == 2, "duplicate and rename candidates were not held out of active intake");
        }

        {
            auto records = ParseText(
                Header +
                Row("Candidate", "", "PENDING REVIEW", 102, "NO PRODUCTION ID — DO NOT ASSIGN",
                    {}, "Missing Canon Name"));
            Require(!Validate(records).Ok(), "unresolved canonical reference was accepted");
        }

        {
            auto records = ParseText(
                Header +
                Row("Out of Bounds", "", "PENDING REVIEW", 103, "NO PRODUCTION ID — DO NOT ASSIGN",
                    {}, {}, {}, {101, 40, 40, 40}, {101, 40, 40, 40}));
            Require(!Validate(records).Ok(), "out-of-range attributes were accepted");
        }

        {
            auto records = ParseText(
                Header +
                Row("Old Entry", "", "RETIRED", 94, "NO PRODUCTION ID — DO NOT ASSIGN",
                    "RETIRED: EXCLUDE FROM ACTIVE ROSTER") +
                Row("Unknown Entry", "", "UNREVIEWED", 95, "NO PRODUCTION ID — DO NOT ASSIGN"));
            const auto report = Validate(records);
            Require(!report.Ok(), "unknown status was accepted");
            Require(report.archiveOnlyCount == 1, "retired candidate was not classified as archive-only");
        }

        std::cout << "Eco-Kin intake verification passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Eco-Kin intake verification FAILED: " << ex.what() << "\n";
        return 1;
    }
}
