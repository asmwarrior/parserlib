#ifndef PARSERLIB__RULE__HPP
#define PARSERLIB__RULE__HPP


#include <memory>
#include "rule_reference.hpp"
#include "parse_context.hpp"
#include "expression_wrapper.hpp"


namespace parserlib
{


    /**
        A grammar rule.
        @param ParseContext type of parse context to use.
     */
    template <typename ParseContext = parse_context<>>
    class rule : public expression
    {
    public:
        /**
            Constructor from expression.
            @param expression expression.
         */
        template <typename T>
        rule(T&& expression)
            : m_expression(std::make_unique<expression_wrapper<ParseContext, expression_type_t<T>>>(std::forward<T>(expression)))
        {
        }

        /**
            Constructor from rule reference.
            @param r rule.
         */
        rule(rule&& r)
            : m_expression(std::make_unique<expression_wrapper<rule_reference<ParseContext>>>(r))
        {
        }

        /**
            Parses the input with the underlying expression.
            @param pc parse context.
            @return parse result.
         */
        parse_result parse(ParseContext& pc) const
        {
            parse_result result;

            const bool is_left_recursive = activate(pc);

            switch (pc.left_recursion.state)
            {
                case left_recursion_state::inactive:
                    if (!is_left_recursive)
                    {
                        const auto prev_left_recursion = pc.left_recursion;

                        result = m_expression->parse(pc);

                        if (pc.left_recursion.state == left_recursion_state::reject)
                        {
                            if (result == parse_result::accepted)
                            {
                                pc.left_recursion.state = left_recursion_state::accept;

                                for (bool loop = true; loop && pc.valid(); )
                                {
                                    pc.left_recursion.position = pc.position;

                                    const parse_result lr_result = m_expression->parse(pc);

                                    switch (lr_result)
                                    {
                                        case parse_result::accepted:
                                        case parse_result::accepted_left_recursion:
                                            break;

                                        case parse_result::rejected:
                                            loop = false;
                                            break;

                                        case parse_result::rejected_left_recursion:
                                            result = parse_result::rejected_left_recursion;
                                            loop = false;
                                            break;
                                    }
                                }
                            }

                            pc.left_recursion = prev_left_recursion;
                        }
                    }
                    else
                    {
                        pc.left_recursion = { left_recursion_state::reject, pc.position };
                        result = parse_result::rejected_left_recursion;
                    }
                    break;

                case left_recursion_state::reject:
                    if (!is_left_recursive)
                    {
                        if (pc.position > pc.left_recursion.position)
                        {
                            const auto prev_left_recursion = pc.left_recursion;
                            pc.left_recursion = { left_recursion_state::inactive, pc.position };
                            result = m_expression->parse(pc);
                            pc.left_recursion = prev_left_recursion;
                        }
                        else
                        {
                            result = m_expression->parse(pc);
                        }
                    }
                    else
                    {
                        result = parse_result::rejected_left_recursion;
                    }
                    break;

                case left_recursion_state::accept:
                    if (!is_left_recursive)
                    {
                        if (pc.position > pc.left_recursion.position)
                        {
                            const auto prev_left_recursion = pc.left_recursion;
                            pc.left_recursion = { left_recursion_state::inactive, pc.position };
                            result = m_expression->parse(pc);
                            pc.left_recursion = prev_left_recursion;
                        }
                        else
                        {
                            result = m_expression->parse(pc);
                            if (result == parse_result::accepted)
                            {
                                result = parse_result::accepted_left_recursion;
                            }
                        }
                    }
                    else
                    {
                        result = parse_result::accepted;
                    }
                    break;
            }

            deactivate(pc);

            return result;
        }

    private:
        //the expression is wrapped by a polymorphic type.
        std::unique_ptr<expression_interface<ParseContext>> m_expression;

        //activate; returns true on left recursion.
        bool activate(ParseContext& pc) const
        {
            auto& positions = pc.positions[this];
            positions.push_back(pc.position);
            return positions.size() >= 2 && positions[positions.size() - 1] == positions[positions.size() - 2];
        }

        //deactivate.
        void deactivate(ParseContext& pc) const
        {
            pc.positions.find(this)->second.pop_back();
        }
    };


    /**
        Specialization for rules.
        Allows the implicit conversion to rules to rule references.
        @param ParseContext type of parse context to use.
     */
    template <typename ParseContext>
    class expression_type<rule<ParseContext>>
    {
    public:
        /**
            The expression type of a rule is a rule reference,
            in order to allow recursive declarations of rules.
         */
        typedef rule_reference<ParseContext> type;
    };


} //namespace parserlib


#endif //PARSERLIB__RULE__HPP
