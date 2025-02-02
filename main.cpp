#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <string>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include "gtest/gtest.h"

using json = nlohmann::json;
std::mutex index_mutex;

struct Entry {
    size_t doc_id;
    size_t count;
    bool operator==(const Entry& other) const {
        return doc_id == other.doc_id && count == other.count;
    }
};

struct RelativeIndex {
    size_t doc_id;
    float rank;
    bool operator==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id) && (std::abs(rank - other.rank) < 1e-6);
    }
};

class ConverterJSON {
public:
    ConverterJSON() = default;
    std::vector<std::string> GetTextDocuments();
    int GetResponsesLimit();
    std::vector<std::string> GetRequests();
    void putAnswers(const std::vector<std::vector<RelativeIndex>>& answers); // Теперь RelativeIndex виден
private:
    const std::string configFile = "config.json";
    const std::string requestsFile = "requests.json";
    const std::string answersFile = "answers.json";
};

std::vector<std::string> ConverterJSON::GetTextDocuments() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("config file is missing");
    }
    json config;
    file >> config;
    file.close();
    if (!config.contains("config")) {
        throw std::runtime_error("config file is empty");
    }
    return config.value("files", std::vector<std::string>{});
}

int ConverterJSON::GetResponsesLimit() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("config file is missing");
    }
    json config;
    file >> config;
    file.close();
    return config["config"].value("max_responses", 5);
}

std::vector<std::string> ConverterJSON::GetRequests() {
    std::ifstream file(requestsFile);
    if (!file.is_open()) {
        return {};
    }
    json requests;
    file >> requests;
    file.close();
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
                result["relevance"].push_back({{"docid", relIndex.doc_id}, {"rank", relIndex.rank}});
            }
        }
        output["answers"]["request" + std::to_string(i + 1)] = result;
    }
    std::ofstream outFile(answersFile);
    outFile << output.dump(4);
}

class InvertedIndex {
public:
    void UpdateDocumentBase(const std::vector<std::string>& input_docs);
    std::vector<Entry> GetWordCount(const std::string& word) const;
private:
    std::vector<std::string> docs;
    std::map<std::string, std::vector<Entry>> freq_dictionary;
};

void TestInvertedIndexFunctionality(
const std::vector<std::string>& docs,
const std::vector<std::string>& requests,
const std::vector<std::vector<Entry>>& expected
) {
    std::vector<std::vector<Entry>> result;
    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);

    for(auto& request : requests) {
        std::vector<Entry> word_count = idx.GetWordCount(request);
        result.push_back(word_count);
    }
    ASSERT_EQ(result, expected);
}

TEST(TestCaseInvertedIndex, TestBasic) {
    const std::vector<std::string> docs = {
        "london is the capital of great britain",
        "big ben is the nickname for the Great bell of the striking clock"
    };
    const std::vector<std::string> requests = {"london", "the"};
    const std::vector<std::vector<Entry>> expected = {
        {

        {0, 1}
        }, {
        {0, 1}, {1, 3}
        }
    };

    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseInvertedIndex, TestBasic2) {
    const std::vector<std::string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"
    };
    const std::vector<std::string> requests = {"milk", "water", "cappuchino"};
    const std::vector<std::vector<Entry>> expected = {
              {

                            {0, 4}, {1, 1}, {2, 5}
                 },{
                            {0, 2}, {1, 2}, {2, 5}
                 },{
                               {3, 1}
                 }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {
    const std::vector<std::string> docs = {
        "a b c d e f g h i j k l",
        "statement"
    };
    const std::vector<std::string> requests = {"m", "statement"};
    const std::vector<std::vector<Entry>> expected = {
        {

        },  {
                   {1, 1}
        }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

void InvertedIndex::UpdateDocumentBase(const std::vector<std::string>& input_docs) {
    docs = input_docs;
    freq_dictionary.clear();

    for (size_t doc_id = 0; doc_id < docs.size(); ++doc_id) {
        std::istringstream stream(docs[doc_id]);
        std::string word;
        std::map<std::string, size_t> word_count;

        while (stream >> word) {
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            word_count[word]++;
        }

        for (const auto& [word, count] : word_count) {
            freq_dictionary[word].push_back({doc_id, count});
        }
    }
}

std::vector<Entry> InvertedIndex::GetWordCount(const std::string& word) const {
    std::string lower_word = word;
    std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);

    if (auto it = freq_dictionary.find(lower_word); it != freq_dictionary.end()) {
        return it->second;
    }
    return {};
}

class SearchServer {
public:
    SearchServer(InvertedIndex& idx) : index(idx) {}
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input);

private:
    InvertedIndex& index;
};

TEST(TestCaseSearchServer, TestSimple) {
    const std::vector<std::string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"
    };

    const std::vector<std::string> request = {"milk water", "sugar"};
    const std::vector<std::vector<RelativeIndex>> expected = {
        {
            {2, 1},
            {0, 0.7},
            {1, 0.3}
        },
     {

        }
    };

    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);

    SearchServer srv(idx);

    std::vector<std::vector<RelativeIndex>> result = srv.search(request);

    ASSERT_EQ(result, expected);
}

