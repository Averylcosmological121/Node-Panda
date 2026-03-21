#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace nodepanda {

struct GitCommitInfo {
    std::string hash;       // short hash
    std::string message;
    std::string date;       // YYYY-MM-DD HH:MM
    std::string author;
    int filesChanged = 0;
};

struct GitDiffLine {
    std::string text;
    int type = 0;  // 0=context, 1=added, -1=removed
};

class GitManager {
public:
    // Initialize git repo in the notes directory if not exists
    bool init(const std::filesystem::path& notesDir);

    // Auto-commit all changes with a message
    bool autoCommit(const std::string& message = "");

    // Get commit history for a specific file or all
    std::vector<GitCommitInfo> getHistory(const std::string& filename = "", int maxCount = 50) const;

    // Get diff between working copy and last commit for a file
    std::vector<GitDiffLine> getDiff(const std::string& filename) const;

    // Get diff between two commits for a file
    std::vector<GitDiffLine> getDiffBetween(const std::string& commitHash1,
                                             const std::string& commitHash2,
                                             const std::string& filename) const;

    // Restore a file from a specific commit
    bool restoreFile(const std::string& commitHash, const std::string& filename);

    // Check if git is available
    bool isAvailable() const { return m_available; }

    // Check if there are uncommitted changes
    bool hasChanges() const;

    // Get repo status
    struct RepoStatus {
        int modified = 0;
        int added = 0;
        int deleted = 0;
    };
    RepoStatus getStatus() const;

private:
    std::string runGitCommand(const std::string& args) const;

    std::filesystem::path m_notesDir;
    bool m_available = false;
};

} // namespace nodepanda
