#include "Event.hpp"
#include "../io/nhx_parser.hpp"

static const char* EVENT_KEY = "event";

Event::Event(const TaggedNode& tagnode)
{
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
}

Event::operator TaggedNode() const
{
    TaggedNode result;
    std::string event_as_str;

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
    return out << "Event {\n\ttype = " << event.type << "\n\tsynteny = "
        << event.synteny << "\n}";
}
