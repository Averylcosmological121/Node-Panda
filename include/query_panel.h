#pragma once

#include <string>
#include <vector>
#include <utility>

namespace nodepanda {

class App;

/**
 * Query Panel for executing and displaying Dataview-style queries.
 * Provides UI for entering queries, executing them, and selecting results.
 */
class QueryPanel {
public:
    QueryPanel() = default;
    ~QueryPanel() = default;

    /**
     * Render the query panel UI.
     * @param app Reference to the App instance
     */
    void render(App& app);

private:
    char m_queryBuffer[1024] = {};
    bool m_hasResults = false;
    std::vector<std::pair<std::string, std::string>> m_results;  // id, display line
    std::string m_errorMsg;
    int m_resultCount = 0;
    int m_selectedResultIndex = -1;

    /**
     * Execute the query from the buffer.
     * @param app Reference to the App instance
     */
    void executeQuery(App& app);

    /**
     * Display example queries as help text.
     */
    void displayExamples() const;

    /**
     * Format a query result for display.
     * @param id The note id
     * @param title The note title
     * @return Formatted display string
     */
    std::string formatResultLine(const std::string& id, const std::string& title) const;
};

}  // namespace nodepanda
