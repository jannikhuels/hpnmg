#include "gtest/gtest.h"
#include "ReadFormula.h"

using namespace hpnmg;

TEST(DiscreteFormula, DiscreteAtomicProperty) {
    Formula formula = ReadFormula{}.readFormula("DiscreteAtomicFormula.xml");

    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty,formula.getType());
    EXPECT_EQ( "test", formula.getDiscreteAtomicProperty()->place);
    EXPECT_EQ(1, formula.getDiscreteAtomicProperty()->value);
}

TEST(ContinuousFormula, ContinuousAtomicProperty){
    Formula formula = ReadFormula{}.readFormula("ContinuousAtomicFormula.xml");

    EXPECT_EQ(Formula::Type::ContinuousAtomicProperty, formula.getType());
    EXPECT_EQ("continuous", formula.getContinuousAtomicProperty()->place);
    EXPECT_EQ(13.8, formula.getContinuousAtomicProperty()->value);
}

TEST(ConjunctionFormula, DiscreteAtomicProperty) {
    Formula formula = ReadFormula{}.readFormula("ConjunctionFormula.xml");
    EXPECT_EQ(Formula::Type::Conjunction, formula.getType());
    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty, formula.getConjunction()->left.getType());
    EXPECT_EQ("disc1", formula.getConjunction()->left.getDiscreteAtomicProperty()->place);
    EXPECT_EQ(13, formula.getConjunction()->left.getDiscreteAtomicProperty()->value);
    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty, formula.getConjunction()->right.getType());
    EXPECT_EQ("disc2", formula.getConjunction()->right.getDiscreteAtomicProperty()->place);
    EXPECT_EQ(27, formula.getConjunction()->right.getDiscreteAtomicProperty()->value);
}

TEST(NegationFormula, ContinuousAtomicProperty) {
    Formula formula = ReadFormula{}.readFormula("NegationFormula.xml");
    EXPECT_EQ(Formula::Type::Negation, formula.getType());
    EXPECT_EQ(Formula::Type::ContinuousAtomicProperty, formula.getNegation()->formula.getType());
    EXPECT_EQ("cont1", formula.getNegation()->formula.getContinuousAtomicProperty()->place);
    EXPECT_EQ(11, formula.getNegation()->formula.getContinuousAtomicProperty()->value);
}

TEST(Interleaved, NegationOfConjunction) {
    Formula formula = ReadFormula{}.readFormula("InterleavedFormula.xml");
    EXPECT_EQ(Formula::Type::Negation, formula.getType());
    EXPECT_EQ(Formula::Type::Conjunction, formula.getNegation()->formula.getType());

    std::shared_ptr<Conjunction> conjunction = formula.getNegation()->formula.getConjunction();

    EXPECT_EQ("disc1", conjunction->left.getDiscreteAtomicProperty()->place);
    EXPECT_EQ(13, conjunction->left.getDiscreteAtomicProperty()->value);
    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty, conjunction->right.getType());
    EXPECT_EQ("disc2", conjunction->right.getDiscreteAtomicProperty()->place);
    EXPECT_EQ(27, conjunction->right.getDiscreteAtomicProperty()->value);
}

TEST(Until, DiscreteUntilDiscrete) {
    Formula formula = ReadFormula{}.readFormula("UntilFormula.xml");
    EXPECT_EQ(Formula::Type::Until, formula.getType());
    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty, formula.getUntil()->pre.getType());
    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty, formula.getUntil()->goal.getType());

    std::shared_ptr<DiscreteAtomicProperty> pre = formula.getUntil()->pre.getDiscreteAtomicProperty();
    EXPECT_EQ("disc1", pre->place);
    EXPECT_EQ(1, pre->value);

    std::shared_ptr<DiscreteAtomicProperty> goal = formula.getUntil()->goal.getDiscreteAtomicProperty();
    EXPECT_EQ("disc2", goal->place);
    EXPECT_EQ(2, goal->value);

}

TEST(Until, UntilWithNegation) {
    Formula formula = ReadFormula{}.readFormula("UntilWithNegationFormula.xml");
    EXPECT_EQ(Formula::Type::Until, formula.getType());
    EXPECT_EQ(Formula::Type::Negation, formula.getUntil()->pre.getType());
    EXPECT_EQ(Formula::Type::DiscreteAtomicProperty, formula.getUntil()->goal.getType());

    std::shared_ptr<Negation> neg = formula.getUntil()->pre.getNegation();
    EXPECT_EQ(Formula::Type::ContinuousAtomicProperty, neg->formula.getType());

    std::shared_ptr<ContinuousAtomicProperty> pre = neg->formula.getContinuousAtomicProperty();
    EXPECT_EQ("cont1", pre->place);
    EXPECT_EQ(11, pre->value);

    std::shared_ptr<DiscreteAtomicProperty> goal = formula.getUntil()->goal.getDiscreteAtomicProperty();
    EXPECT_EQ("disc2", goal->place);
    EXPECT_EQ(2, goal->value);
}

TEST(TrueFormula, TrueProperty) {
    Formula formula = ReadFormula{}.readFormula("TrueFormula.xml");
    EXPECT_EQ(Formula::Type::True, formula.getType());
}

TEST(FalseFormula, FalseProperty) {
    Formula formula = ReadFormula{}.readFormula("FalseFormula.xml");
    EXPECT_EQ(Formula::Type::False, formula.getType());
}
