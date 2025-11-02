#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "cpp_parser_grammar.h"


// ----------------------------------------------------------------------
// AST Visitor Implementation
// ----------------------------------------------------------------------

/**
 * @brief Base AST Visitor class.
 *
 * This provides the interface for visiting each specific node type.
 * Note: Node is expected to be an AST node wrapper (e.g., std::shared_ptr<...>)
 * based on the usage in cpp_parser_grammar::print_ast.
 */
template <typename Node>
class AstVisitor {
public:
    using MatchId = typename cpp_parser_grammar::match_id_type;

    virtual ~AstVisitor() = default;

    // General visit function (dispatches to specific visit methods)
    void visit(const Node& node) {
        switch (node->id()) {
            case MatchId::TOP_LEVEL: visit_top_level(node); break;
            case MatchId::CLASS_DEF: visit_class_def(node); break;
            case MatchId::CLASS_ID:  visit_class_id(node); break;
            case MatchId::VAR_DECL:  visit_var_decl(node); break;
            case MatchId::FUNC_DECL: visit_func_decl(node); break;
            case MatchId::FUNC_DEF:  visit_func_def(node); break;
            case MatchId::BLOCK:     visit_block(node); break;
            case MatchId::BLOCK_SKIP: visit_block_skip(node); break;
            default:
                // Handle unexpected or generic nodes
                std::cout << "Warning: Unhandled node type: "
                          << cpp_parser_grammar::match_id_to_string(node->id()) << "\n";
                visit_generic(node);
                break;
        }
    }

protected:
    // Fallback for nodes that don't have a specific handler
    virtual void visit_generic(const Node& node) {
        // Default behavior: just traverse children
        for (const auto& child : node->children()) {
            visit(child);
        }
    }

    // Specific visit methods (Override these in derived classes)
    virtual void visit_top_level(const Node& node)  { visit_generic(node); }
    virtual void visit_class_def(const Node& node)  { visit_generic(node); }
    virtual void visit_class_id(const Node& node)   { visit_generic(node); }
    virtual void visit_var_decl(const Node& node)   { visit_generic(node); }
    virtual void visit_func_decl(const Node& node)  { visit_generic(node); }
    virtual void visit_func_def(const Node& node)   { visit_generic(node); }
    virtual void visit_block(const Node& node)      { visit_generic(node); }
    virtual void visit_block_skip(const Node& node) { visit_generic(node); }
};

/**
 * @brief Concrete Visitor to print the AST structure and a summary.
 */
template <typename Node>
class AstPrinterVisitor : public AstVisitor<Node> {
    using Base = AstVisitor<Node>;
    int indent_level = 0;

    void print_node(const Node& node, const std::string& type_name) {
        std::string space(indent_level * 2, ' ');
        std::cout << space << "-> " << type_name
                  << " [" << node->source() << "]\n";
    }

public:
    void traverse_children(const Node& node) {
        indent_level++;
        for (const auto& child : node->children()) {
            this->visit(child); // Use this-> for base class call
        }
        indent_level--;
    }

protected:
    void visit_top_level(const Node& node) override {
        print_node(node, "TOP_LEVEL");
        traverse_children(node);
    }

    void visit_class_def(const Node& node) override {
        // Children: [0] = 'class', [1] = IDENTIFIER (Class Name), [2] = BLOCK
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> CLASS_DEF: " << node->children()[0]->source() << "\n";
        traverse_children(node);
    }

    void visit_class_id(const Node& node) override {
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> CLASS_ID: " << node->source() << "\n";
        traverse_children(node);
    }

    void visit_var_decl(const Node& node) override {
        // Children: [0] = IDENTIFIER (Type), [1] = IDENTIFIER (Name), [2] = SEMICOLON
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> VAR_DECL: " << node->source() << "\n";
    }

    void visit_func_decl(const Node& node) override {
        // Children: [0] = IDENTIFIER (Return Type), [1] = IDENTIFIER (Name), ...
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> FUNC_DECL: " << node->source() << "\n";
    }

    void visit_func_def(const Node& node) override {
        // Children: [0] = IDENTIFIER (Return Type), [1] = IDENTIFIER (Name), ..., [4] = BLOCK_SKIP
        std::cout << std::string(indent_level * 2, ' ')
                  << "-> FUNC_DEF: " << node->source() << "'\n";
        traverse_children(node); // Traverses the BLOCK_SKIP child
    }

    void visit_block(const Node& node) override {
        print_node(node, "BLOCK");
        traverse_children(node);
    }

    void visit_block_skip(const Node& node) override {
        print_node(node, "BLOCK_SKIP (Skipped)");
        // No traverse_children here, as the contents were skipped by the parser.
    }

    void visit_generic(const Node& node) override {
        // This is primarily for skipping non-structural elements like terminal tokens
        // which often don't need dedicated handling in a high-level AST visitor.
        // For example, in the ClassDef, we skip the 'class' token and the braces.
        // We only traverse children for non-terminal nodes that are structural.

        // If a node is a structural node but has no specific handler,
        // we can still traverse its children.
        if (node->children().size() > 0) {
            traverse_children(node);
        }
    }
};

#endif // AST_VISITOR_H
