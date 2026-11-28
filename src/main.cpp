#include "Network.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

void printBanner() {
    std::cout << "====================================================================\n";
    std::cout << "  CXOR - Wireless Network Coding Simulator (C++20)\n";
    std::cout << "  Topology:  [Node A] <---> [Relay R (XOR)] <---> [Node B]\n";
    std::cout << "====================================================================\n\n";
}

void printReport(const cxor::ComparisonReport& report) {
    const auto& n = report.normal;
    const auto& c = report.coded;

    std::cout << "Scenario: " << n.scenarioName << '\n';
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(10) << "Mode"
              << std::right << std::setw(8) << "Sent"
              << std::setw(11) << "Delivered"
              << std::setw(10) << "Uplink"
              << std::setw(11) << "Relay TX"
              << std::setw(10) << "Coded"
              << std::setw(10) << "Total TX" << '\n';

    auto printRow = [](const char* label, const cxor::SimulationMetrics& m) {
        std::cout << std::left << std::setw(10) << label
                  << std::right << std::setw(8) << m.packetsSent
                  << std::setw(11) << m.packetsDelivered
                  << std::setw(10) << m.uplinkTransmissions
                  << std::setw(11) << m.relayTransmissions()
                  << std::setw(10) << m.relayCodedTransmissions
                  << std::setw(10) << m.totalTransmissions() << '\n';
    };

    printRow("Normal", n);
    printRow("Coded", c);

    std::cout << "Savings : " << report.transmissionsSaved() << " transmissions saved ("
              << std::fixed << std::setprecision(1) << report.savingsPercent() << "%)\n\n";
}

bool writeCsv(const std::filesystem::path& path,
              const std::vector<cxor::ComparisonReport>& reports) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    out << "scenario,mode,packets_sent,packets_delivered,uplink_tx,"
           "relay_uncoded_tx,relay_coded_tx,relay_total_tx,total_tx,savings_percent\n";

    auto writeRow = [&out](const cxor::SimulationMetrics& m, double savingsPct) {
        out << m.scenarioName << ','
            << (m.codingEnabled ? "coded" : "normal") << ','
            << m.packetsSent << ','
            << m.packetsDelivered << ','
            << m.uplinkTransmissions << ','
            << m.relayUncodedTransmissions << ','
            << m.relayCodedTransmissions << ','
            << m.relayTransmissions() << ','
            << m.totalTransmissions() << ','
            << std::fixed << std::setprecision(2) << savingsPct << '\n';
    };

    for (const auto& report : reports) {
        writeRow(report.normal, 0.0);
        writeRow(report.coded, report.savingsPercent());
    }

    return true;
}

}  // namespace

int main() {
    printBanner();

    const std::vector<std::pair<std::string, std::vector<cxor::TrafficEvent>>> scenarios{
        {
            "Symmetric Bidirectional",
            {
                {"A", "B", "A1"}, {"B", "A", "B1"},
                {"A", "B", "A2"}, {"B", "A", "B2"},
                {"A", "B", "A3"}, {"B", "A", "B3"},
                {"A", "B", "A4"}, {"B", "A", "B4"},
                {"A", "B", "A5"}, {"B", "A", "B5"},
            },
        },
        {
            "Asymmetric Bidirectional",
            {
                {"A", "B", "A1"}, {"B", "A", "B1"},
                {"A", "B", "A2"}, {"B", "A", "B2"},
                {"A", "B", "A3"}, {"A", "B", "A4"},
                {"A", "B", "A5"}, {"A", "B", "A6"},
            },
        },
        {
            "Mixed Unicast and Bidirectional",
            {
                {"A", "B", "A1"}, {"B", "A", "B1"},
                {"A", "B", "A2"}, {"B", "A", "B2"},
                {"A", "C", "U1"}, {"A", "C", "U2"},
            },
        },
    };

    std::vector<cxor::ComparisonReport> reports;
    reports.reserve(scenarios.size());
    for (const auto& [name, traffic] : scenarios) {
        reports.push_back(cxor::Network::compareScenario(name, traffic));
        printReport(reports.back());
    }

    const std::filesystem::path csvPath = "results/metrics.csv";
    if (writeCsv(csvPath, reports)) {
        std::cout << "Exported CSV metrics to " << csvPath.string() << '\n';
    } else {
        std::cerr << "Warning: failed to write " << csvPath.string() << '\n';
    }

    return 0;
}
