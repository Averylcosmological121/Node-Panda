#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace nodepanda {

// Forward declarations
class NoteManager;
class Graph;

/**
 * Represents a single condition in a query.
 * Example: field="type", op="=", value="project"
 */
struct QueryCondition {
    std::string field;
    std::string op;  // =, !=, >, <, >=, <=, CONTIENE, NO_CONTIENE
    std::string value;
};

/**
 * Represents sorting specification.
 */
struct QuerySort {
    std::string field;
    bool ascending = true;
};

/**
 * Represents a single result from query execution.
 */
struct QueryResult {
    std::string noteId;
    std::string title;
    std::map<std::string, std::string> values;  // field -> value pairs for display
};

/**
 * Represents a complete query definition.
 * Example:
 *   QUERY notas WHERE tipo = "proyecto" AND enlaces > 3 SORT fecha DESC LIMIT 10
 */
struct QueryDefinition {
    std::vector<QueryCondition> conditions;
    std::string sortField;
    bool sortAscending = true;
    int limit = -1;  // -1 means no limit
};

/**
 * Query Engine for executing Dataview-style queries on notes.
 * Supports filtering, sorting, and limiting based on note properties.
 */
class QueryEngine {
public:
    QueryEngine() = default;
    ~QueryEngine() = default;

    /**
     * Execute a query definition and return matching results.
     * @param query The query definition
     * @param nm The NoteManager containing all notes
     * @param graph The Graph for computing graph-related fields
     * @return Vector of matching QueryResults
     */
    std::vector<QueryResult> execute(
        const QueryDefinition& query,
        const NoteManager& nm,
        const Graph& graph
    ) const;

    /**
     * Parse a query string into a QueryDefinition.
     * Format: QUERY notas WHERE [conditions] SORT [field] ASC|DESC LIMIT N
     *
     * Example:
     *   QUERY notas WHERE tipo = "proyecto" AND enlaces > 3 SORT fecha DESC LIMIT 10
     *
     * @param queryStr The query string to parse
     * @return Parsed QueryDefinition
     * @throws std::runtime_error on parse error
     */
    QueryDefinition parse(const std::string& queryStr) const;

private:
    /**
     * Evaluate a single condition against a note's field value.
     * @param condition The condition to evaluate
     * @param fieldValue The actual field value from the note
     * @return true if condition is satisfied
     */
    bool evaluateCondition(
        const QueryCondition& condition,
        const std::string& fieldValue
    ) const;

    /**
     * Get a field value from a note.
     * Supports both direct fields (title, id) and computed fields (palabras, enlaces, etc.)
     * Also supports frontmatter keys.
     *
     * Supported fields:
     *   - titulo: note title
     *   - id: note id
     *   - tipo: frontmatter["tipo"]
     *   - tags: frontmatter["tags"]
     *   - palabras: word count in content
     *   - caracteres: character count in content
     *   - enlaces: number of links
     *   - conexiones: graph connection count
     *   - fecha: last modified date as YYYY-MM-DD
     *   - any other: frontmatter[field]
     *
     * @param fieldName The name of the field to retrieve
     * @param noteId The id of the note
     * @param nm The NoteManager
     * @param graph The Graph
     * @return The field value as a string, or empty string if not found
     */
    std::string getFieldValue(
        const std::string& fieldName,
        const std::string& noteId,
        const NoteManager& nm,
        const Graph& graph
    ) const;

    /**
     * Count words in text.
     * @param text The text to count words in
     * @return Number of words
     */
    int countWords(const std::string& text) const;

    /**
     * Trim leading and trailing whitespace.
     * @param str The string to trim
     * @return Trimmed string
     */
    std::string trim(const std::string& str) const;

    /**
     * Split a string by delimiter.
     * @param str The string to split
     * @param delimiter The delimiter character
     * @return Vector of split strings
     */
    std::vector<std::string> split(
        const std::string& str,
        char delimiter
    ) const;

    /**
     * Convert string to uppercase.
     * @param str The string to convert
     * @return Uppercase string
     */
    std::string toUpper(const std::string& str) const;
};

}  // namespace nodepanda
