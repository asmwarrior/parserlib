#ifndef PARSERLIB_CHOICEPARSER_HPP
#define PARSERLIB_CHOICEPARSER_HPP


#include "ParserNode.hpp"
#include "LeftRecursionException.hpp"


namespace parserlib {


    /**
     * Choice of parsers.
     * At least one parser must parse successfully in order to parse the choice successfully.
     * @param Children children parser nodes.
     */
    template <class ...Children> class ChoiceParser : public ParserNode<ChoiceParser<Children...>> {
    public:
        /**
         * Constructor.
         * @param children children nodes.
         */
        ChoiceParser(const std::tuple<Children...>& children) 
            : m_children(children) {
        }

        /**
         * Returns the children nodes.
         * @return the children nodes.
         */
        const std::tuple<Children...>& children() const {
            return m_children;
        }

        /**
         * Invokes all child parsers, one by one, until one returns true.
         * @param pc parse context.
         * @return true if parsing succeeds, false otherwise.
         */
        template <class ParseContextType> bool operator ()(ParseContextType& pc) const {
            return parseTuple<0>(pc);
        }

    private:
        std::tuple<Children...> m_children;

        //tuple parse
        template <size_t Index, class ParseContextType> bool parseTuple(ParseContextType& pc) const {
            if constexpr (Index < sizeof...(Children)) {
                const auto state = pc.state();

                //try branch
                try {
                    if (std::get<Index>(m_children)(pc)) {
                        return true;
                    }
                }                

                //found left recursion
                catch (const LeftRecursionException<ParseContextType>& lre) {
                    //parse non-left recursive part; on failure, pass the exception up the stack
                    //for another choice to parse it
                    lre.rule().setRejectState();
                    if (!parseTuple<Index + 1>(pc)) {
                        throw lre;
                    }

                    //parse after the left recursion until failure or input end
                    lre.rule().setAcceptState();
                    while (true) {
                        lre.rule().setParsePosition(pc.sourcePosition());
                        if (!std::get<Index>(m_children)(pc)) {
                            break;
                        }
                        if (pc.sourcePosition() == pc.sourceEndPosition()) {
                            break;
                        }
                    }

                    return true;
                }

                //try next branch
                pc.setState(state);
                return parseTuple<Index + 1>(pc);
            }
            else {
                return false;
            }
        }
    };


    /**
     * Creates a choice of parsers out of two parsers.
     * @param node1 1st node.
     * @param node2 2nd node.
     * @return a choice of parsers.
     */
    template <class ParserNodeType1, class ParserNodeType2>
    ChoiceParser<ParserNodeType1, ParserNodeType2>
    operator | (const ParserNode<ParserNodeType1>& node1, const ParserNode<ParserNodeType2>& node2) {
        return ChoiceParser<ParserNodeType1, ParserNodeType2>(
            std::make_tuple(
                static_cast<const ParserNodeType1&>(node1), 
                static_cast<const ParserNodeType2&>(node2)));
    }


    /**
     * Creates a choice of parsers out of two choice parsers.
     * It flattens the structures into one choice.
     * @param node1 1st node.
     * @param node2 2nd node.
     * @return a choice of parsers.
     */
    template <class ...Children1, class ...Children2>
    ChoiceParser<Children1..., Children2...>
    operator | (const ParserNode<ChoiceParser<Children1...>>& node1, const ParserNode<ChoiceParser<Children2...>>& node2) {
        return ChoiceParser<Children1..., Children2...>(
            std::tuple_cat(
                static_cast<const ChoiceParser<Children1...>&>(node1).children(), 
                static_cast<const ChoiceParser<Children2...>&>(node2).children()));
    }


    /**
     * Creates a choice of parsers out of a choice parser and a non-choice parser.
     * @param node1 1st node.
     * @param node2 2nd node.
     * @return a choice of parsers.
     */
    template <class ...Children1, class ParserNodeType2>
    ChoiceParser<Children1..., ParserNodeType2>
    operator | (const ParserNode<ChoiceParser<Children1...>>& node1, const ParserNode<ParserNodeType2>& node2) {
        return ChoiceParser<Children1..., ParserNodeType2>(
            std::tuple_cat(
                static_cast<const ChoiceParser<Children1...>&>(node1).children(), 
                std::make_tuple(static_cast<const ParserNodeType2&>(node2))));
    }


    /**
     * Creates a choice of parsers out of a non-choice parser and a choice parser.
     * It flattens the structures into one choice.
     * @param node1 1st node.
     * @param node2 2nd node.
     * @return a choice of parsers.
     */
    template <class ParserNodeType1, class ...Children2>
    ChoiceParser<ParserNodeType1, Children2...>
        operator | (const ParserNode<ParserNodeType1>& node1, const ParserNode<ChoiceParser<Children2...>>& node2) {
        return ChoiceParser<ParserNodeType1, Children2...>(
            std::tuple_cat(
                std::make_tuple(static_cast<const ParserNodeType1&>(node1)), 
                static_cast<const ChoiceParser<Children2...>&>(node2).children()));
    }


} //namespace parserlib


#endif //PARSERLIB_CHOICEPARSER_HPP
