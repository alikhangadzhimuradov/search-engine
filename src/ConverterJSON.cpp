#include "ConverterJSON.h"
#include "SearchServer.h"
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

ConverterJSON::ConverterJSON() = default;

std::vector<std::string> ConverterJSON::GetTextDocuments() {
    std::ifstream file(configFile);
    if (!file.is_open()) throw std::runtime_error("config file is missing");

    json config;
    file >> config;
    return config["config"].value("files", std::vector<std::string>{});
}

int ConverterJSON::GetResponsesLimit() {
    std::ifstream file(configFile);
    json config;
    file >> config;
    return config["config"].value("max_responses", 5);
}

std::vector<std::string> ConverterJSON::GetRequests() {
    std::ifstream file(requestsFile);
    if (!file.is_open()) return {};

    json requests;
    file >> requests;
    return requests.value("requests", std::vector<std::string>{});
}

void ConverterJSON::putAnswers(const std::vector<std::vector<RelativeIndex>>& answers) {
    json output;
    for (size_t i = 0; i < answers.size(); ++i) {
        json result;
        if (answers[i].empty()) {
            result["result"] = false;
        } else {
            result["result"] = true;
            for (const auto& relIndex : answers[i]) {
                result["relevance"].push_back({
                                                      {"docid", relIndex.doc_id},
                                                      {"rank", relIndex.rank}
                                              });
            }
        }
        output["answers"]["request" + std::to_string(i + 1)] = result;
    }
    std::ofstream(answersFile) << output.dump(4);
}