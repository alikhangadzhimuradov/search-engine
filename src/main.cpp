#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"
#include <iostream>

int main() {
    try {
        ConverterJSON converter;
        InvertedIndex index;
        index.UpdateDocumentBase(converter.GetTextDocuments());

        SearchServer server(index);
        auto results = server.search(converter.GetRequests());

        converter.putAnswers(results);
        std::cout << "Search completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}