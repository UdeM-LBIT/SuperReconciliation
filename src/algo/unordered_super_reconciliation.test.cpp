#include "unordered_super_reconciliation.hpp"
#include "../io/nhx.hpp"
#include "../model/Event.hpp"
#include "../util/tree.hpp"
#include <catch.hpp>

void expect_reconciles_to(
    const ::tree<TaggedNode>& input,
    const ::tree<TaggedNode>& expected)
{
    auto reconciled_event = tree_cast<TaggedNode, Event>(input);
    unordered_super_reconciliation(reconciled_event);
    auto expected_event = tree_cast<TaggedNode, Event>(expected);

    auto it_reconciled = reconciled_event.begin();
    auto it_expected = expected_event.begin();

    while (it_reconciled != reconciled_event.end()
            && it_expected != expected_event.end())
    {
        REQUIRE(*it_reconciled == *it_expected);
        REQUIRE(it_reconciled.number_of_children()
                == it_expected.number_of_children());

        ++it_reconciled;
        ++it_expected;
    }

    REQUIRE(it_reconciled == reconciled_event.end());
    REQUIRE(it_expected == expected_event.end());
}

TEST_CASE("Unordered Super-Reconciliation examples")
{
    SECTION("Propagate duplications with leaf/loss children")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                (
                    [&&NHX:event=loss],
                    a
                )[&&NHX:event=speciation],
                (
                    b,
                    [&&NHX:event=loss]
                [It should propagate the parent synteny here because the
                 duplication has one full loss child node.]
                )[&&NHX:event=duplication]
            )"b a"[&&NHX:event=duplication];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                (
                    [&&NHX:event=loss],
                    a
                )a[&&NHX:event=speciation],
                (
                    b,
                    [&&NHX:event=loss]
                )"b a"[&&NHX:event=duplication:segment="0 - 1"]
            )"a b"[&&NHX:event=duplication:segment="0 - 1"];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }

    SECTION("Propagate nodes with propagable children")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                [Both children are propagable here because they only
                 have propagable children.]
                (
                    [Both children are propagable here because they only
                     have propagable children.]
                    (
                        [Both children are propagable here because they each
                         have at least one child full loss.]
                        (
                            [&&NHX:event=loss],
                            b
                        )[&&NHX:event=duplication],
                        (
                            [&&NHX:event=loss],
                            b
                        )[&&NHX:event=duplication]
                    )[&&NHX:event=speciation],
                    [Both children are propagable here because they only
                     have propagable children.]
                    (
                        [Both children are propagable here because they each
                         have at least one child full loss.]
                        (
                            [&&NHX:event=loss],
                            a
                        )[&&NHX:event=duplication],
                        (
                            [&&NHX:event=loss],
                            a
                        )[&&NHX:event=duplication]
                    )[&&NHX:event=speciation]
                )[&&NHX:event=speciation],
                "a c"
            )"a b c"[&&NHX:event=speciation];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                (
                    (
                        (
                            [&&NHX:event=loss],
                            b
                        )"a c b"[&&NHX:event=duplication:segment="2 - 3"],
                        (
                            [&&NHX:event=loss],
                            b
                        )"a c b"[&&NHX:event=duplication:segment="2 - 3"]
                    )"a b c"[&&NHX:event=speciation],
                    (
                        (
                            [&&NHX:event=loss],
                            a
                        )"b c a"[&&NHX:event=duplication:segment="2 - 3"],
                        (
                            [&&NHX:event=loss],
                            a
                        )"b c a"[&&NHX:event=duplication:segment="2 - 3"]
                    )"a b c"[&&NHX:event=speciation]
                )"a b c"[&&NHX:event=speciation],
                ("a c")"a c b"[&&NHX:event=loss:segment="2 - 3"]
            )"a c b"[&&NHX:event=speciation];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }

    SECTION("Propagate duplications with subtree/loss children")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                (
                    "a b",
                    (
                        a,
                        [&&NHX:event=loss]
                    )[&&NHX:event=speciation]
                )[&&NHX:event=duplication],
                (
                    [However, because this duplication has a full loss node
                     child, and regardless of the other child’s subtree,
                     it should propagate.]
                    (
                        [Nothing is propagable up to here.]
                        (
                            (
                                b,
                                b
                            )[&&NHX:event=speciation],
                            (
                                b,
                                b
                            )[&&NHX:event=duplication]
                        )[&&NHX:event=speciation],
                        [&&NHX:event=loss]
                    )[&&NHX:event=duplication],
                    [&&NHX:event=loss]
                )[&&NHX:event=speciation]
            )"a b"[&&NHX:event=speciation];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                (
                    "a b",
                    [But this is not the case with speciations.]
                    (
                        a,
                        [&&NHX:event=loss]
                    )"a"[&&NHX:event=speciation]
                )"a b"[&&NHX:event=duplication:segment="0 - 1"],
                (
                    (
                        (
                            (
                                b,
                                b
                            )"b"[&&NHX:event=speciation],
                            (
                                b,
                                b
                            )"b"[&&NHX:event=duplication:segment="0 - 1"]
                        )"b"[&&NHX:event=speciation],
                        [&&NHX:event=loss]
                    )"b a"[&&NHX:event=duplication:segment="0 - 1"],
                    [&&NHX:event=loss]
                )"a b"[&&NHX:event=speciation]
            )"a b"[&&NHX:event=speciation];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }

    SECTION("Propagate nodes with loss/propagable children")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                [It propagates because it has a full loss and a
                 propagable child.]
                (
                    [&&NHX:event=loss],
                    [This duplication propagates because it has
                     two disjoint children.]
                    (
                        c,
                        b
                    )[&&NHX:event=duplication]
                )[&&NHX:event=speciation],
                "a b c"
            )"a b c"[&&NHX:event=speciation];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                (
                    [&&NHX:event=loss],
                    (
                        c,
                        (b)"c a b"[&&NHX:event=loss:segment="0 - 2"]
                    )"c a b"[&&NHX:event=duplication:segment="0 - 1"]
                )"a b c"[&&NHX:event=speciation],
                "a b c"
            )"a b c"[&&NHX:event=speciation];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }

    SECTION("Propagate nodes with two disjoint children sets")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                [It should propagate because both children
                 are disjoint.]
                (
                    (
                        e,
                        "e a"
                    )[&&NHX:event=duplication],
                    b
                )[&&NHX:event=duplication],
                "e b c"
            )"a b c e"[&&NHX:event=duplication];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                (
                    (
                        e,
                        "e a"
                    )"e a"[&&NHX:event=duplication:segment="0 - 1"],
                    (b)"a e c b"[&&NHX:event=loss:segment="0 - 3"]
                )"a e c b"[&&NHX:event=duplication:segment="0 - 2"],
                "e b c"
            )"b c e a"[&&NHX:event=duplication:segment="0 - 3"];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }

    SECTION("Propagate duplications with subtree/propagable children")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                "a b c",
                [It should propagate here because it is a duplication
                 whose children contain at least one propagable node.]
                (
                    [It should propagate here because the two children
                     are disjoint.]
                    (
                        b,
                        a
                    )[&&NHX:event=duplication],
                    (
                        "b a",
                        "b a"
                    )[&&NHX:event=speciation]
                )[&&NHX:event=duplication]
            )"a b c"[&&NHX:event=speciation];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                "a b c",
                (
                    (
                        b,
                        (a)"b c a"[&&NHX:event=loss:segment="0 - 2"]
                    )"b c a"[&&NHX:event=duplication:segment="0 - 1"],
                    (
                        "b a",
                        "b a"
                    )"a b"[&&NHX:event=speciation]
                )"a b c"[&&NHX:event=duplication:segment="0 - 2"]
            )"a b c"[&&NHX:event=speciation];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }

    SECTION("NOT propagate speciations with only one propagable child")
    {
        auto input_tree = parse_nhx_tree(R"NHX(
            (
                [Because it is a speciation and has only one
                 propagable child, it is not itself propagable.]
                (
                    [This is propagable.]
                    (
                        [&&NHX:event=loss],
                        (
                            [&&NHX:event=loss],
                            a
                        )[&&NHX:event=speciation]
                    )[&&NHX:event=speciation],
                    [But this is not.]
                    (
                        (
                            a,
                            a
                        )[&&NHX:event=duplication],
                        (
                            a,
                            a
                        )[&&NHX:event=speciation]
                    )[&&NHX:event=speciation]
                )[&&NHX:event=speciation],
                "a b c d"
            )"a b c d"[&&NHX:event=duplication];
        )NHX");

        auto expected_tree = parse_nhx_tree(R"NHX(
            (
                (
                    (
                        [&&NHX:event=loss],
                        (
                            [&&NHX:event=loss],
                            a
                        )"a"[&&NHX:event=speciation]
                    )"a"[&&NHX:event=speciation],
                    (
                        (
                            a,
                            a
                        )"a"[&&NHX:event=duplication:segment="0 - 1"],
                        (
                            a,
                            a
                        )"a"[&&NHX:event=speciation]
                    )"a"[&&NHX:event=speciation]
                )"a"[&&NHX:event=speciation],
                "a b c d"
            )"a b c d"[&&NHX:event=duplication:segment="0 - 1"];
        )NHX");

        expect_reconciles_to(input_tree, expected_tree);
    }
}
