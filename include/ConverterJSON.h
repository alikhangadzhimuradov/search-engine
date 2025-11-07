#pragma once
#include <vector>
#include <string>
#include "InvertedIndex.h"
#include "SearchServer.h"

class ConverterJSON {
public:
    ConverterJSON();
    std::vector<std::string> GetTextDocuments();
    int GetResponsesLimit();
    std::vector<std::string> GetRequests();
    void putAnswers(const std::vector<std::vector<RelativeIndex>>& answers);

private:
    const std::string configFile = "config.json";
    const std::string requestsFile = "requests.json";
    const std::string answersFile = "answers.json";
};