TEST(TestCaseSearchServer, TestTop5) {
    const std::vector<std::string> docs = {
        "london is the capital of great britain",
        "paris is the capital of france",
        "berlin is the capital of germany",
        "rome is the capital of italy",
        "madrid is the capital of spain",
        "lisboa is the capital of portugal",
        "bern is the capital of switzerland",
        "moscow is the capital of russia",
        "kiev is the capital of ukraine",
        "minsk is the capital of belarus",
        "astana is the capital of kazakhstan",
        "beijing is the capital of china",
        "tokyo is the capital of japan",
        "bangkok is the capital of thailand",
        "welcome to moscow the capital of russia the third rome",
        "amsterdam is the capital of netherlands",
        "helsinki is the capital of finland",
        "oslo is the capital of norway",
        "stockholm is the capital of sweden",
        "riga is the capital of latvia",
        "tallinn is the capital of estonia",
        "warsaw is the capital of poland",
    };
    const std::vector<std::string> request = {"moscow is the capital of russia"};
    const std::vector<std::vector<RelativeIndex>> expected = {
        {
            {7, 1},
            {14, 1},
            {0, 0.666666687},
            {1, 0.666666687},
            {2, 0.666666687}
        }
    };
    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);

    SearchServer srv(idx);

    std::vector<std::vector<RelativeIndex>> result = srv.search(request);

    ASSERT_EQ(result, expected);
 }

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> results;
    for (const auto& query : queries_input) {
        std::istringstream iss(query);
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            words.push_back(word);
        }

        if (words.empty()) {
            results.emplace_back();
            continue;
        }

        std::vector<std::unordered_set<size_t>> doc_sets;
        for (const auto& w : words) {
            auto entries = index.GetWordCount(w);
            std::unordered_set<size_t> docs;
            for (const auto& e : entries) docs.insert(e.doc_id);
            doc_sets.push_back(docs);
        }

        std::unordered_set<size_t> common_docs = doc_sets[0];
        for (size_t i = 1; i < doc_sets.size(); ++i) {
            std::unordered_set<size_t> temp;
            for (auto doc : common_docs) {
                if (doc_sets[i].count(doc)) temp.insert(doc);
            }
            common_docs = temp;
            if (common_docs.empty()) break;
        }

        if (common_docs.empty()) {
            results.emplace_back();
            continue;
        }

        std::map<size_t, size_t> doc_to_relevance;
        for (const auto& w : words) {
            auto entries = index.GetWordCount(w);
            for (const auto& e : entries) {
                if (common_docs.count(e.doc_id)) {
                    doc_to_relevance[e.doc_id] += e.count;
                }
            }
        }

        size_t max_relevance = 0;
        for (const auto& [doc_id, rel] : doc_to_relevance) {
            if (rel > max_relevance) max_relevance = rel;
        }

        std::vector<RelativeIndex> rel_indices;
        for (const auto& [doc_id, rel] : doc_to_relevance) {
            rel_indices.push_back({doc_id, static_cast<float>(rel) / max_relevance});
        }

        std::sort(rel_indices.begin(), rel_indices.end(),
            [](const RelativeIndex& a, const RelativeIndex& b) { return a.rank > b.rank; });

        results.push_back(rel_indices);
    }
    return results;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}