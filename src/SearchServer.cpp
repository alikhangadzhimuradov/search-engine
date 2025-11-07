#include "SearchServer.h"
#include <sstream>
#include <algorithm>
#include <unordered_set>

bool RelativeIndex::operator==(const RelativeIndex& other) const {
    return (doc_id == other.doc_id) && (std::abs(rank - other.rank) < 1e-6);
}

SearchServer::SearchServer(InvertedIndex& idx) : index(idx) {}

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
                  [](const RelativeIndex& a, const RelativeIndex& b) {
                      return a.rank > b.rank;
                  });

        results.push_back(rel_indices);
    }

    return results;
}