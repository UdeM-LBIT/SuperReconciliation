#include "Event.hpp"
#include "../io/nhx.hpp"

static const char* EVENT_KEY = "event";
static const char* SEGMENT_KEY = "segment";

Event::Event(const TaggedNode& tagnode)
{
    // Read the event type
    if (tagnode.tags.count(EVENT_KEY))
    {
        const auto& event_str = tagnode.tags.at(EVENT_KEY);

        if (event_str == "duplication")
        {
            this->type = Type::Duplication;
        }
        else if (event_str == "speciation")
        {
            this->type = Type::Speciation;
        }
        else if (event_str == "loss")
        {
            this->type = Type::Loss;
        }
    }

    // Read the synteny linked to this node, which is encoded as a
    // whitespace-separated list of strings
    if (!tagnode.name.empty())
    {
        std::istringstream synteny_tok{tagnode.name};

        while (synteny_tok)
        {
            Gene gene;
            synteny_tok >> gene;

            if (!gene.empty())
            {
                this->synteny.push_back(gene);
            }
        }
    }

    // An empty leaf node is actually a full loss node
    if (this->type == Type::None && this->synteny.empty())
    {
        this->type = Type::Loss;
    }

    // Read the segment linked to this node, if applicable, which is
    // formatted as a number followed by an hyphen and another number
    if (tagnode.tags.count(SEGMENT_KEY)
            && ((this->type == Type::Loss && !this->synteny.empty())
                || this->type == Type::Duplication))
    {
        std::istringstream segment_tok{tagnode.tags.at(SEGMENT_KEY)};
        std::size_t start = 0, end = 0;

        segment_tok >> start;
        segment_tok.ignore(std::numeric_limits<std::streamsize>::max(), '-');
        segment_tok >> end;

        this->segment.first = start;
        this->segment.second = end;
    }
}

Event::operator TaggedNode() const
{
    TaggedNode result;

    switch (this->type)
    {
    case Event::Type::None:
        break;

    case Event::Type::Duplication:
        result.tags.emplace(EVENT_KEY, "duplication");
        break;

    case Event::Type::Speciation:
        result.tags.emplace(EVENT_KEY, "speciation");
        break;

    case Event::Type::Loss:
        result.tags.emplace(EVENT_KEY, "loss");
        break;
    }

    if (!this->synteny.empty())
    {
        std::string synteny_as_str;

        for (auto it = std::cbegin(this->synteny);
                it != std::cend(this->synteny);
                ++it)
        {
            synteny_as_str += *it;

            if (std::next(it) != std::cend(this->synteny))
            {
                synteny_as_str += " ";
            }
        }

        result.name = synteny_as_str;
    }

    if (this->segment != Synteny::NoSegment
            && (this->type == Type::Loss || this->type == Type::Duplication))
    {
        std::ostringstream segment_as_str;
        segment_as_str
            << this->segment.first << " - "
            << this->segment.second;

        result.tags.emplace(SEGMENT_KEY, segment_as_str.str());
    }

    return result;
}

std::ostream& operator<<(std::ostream& out, const Event::Type& type)
{
    switch (type)
    {
    case Event::Type::None:
        return out << "None";

    case Event::Type::Duplication:
        return out << "Duplication";

    case Event::Type::Speciation:
        return out << "Speciation";

    case Event::Type::Loss:
        return out << "Loss";
    }

    return out << "[Invalid type: "
        << std::to_string(static_cast<int>(type)) << "]";
}

std::ostream& operator<<(std::ostream& out, const Event& event)
{
    out << "{type=" << event.type
        << ", synteny=\"" << event.synteny << "\"";

    if ((event.type == Event::Type::Duplication
                || event.type == Event::Type::Loss)
            && event.segment != Synteny::NoSegment)
    {
        out << ", segment=[" << event.segment.first
            << " - " << event.segment.second << "[";
    }

    return out << "}";
}

bool operator==(const Event& lhs, const Event& rhs)
{
    if (lhs.type != rhs.type)
    {
        return false;
    }

    if ((lhs.type == Event::Type::Duplication
                || lhs.type == Event::Type::Loss)
            && lhs.segment != rhs.segment)
    {
        return false;
    }

    return lhs.synteny == rhs.synteny;
}
