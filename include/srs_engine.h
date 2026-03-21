#pragma once
#include <string>
#include <vector>
#include <chrono>

namespace nodepanda {

class NoteManager;

struct SRSCard {
    std::string noteId;
    std::string title;
    int interval = 1;        // days until next review
    float easeFactor = 2.5f; // SM-2 ease factor
    int repetitions = 0;     // successful repetitions count
    std::string nextReview;  // YYYY-MM-DD
    std::string lastReview;  // YYYY-MM-DD
    bool isDue = false;
};

// SM-2 quality ratings
enum class SRSRating {
    Again = 0,  // Complete blackout
    Hard  = 2,  // Remembered with difficulty
    Good  = 3,  // Correct with some hesitation
    Easy  = 5   // Perfect recall
};

class SRSEngine {
public:
    // Scan all notes and build review queue from frontmatter
    void buildQueue(NoteManager& nm);

    // Rate a card and update its SRS data in frontmatter
    void rateCard(const std::string& noteId, SRSRating rating, NoteManager& nm);

    // Enroll a note into SRS (sets initial frontmatter)
    void enrollNote(const std::string& noteId, NoteManager& nm);

    // Remove note from SRS
    void unenrollNote(const std::string& noteId, NoteManager& nm);

    // Is this note enrolled in SRS?
    bool isEnrolled(const std::string& noteId, NoteManager& nm) const;

    // Get today's due cards
    const std::vector<SRSCard>& getDueCards() const { return m_dueCards; }

    // Get all enrolled cards
    const std::vector<SRSCard>& getAllCards() const { return m_allCards; }

    // Stats
    int totalEnrolled() const { return (int)m_allCards.size(); }
    int dueToday() const { return (int)m_dueCards.size(); }

    static std::string todayStr();

private:
    void applySmTwo(SRSCard& card, SRSRating rating);
    SRSCard readCardFromNote(const std::string& noteId, NoteManager& nm) const;
    void writeCardToNote(const SRSCard& card, NoteManager& nm);

    std::vector<SRSCard> m_dueCards;
    std::vector<SRSCard> m_allCards;
};

} // namespace nodepanda
