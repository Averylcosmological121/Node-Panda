#include "query_engine.h"
#include "note_manager.h"
#include "graph.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <stdexcept>

namespace nodepanda {

std::vector<QueryResult> QueryEngine::execute(
    const QueryDefinition& query,
    const NoteManager& nm,
    const Graph& graph
) const {
    std::vector<QueryResult> results;

    // Get all notes from the NoteManager
    const auto& allNotes = nm.getAllNotes();

    // Filter notes based on conditions
    for (const auto& [noteId, note] : allNotes) {
        bool matches = true;

        // Check all conditions (AND logic)
        for (const auto& condition : query.conditions) {
            std::string fieldValue = getFieldValue(condition.field, noteId, nm, graph);
            if (!evaluateCondition(condition, fieldValue)) {
                matches = false;
                break;
            }
        }

        if (matches) {
            QueryResult result;
            result.noteId = noteId;
            result.title = note.title;

            // Populate values map with requested fields
            for (const auto& condition : query.conditions) {
                std::string fieldValue = getFieldValue(condition.field, noteId, nm, graph);
                result.values[condition.field] = fieldValue;
            }

            // Also add sort field if not already present
            if (!query.sortField.empty()) {
                std::string sortValue = getFieldValue(query.sortField, noteId, nm, graph);
                result.values[query.sortField] = sortValue;
            }

            results.push_back(result);
        }
    }

    // Sort results
    if (!query.sortField.empty()) {
        std::sort(results.begin(), results.end(),
            [this, &query](const QueryResult& a, const QueryResult& b) {
                std::string aVal = a.values.at(query.sortField);
                std::string bVal = b.values.at(query.sortField);

                // Try to parse as numbers for numeric comparison
                char* endA = nullptr;
                char* endB = nullptr;
                double numA = std::strtod(aVal.c_str(), &endA);
                double numB = std::strtod(bVal.c_str(), &endB);

                // If both are valid numbers, compare numerically
                if (*endA == '\0' && *endB == '\0') {
                    return query.sortAscending ? (numA < numB) : (numA > numB);
                }

                // Otherwise, compare as strings
                int cmp = aVal.compare(bVal);
                return query.sortAscending ? (cmp < 0) : (cmp > 0);
            }
        );
    }

    // Apply limit
    if (query.limit > 0 && results.size() > static_cast<size_t>(query.limit)) {
        results.resize(query.limit);
    }

    return results;
}

QueryDefinition QueryEngine::parse(const std::string& queryStr) const {
    QueryDefinition result;

    // Normalize whitespace and uppercase key
    std::string normalized;
    for (char c : queryStr) {
        normalized += std::toupper(static_cast<unsigned char>(c));
    }

    // Find QUERY keyword
    size_t queryPos = normalized.find("QUERY");
    if (queryPos == std::string::npos) {
        throw std::runtime_error("Query must start with QUERY keyword");
    }

    // Find WHERE keyword
    size_t wherePos = normalized.find("WHERE");
    if (wherePos == std::string::npos) {
        throw std::runtime_error("Query must contain WHERE clause");
    }

    // Find SORT keyword (optional)
    size_t sortPos = normalized.find("SORT");
    size_t conditionsEnd = sortPos;
    if (sortPos == std::string::npos) {
        conditionsEnd = normalized.find("LIMIT");
    }
    if (conditionsEnd == std::string::npos) {
        conditionsEnd = queryStr.length();
    }

    // Extract conditions
    std::string conditionsStr = queryStr.substr(wherePos + 5, conditionsEnd - (wherePos + 5));
    conditionsStr = trim(conditionsStr);

    // Split conditions by AND
    std::vector<std::string> conditionStrs;
    size_t start = 0;
    size_t pos = 0;
    while ((pos = normalized.find("AND", wherePos + 5 + start)) != std::string::npos &&
           pos < conditionsEnd) {
        size_t condStart = wherePos + 5 + start;
        conditionStrs.push_back(conditionsStr.substr(start, pos - condStart));
        start = pos - (wherePos + 5) + 3;
        pos = start;
    }
    if (start < conditionsStr.length()) {
        conditionStrs.push_back(conditionsStr.substr(start));
    }

    // Parse each condition
    for (const auto& condStr : conditionStrs) {
        std::string trimmed = trim(condStr);
        if (trimmed.empty()) continue;

        QueryCondition condition;

        // Try to find operator (longest first to handle >= and <=)
        const std::vector<std::string> operators = {">=", "<=", "!=", "CONTIENE", "NO_CONTIENE", "=", ">", "<"};
        size_t opPos = std::string::npos;
        std::string foundOp;

        for (const auto& op : operators) {
            size_t pos = 0;
            while ((pos = normalized.find(op, wherePos + 5 + start)) != std::string::npos) {
                if (pos >= wherePos + 5) {
                    opPos = pos;
                    foundOp = op;
                    break;
                }
                break;
            }
            if (opPos != std::string::npos) break;
        }

        if (opPos == std::string::npos) {
            throw std::runtime_error("Invalid condition: " + trimmed);
        }

        // Extract field, operator, and value
        condition.field = trim(trimmed.substr(0, opPos - (wherePos + 5 + start)));
        condition.op = foundOp;

        size_t valueStart = opPos - (wherePos + 5 + start) + foundOp.length();
        std::string valueStr = trim(trimmed.substr(valueStart));

        // Remove quotes if present
        if ((valueStr.front() == '"' && valueStr.back() == '"') ||
            (valueStr.front() == '\'' && valueStr.back() == '\'')) {
            valueStr = valueStr.substr(1, valueStr.length() - 2);
        }

        condition.value = valueStr;
        result.conditions.push_back(condition);
    }

    // Parse SORT clause
    if (sortPos != std::string::npos) {
        size_t limitPos = normalized.find("LIMIT", sortPos);
        size_t sortEnd = limitPos;
        if (limitPos == std::string::npos) {
            sortEnd = queryStr.length();
        }

        std::string sortStr = queryStr.substr(sortPos + 4, sortEnd - (sortPos + 4));
        sortStr = trim(sortStr);

        // Check for ASC/DESC
        if (sortStr.find("DESC") != std::string::npos) {
            result.sortAscending = false;
            size_t descPos = normalized.find("DESC", sortPos);
            result.sortField = trim(sortStr.substr(0, descPos - (sortPos + 4)));
        } else if (sortStr.find("ASC") != std::string::npos) {
            result.sortAscending = true;
            size_t ascPos = normalized.find("ASC", sortPos);
            result.sortField = trim(sortStr.substr(0, ascPos - (sortPos + 4)));
        } else {
            result.sortField = trim(sortStr);
            result.sortAscending = true;
        }
    }

    // Parse LIMIT clause
    if (normalized.find("LIMIT") != std::string::npos) {
        size_t limitPos = normalized.find("LIMIT");
        std::string limitStr = queryStr.substr(limitPos + 5);
        limitStr = trim(limitStr);

        try {
            result.limit = std::stoi(limitStr);
        } catch (...) {
            throw std::runtime_error("Invalid LIMIT value: " + limitStr);
        }
    }

    return result;
}

bool QueryEngine::evaluateCondition(
    const QueryCondition& condition,
    const std::string& fieldValue
) const {
    const auto& op = condition.op;
    const auto& value = condition.value;

    if (op == "=") {
        return fieldValue == value;
    } else if (op == "!=") {
        return fieldValue != value;
    } else if (op == ">") {
        try {
            double fVal = std::stod(fieldValue);
            double cVal = std::stod(value);
            return fVal > cVal;
        } catch (...) {
            return fieldValue > value;
        }
    } else if (op == "<") {
        try {
            double fVal = std::stod(fieldValue);
            double cVal = std::stod(value);
            return fVal < cVal;
        } catch (...) {
            return fieldValue < value;
        }
    } else if (op == ">=") {
        try {
            double fVal = std::stod(fieldValue);
            double cVal = std::stod(value);
            return fVal >= cVal;
        } catch (...) {
            return fieldValue >= value;
        }
    } else if (op == "<=") {
        try {
            double fVal = std::stod(fieldValue);
            double cVal = std::stod(value);
            return fVal <= cVal;
        } catch (...) {
            return fieldValue <= value;
        }
    } else if (op == "CONTIENE") {
        return fieldValue.find(value) != std::string::npos;
    } else if (op == "NO_CONTIENE") {
        return fieldValue.find(value) == std::string::npos;
    }

    return false;
}

std::string QueryEngine::getFieldValue(
    const std::string& fieldName,
    const std::string& noteId,
    const NoteManager& nm,
    const Graph& graph
) const {
    const auto& allNotes = nm.getAllNotes();
    auto it = allNotes.find(noteId);
    if (it == allNotes.end()) {
        return "";
    }

    const auto& note = it->second;
    std::string field = toUpper(fieldName);

    // Direct fields
    if (field == "TITULO") {
        return note.title;
    } else if (field == "ID") {
        return note.id;
    }

    // Computed fields
    if (field == "PALABRAS") {
        return std::to_string(countWords(note.content));
    } else if (field == "CARACTERES") {
        return std::to_string(note.content.size());
    } else if (field == "ENLACES") {
        return std::to_string(note.links.size());
    } else if (field == "FECHA") {
        // Format lastModified as YYYY-MM-DD
        std::time_t t = std::chrono::system_clock::to_time_t(note.lastModified);
        std::tm* timeinfo = std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(timeinfo, "%Y-%m-%d");
        return oss.str();
    }

    // Graph-related fields
    if (field == "CONEXIONES") {
        const auto& nodes = graph.getNodes();
        for (const auto& node : nodes) {
            if (node.noteId == noteId) {
                return std::to_string(node.connectionCount);
            }
        }
        return "0";
    }

    // Frontmatter fields
    auto fmIt = note.frontmatter.find(fieldName);
    if (fmIt != note.frontmatter.end()) {
        return fmIt->second;
    }

    // Lowercase check for frontmatter
    auto fmIt2 = note.frontmatter.find(toUpper(fieldName));
    if (fmIt2 != note.frontmatter.end()) {
        return fmIt2->second;
    }

    return "";
}

int QueryEngine::countWords(const std::string& text) const {
    std::istringstream iss(text);
    int wordCount = 0;
    std::string word;
    while (iss >> word) {
        wordCount++;
    }
    return wordCount;
}

std::string QueryEngine::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> QueryEngine::split(
    const std::string& str,
    char delimiter
) const {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string QueryEngine::toUpper(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

}  // namespace nodepanda
