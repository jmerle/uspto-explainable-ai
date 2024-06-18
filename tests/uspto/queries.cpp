#include <gtest/gtest.h>

#include <uspto/queries.h>

TEST(Term, constructorDecomposesString) {
    Term term1("ti:title");
    EXPECT_EQ(term1.category, TermCategory::Title);
    EXPECT_EQ(term1.token, "title");

    Term term2("ab:abstract");
    EXPECT_EQ(term2.category, TermCategory::Abstract);
    EXPECT_EQ(term2.token, "abstract");

    Term term3("clm:claims");
    EXPECT_EQ(term3.category, TermCategory::Claims);
    EXPECT_EQ(term3.token, "claims");

    Term term4("detd:description");
    EXPECT_EQ(term4.category, TermCategory::Description);
    EXPECT_EQ(term4.token, "description");

    Term term5("cpc:cpc");
    EXPECT_EQ(term5.category, TermCategory::Cpc);
    EXPECT_EQ(term5.token, "cpc");
}
