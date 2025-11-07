#include "InvertedIndex.h"
#include <sstream>
#include <algorithm>

bool Entry::operator==(const Entry& other) const {
    return doc_id == other.doc_id && count == other.count;
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