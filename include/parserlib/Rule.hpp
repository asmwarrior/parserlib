#ifndef PARSERLIB_RULE_HPP
#define PARSERLIB_RULE_HPP


#include <memory>
#include <optional>
#include "ParserWrapper.hpp"
#include "RuleReference.hpp"
#include "SequenceParser.hpp"
#include "ChoiceParser.hpp"
#include "LoopParser.hpp"
#include "Loop1Parser.hpp"
#include "OptionalParser.hpp"
#include "AndParser.hpp"
#include "NotParser.hpp"
#include "Match.hpp"
#include "LeftRecursionException.hpp"


namespace parserlib {


    /**
     * An interface to a parser that can be recursive.
     * @param ParseContextType type of context to pass to the parse function.
     */
    template <class ParseContextType = ParseContext<>> class Rule {
    public:
        /**
         * Constructor.
         * Allocates a copy of the given parser on the heap.
         * @param parser parser to copy.
         */
        template <class ParserType> Rule(const ParserType& parser)
            : m_parser(std::make_shared<ParserWrapper<ParseContextType, ParserType>>(parser))
        {
        }

        /**
         * Returns the parser.
         * @return the parser.
         */
        const std::shared_ptr<ParserInterface<ParseContextType>>& parser() const {
            return m_parser;
        }

        /**
         * Invokes the underlying parser.
         * @param pc parse context.
         * @return whatever the underlying parser returns.
         * @exception LeftRecursionException thrown when left recursion is found upon the referenced rule.
         */
        bool operator ()(ParseContextType& pc) {
            //check for left recursion
            if (m_parsePosition == pc.sourcePosition()) {
                switch (m_state) {
                    case Normal: throw LeftRecursionException<ParseContextType>(*this);
                    case Reject: return false;
                    case Accept: return true;
                }
            }

            //save state
            const auto prevParsePosition = m_parsePosition;
            const auto prevState = m_state;

            //set new state
            m_parsePosition = pc.sourcePosition();
            m_state = Normal;

            //parse
            const bool result = m_parser->operator ()(pc);

            //restore state
            m_parsePosition = prevParsePosition;
            m_state = prevState;

            return result;
        }

        /**
         * Returns the parse position of the rule.
         * @return the parse position of the rule.
         */
        const std::optional<typename ParseContextType::Position> getParsePosition() const {
            return m_parsePosition;
        }

        /**
         * Sets the parse position.
         * @param position new parse position.
         */
        void setParsePosition(const typename ParseContextType::Position& position) {
            m_parsePosition = position;
        }

        /**
         * Sets the state to reject.
         */
        void setRejectState() {
            m_state = Reject;
        }

        /**
         * Sets the state to accept.
         */
        void setAcceptState() {
            m_state = Accept;
        }

    private:
        enum State {
            Normal,
            Reject,
            Accept
        };
        std::shared_ptr<ParserInterface<ParseContextType>> m_parser;
        std::optional<typename ParseContextType::Position> m_parsePosition;
        State m_state{ Normal };
    };


    /**
     * Sequence between two rules.
     * @param rule1 the 1st rule.
     * @param rule2 the 2nd rule.
     * @return a sequence of the two rules.
     */
    template <class ParseContextType>
    auto operator >> (Rule<ParseContextType>& rule1, Rule<ParseContextType>& rule2) {
        return RuleReference<ParseContextType>(rule1) >> RuleReference<ParseContextType>(rule2);
    }


    /**
     * Sequence between a rule and a parser node.
     * @param rule the rule parser.
     * @param node the node parser.
     * @return a sequence of the rule and node.
     */
    template <class ParseContextType, class ParserNodeType>
    auto operator >> (Rule<ParseContextType>& rule, const ParserNode<ParserNodeType>& node) {
        return RuleReference<ParseContextType>(rule) >> node;
    }


    /**
     * Sequence between a parser node and a rule.
     * @param node the node parser.
     * @param rule the rule parser.
     * @return a sequence of the two nodes.
     */
    template <class ParserNodeType, class ParseContextType>
    auto operator >> (const ParserNode<ParserNodeType>& node, Rule<ParseContextType>& rule) {
        return node >> RuleReference<ParseContextType>(rule);
    }


    /**
     * Choice between two rules.
     * @param rule1 the 1st rule.
     * @param rule2 the 2nd rule.
     * @return a sequence of the two rules.
     */
    template <class ParseContextType>
    auto operator | (Rule<ParseContextType>& rule1, Rule<ParseContextType>& rule2) {
        return RuleReference<ParseContextType>(rule1) | RuleReference<ParseContextType>(rule2);
    }


    /**
     * Choice between a rule and a parser node.
     * @param rule the rule parser.
     * @param node the node parser.
     * @return a sequence of the rule and node.
     */
    template <class ParseContextType, class ParserNodeType>
    auto operator | (Rule<ParseContextType>& rule, const ParserNode<ParserNodeType>& node) {
        return RuleReference<ParseContextType>(rule) | node;
    }


    /**
     * Choice between a parser node and a rule.
     * @param node the node parser.
     * @param rule the rule parser.
     * @return a sequence of the two nodes.
     */
    template <class ParserNodeType, class ParseContextType>
    auto operator | (const ParserNode<ParserNodeType>& node, Rule<ParseContextType>& rule) {
        return node | RuleReference<ParseContextType>(rule);
    }


    /**
     * Creates a loop parser out of a rule.
     * @param rule rule.
     * @return the loop parser for the given rule.
     */
    template <class ParseContextType> auto operator *(Rule<ParseContextType>& rule) {
        return *RuleReference<ParseContextType>(rule);
    }


    /**
     * Creates a loop-1 parser out of a rule.
     * @param rule rule.
     * @return the loop-1 parser for the given rule.
     */
    template <class ParseContextType> auto operator +(Rule<ParseContextType>& rule) {
        return +RuleReference<ParseContextType>(rule);
    }


    /**
     * Creates an optional parser out of a rule.
     * @param rule rule.
     * @return the optional parser for the given rule.
     */
    template <class ParseContextType> auto operator -(Rule<ParseContextType>& rule) {
        return -RuleReference<ParseContextType>(rule);
    }


    /**
     * Creates an and parser out of a rule.
     * @param rule rule.
     * @return the and parser for the given rule.
     */
    template <class ParseContextType> auto operator &(Rule<ParseContextType>& rule) {
        return &RuleReference<ParseContextType>(rule);
    }


    /**
     * Creates a not parser out of a rule.
     * @param rule rule.
     * @return the not parser for the given rule.
     */
    template <class ParseContextType> auto operator !(Rule<ParseContextType>& rule) {
        return !RuleReference<ParseContextType>(rule);
    }


    /**
     * Operator that allows a match to be inserted into the parser for a rule.
     * @param rule rule to apply the operator to.
     * @param matchId match id.
     * @return a match parser.
     */
    template <class ParseContextType, class MatchIdType>
    Match<RuleReference<ParseContextType>, MatchIdType>
        operator == (Rule<ParseContextType>& rule, const MatchIdType& matchId) {
        return Match<RuleReference<ParseContextType>, MatchIdType>(RuleReference<ParseContextType>(rule), matchId);
    }


} //namespace parserlib


#endif //PARSERLIB_RULE_HPP
