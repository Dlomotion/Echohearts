#include "EchoheartsEcoKinIntake.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        std::cout << "Usage: echo_ecokin_intake_verify <corrected-registry.csv>\n"
                  << "Validates the 554-row intake without applying proposals or promoting records.\n";
        return argc == 2 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    try {
        std::ifstream input(argv[1], std::ios::binary);
        if (!input) throw std::runtime_error("unable to open CSV input");
        const auto records = Echohearts::EcoKinIntake::Parse(input);
        const auto report = Echohearts::EcoKinIntake::Validate(records, {true});

        std::cout << "Eco-Kin intake rows: " << report.recordCount << "\n"
                  << "Protected permanent IDs: " << report.protectedIdCount << "\n"
                  << "Unnumbered archive candidates: " << report.unnumberedCount << "\n"
                  << "Unapproved stat proposals retained: " << report.proposedChangeCount << "\n"
                  << "Archive-only entries: " << report.archiveOnlyCount << "\n"
                  << "Canonical promotion: not performed (intake has no approval authority)\n";
        for (const auto& issue : report.issues) {
            std::cerr << "[FAIL]";
            if (issue.csvLine != 0) std::cerr << " line " << issue.csvLine;
            std::cerr << " " << issue.message << "\n";
        }
        if (!report.Ok()) return EXIT_FAILURE;
        std::cout << "INTAKE VALIDATION PASSED\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Intake validation error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
